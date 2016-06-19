package net.minecraft.client.audio;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.Map;
import java.util.Map.Entry;
import net.minecraft.client.resources.I18n;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.ITickable;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentTranslation;
import org.apache.commons.io.IOUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class SoundHandler implements IResourceManagerReloadListener, ITickable
{
    public static final Sound missing_sound = new Sound("meta:missing_sound", 1.0F, 1.0F, 1, Sound.Type.FILE, false);
    private static final Logger logger = LogManager.getLogger();
    private static final Gson GSON = (new GsonBuilder()).registerTypeHierarchyAdapter(ITextComponent.class, new ITextComponent.Serializer()).registerTypeAdapter(SoundList.class, new SoundListSerializer()).create();
    private static final ParameterizedType TYPE = new ParameterizedType()
    {
        public Type[] getActualTypeArguments()
        {
            return new Type[] {String.class, SoundList.class};
        }
        public Type getRawType()
        {
            return Map.class;
        }
        public Type getOwnerType()
        {
            return null;
        }
    };
    private final SoundRegistry sndRegistry = new SoundRegistry();
    private final SoundManager sndManager;
    private final IResourceManager mcResourceManager;

    public SoundHandler(IResourceManager manager, GameSettings gameSettingsIn)
    {
        this.mcResourceManager = manager;
        this.sndManager = new SoundManager(this, gameSettingsIn);
    }

    public void onResourceManagerReload(IResourceManager resourceManager)
    {
        this.sndRegistry.clearMap();

        for (String s : resourceManager.getResourceDomains())
        {
            try
            {
                for (IResource iresource : resourceManager.getAllResources(new ResourceLocation(s, "sounds.json")))
                {
                    try
                    {
                        Map<String, SoundList> map = this.getSoundMap(iresource.getInputStream());

                        for (Entry<String, SoundList> entry : map.entrySet())
                        {
                            this.loadSoundResource(new ResourceLocation(s, (String)entry.getKey()), (SoundList)entry.getValue());
                        }
                    }
                    catch (RuntimeException runtimeexception)
                    {
                        logger.warn((String)"Invalid sounds.json", (Throwable)runtimeexception);
                    }
                }
            }
            catch (IOException var11)
            {
                ;
            }
        }

        for (ResourceLocation resourcelocation : this.sndRegistry.getKeys())
        {
            SoundEventAccessor soundeventaccessor = (SoundEventAccessor)this.sndRegistry.getObject(resourcelocation);

            if (soundeventaccessor.func_188712_c() instanceof TextComponentTranslation)
            {
                String s1 = ((TextComponentTranslation)soundeventaccessor.func_188712_c()).getKey();

                if (!I18n.func_188566_a(s1))
                {
                    logger.debug("Missing subtitle {} for event: {}", new Object[] {s1, resourcelocation});
                }
            }
        }

        for (ResourceLocation resourcelocation1 : this.sndRegistry.getKeys())
        {
            if (SoundEvent.soundEventRegistry.getObject(resourcelocation1) == null)
            {
                logger.debug("Not having sound event for: {}", new Object[] {resourcelocation1});
            }
        }

        this.sndManager.reloadSoundSystem();
    }

    protected Map<String, SoundList> getSoundMap(InputStream stream)
    {
        Map map;

        try
        {
            map = (Map)GSON.fromJson((Reader)(new InputStreamReader(stream)), TYPE);
        }
        finally
        {
            IOUtils.closeQuietly(stream);
        }

        return map;
    }

    private void loadSoundResource(ResourceLocation location, SoundList sounds)
    {
        SoundEventAccessor soundeventaccessor = (SoundEventAccessor)this.sndRegistry.getObject(location);
        boolean flag = soundeventaccessor == null;

        if (flag || sounds.canReplaceExisting())
        {
            if (!flag)
            {
                logger.debug("Replaced sound event location {}", new Object[] {location});
            }

            soundeventaccessor = new SoundEventAccessor(location, sounds.func_188701_c());
            this.sndRegistry.func_186803_a(soundeventaccessor);
        }

        for (Sound sound : sounds.func_188700_a())
        {
            final ResourceLocation resourcelocation = sound.func_188719_a();
            ISoundEventAccessor<Sound> isoundeventaccessor;

            switch (sound.func_188722_g())
            {
                case FILE:
                    if (!this.func_184401_a(sound, location))
                    {
                        continue;
                    }

                    isoundeventaccessor = sound;
                    break;

                case SOUND_EVENT:
                    isoundeventaccessor = new ISoundEventAccessor<Sound>()
                    {
                        public int getWeight()
                        {
                            SoundEventAccessor soundeventaccessor1 = (SoundEventAccessor)SoundHandler.this.sndRegistry.getObject(resourcelocation);
                            return soundeventaccessor1 == null ? 0 : soundeventaccessor1.getWeight();
                        }
                        public Sound cloneEntry()
                        {
                            SoundEventAccessor soundeventaccessor1 = (SoundEventAccessor)SoundHandler.this.sndRegistry.getObject(resourcelocation);
                            return soundeventaccessor1 == null ? SoundHandler.missing_sound : soundeventaccessor1.cloneEntry();
                        }
                    };

                    break;
                default:
                    throw new IllegalStateException("Unknown SoundEventRegistration type: " + sound.func_188722_g());
            }

            soundeventaccessor.func_188715_a(isoundeventaccessor);
        }
    }

    private boolean func_184401_a(Sound p_184401_1_, ResourceLocation p_184401_2_)
    {
        ResourceLocation resourcelocation = p_184401_1_.func_188721_b();
        IResource iresource = null;
        boolean flag;

        try
        {
            iresource = this.mcResourceManager.getResource(resourcelocation);
            iresource.getInputStream();
            return true;
        }
        catch (FileNotFoundException var11)
        {
            logger.warn("File {} does not exist, cannot add it to event {}", new Object[] {resourcelocation, p_184401_2_});
            flag = false;
        }
        catch (IOException ioexception)
        {
            logger.warn((String)("Could not load sound file " + resourcelocation + ", cannot add it to event " + p_184401_2_), (Throwable)ioexception);
            flag = false;
            return flag;
        }
        finally
        {
            IOUtils.closeQuietly((Closeable)iresource);
        }

        return flag;
    }

    public SoundEventAccessor func_184398_a(ResourceLocation p_184398_1_)
    {
        return (SoundEventAccessor)this.sndRegistry.getObject(p_184398_1_);
    }

    /**
     * Play a sound
     */
    public void playSound(ISound sound)
    {
        this.sndManager.playSound(sound);
    }

    /**
     * Plays the sound in n ticks
     */
    public void playDelayedSound(ISound sound, int delay)
    {
        this.sndManager.playDelayedSound(sound, delay);
    }

    public void setListener(EntityPlayer player, float p_147691_2_)
    {
        this.sndManager.setListener(player, p_147691_2_);
    }

    public void pauseSounds()
    {
        this.sndManager.pauseAllSounds();
    }

    public void stopSounds()
    {
        this.sndManager.stopAllSounds();
    }

    public void unloadSounds()
    {
        this.sndManager.unloadSoundSystem();
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        this.sndManager.updateAllSounds();
    }

    public void resumeSounds()
    {
        this.sndManager.resumeAllSounds();
    }

    public void func_184399_a(SoundCategory p_184399_1_, float p_184399_2_)
    {
        if (p_184399_1_ == SoundCategory.MASTER && p_184399_2_ <= 0.0F)
        {
            this.stopSounds();
        }

        this.sndManager.func_188771_a(p_184399_1_, p_184399_2_);
    }

    public void stopSound(ISound p_147683_1_)
    {
        this.sndManager.stopSound(p_147683_1_);
    }

    public boolean isSoundPlaying(ISound sound)
    {
        return this.sndManager.isSoundPlaying(sound);
    }

    public void func_184402_a(ISoundEventListener p_184402_1_)
    {
        this.sndManager.func_188774_a(p_184402_1_);
    }

    public void func_184400_b(ISoundEventListener p_184400_1_)
    {
        this.sndManager.func_188773_b(p_184400_1_);
    }
}
