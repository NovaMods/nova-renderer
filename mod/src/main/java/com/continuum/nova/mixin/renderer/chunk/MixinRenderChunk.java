package com.continuum.nova.mixin.renderer.chunk;

import com.continuum.nova.NovaRenderer;
import com.continuum.nova.chunks.CapturingVertexBuffer;
import com.continuum.nova.chunks.IGeometryFilter;
import com.continuum.nova.chunks.IndexList;
import com.continuum.nova.system.NovaNative;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.BlockRendererDispatcher;
import net.minecraft.client.renderer.BufferBuilder;
import net.minecraft.client.renderer.chunk.ChunkCompileTaskGenerator;
import net.minecraft.client.renderer.chunk.CompiledChunk;
import net.minecraft.client.renderer.chunk.RenderChunk;
import net.minecraft.client.renderer.chunk.VisGraph;
import net.minecraft.client.renderer.vertex.VertexBuffer;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.Redirect;
import org.spongepowered.asm.mixin.injection.Slice;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;
import org.spongepowered.asm.mixin.injection.callback.LocalCapture;

import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@Mixin(RenderChunk.class)
public abstract class MixinRenderChunk {

    @Shadow protected abstract void preRenderBlocks(BufferBuilder bufferBuilderIn, BlockPos pos);

    @Shadow @Final private BlockPos.MutableBlockPos position;
    @Shadow private World world;
    @Shadow @Final private int index;
    private Map<String, IGeometryFilter> filters = NovaRenderer.getInstance().getFilterMap();
    private Map<String, CapturingVertexBuffer> blockLayers = new HashMap<>();

