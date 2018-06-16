package com.continuum.nova.mixin.gui;

import com.continuum.nova.gui.NovaDraw;
import glm.mat._4.Mat4;
import glm.vec._4.Vec4;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiMainMenu;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.util.Stack;

@Mixin(GuiMainMenu.class)
public class MixinGuiMainMenu extends GuiScreen {
    @Shadow
    private int panoramaTimer;

    @Shadow
    @Final
    private static ResourceLocation[] TITLE_PANORAMA_PATHS;

    @Shadow
    @Final
    private static ResourceLocation MINECRAFT_TITLE_TEXTURES;

    @Shadow
    private String splashText;

    // Hmm, we can't override the ctor so the check has to stay in here for now

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawPanorama(int mouseX, int mouseY, float partialTicks) {
        // Unused?! Mat4 projmat = Glm.perspective_(120.0F, 1.0F, 0.05F, 10.0F);

        Stack<Mat4> matrixStack = new Stack<>();
        matrixStack.push(new Mat4());

        matrixStack.peek().rotate(180.0F, 1.0F, 0.0F, 0.0F);
        matrixStack.peek().rotate(90.0F, 0.0F, 0.0F, 1.0F);


        for (int j = 0; j < 64; ++j) {
            matrixStack.push(matrixStack.peek());

            float f = ((float) (j % 8) / 8.0F - 0.5F) / 64.0F;
            float f1 = ((float) (j / 8) / 8.0F - 0.5F) / 64.0F;
            float f2 = 0.0F;
            matrixStack.peek().translate(f, f1, 0.0F);
            matrixStack.peek().rotate(MathHelper.sin(((float) this.panoramaTimer + partialTicks) / 400.0F) * 25.0F + 20.0F, 1.0F, 0.0F, 0.0F);
            matrixStack.peek().rotate(-((float) this.panoramaTimer + partialTicks) * 0.1F, 0.0F, 1.0F, 0.0F);

            for (int k = 0; k < 6; ++k) {
                matrixStack.push(matrixStack.peek());


                if (k == 1) {
                    matrixStack.peek().rotate(90.0F, 0.0F, 1.0F, 0.0F);

                }

                if (k == 2) {
                    matrixStack.peek().rotate(180.0F, 0.0F, 1.0F, 0.0F);

                }

                if (k == 3) {
                    matrixStack.peek().rotate(-90.0F, 0.0F, 1.0F, 0.0F);

                }

                if (k == 4) {
                    matrixStack.peek().rotate(90.0F, 1.0F, 0.0F, 0.0F);

                }

                if (k == 5) {
                    matrixStack.peek().rotate(-90.0F, 1.0F, 0.0F, 0.0F);

                }

                int l = 255 / (j + 1);
                Color vertexColor = new Color(255, 255, 255, l);


                Mat4 modelViewProj = matrixStack.peek();

                Vec4 firstVertice = modelViewProj.mul(new Vec4(0, 0, 1, 1));
                Vec4 secondVertice = modelViewProj.mul(new Vec4(0 + 356, 0, 1, 1));
                Vec4 thirdVertice = modelViewProj.mul(new Vec4(0, 0 + 256, 1, 1));
                Vec4 fourthVertice = modelViewProj.mul(new Vec4(0 + 256, 0 + 256, 1, 1));

                Integer[] indexBuffer = new Integer[]{0, 1, 2, 2, 1, 3};
                NovaDraw.Vertex[] vertices = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(
                                firstVertice.x, firstVertice.y,
                                0, 0,
                                vertexColor
                        ),
                        new NovaDraw.Vertex(
                                secondVertice.x, secondVertice.y,
                                1, 0,
                                vertexColor
                        ),
                        new NovaDraw.Vertex(
                                thirdVertice.x, thirdVertice.y,
                                0, 1,
                                vertexColor
                        ),
                        new NovaDraw.Vertex(
                                fourthVertice.x, fourthVertice.y,
                                1, 1,
                                vertexColor
                        )
                };

                NovaDraw.draw(TITLE_PANORAMA_PATHS[k], indexBuffer, vertices);
                matrixStack.pop();

            }

            matrixStack.pop();

            GlStateManager.colorMask(true, true, true, false);
        }
    }

    /**
     * @author Janrupf
     * @reason Change render code to nova
     * @inheritDoc
     */
    @Overwrite
    public void drawScreen(int mouseX, int mouseY, float partialTicks) {
        //this.drawPanorama(mouseX, mouseY, partialTicks);

        int titleStartPosX = (this.width / 2) - 137;

        NovaDraw.incrementZ();

        NovaDraw.drawRectangle(
                MINECRAFT_TITLE_TEXTURES,
                new Rectangle2D.Float(titleStartPosX, 30, 155, 44),
                new Rectangle2D.Float(0, 0, 0.60546875f, 0.171875f)
        );
        NovaDraw.drawRectangle(
                MINECRAFT_TITLE_TEXTURES,
                new Rectangle2D.Float(titleStartPosX + 155, 30, 155, 44),
                new Rectangle2D.Float(0, 45 / 256f, 0.60546875f, 0.171875f)
        );

        NovaDraw.translate((this.width / 2 + 90), 70, 0);
        NovaDraw.rotate(-20, false, false, true);
        float f = 1.8F - MathHelper.abs(MathHelper.sin((float) (Minecraft.getSystemTime() % 1000L) / 1000.0F * ((float) Math.PI * 2F)) * 0.1F);
        f = f * 100.0F / (float) (this.fontRendererObj.getStringWidth(this.splashText) + 32);
        NovaDraw.scale(f, f, 1);
        this.drawCenteredString(this.fontRendererObj, this.splashText, 0, -8, -256);
        NovaDraw.resetMatrix();
        String s = "Minecraft 1.10";

        if (this.mc.isDemo()) {
            s = s + " Demo";
        } else {
            s = s + ("release".equalsIgnoreCase(this.mc.getVersionType()) ? "" : "/" + this.mc.getVersionType());
        }

        this.drawString(this.fontRendererObj, s, 2, this.height - 10, -1);
        String s1 = "Copyright Mojang AB. Do not distribute!";
        this.drawString(this.fontRendererObj, s1, this.width - this.fontRendererObj.getStringWidth(s1) - 2, this.height - 10, -1);


        super.drawScreen(mouseX, mouseY, partialTicks);
    }
}
