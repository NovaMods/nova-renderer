package net.minecraft.client.renderer.texture;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import java.io.Closeable;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.Callable;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.StitcherException;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.util.ReportedException;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import org.apache.commons.io.IOUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class TextureMap extends AbstractTexture implements ITickableTextureObject
{
    private static final Logger logger = LogManager.getLogger();
    public static final ResourceLocation LOCATION_MISSING_TEXTURE = new ResourceLocation("missingno");
    public static final ResourceLocation locationBlocksTexture = new ResourceLocation("textures/atlas/blocks.png");
    private final List<TextureAtlasSprite> listAnimatedSprites;
    private final Map<String, TextureAtlasSprite> mapRegisteredSprites;
    private final Map<String, TextureAtlasSprite> mapUploadedSprites;
    private final String basePath;
    private final IIconCreator iconCreator;
    private int mipmapLevels;
    private final TextureAtlasSprite missingImage;

    public TextureMap(String p_i46099_1_)
    {
        this(p_i46099_1_, (IIconCreator)null);
    }

    public TextureMap(String p_i46100_1_, IIconCreator iconCreatorIn)
    {
        this.listAnimatedSprites = Lists.<TextureAtlasSprite>newArrayList();
        this.mapRegisteredSprites = Maps.<String, TextureAtlasSprite>newHashMap();
        this.mapUploadedSprites = Maps.<String, TextureAtlasSprite>newHashMap();
        this.missingImage = new TextureAtlasSprite("missingno");
        this.basePath = p_i46100_1_;
        this.iconCreator = iconCreatorIn;
    }

    private void initMissingImage()
    {
        int[] aint = TextureUtil.missingTextureData;
        this.missingImage.setIconWidth(16);
        this.missingImage.setIconHeight(16);
        int[][] aint1 = new int[this.mipmapLevels + 1][];
        aint1[0] = aint;
        this.missingImage.setFramesTextureData(Lists.newArrayList(new int[][][] {aint1}));
    }

    public void loadTexture(IResourceManager resourceManager) throws IOException
    {
        if (this.iconCreator != null)
        {
            this.loadSprites(resourceManager, this.iconCreator);
        }
    }

    public void loadSprites(IResourceManager resourceManager, IIconCreator p_174943_2_)
    {
        this.mapRegisteredSprites.clear();
        p_174943_2_.registerSprites(this);
        this.initMissingImage();
        this.deleteGlTexture();
        this.loadTextureAtlas(resourceManager);
    }

    public void loadTextureAtlas(IResourceManager resourceManager)
    {
        int i = Minecraft.getGLMaximumTextureSize();
        Stitcher stitcher = new Stitcher(i, i, 0, this.mipmapLevels);
        this.mapUploadedSprites.clear();
        this.listAnimatedSprites.clear();
        int j = Integer.MAX_VALUE;
        int k = 1 << this.mipmapLevels;

        for (Entry<String, TextureAtlasSprite> entry : this.mapRegisteredSprites.entrySet())
        {
            TextureAtlasSprite textureatlassprite = (TextureAtlasSprite)entry.getValue();
            ResourceLocation resourcelocation = this.func_184396_a(textureatlassprite);
            IResource iresource = null;

            try
            {
                PngSizeInfo pngsizeinfo = PngSizeInfo.func_188532_a(resourceManager.getResource(resourcelocation));
                iresource = resourceManager.getResource(resourcelocation);
                boolean flag = iresource.getMetadata("animation") != null;
                textureatlassprite.func_188538_a(pngsizeinfo, flag);
            }
            catch (RuntimeException runtimeexception)
            {
                logger.error((String)("Unable to parse metadata from " + resourcelocation), (Throwable)runtimeexception);
                continue;
            }
            catch (IOException ioexception)
            {
                logger.error((String)("Using missing texture, unable to load " + resourcelocation), (Throwable)ioexception);
                continue;
            }
            finally
            {
                IOUtils.closeQuietly((Closeable)iresource);
            }

            j = Math.min(j, Math.min(textureatlassprite.getIconWidth(), textureatlassprite.getIconHeight()));
            int lvt_11_2_ = Math.min(Integer.lowestOneBit(textureatlassprite.getIconWidth()), Integer.lowestOneBit(textureatlassprite.getIconHeight()));

            if (lvt_11_2_ < k)
            {
                logger.warn("Texture {} with size {}x{} limits mip level from {} to {}", new Object[] {resourcelocation, Integer.valueOf(textureatlassprite.getIconWidth()), Integer.valueOf(textureatlassprite.getIconHeight()), Integer.valueOf(MathHelper.calculateLogBaseTwo(k)), Integer.valueOf(MathHelper.calculateLogBaseTwo(lvt_11_2_))});
                k = lvt_11_2_;
            }

            stitcher.addSprite(textureatlassprite);
        }

        int l = Math.min(j, k);
        int i1 = MathHelper.calculateLogBaseTwo(l);

        if (i1 < this.mipmapLevels)
        {
            logger.warn("{}: dropping miplevel from {} to {}, because of minimum power of two: {}", new Object[] {this.basePath, Integer.valueOf(this.mipmapLevels), Integer.valueOf(i1), Integer.valueOf(l)});
            this.mipmapLevels = i1;
        }

        this.missingImage.generateMipmaps(this.mipmapLevels);
        stitcher.addSprite(this.missingImage);

        try
        {
            stitcher.doStitch();
        }
        catch (StitcherException stitcherexception)
        {
            throw stitcherexception;
        }

        logger.info("Created: {}x{} {}-atlas", new Object[] {Integer.valueOf(stitcher.getCurrentWidth()), Integer.valueOf(stitcher.getCurrentHeight()), this.basePath});
        TextureUtil.allocateTextureImpl(this.getGlTextureId(), this.mipmapLevels, stitcher.getCurrentWidth(), stitcher.getCurrentHeight());
        Map<String, TextureAtlasSprite> map = Maps.<String, TextureAtlasSprite>newHashMap(this.mapRegisteredSprites);

        for (TextureAtlasSprite textureatlassprite1 : stitcher.getStichSlots())
        {
            if (textureatlassprite1 == this.missingImage || this.func_184397_a(resourceManager, textureatlassprite1))
            {
                String s = textureatlassprite1.getIconName();
                map.remove(s);
                this.mapUploadedSprites.put(s, textureatlassprite1);

                try
                {
                    TextureUtil.uploadTextureMipmap(textureatlassprite1.getFrameTextureData(0), textureatlassprite1.getIconWidth(), textureatlassprite1.getIconHeight(), textureatlassprite1.getOriginX(), textureatlassprite1.getOriginY(), false, false);
                }
                catch (Throwable throwable)
                {
                    CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Stitching texture atlas");
                    CrashReportCategory crashreportcategory = crashreport.makeCategory("Texture being stitched together");
                    crashreportcategory.addCrashSection("Atlas path", this.basePath);
                    crashreportcategory.addCrashSection("Sprite", textureatlassprite1);
                    throw new ReportedException(crashreport);
                }

                if (textureatlassprite1.hasAnimationMetadata())
                {
                    this.listAnimatedSprites.add(textureatlassprite1);
                }
            }
        }

        for (TextureAtlasSprite textureatlassprite2 : map.values())
        {
            textureatlassprite2.copyFrom(this.missingImage);
        }
    }

    private boolean func_184397_a(IResourceManager p_184397_1_, final TextureAtlasSprite p_184397_2_)
    {
        ResourceLocation resourcelocation = this.func_184396_a(p_184397_2_);
        IResource iresource = null;
        label9:
        {
            boolean flag;

            try
            {
                iresource = p_184397_1_.getResource(resourcelocation);
                p_184397_2_.func_188539_a(iresource, this.mipmapLevels + 1);
                break label9;
            }
            catch (RuntimeException runtimeexception)
            {
                logger.error((String)("Unable to parse metadata from " + resourcelocation), (Throwable)runtimeexception);
                flag = false;
            }
            catch (IOException ioexception)
            {
                logger.error((String)("Using missing texture, unable to load " + resourcelocation), (Throwable)ioexception);
                flag = false;
                return flag;
            }
            finally
            {
                IOUtils.closeQuietly((Closeable)iresource);
            }

            return flag;
        }

        try
        {
            p_184397_2_.generateMipmaps(this.mipmapLevels);
            return true;
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Applying mipmap");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Sprite being mipmapped");
            crashreportcategory.addCrashSectionCallable("Sprite name", new Callable<String>()
            {
                public String call() throws Exception
                {
                    return p_184397_2_.getIconName();
                }
            });
            crashreportcategory.addCrashSectionCallable("Sprite size", new Callable<String>()
            {
                public String call() throws Exception
                {
                    return p_184397_2_.getIconWidth() + " x " + p_184397_2_.getIconHeight();
                }
            });
            crashreportcategory.addCrashSectionCallable("Sprite frames", new Callable<String>()
            {
                public String call() throws Exception
                {
                    return p_184397_2_.getFrameCount() + " frames";
                }
            });
            crashreportcategory.addCrashSection("Mipmap levels", Integer.valueOf(this.mipmapLevels));
            throw new ReportedException(crashreport);
        }
    }

    private ResourceLocation func_184396_a(TextureAtlasSprite p_184396_1_)
    {
        ResourceLocation resourcelocation = new ResourceLocation(p_184396_1_.getIconName());
        return new ResourceLocation(resourcelocation.getResourceDomain(), String.format("%s/%s%s", new Object[] {this.basePath, resourcelocation.getResourcePath(), ".png"}));
    }

    public TextureAtlasSprite getAtlasSprite(String iconName)
    {
        TextureAtlasSprite textureatlassprite = (TextureAtlasSprite)this.mapUploadedSprites.get(iconName);

        if (textureatlassprite == null)
        {
            textureatlassprite = this.missingImage;
        }

        return textureatlassprite;
    }

    public void updateAnimations()
    {
        TextureUtil.bindTexture(this.getGlTextureId());

        for (TextureAtlasSprite textureatlassprite : this.listAnimatedSprites)
        {
            textureatlassprite.updateAnimation();
        }
    }

    public TextureAtlasSprite registerSprite(ResourceLocation location)
    {
        if (location == null)
        {
            throw new IllegalArgumentException("Location cannot be null!");
        }
        else
        {
            TextureAtlasSprite textureatlassprite = (TextureAtlasSprite)this.mapRegisteredSprites.get(location);

            if (textureatlassprite == null)
            {
                textureatlassprite = TextureAtlasSprite.makeAtlasSprite(location);
                this.mapRegisteredSprites.put(location.toString(), textureatlassprite);
            }

            return textureatlassprite;
        }
    }

    public void tick()
    {
        this.updateAnimations();
    }

    public void setMipmapLevels(int mipmapLevelsIn)
    {
        this.mipmapLevels = mipmapLevelsIn;
    }

    public TextureAtlasSprite getMissingSprite()
    {
        return this.missingImage;
    }
}
