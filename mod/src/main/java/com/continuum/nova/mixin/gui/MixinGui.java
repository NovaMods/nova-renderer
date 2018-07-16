package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.NovaRenderer;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;

import java.awt.Color;
import java.awt.geom.Rectangle2D;

@Mixin(Gui.class)
public class MixinGui {

    /**
     * @author Barteks2x, coler706
     * @reason Replace opengl with nova code
     */
    @Overwrite
    public static void drawRect(int left, int top, int right, int bottom, int ARGB) {
        if (left < right) {
            int i = left;
            left = right;
            right = i;
        }

        if (top < bottom) {
            int j = top;
            top = bottom;
            bottom = j;
        }
        NovaDraw.drawRectangle(
                NovaConstants.WHITE_TEXTURE_GUI_LOCATION,
                new Rectangle2D.Float((float) left, (float) bottom, (float) (right - left), (float) (top - bottom)),
                new Rectangle2D.Float(0, 0, 0, 0), // it's a white texture, that we apply color to, so use just one pixel
                fromArgb(ARGB)
        );
        NovaDraw.incrementZ();
    }

    /**
     * @author Barteks2x
     * @reason Replace opengl with nova code
     */
    @Overwrite
    protected void drawGradientRect(int left, int top, int right, int bottom, int startARGB, int endARGB) {
        Color startCol = fromArgb(startARGB);
        Color endCol = fromArgb(endARGB);

        Integer[] indexBuffer = new Integer[]{0, 1, 2, 0, 2, 3};
        NovaDraw.Vertex[] vertices = new NovaDraw.Vertex[]{
                new NovaDraw.Vertex(left, bottom, 0, 0, endCol),
                new NovaDraw.Vertex(left, top, 0, 0, startCol),
                new NovaDraw.Vertex(right, top, 0, 0, startCol),
                new NovaDraw.Vertex(right, bottom, 0, 0, endCol)
        };
        NovaDraw.draw(NovaConstants.WHITE_TEXTURE_GUI_LOCATION, indexBuffer, vertices);
        NovaDraw.incrementZ();
    }

    /**
     * @author Barteks2x, coler706
     * @reason Replace opengl with nova code
     */
    @Overwrite
    public void drawTexturedModalRect(int x, int y, int textureX, int textureY, int width, int height) {
        NovaDraw.drawRectangle(
                NovaRenderer.getTextureManager().getCurrentTexture(),
                new Rectangle2D.Float((float) x, (float) y, (float) width, (float) height),
                new Rectangle2D.Float(
                        (float) (textureX) * 0.00390625F,
                        (float) (textureY) * 0.00390625F,

                        (float) (width) * 0.00390625F,
                        (float) (height) * 0.00390625F)
        );
        NovaDraw.incrementZ();
    }

    /**
     * @author Barteks2x, coler706
     * @reason Replace opengl with nova code
     */
    @Overwrite
    public void drawTexturedModalRect(float xCoord, float yCoord, int minU, int minV, int maxU, int maxV) {
        NovaDraw.drawRectangle(
                NovaRenderer.getTextureManager().getCurrentTexture(),
                new Rectangle2D.Float(xCoord, yCoord, maxU, maxV),
                new Rectangle2D.Float(minU * 0.00390625F, maxU * 0.00390625F,
                        minV * 0.00390625F, maxV * 0.00390625F)
        );
        NovaDraw.incrementZ();
    }

    /**
     * @author Barteks2x
     * @reason Replace opengl with nova code
     */
    @Overwrite
    public void drawTexturedModalRect(int xCoord, int yCoord, TextureAtlasSprite textureSprite, int widthIn, int heightIn) {
        NovaDraw.drawRectangle(
                NovaRenderer.getTextureManager().getCurrentTexture(),
                new Rectangle2D.Float(xCoord, yCoord, widthIn, heightIn),
                new Rectangle2D.Float(textureSprite.getMinU(), textureSprite.getMaxU(),
                        textureSprite.getMinV(), textureSprite.getMaxV())
        );
        NovaDraw.incrementZ();
    }

    /**
     * @author Barteks2x
     * @reason Replace opengl with nova code
     */
    @Overwrite
    public static void drawModalRectWithCustomSizedTexture(int x, int y, float u, float v, int width, int height, float textureWidth,
            float textureHeight) {
        float uScale = 1.0F / textureWidth;
        float vScale = 1.0F / textureHeight;
        NovaDraw.drawRectangle(
                NovaRenderer.getTextureManager().getCurrentTexture(),
                new Rectangle2D.Float(x, y, width, height),
                new Rectangle2D.Float(u * uScale, (u + width) * uScale,
                        v * vScale, (v + height) * vScale)
        );
        NovaDraw.incrementZ();
    }

    /**
     * @author Barteks2x
     * @reason Not yet implemented and never used by vanilla according to MCP comment. Tileable textures need more work from nova side.
     */
    @Overwrite
    public static void drawScaledCustomSizeModalRect(int x, int y, float u, float v, int uWidth, int vHeight, int width, int height, float tileWidth,
            float tileHeight) {
        throw new UnsupportedOperationException("Not yet implemented");
    }


    private static Color fromArgb(int color) {
        int a = (color >> 24 & 255);
        int r = (color >> 16 & 255);
        int g = (color >> 8 & 255);
        int b = (color & 255);
        return new Color(r, g, b, a);
    }
}