    @Redirect(method = "<init>", at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/OpenGlHelper;useVbo()Z"))
    private boolean useVboCodePath() {
        return true; // always use the "VBO" vanilla code path
    }

    @Inject(method = "<init>", at = @At("RETURN"))
    private void onConstruct(CallbackInfo cbi) {
        for (String layer : filters.keySet()) {
            blockLayers.put(layer, new CapturingVertexBuffer(new BlockPos(0, 0, 0)));
        }
    }

    @Inject(method = "stopCompileTask",
            at = @At(
                    value = "FIELD", // this.compiledChunk = CompiledChunk.DUMMY;
                    target = "Lnet/minecraft/client/renderer/chunk/RenderChunk;compiledChunk:Lnet/minecraft/client/renderer/chunk/CompiledChunk;"
            ))
    private void onReset(CallbackInfo cbi) {
        // setting the CompiledChunk to DUMMY means it will no longer render. This happens for RenderChunks that move out of view and have their
        // positions changed to those on the newly in view. So remove the nova geometry, if any
        NovaNative.mc_chunk_render_object obj = new NovaNative.mc_chunk_render_object();
        // setting these should be enough for remove
        obj.id = index;
        obj.x = this.position.getX();
        obj.y = this.position.getY();
        obj.z = this.position.getZ();

        for (Map.Entry<String, CapturingVertexBuffer> data :  blockLayers.entrySet()) {
            if (data.getValue().isEmpty()) {
                continue;
            }
            NovaRenderer.getInstance().getNative().remove_chunk_geometry_for_filter(data.getKey(), obj);
            data.getValue().reset();
        }
    }

    private Optional<NovaNative.mc_chunk_render_object> makeMeshForBuffer(CapturingVertexBuffer capturingVertexBuffer) {
        if (capturingVertexBuffer.isEmpty()) {
            return Optional.empty();
        }

        IndexList indices = new IndexList();
        NovaNative.mc_chunk_render_object chunk_render_object = new NovaNative.mc_chunk_render_object();
        IntBuffer dat = capturingVertexBuffer.getRawData();
        int dat_len = dat.limit();
        int offset = indices.size();
        for (int i = 0; i < dat_len / 7 / 4; i++) {
            indices.addIndicesForFace(offset, 0);
            offset += 4;
        }

        chunk_render_object.setVertex_data(dat);
        chunk_render_object.setIndices(indices);
        chunk_render_object.format = NovaNative.NovaVertexFormat.POS_UV_LIGHTMAPUV_NORMAL_TANGENT.ordinal();

        return Optional.of(chunk_render_object);
    }

    @Inject(method = "rebuildChunk",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/chunk/VisGraph;<init>()V", shift = At.Shift.AFTER))
    private void preRender(CallbackInfo cbi) {
        for (Map.Entry<String, IGeometryFilter> entry : filters.entrySet()) {
            if (this.blockLayers.containsKey(entry.getKey())) {
                this.blockLayers.put(entry.getKey(), new CapturingVertexBuffer(this.position));
                this.preRenderBlocks(this.blockLayers.get(entry.getKey()), this.position);
            }
        }
    }

    // TODO: can this be moved to place that we can more easily target>
    @Inject(method = "rebuildChunk",
            slice = @Slice(
                    from = @At(value = "INVOKE", target = "Lnet/minecraftforge/client/ForgeHooksClient;setRenderLayer"
                            + "(Lnet/minecraft/util/BlockRenderLayer;)V", ordinal = 0),
                    to = @At(value = "INVOKE", target = "Lnet/minecraftforge/client/ForgeHooksClient;setRenderLayer"
                            + "(Lnet/minecraft/util/BlockRenderLayer;)V", ordinal = 1)
            ),
            at = @At(
                    value = "INVOKE",
                    target = "Lnet/minecraft/client/renderer/chunk/ChunkCompileTaskGenerator;getRegionRenderCacheBuilder()"
                            + "Lnet/minecraft/client/renderer/RegionRenderCacheBuilder;"
            ),
            locals = LocalCapture.CAPTURE_FAILHARD
    )
    private void handleLayer(float x, float y, float z, ChunkCompileTaskGenerator generator, CallbackInfo ci, CompiledChunk compiledchunk, int i,
            BlockPos pos1, BlockPos pos2, VisGraph vis, HashSet set, boolean aboolean[], BlockRendererDispatcher
            renderDis, Iterator it, BlockPos.MutableBlockPos mutablePos, IBlockState blockState, Block block, BlockRenderLayer
            layers[], int var18, int var19, BlockRenderLayer blockrenderlayer1, int j) {
        for (Map.Entry<String, IGeometryFilter> entry : filters.entrySet()) {
            if (entry.getValue().matches(block.getDefaultState())) {
                renderDis.renderBlock(blockState, mutablePos, this.world, this.blockLayers.get(entry.getKey()));
            }
        }
    }

    // "comment out" the parts in that if() for now
    @Redirect(method = "rebuildChunk",
            at = @At(
                    value = "INVOKE",
                    target = "Lnet/minecraft/client/renderer/chunk/CompiledChunk;isLayerStarted(Lnet/minecraft/util/BlockRenderLayer;)Z",
                    ordinal = 0
            ))
    private boolean redirectIsLayerStarted0(CompiledChunk _this, BlockRenderLayer layer) {
        return true;
    }

    @Redirect(method = "rebuildChunk",
            at = @At(
                    value = "INVOKE",
                    target = "Lnet/minecraft/client/renderer/chunk/CompiledChunk;isLayerStarted(Lnet/minecraft/util/BlockRenderLayer;)Z",
                    ordinal = 1
            ))
    private boolean redirectIsLayerStarted1(CompiledChunk _this, BlockRenderLayer layer) {
        return false;
    }

    // this serves both as injection and "comment out code"
    @Redirect(
            method = "rebuildChunk",
            at = @At(
                    value = "INVOKE",
                    target = "Lnet/minecraft/util/BlockRenderLayer;values()[Lnet/minecraft/util/BlockRenderLayer;",
                    ordinal = 2
            ))
    private BlockRenderLayer[] preIterateBlockRenderLayers() {
        for (Map.Entry<String, CapturingVertexBuffer> entry : blockLayers.entrySet()) {
            //entry.getValue().finishDrawing();
            CapturingVertexBuffer b = entry.getValue();
            //this.blockLayers.put(entry.getKey(),new CapturingVertexBuffer(minPos));
            Optional<NovaNative.mc_chunk_render_object> renderObj = makeMeshForBuffer(b);

            renderObj.ifPresent(obj -> {
                obj.id = index;
                obj.x = this.position.getX();
                obj.y = this.position.getY();
                obj.z = this.position.getZ();

                NovaNative.LOG.info("Adding render geometry for chunk {}, layer {}", this.position, entry.getKey());
                NovaRenderer.getInstance().getNative().add_chunk_geometry_for_filter(entry.getKey(), obj);
            });

        }
        return new BlockRenderLayer[0]; // do not iterate, skip this code
    }

    @Redirect(method = "rebuildChunk", at = @At(
            value = "INVOKE",
            target = "Lnet/minecraft/client/renderer/BlockRendererDispatcher;renderBlock(Lnet/minecraft/block/state/IBlockState;"
                    + "Lnet/minecraft/util/math/BlockPos;Lnet/minecraft/world/IBlockAccess;Lnet/minecraft/client/renderer/BufferBuilder;)Z"
    ))
    public boolean noopRenderBlock(BlockRendererDispatcher _this, IBlockState state, BlockPos pos, IBlockAccess blockAccess, BufferBuilder
            bufferBuilderIn) {
        return false;
    }
}
