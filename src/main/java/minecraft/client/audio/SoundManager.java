package net.minecraft.client.audio;

import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Multimap;
import com.google.common.collect.Sets;
import io.netty.util.internal.ThreadLocalRandom;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandler;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import net.minecraft.client.Minecraft;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.MathHelper;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.Marker;
import org.apache.logging.log4j.MarkerManager;
import paulscode.sound.SoundSystem;
import paulscode.sound.SoundSystemConfig;
import paulscode.sound.SoundSystemException;
import paulscode.sound.SoundSystemLogger;
import paulscode.sound.Source;
import paulscode.sound.codecs.CodecJOrbis;
import paulscode.sound.libraries.LibraryLWJGLOpenAL;

public class SoundManager
{
    /** The marker used for logging */
    private static final Marker LOG_MARKER = MarkerManager.getMarker("SOUNDS");
    private static final Logger logger = LogManager.getLogger();
    private static final Set<ResourceLocation> field_188775_c = Sets.<ResourceLocation>newHashSet();

    /** A reference to the sound handler. */
    private final SoundHandler sndHandler;

    /** Reference to the GameSettings object. */
    private final GameSettings options;

    /** A reference to the sound system. */
    private SoundManager.SoundSystemStarterThread sndSystem;

    /** Set to true when the SoundManager has been initialised. */
    private boolean loaded;

    /** A counter for how long the sound manager has been running */
    private int playTime = 0;
    private final Map<String, ISound> playingSounds = HashBiMap.<String, ISound>create();
    private final Map<ISound, String> invPlayingSounds;
    private final Multimap<SoundCategory, String> field_188776_k;
    private final List<ITickableSound> tickableSounds;
    private final Map<ISound, Integer> delayedSounds;
    private final Map<String, Integer> playingSoundsStopTime;
    private final List<ISoundEventListener> field_188777_o;
    private final List<String> field_189000_p;

    public SoundManager(SoundHandler p_i45119_1_, GameSettings p_i45119_2_)
    {
        this.invPlayingSounds = ((BiMap)this.playingSounds).inverse();
        this.field_188776_k = HashMultimap.<SoundCategory, String>create();
        this.tickableSounds = Lists.<ITickableSound>newArrayList();
        this.delayedSounds = Maps.<ISound, Integer>newHashMap();
        this.playingSoundsStopTime = Maps.<String, Integer>newHashMap();
        this.field_188777_o = Lists.<ISoundEventListener>newArrayList();
        this.field_189000_p = Lists.<String>newArrayList();
        this.sndHandler = p_i45119_1_;
        this.options = p_i45119_2_;

        try
        {
            SoundSystemConfig.addLibrary(LibraryLWJGLOpenAL.class);
            SoundSystemConfig.setCodec("ogg", CodecJOrbis.class);
        }
        catch (SoundSystemException soundsystemexception)
        {
            logger.error(LOG_MARKER, (String)"Error linking with the LibraryJavaSound plug-in", (Throwable)soundsystemexception);
        }
    }

    public void reloadSoundSystem()
    {
        field_188775_c.clear();

        for (SoundEvent soundevent : SoundEvent.soundEventRegistry)
        {
            ResourceLocation resourcelocation = soundevent.func_187503_a();

            if (this.sndHandler.func_184398_a(resourcelocation) == null)
            {
                logger.warn("Missing sound for event: " + SoundEvent.soundEventRegistry.getNameForObject(soundevent));
                field_188775_c.add(resourcelocation);
            }
        }

        this.unloadSoundSystem();
        this.loadSoundSystem();
    }

    /**
     * Tries to add the paulscode library and the relevant codecs. If it fails, the master volume  will be set to zero.
     */
    private synchronized void loadSoundSystem()
    {
        if (!this.loaded)
        {
            try
            {
                (new Thread(new Runnable()
                {
                    public void run()
                    {
                        SoundSystemConfig.setLogger(new SoundSystemLogger()
                        {
                            public void message(String p_message_1_, int p_message_2_)
                            {
                                if (!p_message_1_.isEmpty())
                                {
                                    SoundManager.logger.info(p_message_1_);
                                }
                            }
                            public void importantMessage(String p_importantMessage_1_, int p_importantMessage_2_)
                            {
                                if (!p_importantMessage_1_.isEmpty())
                                {
                                    SoundManager.logger.warn(p_importantMessage_1_);
                                }
                            }
                            public void errorMessage(String p_errorMessage_1_, String p_errorMessage_2_, int p_errorMessage_3_)
                            {
                                if (!p_errorMessage_2_.isEmpty())
                                {
                                    SoundManager.logger.error("Error in class \'" + p_errorMessage_1_ + "\'");
                                    SoundManager.logger.error(p_errorMessage_2_);
                                }
                            }
                        });
                        SoundManager.this.sndSystem = SoundManager.this.new SoundSystemStarterThread();
                        SoundManager.this.loaded = true;
                        SoundManager.this.sndSystem.setMasterVolume(SoundManager.this.options.func_186711_a(SoundCategory.MASTER));
                        SoundManager.logger.info(SoundManager.LOG_MARKER, "Sound engine started");
                    }
                }, "Sound Library Loader")).start();
            }
            catch (RuntimeException runtimeexception)
            {
                logger.error(LOG_MARKER, (String)"Error starting SoundSystem. Turning off sounds & music", (Throwable)runtimeexception);
                this.options.func_186712_a(SoundCategory.MASTER, 0.0F);
                this.options.saveOptions();
            }
        }
    }

