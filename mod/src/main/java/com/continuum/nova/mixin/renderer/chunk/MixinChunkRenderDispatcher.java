package com.continuum.nova.mixin.renderer.chunk;

import com.continuum.nova.NovaRenderer;
import net.minecraft.client.renderer.chunk.ChunkRenderDispatcher;
import net.minecraft.client.renderer.chunk.RenderChunk;
import net.minecraft.util.math.AxisAlignedBB;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfoReturnable;

@Mixin(ChunkRenderDispatcher.class)
public class MixinChunkRenderDispatcher {

    // MinecraftDev plugin complains, but this is actually correct. TODO: Report MinecraftDev bug
    @Inject(method = {"updateChunkLater", "updateChunkNow"}, at = @At("HEAD"))
    private void onUpdateChunkLater(RenderChunk chunkRenderer, CallbackInfoReturnable<Boolean> cbi) {
        AxisAlignedBB bb = chunkRenderer.boundingBox;
        NovaRenderer.getInstance().chunkUpdateListener
                .markBlockRangeForRenderUpdate((int) bb.minX, (int) bb.minY, (int) bb.minZ, (int) bb.maxX, (int) bb.maxY, (int) bb.maxZ);

    }
}

