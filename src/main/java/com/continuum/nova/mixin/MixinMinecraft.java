package com.continuum.nova.mixin;

import net.minecraft.client.Minecraft;
import org.apache.logging.log4j.Logger;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(Minecraft.class)
public class MixinMinecraft {

    @Shadow @Final private static Logger LOGGER;

    @Inject(method = "<init>", at = @At("RETURN"))
    public void constructorEnd(CallbackInfo info) {
        LOGGER.info("Nova hooked into Minecraft (This is just a test)");
    }
}
