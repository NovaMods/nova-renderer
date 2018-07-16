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
import org.lwjgl.LWJGLException;
import org.lwjgl.input.Mouse;
import org.lwjgl.opengl.ContextCapabilities;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Redirect;

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
     * @author Barteks2x
     * @reason It's not going to workf (at least now)
     */
    @Overwrite
    private void renderSkybox(int mouseX, int mouseY, float partialTicks) {

    }

}
