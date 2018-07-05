package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaRenderer;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.gui.FontRenderer;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.Tessellator;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.texture.TextureUtil;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.util.ResourceLocation;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfoReturnable;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.Locale;
import java.util.Random;

@Mixin(FontRenderer.class)
public abstract class MixinFontRenderer {
    @Shadow @Final protected ResourceLocation locationFontTexture;

    @Shadow @Final protected int[] charWidth;

    @Shadow protected float posX;

    @Shadow protected float posY;

    @Shadow protected abstract ResourceLocation getUnicodePageLocation(int page);

    @Shadow protected abstract void loadGlyphTexture(int page);

    @Shadow @Final protected byte[] glyphWidth;
    @Shadow private boolean randomStyle;
    @Shadow private boolean boldStyle;
    @Shadow private boolean strikethroughStyle;
    @Shadow private boolean underlineStyle;
    @Shadow private boolean italicStyle;
    @Shadow @Final private int[] colorCode;
    @Shadow private int textColor;
    @Shadow private float alpha;
    @Shadow private float red;
    @Shadow private float green;
    @Shadow private float blue;

    @Shadow public abstract int getCharWidth(char character);

    @Shadow public Random fontRandom;
    @Shadow private boolean unicodeFlag;

    @Shadow protected abstract float renderChar(char ch, boolean italic);

    @Shadow public int FONT_HEIGHT;
    @Shadow private boolean bidiFlag;

    @Shadow protected abstract String bidiReorder(String text);

    private Color color;

    /**
     * @author Janrupf
     * @reason Use changed {@link MixinFontRenderer#readFontTexture(IResourceManager)}
     * @inheritDoc
     */
    @Overwrite
    public void onResourceManagerReload(IResourceManager resourceManager) {
        this.readFontTexture(resourceManager);
    }

