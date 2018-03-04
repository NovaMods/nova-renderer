package com.continuum.nova.chunks;

import com.continuum.nova.NovaNative;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.BlockFluidRenderer;
import net.minecraft.client.renderer.BlockRendererDispatcher;
import net.minecraft.client.renderer.block.model.BakedQuad;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.client.renderer.color.BlockColors;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Splits chunks up into meshes with one mesh for each shader
 *
 * @author ddubois
 * @since 27-Jul-17
 */
public class ChunkBuilder {
    private static final Logger LOG = LogManager.getLogger(ChunkBuilder.class);
    private static final int VERTEX_COLOR_OFFSET = 3;
    private static final int LIGHTMAP_COORD_OFFSET = 6;
    private World world;

    private final Map<String, IGeometryFilter> filters;

    private final BlockColors blockColors;

    private BlockRendererDispatcher blockRendererDispatcher;

    public ChunkBuilder(Map<String, IGeometryFilter> filters, World world, BlockColors blockColors) {
        this.filters = filters;
        this.world = world;
        this.blockColors = blockColors;
    }

    public void createMeshesForChunk(ChunkUpdateListener.BlockUpdateRange range) {

    }

    /**
     * Adds the block at the given position to the blocksForFilter map under each filter that matches the block
     *
     * @param blocksForFilter The map of blocks to potentially add the given block to
     * @param pos The position of the block to add
     */
    private void filterBlockAtPos(Map<String, List<BlockPos>> blocksForFilter, BlockPos pos) {
        IBlockState blockState = world.getBlockState(pos);


        for(Map.Entry<String, IGeometryFilter> entry : filters.entrySet()) {
            if(blockState.getRenderType().equals(EnumBlockRenderType.LIQUID)) {
                if(entry.getValue().matches(blockState)) {
                    if(!blocksForFilter.containsKey(entry.getKey())) {
                        blocksForFilter.put(entry.getKey(), new ArrayList<>());
                    }
                    blocksForFilter.get(entry.getKey()).add(pos);
                }
            }
        }
    }

    private void setLightmapCoord(int[] quadVertexData, int lmCoords) {
        for(int i = 0; i < 4; i++) {
            quadVertexData[i * 7 + LIGHTMAP_COORD_OFFSET] = lmCoords;
        }
    }

    private void setVertexColor(int[] vertexData, int vertexColor) {
        for(int i = 0; i < 4; i++) {
            vertexData[i * 7 + VERTEX_COLOR_OFFSET] = vertexColor;
        }
    }

    private int[] addPosition(BakedQuad quad, BlockPos blockPos) {
        int[] data = Arrays.copyOf(quad.getVertexData(), quad.getVertexData().length);

        for(int vertex = 0; vertex < 28; vertex += 7) {
            addPosToVertex(blockPos, data, vertex);
        }

        return data;
    }

    private void addPosToVertex(BlockPos blockPos, int[] data, int vertex) {
        float x = Float.intBitsToFloat(data[vertex + 0]);
        float y = Float.intBitsToFloat(data[vertex + 1]);
        float z = Float.intBitsToFloat(data[vertex + 2]);

        x += blockPos.getX();
        y += blockPos.getY();
        z += blockPos.getZ();

        data[vertex + 0] = Float.floatToIntBits(x);
        data[vertex + 1] = Float.floatToIntBits(y);
        data[vertex + 2] = Float.floatToIntBits(z);
    }

    public void setWorld(World world) {
        this.world = world;
    }

    /**
     * Returns the IBlockState of the block next to the block at the provided position in the given direction
     *
     * @param pos The position to get the block next to
     * @param world The world that all these blocks are in
     * @param direction The direction to look in
     * @return The IBlockState of the block in the provided direction
     */
    private static IBlockState blockNextTo(BlockPos pos, World world, EnumFacing direction) {
        BlockPos lookupPos = pos.add(direction.getDirectionVec());
        return world.getBlockState(lookupPos);
    }
}
