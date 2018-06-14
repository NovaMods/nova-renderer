package com.continuum.nova.mixin.texture;

import com.continuum.nova.gui.MemoryTextureAtlasSprite;
import com.continuum.nova.interfaces.INovaTextureMap;
import com.google.common.collect.Lists;
import net.minecraft.client.renderer.texture.*;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.util.ResourceLocation;
import org.apache.logging.log4j.Logger;
import org.spongepowered.asm.mixin.*;

import java.util.List;
import java.util.Map;
import java.util.Optional;

@Mixin(TextureMap.class)
@Implements(@Interface(iface = INovaTextureMap.class, prefix = "nova$"))
public abstract class MixinTextureMap {
    @Shadow
    private int mipmapLevels;

    @Shadow @Final private Map<String, TextureAtlasSprite> mapUploadedSprites;
    @Shadow @Final private List<TextureAtlasSprite> listAnimatedSprites;
    @Shadow @Final private Map<String, TextureAtlasSprite> mapRegisteredSprites;

    @Shadow protected abstract ResourceLocation getResourceLocation(TextureAtlasSprite p_184396_1_);

    @Shadow @Final private static Logger LOGGER;
    @Shadow @Final private TextureAtlasSprite missingImage;

    @Shadow protected abstract boolean generateMipmaps(IResourceManager resourceManager, TextureAtlasSprite texture);

    @Shadow @Final private String basePath;
    private TextureAtlasSprite whiteImage;
    private int width;
    private int height;

    /**
     * @author Janrupf
     * @reason Overwritten because a lot of changes are required that cannot be done at one specific point in the method
     * @inheritDoc
     */
    /*
    @Overwrite
    public void loadTextureAtlas(IResourceManager resourceManager) {
        int i = Minecraft.getGLMaximumTextureSize();
        Stitcher stitcher = new Stitcher(i, i, 0, this.mipmapLevels);
        this.mapUploadedSprites.clear();
        this.listAnimatedSprites.clear();
        int j = Integer.MAX_VALUE;
        int mipLevel0Size = 1 << this.mipmapLevels;

        for(Map.Entry<String, TextureAtlasSprite> entry : this.mapRegisteredSprites.entrySet()) {
            TextureAtlasSprite sprite = entry.getValue();
            ResourceLocation spriteLocation = this.getResourceLocation(sprite);
            sprite.setLocation(spriteLocation);

            try(IResource spriteResource = resourceManager.getResource(spriteLocation)) {
                PngSizeInfo spriteSizeInfo = PngSizeInfo.makeFromResource(spriteResource);
                boolean isAnimated = spriteResource.getMetadata("animation") != null;
                sprite.setAndValidateSize(spriteSizeInfo, isAnimated);

                // Moved this code up to remove the continue statements
                j = Math.min(j, Math.min(sprite.getIconWidth(), sprite.getIconHeight()));
                int lvt_11_2_ = Math.min(Integer.lowestOneBit(sprite.getIconWidth()), Integer.lowestOneBit(sprite.getIconHeight()));

                if (lvt_11_2_ < mipLevel0Size) {
                    LOGGER.warn("Texture {} with size {}x{} limits mip level from {} to {}", spriteLocation, sprite.getIconWidth(), sprite.getIconHeight(), MathHelper.calculateLogBaseTwo(mipLevel0Size), MathHelper.calculateLogBaseTwo(lvt_11_2_));
                    mipLevel0Size = lvt_11_2_;
                }

                stitcher.addSprite(sprite);

            } catch (RuntimeException runtimeexception) {
                LOGGER.error("Unable to parse metadata from {}", spriteLocation, runtimeexception);

            } catch (IOException ioexception) {
                LOGGER.error("Using missing texture, unable to load {}", spriteLocation, ioexception);

            }
        }

        int l = Math.min(j, mipLevel0Size);
        int i1 = NovaMath.calculateLogBaseTwo(l);

        if (i1 < this.mipmapLevels) {
            LOGGER.warn("{}: dropping miplevel from {} to {}, because of minimum power of two: {}", this.basePath, this.mipmapLevels, i1, l);
            this.mipmapLevels = i1;
        }

        this.missingImage.generateMipmaps(this.mipmapLevels);
        stitcher.addSprite(this.missingImage);

        if(whiteImage != null) {
            stitcher.addSprite(whiteImage);
        }

        stitcher.doStitch();

        width = stitcher.getCurrentWidth();
        height = stitcher.getCurrentHeight();

        LOGGER.info("Created: {}x{} {}-atlas", stitcher.getCurrentWidth(), stitcher.getCurrentHeight(), this.basePath);
        TextureUtil.allocateTextureImpl(this.getGlTextureId(), this.mipmapLevels, stitcher.getCurrentWidth(), stitcher.getCurrentHeight());
        Map<String, TextureAtlasSprite> map = Maps.<String, TextureAtlasSprite>newHashMap(this.mapRegisteredSprites);

        for (TextureAtlasSprite textureatlassprite1 : stitcher.getStichSlots()) {
            if (textureatlassprite1 == this.missingImage || this.generateMipmaps(resourceManager, textureatlassprite1)) {
                String s = textureatlassprite1.getIconName();
                map.remove(s);
                this.mapUploadedSprites.put(s, textureatlassprite1);

                if (textureatlassprite1.hasAnimationMetadata()) {
                    this.listAnimatedSprites.add(textureatlassprite1);
                }
            }
        }

        for (TextureAtlasSprite textureatlassprite2 : map.values()) {
            textureatlassprite2.copyFrom(this.missingImage);
        }
    } */


    public void nova$createWhiteTexture(ResourceLocation resourceLocation) {
        whiteImage = new MemoryTextureAtlasSprite(resourceLocation.toString());

        whiteImage.setIconHeight(16);
        whiteImage.setIconWidth(16);

        int[][] data = new int[mipmapLevels + 1][];
        for (int mipLevel = 0; mipLevel < mipmapLevels + 1; mipLevel++) {
            data[mipLevel] = new int[256];
            for (int i = 0; i < 256; i++) {
                data[mipLevel][i] = 0xFFFFFFFF;
            }
        }

        whiteImage.setFramesTextureData(Lists.newArrayList(new int[][][]{data}));
    }

    public Optional<TextureAtlasSprite> nova$getWhiteImage() {
        return Optional.ofNullable(whiteImage);
    }

    public int nova$getWidth() {
        return width;
    }

    public int nova$getHeight() {
        return height;
    }
}