    private void readFontTexture(IResourceManager resourceManager) {
        BufferedImage bufferedimage;

        String path = locationFontTexture.getResourcePath();
        ResourceLocation fontTextureLocation = new ResourceLocation("textures/" + path + ".png");

        try(IResource iresource = resourceManager.getResource(fontTextureLocation)) {
            bufferedimage = TextureUtil.readBufferedImage(iresource.getInputStream());
        } catch(IOException ioexception) {
            NovaRenderer.getInstance().getLog().warn("Failed to load FontTexture " + fontTextureLocation, ioexception);
            return;
        }

        int imageWidth = bufferedimage.getWidth();
        int imageHeight = bufferedimage.getHeight();
        int[] imageSize = new int[imageWidth * imageHeight];
        bufferedimage.getRGB(0, 0, imageWidth, imageHeight, imageSize, 0, imageWidth);
        int pixelHeightOrChars = imageHeight / 16;   // Cause there's 16 chars in the x and y direction
        int pixelWidthOfChars = imageWidth / 16;
        boolean lvt_8_1_ = true;
        float lvt_9_1_ = 8.0F / (float) pixelWidthOfChars;
        for(int i = 0; i < 256; ++i) {
            int x = i % 16;
            int y = i / 16;
            if(i == 32) {
                this.charWidth[i] = 4;
            }
            int charsLeftInLine;
            for(charsLeftInLine = pixelWidthOfChars - 1; charsLeftInLine >= 0; --charsLeftInLine) {
                int posInCharArray = x * pixelWidthOfChars + charsLeftInLine;
                boolean flag1 = true;
                for(int curX = 0; curX < pixelHeightOrChars && flag1; ++curX) {
                    int k2 = (y * pixelWidthOfChars + curX) * imageWidth;
                    if((imageSize[posInCharArray + k2] >> 24 & 255) != 0) {
                        flag1 = false;
                    }
                }
                if(!flag1) {
                    break;
                }
            }
            ++charsLeftInLine;
            this.charWidth[i] = (int) (0.5 + ((float) charsLeftInLine * lvt_9_1_)) + 1;
        }
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    protected float renderDefaultChar(int ch, boolean italic) {
        int i = ch % 16 * 8;
        int j = ch / 16 * 8;
        int k = italic ? 1 : 0;
        int l = this.charWidth[ch];
        float f = (float)l - 0.01F;

        NovaDraw.Vertex[] vertices = new NovaDraw.Vertex[]{
                new NovaDraw.Vertex(this.posX + (float)k, this.posY,  (float) i / 128.0F, (float) j / 128.0F, color),
                new NovaDraw.Vertex(this.posX - (float)k, this.posY + 7.99F,  (float)i / 128.0F, ((float)j + 7.99F) / 128.0F, color),
                new NovaDraw.Vertex(this.posX + f - 1.0F + (float)k, this.posY,  ((float)i + f - 1.0F) / 128.0F, (float)j / 128.0F, color),
                new NovaDraw.Vertex(this.posX + f - 1.0F - (float)k, this.posY + 7.99F,  ((float)i + f - 1.0F) / 128.0F, ((float)j + 7.99F) / 128.0F, color)
        };

        Integer[] indices = new Integer[] {0, 1, 2, 2, 1, 3};

        NovaDraw.draw(locationFontTexture, indices, vertices);

        return (float)l;
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    protected float renderUnicodeChar(char ch, boolean italic) {
        int i = this.glyphWidth[ch] & 255;

        if (i == 0) {
            return 0.0F;
        } else {
            int page = ch / 256;
            this.loadGlyphTexture(page);
            int k = i >>> 4;
            int l = i & 15;
            float f = (float)k;
            float f1 = (float)(l + 1);
            float f2 = (float)(ch % 16 * 16) + f;
            float f3 = (float)((ch & 255) / 16 * 16);
            float f4 = f1 - f - 0.02F;
            float f5 = italic ? 1.0F : 0.0F;

            ResourceLocation glyphTexture = getUnicodePageLocation(page);

            NovaDraw.Vertex[] vertices = new NovaDraw.Vertex[]{
                    new NovaDraw.Vertex(this.posX + f5, this.posY,  f2 / 256.0F, f3 / 256.0F, color),
                    new NovaDraw.Vertex(this.posX - f5, this.posY + 7.99F,  f2 / 256.0F, (f3 + 15.98F) / 256.0F, color),
                    new NovaDraw.Vertex(this.posX + f4 / 2.0F + f5, this.posY,  (f2 + f4) / 256.0F, f3 / 256.0F, color),
                    new NovaDraw.Vertex(this.posX + f4 / 2.0F - f5, this.posY + 7.99F,  (f2 + f4) / 256.0F, (f3 + 15.98F) / 256.0F, color)
            };

            Integer[] indices = new Integer[] {0, 1, 2, 2, 1, 3};

            NovaDraw.draw(glyphTexture, indices, vertices);

            return (f1 - f) / 2.0F + 1.0F;
        }
    }

    @Inject(method = "drawStringWithShadow", at = @At(value = "RETURN", shift = At.Shift.BEFORE))
    private void hookDrawStringWithShadow(String text, float x, float y, int color, CallbackInfoReturnable<Integer> cir) {
        NovaDraw.incrementZ();
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    private void renderStringAtPos(String text, boolean shadow) {
        for (int i = 0; i < text.length(); ++i) {
            char currentChar = text.charAt(i);

            if (currentChar == 167 && i + 1 < text.length()) {
                int i1 = "0123456789abcdefklmnor".indexOf(text.toLowerCase(Locale.ENGLISH).charAt(i + 1));

                if (i1 < 16) {
                    this.randomStyle = false;
                    this.boldStyle = false;
                    this.strikethroughStyle = false;
                    this.underlineStyle = false;
                    this.italicStyle = false;

                    if(i1 < 0 || i1 > 15) {
                        i1 = 15;
                    }

                    if(shadow) {
                        i1 += 16;
                    }

                    int colorCode = this.colorCode[i1];
                    this.textColor = colorCode;
                    try {
                        color = new Color(colorCode >> 16, colorCode >> 8 & 255, colorCode & 255, this.alpha * 255);
                    } catch(IllegalArgumentException e) {
                        boolean break_here = true;
                    }
                } else if(i1 == 16) {
                    this.randomStyle = true;
                } else if(i1 == 17) {
                    this.boldStyle = true;
                } else if(i1 == 18) {
                    this.strikethroughStyle = true;
                } else if(i1 == 19) {
                    this.underlineStyle = true;
                } else if(i1 == 20) {
                    this.italicStyle = true;
                } else if(i1 == 21) {
                    this.randomStyle = false;
                    this.boldStyle = false;
                    this.strikethroughStyle = false;
                    this.underlineStyle = false;
                    this.italicStyle = false;
                    color = new Color(red, green, blue, alpha);
                }

                ++i;
            } else {
                int characterIndex = "\u00c0\u00c1\u00c2\u00c8\u00ca\u00cb\u00cd\u00d3\u00d4\u00d5\u00da\u00df\u00e3\u00f5\u011f\u0130\u0131\u0152\u0153\u015e\u015f\u0174\u0175\u017e\u0207\u0000\u0000\u0000\u0000\u0000\u0000\u0000 !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u0000\u00c7\u00fc\u00e9\u00e2\u00e4\u00e0\u00e5\u00e7\u00ea\u00eb\u00e8\u00ef\u00ee\u00ec\u00c4\u00c5\u00c9\u00e6\u00c6\u00f4\u00f6\u00f2\u00fb\u00f9\u00ff\u00d6\u00dc\u00f8\u00a3\u00d8\u00d7\u0192\u00e1\u00ed\u00f3\u00fa\u00f1\u00d1\u00aa\u00ba\u00bf\u00ae\u00ac\u00bd\u00bc\u00a1\u00ab\u00bb\u2591\u2592\u2593\u2502\u2524\u2561\u2562\u2556\u2555\u2563\u2551\u2557\u255d\u255c\u255b\u2510\u2514\u2534\u252c\u251c\u2500\u253c\u255e\u255f\u255a\u2554\u2569\u2566\u2560\u2550\u256c\u2567\u2568\u2564\u2565\u2559\u2558\u2552\u2553\u256b\u256a\u2518\u250c\u2588\u2584\u258c\u2590\u2580\u03b1\u03b2\u0393\u03c0\u03a3\u03c3\u03bc\u03c4\u03a6\u0398\u03a9\u03b4\u221e\u2205\u2208\u2229\u2261\u00b1\u2265\u2264\u2320\u2321\u00f7\u2248\u00b0\u2219\u00b7\u221a\u207f\u00b2\u25a0\u0000".indexOf(currentChar);

                if(this.randomStyle && characterIndex != -1) {
                    int currentCharWidth = this.getCharWidth(currentChar);
                    char c1;

                    while(true) {
                        characterIndex = this.fontRandom.nextInt("\u00c0\u00c1\u00c2\u00c8\u00ca\u00cb\u00cd\u00d3\u00d4\u00d5\u00da\u00df\u00e3\u00f5\u011f\u0130\u0131\u0152\u0153\u015e\u015f\u0174\u0175\u017e\u0207\u0000\u0000\u0000\u0000\u0000\u0000\u0000 !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u0000\u00c7\u00fc\u00e9\u00e2\u00e4\u00e0\u00e5\u00e7\u00ea\u00eb\u00e8\u00ef\u00ee\u00ec\u00c4\u00c5\u00c9\u00e6\u00c6\u00f4\u00f6\u00f2\u00fb\u00f9\u00ff\u00d6\u00dc\u00f8\u00a3\u00d8\u00d7\u0192\u00e1\u00ed\u00f3\u00fa\u00f1\u00d1\u00aa\u00ba\u00bf\u00ae\u00ac\u00bd\u00bc\u00a1\u00ab\u00bb\u2591\u2592\u2593\u2502\u2524\u2561\u2562\u2556\u2555\u2563\u2551\u2557\u255d\u255c\u255b\u2510\u2514\u2534\u252c\u251c\u2500\u253c\u255e\u255f\u255a\u2554\u2569\u2566\u2560\u2550\u256c\u2567\u2568\u2564\u2565\u2559\u2558\u2552\u2553\u256b\u256a\u2518\u250c\u2588\u2584\u258c\u2590\u2580\u03b1\u03b2\u0393\u03c0\u03a3\u03c3\u03bc\u03c4\u03a6\u0398\u03a9\u03b4\u221e\u2205\u2208\u2229\u2261\u00b1\u2265\u2264\u2320\u2321\u00f7\u2248\u00b0\u2219\u00b7\u221a\u207f\u00b2\u25a0\u0000".length());
                        c1 = "\u00c0\u00c1\u00c2\u00c8\u00ca\u00cb\u00cd\u00d3\u00d4\u00d5\u00da\u00df\u00e3\u00f5\u011f\u0130\u0131\u0152\u0153\u015e\u015f\u0174\u0175\u017e\u0207\u0000\u0000\u0000\u0000\u0000\u0000\u0000 !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\u0000\u00c7\u00fc\u00e9\u00e2\u00e4\u00e0\u00e5\u00e7\u00ea\u00eb\u00e8\u00ef\u00ee\u00ec\u00c4\u00c5\u00c9\u00e6\u00c6\u00f4\u00f6\u00f2\u00fb\u00f9\u00ff\u00d6\u00dc\u00f8\u00a3\u00d8\u00d7\u0192\u00e1\u00ed\u00f3\u00fa\u00f1\u00d1\u00aa\u00ba\u00bf\u00ae\u00ac\u00bd\u00bc\u00a1\u00ab\u00bb\u2591\u2592\u2593\u2502\u2524\u2561\u2562\u2556\u2555\u2563\u2551\u2557\u255d\u255c\u255b\u2510\u2514\u2534\u252c\u251c\u2500\u253c\u255e\u255f\u255a\u2554\u2569\u2566\u2560\u2550\u256c\u2567\u2568\u2564\u2565\u2559\u2558\u2552\u2553\u256b\u256a\u2518\u250c\u2588\u2584\u258c\u2590\u2580\u03b1\u03b2\u0393\u03c0\u03a3\u03c3\u03bc\u03c4\u03a6\u0398\u03a9\u03b4\u221e\u2205\u2208\u2229\u2261\u00b1\u2265\u2264\u2320\u2321\u00f7\u2248\u00b0\u2219\u00b7\u221a\u207f\u00b2\u25a0\u0000".charAt(characterIndex);

                        if(currentCharWidth == this.getCharWidth(c1)) {
                            break;
                        }
                    }

                    currentChar = c1;
                }

                float f1 = this.unicodeFlag ? 0.5F : 1.0F;
                boolean shiftForShadow = (currentChar == 0 || characterIndex == -1 || this.unicodeFlag) && shadow;

                if (shiftForShadow) {
                    this.posX -= f1;
                    this.posY -= f1;
                }

                float f = this.renderChar(currentChar, this.italicStyle);

                if (shiftForShadow) {
                    this.posX += f1;
                    this.posY += f1;
                }

                if (this.boldStyle) {
                    this.posX += f1;

                    if(shiftForShadow) {
                        this.posX -= f1;
                        this.posY -= f1;
                    }

                    this.renderChar(currentChar, this.italicStyle);
                    this.posX -= f1;

                    if(shiftForShadow) {
                        this.posX += f1;
                        this.posY += f1;
                    }

                    ++f;
                }

                if (this.strikethroughStyle) {
                    Tessellator tessellator = Tessellator.getInstance();
                    VertexBuffer vertexbuffer = tessellator.getBuffer();
                    GlStateManager.disableTexture2D();
                    vertexbuffer.begin(7, DefaultVertexFormats.POSITION);
                    vertexbuffer.pos((double)this.posX,         (double)(this.posY + (float)(this.FONT_HEIGHT / 2)),        0.0D).endVertex();
                    vertexbuffer.pos((double)(this.posX + f),   (double)(this.posY + (float)(this.FONT_HEIGHT / 2)),        0.0D).endVertex();
                    vertexbuffer.pos((double)(this.posX + f),   (double)(this.posY + (float)(this.FONT_HEIGHT / 2) - 1.0F), 0.0D).endVertex();
                    vertexbuffer.pos((double)this.posX,         (double)(this.posY + (float)(this.FONT_HEIGHT / 2) - 1.0F), 0.0D).endVertex();
                    tessellator.draw();
                    GlStateManager.enableTexture2D();
                }

                if (this.underlineStyle) {
                    Tessellator tessellator1 = Tessellator.getInstance();
                    VertexBuffer vertexbuffer1 = tessellator1.getBuffer();
                    GlStateManager.disableTexture2D();
                    vertexbuffer1.begin(7, DefaultVertexFormats.POSITION);
                    int l = this.underlineStyle ? -1 : 0;
                    vertexbuffer1.pos((double)(this.posX + (float)l),   (double)(this.posY + (float)this.FONT_HEIGHT),          0.0D).endVertex();
                    vertexbuffer1.pos((double)(this.posX + f),          (double)(this.posY + (float)this.FONT_HEIGHT),          0.0D).endVertex();
                    vertexbuffer1.pos((double)(this.posX + f),          (double)(this.posY + (float)this.FONT_HEIGHT - 1.0F),   0.0D).endVertex();
                    vertexbuffer1.pos((double)(this.posX + (float)l),   (double)(this.posY + (float)this.FONT_HEIGHT - 1.0F),   0.0D).endVertex();
                    tessellator1.draw();
                    GlStateManager.enableTexture2D();
                }

                this.posX += (float)((int)f);
            }
        }
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    private int renderString(String text, float x, float y, int color, boolean dropShadow) {
        if(text == null) {
            return 0;
        } else {
            if(this.bidiFlag) {
                text = this.bidiReorder(text);
            }

            if((color & -67108864) == 0) {
                color |= -16777216;
            }

            if(dropShadow) {
                color = (color & 0x00FCFCFC) >> 2 | color & -16777216;
            }

            this.red = (float)(color >> 16 & 255) / 255.0F;
            this.green = (float)(color >> 8 & 255) / 255.0F;
            this.blue = (float)(color & 255) / 255.0F;
            this.alpha = (float)(color >> 24 & 255) / 255.0F;
            this.color = new Color(red, green, blue, alpha);
            this.posX = x;
            this.posY = y;
            this.renderStringAtPos(text, dropShadow);
            return (int)this.posX;
        }
    }
}
