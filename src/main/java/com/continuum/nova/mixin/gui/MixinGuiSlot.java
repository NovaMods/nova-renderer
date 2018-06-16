package com.continuum.nova.mixin.gui;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.gui.NovaDraw;
import net.minecraft.client.gui.Gui;
import net.minecraft.client.gui.GuiSlot;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.Tessellator;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;

import java.awt.*;

@Mixin(GuiSlot.class)
public abstract class MixinGuiSlot {
    @Shadow
    protected boolean visible;

    @Shadow
    protected int mouseX;

    @Shadow
    protected int mouseY;

    @Shadow
    protected abstract void drawBackground();

    @Shadow
    protected abstract int getScrollBarX();

    @Shadow
    protected abstract void bindAmountScrolled();

    @Shadow
    public int left;

    @Shadow
    public int bottom;

    @Shadow
    public int right;

    @Shadow
    public int top;

    @Shadow
    protected float amountScrolled;

    @Shadow
    public int width;

    @Shadow
    public abstract int getListWidth();

    @Shadow
    protected boolean hasListHeader;

    @Shadow
    protected abstract void drawListHeader(int insideLeft, int insideTop, Tessellator tessellatorIn);

    @Shadow
    public int height;

    @Shadow
    public abstract int getMaxScroll();

    @Shadow
    protected abstract int getContentHeight();

    @Shadow
    protected abstract void renderDecorations(int mouseXIn, int mouseYIn);

    @Shadow
    protected abstract int getSize();

    @Shadow
    @Final
    public int slotHeight;

    @Shadow
    public int headerPadding;

    @Shadow
    protected abstract void updateItemPos(int entryID, int insideLeft, int yPos);

    @Shadow
    protected boolean showSelectionBox;

    @Shadow
    protected abstract boolean isSelected(int slotIndex);

