package net.minecraft.client.resources;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.Lists;
import com.google.common.util.concurrent.FutureCallback;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.SettableFuture;
import java.awt.image.BufferedImage;
import java.io.Closeable;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.ReentrantLock;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiScreenWorking;
import net.minecraft.client.renderer.texture.DynamicTexture;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.client.resources.data.IMetadataSerializer;
import net.minecraft.client.resources.data.PackMetadataSection;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.util.HttpUtil;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.TextFormatting;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.comparator.LastModifiedFileComparator;
import org.apache.commons.io.filefilter.IOFileFilter;
import org.apache.commons.io.filefilter.TrueFileFilter;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ResourcePackRepository
{
    private static final Logger logger = LogManager.getLogger();
    private static final FileFilter resourcePackFilter = new FileFilter()
    {
        public boolean accept(File p_accept_1_)
        {
            boolean flag = p_accept_1_.isFile() && p_accept_1_.getName().endsWith(".zip");
            boolean flag1 = p_accept_1_.isDirectory() && (new File(p_accept_1_, "pack.mcmeta")).isFile();
            return flag || flag1;
        }
    };
    private final File dirResourcepacks;
    public final IResourcePack rprDefaultResourcePack;
    private final File dirServerResourcepacks;
    public final IMetadataSerializer rprMetadataSerializer;
    private IResourcePack resourcePackInstance;
    private final ReentrantLock lock = new ReentrantLock();
    private ListenableFuture<Object> downloadingPacks;
    private List<ResourcePackRepository.Entry> repositoryEntriesAll = Lists.<ResourcePackRepository.Entry>newArrayList();
    private List<ResourcePackRepository.Entry> repositoryEntries = Lists.<ResourcePackRepository.Entry>newArrayList();

    public ResourcePackRepository(File dirResourcepacksIn, File dirServerResourcepacksIn, IResourcePack rprDefaultResourcePackIn, IMetadataSerializer rprMetadataSerializerIn, GameSettings settings)
    {
        this.dirResourcepacks = dirResourcepacksIn;
        this.dirServerResourcepacks = dirServerResourcepacksIn;
        this.rprDefaultResourcePack = rprDefaultResourcePackIn;
        this.rprMetadataSerializer = rprMetadataSerializerIn;
        this.fixDirResourcepacks();
        this.updateRepositoryEntriesAll();
        Iterator<String> iterator = settings.resourcePacks.iterator();

        while (iterator.hasNext())
        {
            String s = (String)iterator.next();

            for (ResourcePackRepository.Entry resourcepackrepository$entry : this.repositoryEntriesAll)
            {
                if (resourcepackrepository$entry.getResourcePackName().equals(s))
                {
                    if (resourcepackrepository$entry.func_183027_f() == 2 || settings.incompatibleResourcePacks.contains(resourcepackrepository$entry.getResourcePackName()))
                    {
                        this.repositoryEntries.add(resourcepackrepository$entry);
                        break;
                    }

                    iterator.remove();
                    logger.warn("Removed selected resource pack {} because it\'s no longer compatible", new Object[] {resourcepackrepository$entry.getResourcePackName()});
                }
            }
        }
    }

    private void fixDirResourcepacks()
    {
        if (this.dirResourcepacks.exists())
        {
            if (!this.dirResourcepacks.isDirectory() && (!this.dirResourcepacks.delete() || !this.dirResourcepacks.mkdirs()))
            {
                logger.warn("Unable to recreate resourcepack folder, it exists but is not a directory: " + this.dirResourcepacks);
            }
        }
        else if (!this.dirResourcepacks.mkdirs())
        {
            logger.warn("Unable to create resourcepack folder: " + this.dirResourcepacks);
        }
    }

    private List<File> getResourcePackFiles()
    {
        return this.dirResourcepacks.isDirectory() ? Arrays.asList(this.dirResourcepacks.listFiles(resourcePackFilter)) : Collections.<File>emptyList();
    }

    public void updateRepositoryEntriesAll()
    {
        List<ResourcePackRepository.Entry> list = Lists.<ResourcePackRepository.Entry>newArrayList();

        for (File file1 : this.getResourcePackFiles())
        {
            ResourcePackRepository.Entry resourcepackrepository$entry = new ResourcePackRepository.Entry(file1);

            if (!this.repositoryEntriesAll.contains(resourcepackrepository$entry))
            {
                try
                {
                    resourcepackrepository$entry.updateResourcePack();
                    list.add(resourcepackrepository$entry);
                }
                catch (Exception var6)
                {
                    list.remove(resourcepackrepository$entry);
                }
            }
            else
            {
                int i = this.repositoryEntriesAll.indexOf(resourcepackrepository$entry);

                if (i > -1 && i < this.repositoryEntriesAll.size())
                {
                    list.add(this.repositoryEntriesAll.get(i));
                }
            }
        }

        this.repositoryEntriesAll.removeAll(list);

        for (ResourcePackRepository.Entry resourcepackrepository$entry1 : this.repositoryEntriesAll)
        {
            resourcepackrepository$entry1.closeResourcePack();
        }

        this.repositoryEntriesAll = list;
    }

    public ResourcePackRepository.Entry func_188565_b()
    {
        if (this.resourcePackInstance != null)
        {
            ResourcePackRepository.Entry resourcepackrepository$entry = new ResourcePackRepository.Entry(this.resourcePackInstance);

            try
            {
                resourcepackrepository$entry.updateResourcePack();
                return resourcepackrepository$entry;
            }
            catch (IOException var3)
            {
                ;
            }
        }

        return null;
    }

    public List<ResourcePackRepository.Entry> getRepositoryEntriesAll()
    {
        return ImmutableList.copyOf(this.repositoryEntriesAll);
    }

    public List<ResourcePackRepository.Entry> getRepositoryEntries()
    {
        return ImmutableList.copyOf(this.repositoryEntries);
    }

    public void setRepositories(List<ResourcePackRepository.Entry> repositories)
    {
        this.repositoryEntries.clear();
        this.repositoryEntries.addAll(repositories);
    }

    public File getDirResourcepacks()
    {
        return this.dirResourcepacks;
    }

    public ListenableFuture<Object> downloadResourcePack(String url, String hash)
    {
        String s = DigestUtils.sha1Hex(url);
        String s1 = hash.matches("^[a-f0-9]{40}$") ? hash : "";
        final File file1 = new File(this.dirServerResourcepacks, s);
        this.lock.lock();

        try
        {
            this.clearResourcePack();

            if (file1.exists())
            {
                try
                {
                    String s2 = DigestUtils.sha1Hex((InputStream)(new FileInputStream(file1)));

                    if (!s1.equals("") && s2.equals(s1))
                    {
                        logger.info("Found file " + file1 + " matching requested hash " + s1);
                        ListenableFuture listenablefuture2 = this.setResourcePackInstance(file1);
                        return listenablefuture2;
                    }

                    if (!s1.equals("") && !s2.equals(s1))
                    {
                        logger.warn("File " + file1 + " had wrong hash (expected " + s1 + ", found " + s2 + "). Deleting it.");
                        FileUtils.deleteQuietly(file1);
                    }
                    else if (s1.equals(""))
                    {
                        logger.info("Found file " + file1 + " without verification hash");
                        ListenableFuture lvt_7_1_ = this.setResourcePackInstance(file1);
                        return lvt_7_1_;
                    }
                }
                catch (IOException ioexception)
                {
                    logger.warn((String)("File " + file1 + " couldn\'t be hashed. Deleting it."), (Throwable)ioexception);
                    FileUtils.deleteQuietly(file1);
                }
            }

            this.deleteOldServerResourcesPacks();
            final GuiScreenWorking guiscreenworking = new GuiScreenWorking();
            Map<String, String> map = Minecraft.getSessionInfo();
            final Minecraft minecraft = Minecraft.getMinecraft();
            Futures.getUnchecked(minecraft.addScheduledTask(new Runnable()
            {
                public void run()
                {
                    minecraft.displayGuiScreen(guiscreenworking);
                }
            }));
            final SettableFuture<Object> settablefuture = SettableFuture.<Object>create();
            this.downloadingPacks = HttpUtil.downloadResourcePack(file1, url, map, 52428800, guiscreenworking, minecraft.getProxy());
            Futures.addCallback(this.downloadingPacks, new FutureCallback<Object>()
            {
                public void onSuccess(Object p_onSuccess_1_)
                {
                    ResourcePackRepository.this.setResourcePackInstance(file1);
                    settablefuture.set((Object)null);
                }
                public void onFailure(Throwable p_onFailure_1_)
                {
                    settablefuture.setException(p_onFailure_1_);
                }
            });
            ListenableFuture listenablefuture1 = this.downloadingPacks;
            return listenablefuture1;
        }
        finally
        {
            this.lock.unlock();
        }
    }

    /**
     * Keep only the 10 most recent resources packs, delete the others
     */
    private void deleteOldServerResourcesPacks()
    {
        try
        {
            List<File> list = Lists.newArrayList(FileUtils.listFiles(this.dirServerResourcepacks, TrueFileFilter.TRUE, (IOFileFilter)null));
            Collections.sort(list, LastModifiedFileComparator.LASTMODIFIED_REVERSE);
            int i = 0;

            for (File file1 : list)
            {
                if (i++ >= 10)
                {
                    logger.info("Deleting old server resource pack " + file1.getName());
                    FileUtils.deleteQuietly(file1);
                }
            }
        }
        catch (IllegalArgumentException illegalargumentexception)
        {
            logger.error("Error while deleting old server resource pack : " + illegalargumentexception.getMessage());
        }
    }

    public ListenableFuture<Object> setResourcePackInstance(File resourceFile)
    {
        this.resourcePackInstance = new FileResourcePack(resourceFile);
        return Minecraft.getMinecraft().scheduleResourcesRefresh();
    }

    /**
     * Getter for the IResourcePack instance associated with this ResourcePackRepository
     */
    public IResourcePack getResourcePackInstance()
    {
        return this.resourcePackInstance;
    }

    public void clearResourcePack()
    {
        this.lock.lock();

        try
        {
            if (this.downloadingPacks != null)
            {
                this.downloadingPacks.cancel(true);
            }

            this.downloadingPacks = null;

            if (this.resourcePackInstance != null)
            {
                this.resourcePackInstance = null;
                Minecraft.getMinecraft().scheduleResourcesRefresh();
            }
        }
        finally
        {
            this.lock.unlock();
        }
    }

    public class Entry
    {
        private IResourcePack reResourcePack;
        private PackMetadataSection rePackMetadataSection;
        private ResourceLocation locationTexturePackIcon;

        private Entry(File resourcePackFileIn)
        {
            this((IResourcePack)(resourcePackFileIn.isDirectory() ? new FolderResourcePack(resourcePackFileIn) : new FileResourcePack(resourcePackFileIn)));
        }

        private Entry(IResourcePack p_i46558_2_)
        {
            this.reResourcePack = p_i46558_2_;
        }

        public void updateResourcePack() throws IOException
        {
            this.rePackMetadataSection = (PackMetadataSection)this.reResourcePack.getPackMetadata(ResourcePackRepository.this.rprMetadataSerializer, "pack");
            this.closeResourcePack();
        }

        public void bindTexturePackIcon(TextureManager textureManagerIn)
        {
            BufferedImage bufferedimage = null;

            try
            {
                bufferedimage = this.reResourcePack.getPackImage();
            }
            catch (IOException var5)
            {
                ;
            }

            if (bufferedimage == null)
            {
                try
                {
                    bufferedimage = ResourcePackRepository.this.rprDefaultResourcePack.getPackImage();
                }
                catch (IOException ioexception)
                {
                    throw new Error("Couldn\'t bind resource pack icon", ioexception);
                }
            }

            if (this.locationTexturePackIcon == null)
            {
                this.locationTexturePackIcon = textureManagerIn.getDynamicTextureLocation("texturepackicon", new DynamicTexture(bufferedimage));
            }

            textureManagerIn.bindTexture(this.locationTexturePackIcon);
        }

        public void closeResourcePack()
        {
            if (this.reResourcePack instanceof Closeable)
            {
                IOUtils.closeQuietly((Closeable)this.reResourcePack);
            }
        }

        public IResourcePack getResourcePack()
        {
            return this.reResourcePack;
        }

        public String getResourcePackName()
        {
            return this.reResourcePack.getPackName();
        }

        public String getTexturePackDescription()
        {
            return this.rePackMetadataSection == null ? TextFormatting.RED + "Invalid pack.mcmeta (or missing \'pack\' section)" : this.rePackMetadataSection.getPackDescription().getFormattedText();
        }

        public int func_183027_f()
        {
            return this.rePackMetadataSection == null ? 0 : this.rePackMetadataSection.getPackFormat();
        }

        public boolean equals(Object p_equals_1_)
        {
            return this == p_equals_1_ ? true : (p_equals_1_ instanceof ResourcePackRepository.Entry ? this.toString().equals(p_equals_1_.toString()) : false);
        }

        public int hashCode()
        {
            return this.toString().hashCode();
        }

        public String toString()
        {
            return String.format("%s:%s", new Object[] {this.reResourcePack.getPackName(), this.reResourcePack instanceof FolderResourcePack ? "folder" : "zip"});
        }
    }
}
