package com.continuum.nova.mixin;

import com.continuum.nova.NovaRenderer;
import com.continuum.nova.input.Keyboard;
import com.continuum.nova.input.Mouse;
import com.continuum.nova.system.NovaNative;
import net.minecraft.client.Minecraft;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(Minecraft.class)
public class MixinMinecraft {

    @Shadow public boolean inGameHasFocus;

    private Logger novaLogger;
    private NovaNative novaNative = NovaNative.INSTANCE;

    @Inject(method = "<init>", at = @At("RETURN"))
    private void constructorEnd(CallbackInfo info) {
        novaLogger = LogManager.getLogger("Nova");
        novaLogger.info("Loading nova...");
        NovaRenderer.create();
        inGameHasFocus = novaNative.display_is_active();
        Mouse.create();
        Keyboard.create();
    }


}
