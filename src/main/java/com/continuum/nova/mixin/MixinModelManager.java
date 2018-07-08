package com.continuum.nova.mixin;

import com.continuum.nova.NovaRenderer;
import net.minecraft.client.renderer.block.model.ModelManager;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.resources.IResourceManager;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

@Mixin(ModelManager.class)
public class MixinModelManager {
    @Shadow @Final private TextureMap texMap;

    @Inject(
            method = "onResourceManagerReload",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/BlockModelShapes;reloadModels()V", shift = At.Shift.AFTER)
    )
    private void hookOnResourceManagerReload(IResourceManager resMan, CallbackInfo callbackInfo) {
        // NovaRenderer.getInstance().onResourceManagerReload(resourceManager);
        NovaRenderer.getInstance().addTerrainAtlas(texMap);
    }
}