    @Shadow
    protected abstract void drawSlot(int entryID, int insideLeft, int yPos, int insideSlotHeight, int mouseXIn, int mouseYIn);

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawScreen(int mouseXIn, int mouseYIn, float partialTicks) {
        if (this.visible) {
            this.mouseX = mouseXIn;
            this.mouseY = mouseYIn;

            NovaDraw.incrementZ();
            drawBackground();
            int scrollBarX = this.getScrollBarX();
            int scrollBarOutside = scrollBarX + 6;
            bindAmountScrolled();

            NovaDraw.incrementZ();

            NovaDraw.Vertex[] backgroundVertices = new NovaDraw.Vertex[]{
                    new NovaDraw.Vertex(this.left, this.bottom, this.left / 32.0F, (this.bottom + this.amountScrolled) / 32.0F, new Color(32, 32, 32)),
                    new NovaDraw.Vertex(this.right, this.bottom, this.right / 32.0F, (this.bottom + this.amountScrolled) / 32.0F, new Color(32, 32, 32)),
                    new NovaDraw.Vertex(this.right, this.top, this.right / 32.0F, (this.top + this.amountScrolled) / 32.0F, new Color(32, 32, 32)),
                    new NovaDraw.Vertex(this.left, this.top, this.left / 32.0F, (this.top + this.amountScrolled) / 32.0F, new Color(32, 32, 32))
            };
            Integer[] indices = new Integer[]{
                    0, 1, 2, 2, 3, 0
            };

            NovaDraw.draw(Gui.OPTIONS_BACKGROUND, indices, backgroundVertices);

            int k = left + width / 2 - getListWidth() / 2 + 2;
            int l = top + 4 - (int) amountScrolled;
            NovaDraw.incrementZ();
            if (this.hasListHeader) {
                drawListHeader(k, l, null);
            }
            NovaDraw.incrementZ();

            drawSelectionBox(k, l, mouseXIn, mouseYIn);

            GlStateManager.disableDepth();
            NovaDraw.incrementZ();

            this.overlayBackground(0, this.top, 255, 255);
            this.overlayBackground(this.bottom, this.height, 255, 255);
            GlStateManager.enableBlend();
            GlStateManager.tryBlendFuncSeparate(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA, GlStateManager.SourceFactor.ZERO, GlStateManager.DestFactor.ONE);
            GlStateManager.disableAlpha();
            GlStateManager.shadeModel(7425);
            GlStateManager.disableTexture2D();


            NovaDraw.Vertex[] shade = new NovaDraw.Vertex[]{
                    new NovaDraw.Vertex(this.left, this.top, 0, 0, new Color(0, 0, 0, 255)),
                    new NovaDraw.Vertex(this.left, this.top + 4, 0, 1, new Color(0, 0, 0, 0)),
                    new NovaDraw.Vertex(this.right, this.top + 4, 1, 1, new Color(0, 0, 0, 0)),
                    new NovaDraw.Vertex(this.right, this.top, 1, 0, new Color(0, 0, 0, 255))
            };

            indices = new Integer[]{
                    0, 1, 2, 0, 3, 2
            };

            NovaDraw.draw(NovaConstants.WHITE_TEXTURE_GUI_LOCATION, indices, shade);

            shade = new NovaDraw.Vertex[]{
                    new NovaDraw.Vertex(this.left, this.bottom - 4, 0, 0, new Color(0, 0, 0, 0)),
                    new NovaDraw.Vertex(this.left, this.bottom, 0, 1, new Color(0, 0, 0, 255)),
                    new NovaDraw.Vertex(this.right, this.bottom, 1, 1, new Color(0, 0, 0, 255)),
                    new NovaDraw.Vertex(this.right, this.bottom - 4, 1, 0, new Color(0, 0, 0, 0))
            };

            indices = new Integer[]{
                    0, 1, 2, 0, 2, 3
            };

            NovaDraw.draw(NovaConstants.WHITE_TEXTURE_GUI_LOCATION, indices, shade);

            int j1 = this.getMaxScroll();

            if (j1 > 0) {
                int k1 = (this.bottom - this.top) * (this.bottom - this.top) / this.getContentHeight();
                k1 = MathHelper.clamp(k1, 32, this.bottom - this.top - 8);
                int l1 = (int) this.amountScrolled * (this.bottom - this.top - k1) / j1 + this.top;

                if (l1 < this.top) {
                    l1 = this.top;
                }

                NovaDraw.incrementZ();

                NovaDraw.Vertex[] scrollbarBackground = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(scrollBarX, this.bottom, 0, 1, new Color(0, 0, 0, 255)),
                        new NovaDraw.Vertex(scrollBarOutside, this.bottom, 1, 1, new Color(0, 0, 0, 255)),
                        new NovaDraw.Vertex(scrollBarOutside, this.top, 1, 0, new Color(0, 0, 0, 255)),
                        new NovaDraw.Vertex(scrollBarX, this.top, 0, 0, new Color(0, 0, 0, 255))
                };

                indices = new Integer[]{
                        0, 1, 2, 2, 3, 0
                };

                NovaDraw.draw(NovaConstants.WHITE_TEXTURE_GUI_LOCATION, indices, scrollbarBackground);
                NovaDraw.incrementZ();

                NovaDraw.Vertex[] scrollbarShade = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(scrollBarX, (l1 + k1), 0, 1, new Color(128, 128, 128, 255)),
                        new NovaDraw.Vertex(scrollBarOutside, (l1 + k1), 1, 1, new Color(128, 128, 128, 255)),
                        new NovaDraw.Vertex(scrollBarOutside, l1, 1, 0, new Color(128, 128, 128, 255)),
                        new NovaDraw.Vertex(scrollBarX, l1, 0, 0, new Color(128, 128, 128, 255))
                };

                indices = new Integer[]{
                        0, 1, 2, 2, 3, 0
                };

                NovaDraw.draw(NovaConstants.WHITE_TEXTURE_GUI_LOCATION, indices, scrollbarShade);

                NovaDraw.incrementZ();


                NovaDraw.Vertex[] scrollbar = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(scrollBarX, (l1 + k1) - 1, 0, 1, new Color(192, 192, 192, 255)),
                        new NovaDraw.Vertex(scrollBarOutside - 1, (l1 + k1) - 1, 1, 1, new Color(192, 192, 192, 255)),
                        new NovaDraw.Vertex(scrollBarOutside - 1, l1, 1, 0, new Color(192, 192, 192, 255)),
                        new NovaDraw.Vertex(scrollBarX, l1, 0, 0, new Color(192, 192, 192, 255))
                };