    private float func_188769_a(SoundCategory p_188769_1_)
    {
        return p_188769_1_ != null && p_188769_1_ != SoundCategory.MASTER ? this.options.func_186711_a(p_188769_1_) : 1.0F;
    }

    public void func_188771_a(SoundCategory p_188771_1_, float p_188771_2_)
    {
        if (this.loaded)
        {
            if (p_188771_1_ == SoundCategory.MASTER)
            {
                this.sndSystem.setMasterVolume(p_188771_2_);
            }
            else
            {
                for (String s : this.field_188776_k.get(p_188771_1_))
                {
                    ISound isound = (ISound)this.playingSounds.get(s);
                    float f = this.func_188770_e(isound);

                    if (f <= 0.0F)
                    {
                        this.stopSound(isound);
                    }
                    else
                    {
                        this.sndSystem.setVolume(s, f);
                    }
                }
            }
        }
    }

    /**
     * Cleans up the Sound System
     */
    public void unloadSoundSystem()
    {
        if (this.loaded)
        {
            this.stopAllSounds();
            this.sndSystem.cleanup();
            this.loaded = false;
        }
    }

    /**
     * Stops all currently playing sounds
     */
    public void stopAllSounds()
    {
        if (this.loaded)
        {
            for (String s : this.playingSounds.keySet())
            {
                this.sndSystem.stop(s);
            }

            this.playingSounds.clear();
            this.delayedSounds.clear();
            this.tickableSounds.clear();
            this.field_188776_k.clear();
            this.playingSoundsStopTime.clear();
        }
    }

    public void func_188774_a(ISoundEventListener p_188774_1_)
    {
        this.field_188777_o.add(p_188774_1_);
    }

    public void func_188773_b(ISoundEventListener p_188773_1_)
    {
        this.field_188777_o.remove(p_188773_1_);
    }

    public void updateAllSounds()
    {
        ++this.playTime;

        for (ITickableSound itickablesound : this.tickableSounds)
        {
            itickablesound.update();

            if (itickablesound.isDonePlaying())
            {
                this.stopSound(itickablesound);
            }
            else
            {
                String s = (String)this.invPlayingSounds.get(itickablesound);
                this.sndSystem.setVolume(s, this.func_188770_e(itickablesound));
                this.sndSystem.setPitch(s, this.func_188772_d(itickablesound));
                this.sndSystem.setPosition(s, itickablesound.getXPosF(), itickablesound.getYPosF(), itickablesound.getZPosF());
            }
        }

        Iterator<Entry<String, ISound>> iterator = this.playingSounds.entrySet().iterator();

        while (iterator.hasNext())
        {
            Entry<String, ISound> entry = (Entry)iterator.next();
            String s1 = (String)entry.getKey();
            ISound isound = (ISound)entry.getValue();

            if (!this.sndSystem.playing(s1))
            {
                int i = ((Integer)this.playingSoundsStopTime.get(s1)).intValue();

                if (i <= this.playTime)
                {
                    int j = isound.getRepeatDelay();

                    if (isound.canRepeat() && j > 0)
                    {
                        this.delayedSounds.put(isound, Integer.valueOf(this.playTime + j));
                    }

                    iterator.remove();
                    logger.debug(LOG_MARKER, "Removed channel {} because it\'s not playing anymore", new Object[] {s1});
                    this.sndSystem.removeSource(s1);
                    this.playingSoundsStopTime.remove(s1);

                    try
                    {
                        this.field_188776_k.remove(isound.func_184365_d(), s1);
                    }
                    catch (RuntimeException var8)
                    {
                        ;
                    }

                    if (isound instanceof ITickableSound)
                    {
                        this.tickableSounds.remove(isound);
                    }
                }
            }
        }

        Iterator<Entry<ISound, Integer>> iterator1 = this.delayedSounds.entrySet().iterator();

        while (iterator1.hasNext())
        {
            Entry<ISound, Integer> entry1 = (Entry)iterator1.next();

            if (this.playTime >= ((Integer)entry1.getValue()).intValue())
            {
                ISound isound1 = (ISound)entry1.getKey();

                if (isound1 instanceof ITickableSound)
                {
                    ((ITickableSound)isound1).update();
                }

                this.playSound(isound1);
                iterator1.remove();
            }
        }
    }

