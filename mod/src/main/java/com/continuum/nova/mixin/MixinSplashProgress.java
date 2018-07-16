package com.continuum.nova.mixin;

import net.minecraftforge.fml.client.SplashProgress;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Redirect;

@Mixin(SplashProgress.class)
public class MixinSplashProgress {
    @Redirect(
            method = "start",
            at = @At(target = "getBool", value = "INVOKE"),
            remap = false
    )
    private static boolean splashEnabled() {
        return false;
    }
}