                indices = new Integer[]{
                        0, 1, 2, 2, 3, 0
                };

                NovaDraw.draw(NovaConstants.WHITE_TEXTURE_GUI_LOCATION, indices, scrollbar);

            }

            this.renderDecorations(mouseXIn, mouseYIn);
        }
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    protected void drawSelectionBox(int insideLeft, int insideTop, int mouseXIn, int mouseYIn) {
        int i = this.getSize();

        ResourceLocation whiteTextureLocation = NovaConstants.WHITE_TEXTURE_GUI_LOCATION;

        for (int j = 0; j < i; ++j) {
            int k = insideTop + j * this.slotHeight + this.headerPadding;
            int l = this.slotHeight - 4;

            if (k > this.bottom || k + l < this.top) {
                this.updateItemPos(j, insideLeft, k);
            }

            if (this.showSelectionBox && this.isSelected(j)) {
                int leftX = this.left + (this.width / 2 - this.getListWidth() / 2);
                int rightX = this.left + this.width / 2 + this.getListWidth() / 2;

                NovaDraw.Vertex[] vertices = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(leftX, k - 2, 0, 0, new Color(128, 128, 128, 255)),
                        new NovaDraw.Vertex(leftX, k + l + 2, 0, 1, new Color(128, 128, 128, 255)),
                        new NovaDraw.Vertex(rightX, k + l + 2, 1, 1, new Color(128, 128, 128, 255)),
                        new NovaDraw.Vertex(rightX, k - 2, 1, 0, new Color(128, 128, 128, 255)),
                };

                Integer[] indices = new Integer[]{
                        0, 1, 2,
                        0, 2, 3,
                };

                NovaDraw.draw(whiteTextureLocation, indices, vertices);

                NovaDraw.incrementZ();

                vertices = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(leftX + 1, k - 1, 0, 0, new Color(0, 0, 0, 255)),
                        new NovaDraw.Vertex(leftX + 1, k + l + 1, 0, 1, new Color(0, 0, 0, 255)),
                        new NovaDraw.Vertex(rightX - 1, k + l + 1, 1, 1, new Color(0, 0, 0, 255)),
                        new NovaDraw.Vertex(rightX - 1, k - 1, 1, 0, new Color(0, 0, 0, 255)),
                };

                indices = new Integer[]{
                        0, 1, 2,
                        0, 2, 3,
                };

                NovaDraw.draw(whiteTextureLocation, indices, vertices);


            }
            NovaDraw.incrementZ();

            this.drawSlot(j, insideLeft, k, l, mouseXIn, mouseYIn);
        }
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    protected void overlayBackground(int startY, int endY, int startAlpha, int endAlpha) {

        NovaDraw.Vertex[] backgroundVertices = new NovaDraw.Vertex[]{
                new NovaDraw.Vertex(this.left, endY, 0, endY / 32.0F, new Color(64, 64, 64, endAlpha)),
                new NovaDraw.Vertex(this.left + this.width, endY, this.width / 32.0F, endY / 32.0F, new Color(64, 64, 64, endAlpha)),
                new NovaDraw.Vertex(this.left + this.width, startY, this.width / 32.0F, startY / 32.0F, new Color(64, 64, 64, endAlpha)),
                new NovaDraw.Vertex(this.left, startY, 0.0F, startY / 32.0F, new Color(64, 64, 64, startAlpha))
        };
        Integer[] indices = new Integer[]{
                0, 1, 2, 2, 3, 0
        };

        NovaDraw.draw(Gui.OPTIONS_BACKGROUND, indices, backgroundVertices);

    }
}