    /**
     * Returns true if the sound is playing or still within time
     */
    public boolean isSoundPlaying(ISound sound)
    {
        if (!this.loaded)
        {
            return false;
        }
        else
        {
            String s = (String)this.invPlayingSounds.get(sound);
            return s == null ? false : this.sndSystem.playing(s) || this.playingSoundsStopTime.containsKey(s) && ((Integer)this.playingSoundsStopTime.get(s)).intValue() <= this.playTime;
        }
    }

    public void stopSound(ISound sound)
    {
        if (this.loaded)
        {
            String s = (String)this.invPlayingSounds.get(sound);

            if (s != null)
            {
                this.sndSystem.stop(s);
            }
        }
    }

    public void playSound(ISound p_sound)
    {
        if (this.loaded)
        {
            SoundEventAccessor soundeventaccessor = p_sound.func_184366_a(this.sndHandler);
            ResourceLocation resourcelocation = p_sound.getSoundLocation();

            if (soundeventaccessor == null)
            {
                if (field_188775_c.add(resourcelocation))
                {
                    logger.warn(LOG_MARKER, "Unable to play unknown soundEvent: {}", new Object[] {resourcelocation});
                }
            }
            else
            {
                if (!this.field_188777_o.isEmpty())
                {
                    for (ISoundEventListener isoundeventlistener : this.field_188777_o)
                    {
                        isoundeventlistener.func_184067_a(p_sound, soundeventaccessor);
                    }
                }

                if (this.sndSystem.getMasterVolume() <= 0.0F)
                {
                    logger.debug(LOG_MARKER, "Skipped playing soundEvent: {}, master volume was zero", new Object[] {resourcelocation});
                }
                else
                {
                    Sound sound = p_sound.func_184364_b();

                    if (sound == SoundHandler.missing_sound)
                    {
                        if (field_188775_c.add(resourcelocation))
                        {
                            logger.warn(LOG_MARKER, "Unable to play empty soundEvent: {}", new Object[] {resourcelocation});
                        }
                    }
                    else
                    {
                        float f3 = p_sound.getVolume();
                        float f = 16.0F;

                        if (f3 > 1.0F)
                        {
                            f *= f3;
                        }

                        SoundCategory soundcategory = p_sound.func_184365_d();
                        float f1 = this.func_188770_e(p_sound);
                        float f2 = this.func_188772_d(p_sound);

                        if (f1 == 0.0F)
                        {
                            logger.debug(LOG_MARKER, "Skipped playing sound {}, volume was zero.", new Object[] {sound.func_188719_a()});
                        }
                        else
                        {
                            boolean flag = p_sound.canRepeat() && p_sound.getRepeatDelay() == 0;
                            String s = MathHelper.getRandomUuid(ThreadLocalRandom.current()).toString();
                            ResourceLocation resourcelocation1 = sound.func_188721_b();

                            if (sound.func_188723_h())
                            {
                                this.sndSystem.newStreamingSource(false, s, getURLForSoundResource(resourcelocation1), resourcelocation1.toString(), flag, p_sound.getXPosF(), p_sound.getYPosF(), p_sound.getZPosF(), p_sound.getAttenuationType().getTypeInt(), f);
                            }
                            else
                            {
                                this.sndSystem.newSource(false, s, getURLForSoundResource(resourcelocation1), resourcelocation1.toString(), flag, p_sound.getXPosF(), p_sound.getYPosF(), p_sound.getZPosF(), p_sound.getAttenuationType().getTypeInt(), f);
                            }

                            logger.debug(LOG_MARKER, "Playing sound {} for event {} as channel {}", new Object[] {sound.func_188719_a(), resourcelocation1, s});
                            this.sndSystem.setPitch(s, f2);
                            this.sndSystem.setVolume(s, f1);
                            this.sndSystem.play(s);
                            this.playingSoundsStopTime.put(s, Integer.valueOf(this.playTime + 20));
                            this.playingSounds.put(s, p_sound);

                            if (soundcategory != SoundCategory.MASTER)
                            {
                                this.field_188776_k.put(soundcategory, s);
                            }

                            if (p_sound instanceof ITickableSound)
                            {
                                this.tickableSounds.add((ITickableSound)p_sound);
                            }
                        }
                    }
                }
            }
        }
    }

