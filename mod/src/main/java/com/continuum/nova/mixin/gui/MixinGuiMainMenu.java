package com.continuum.nova.mixin.gui;

import glm.Glm;
import com.continuum.nova.gui.NovaDraw;
import glm.mat._4.Mat4;
import glm.vec._4.Vec4;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiMainMenu;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import org.lwjgl.LWJGLException;
import org.lwjgl.input.Mouse;
import org.lwjgl.opengl.ContextCapabilities;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Redirect;

import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.util.Stack;

@Mixin(GuiMainMenu.class)
public abstract class MixinGuiMainMenu extends GuiScreen {

    @Shadow
    private float panoramaTimer;

    @Shadow
    @Final
    private static ResourceLocation[] TITLE_PANORAMA_PATHS;

    @Shadow
    @Final
    private static ResourceLocation MINECRAFT_TITLE_TEXTURES;

    @Shadow
    private String splashText;

    // Hmm, we can't override the ctor so the check has to stay in here for now

    @Shadow @Final private static ResourceLocation field_194400_H;

    @Shadow @Final private float minceraftRoll;

    @Shadow private int widthCopyrightRest;

    @Shadow private int widthCopyright;

    @Redirect(method = "<init>",
            at = @At(value = "FIELD", target = "Lorg/lwjgl/opengl/ContextCapabilities;OpenGL20:Z"))
    private boolean redirectContextCaps(ContextCapabilities caps) {
        return false;
    }
    /**
     * Draws the main menu panorama
     */
     /**
      * @author Cole Kissane
      * @reason PANORAMA
      */
    @Overwrite
    private void drawPanorama(int mouseX, int mouseY, float partialTicks)
    {

        Mat4 projmat = Glm.perspective_(120.0F, 1.0F, 0.05F, 10.0F);

        Stack<Mat4> matrixStack = new Stack<>();
        matrixStack.push(new Mat4());

        //matrixStack.peek().rotate(180.0F, 1.0F, 0.0F, 0.0F);
        //matrixStack.peek().rotate(90.0F, 0.0F, 0.0F, 1.0F);


        for (int j = 0; j < 1; ++j)
        {
            matrixStack.push(matrixStack.peek());

            float f = ((float)(j % 8) / 8.0F - 0.5F) / 64.0F;
            float f1 = ((float)(j / 8) / 8.0F - 0.5F) / 64.0F;
            float f2 = 0.0F;
            //matrixStack.peek().translate(f, f1, 0.0F);
            matrixStack.peek().rotate(MathHelper.sin(((float)this.panoramaTimer + partialTicks) / 40000.0F) * 25.0F*0.0f + 20.0F, 1.0F, 0.0F, 0.0F);
            matrixStack.peek().rotate(-((float)this.panoramaTimer + partialTicks) * 0.001F, 0.0F, 1.0F, 0.0F);

            for (int k = 0; k < 6; ++k)
            {
                matrixStack.push(matrixStack.peek());

                float shift=128.0f;
                if (k == 1)
                {
                    matrixStack.peek().rotate(90.0F, 0.0F, 1.0F, 0.0F);

                }

                if (k == 2)
                {
                    matrixStack.peek().rotate(180.0F, 0.0F, 1.0F, 0.0F);

                }

                if (k == 3)
                {
                    matrixStack.peek().rotate(-90.0F, 0.0F, 1.0F, 0.0F);

                }

                if (k == 4)
                {
                    matrixStack.peek().rotate(90.0F, 1.0F, 0.0F, 0.0F);

                }

                if (k == 5)
                {
                    matrixStack.peek().rotate(-90.0F, 1.0F, 0.0F, 0.0F);

                }
                //matrixStack.peek().translate(0.0f,0.0F, -shift);
                //matrixStack.peek().rotate(90.0F, 1.0F, 0.0F, 0.0F);

                int l = 255 / (j + 1);
                Color vertexColor = new Color(255, 255, 255, 255);


                Mat4 modelViewProj = matrixStack.peek();

                Vec4 firstVertice =   modelViewProj.mul(new Vec4(-128,-128,1,1));
                Vec4 secondVertice =   modelViewProj.mul(new Vec4(128,-128,1,1));
                Vec4 thirdVertice =   modelViewProj.mul(new Vec4(-128,128,1,1));
                Vec4 fourthVertice =   modelViewProj.mul(new Vec4(128,128,1,1));

                Integer[] indexBuffer = new Integer[]{0, 1, 2, 2, 1, 3};
                float oX=0.0f;
                float oY=0.0f;
                NovaDraw.Vertex[] vertices = new NovaDraw.Vertex[]{
                        new NovaDraw.Vertex(
                                firstVertice.x+oX, firstVertice.y+oY,
                                0, 0,
                                vertexColor
                        ),
                        new NovaDraw.Vertex(
                                secondVertice.x+oX, secondVertice.y+oY,
                                1, 0,
                                vertexColor
                        ),
                        new NovaDraw.Vertex(
                                thirdVertice.x+oX, thirdVertice.y+oY,
                                0, 1,
                                vertexColor
                        ),
                        new NovaDraw.Vertex(
                                fourthVertice.x+oX, fourthVertice.y+oY,
                                1, 1,
                                vertexColor
                        )
                };

                NovaDraw.draw(TITLE_PANORAMA_PATHS[k],indexBuffer,vertices);
                matrixStack.pop();

            }

            matrixStack.pop();

            GlStateManager.colorMask(true, true, true, false);
        }

    }


    /**
     * @author Barteks2x
     * @reason It's not going to workf (at least now)
     */
    @Overwrite
    private void renderSkybox(int mouseX, int mouseY, float partialTicks) {
      this.drawPanorama(mouseX, mouseY, partialTicks);
    }

}