    private float func_188772_d(ISound p_188772_1_)
    {
        return MathHelper.clamp_float(p_188772_1_.getPitch(), 0.5F, 2.0F);
    }

    private float func_188770_e(ISound p_188770_1_)
    {
        return MathHelper.clamp_float(p_188770_1_.getVolume() * this.func_188769_a(p_188770_1_.func_184365_d()), 0.0F, 1.0F);
    }

    /**
     * Pauses all currently playing sounds
     */
    public void pauseAllSounds()
    {
        for (Entry<String, ISound> entry : this.playingSounds.entrySet())
        {
            String s = (String)entry.getKey();
            boolean flag = this.isSoundPlaying((ISound)entry.getValue());

            if (flag)
            {
                logger.debug(LOG_MARKER, "Pausing channel {}", new Object[] {s});
                this.sndSystem.pause(s);
                this.field_189000_p.add(s);
            }
        }
    }

    /**
     * Resumes playing all currently playing sounds (after pauseAllSounds)
     */
    public void resumeAllSounds()
    {
        for (String s : this.field_189000_p)
        {
            logger.debug(LOG_MARKER, "Resuming channel {}", new Object[] {s});
            this.sndSystem.play(s);
        }

        this.field_189000_p.clear();
    }

    /**
     * Adds a sound to play in n tick
     */
    public void playDelayedSound(ISound sound, int delay)
    {
        this.delayedSounds.put(sound, Integer.valueOf(this.playTime + delay));
    }

    private static URL getURLForSoundResource(final ResourceLocation p_148612_0_)
    {
        String s = String.format("%s:%s:%s", new Object[] {"mcsounddomain", p_148612_0_.getResourceDomain(), p_148612_0_.getResourcePath()});
        URLStreamHandler urlstreamhandler = new URLStreamHandler()
        {
            protected URLConnection openConnection(final URL p_openConnection_1_)
            {
                return new URLConnection(p_openConnection_1_)
                {
                    public void connect() throws IOException
                    {
                    }
                    public InputStream getInputStream() throws IOException
                    {
                        return Minecraft.getMinecraft().getResourceManager().getResource(p_148612_0_).getInputStream();
                    }
                };
            }
        };

        try
        {
            return new URL((URL)null, s, urlstreamhandler);
        }
        catch (MalformedURLException var4)
        {
            throw new Error("TODO: Sanely handle url exception! :D");
        }
    }

    /**
     * Sets the listener of sounds
     */
    public void setListener(EntityPlayer player, float p_148615_2_)
    {
        if (this.loaded && player != null)
        {
            float f = player.prevRotationPitch + (player.rotationPitch - player.prevRotationPitch) * p_148615_2_;
            float f1 = player.prevRotationYaw + (player.rotationYaw - player.prevRotationYaw) * p_148615_2_;
            double d0 = player.prevPosX + (player.posX - player.prevPosX) * (double)p_148615_2_;
            double d1 = player.prevPosY + (player.posY - player.prevPosY) * (double)p_148615_2_ + (double)player.getEyeHeight();
            double d2 = player.prevPosZ + (player.posZ - player.prevPosZ) * (double)p_148615_2_;
            float f2 = MathHelper.cos((f1 + 90.0F) * 0.017453292F);
            float f3 = MathHelper.sin((f1 + 90.0F) * 0.017453292F);
            float f4 = MathHelper.cos(-f * 0.017453292F);
            float f5 = MathHelper.sin(-f * 0.017453292F);
            float f6 = MathHelper.cos((-f + 90.0F) * 0.017453292F);
            float f7 = MathHelper.sin((-f + 90.0F) * 0.017453292F);
            float f8 = f2 * f4;
            float f9 = f3 * f4;
            float f10 = f2 * f6;
            float f11 = f3 * f6;
            this.sndSystem.setListenerPosition((float)d0, (float)d1, (float)d2);
            this.sndSystem.setListenerOrientation(f8, f5, f9, f10, f7, f11);
        }
    }

    class SoundSystemStarterThread extends SoundSystem
    {
        private SoundSystemStarterThread()
        {
        }

        public boolean playing(String p_playing_1_)
        {
            synchronized (SoundSystemConfig.THREAD_SYNC)
            {
                if (this.soundLibrary == null)
                {
                    return false;
                }
                else
                {
                    Source source = (Source)this.soundLibrary.getSources().get(p_playing_1_);
                    return source == null ? false : source.playing() || source.paused() || source.preLoad;
                }
            }
        }
    }
}
