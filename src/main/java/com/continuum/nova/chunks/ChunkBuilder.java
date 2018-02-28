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
        blockRendererDispatcher =  Minecraft.getMinecraft().getBlockRenderDispatcher();
        Map<String, List<BlockPos>> blocksForFilter = new HashMap<>();

        for(int x = range.min.x; x <= range.max.x; x++) {
            for(int y = range.min.y; y < range.max.y; y++) {
                for(int z = range.min.z; z <= range.max.z; z++) {
                    filterBlockAtPos(blocksForFilter, new BlockPos(x, y, z));
                }
            }
        }

        final int chunkHashCode = 31 * range.min.x + range.min.z;

        for(String filterName : blocksForFilter.keySet()) {
            Optional<NovaNative.mc_chunk_render_object> renderObj = makeMeshForBlocks(blocksForFilter.get(filterName), world, new BlockPos(range.min.x, range.min.y, range.min.z));
            renderObj.ifPresent(obj -> {
                obj.id = chunkHashCode;
                obj.x = range.min.x;
                obj.y = range.min.y;
                obj.z = range.min.z;

                LOG.info("Adding render geometry for chunk {}", range);
                NovaNative.INSTANCE.add_chunk_geometry_for_filter(filterName, obj);
            });
        }
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
            if(blockState.getRenderType().equals(EnumBlockRenderType.INVISIBLE)) {
                if(!entry.getValue().matches(blockState)) {
                    if(!blocksForFilter.containsKey(entry.getKey())) {
                        blocksForFilter.put(entry.getKey(), new ArrayList<>());
                    }
                    ArrayList<BlockPos> newList=new ArrayList<BlockPos>();
                    for(BlockPos i:blocksForFilter.get(entry.getKey())){
                        if(i.getX()!=pos.getX()||i.getY()!=pos.getY()||i.getZ()!=pos.getZ()){
                            newList.add(i);
                        }else{
                            LOG.info("REMOVE BLOCK?");

                        }
                    }
                    blocksForFilter.put(entry.getKey(),newList);
                    //blocksForFilter.get(entry.getKey()).add(pos);
                }
            }else{
            if(entry.getValue().matches(blockState)) {
                if(!blocksForFilter.containsKey(entry.getKey())) {
                    blocksForFilter.put(entry.getKey(), new ArrayList<>());
                }
                blocksForFilter.get(entry.getKey()).add(pos);
            }
        }
        }
    }

    private Optional<NovaNative.mc_chunk_render_object> makeMeshForBlocks(List<BlockPos> positions, World world, BlockPos chunkPos) {
        List<Integer> vertexData = new ArrayList<>();
        IndexList indices = new IndexList();
        NovaNative.mc_chunk_render_object chunk_render_object = new NovaNative.mc_chunk_render_object();
        CapturingVertexBuffer capturingVertexBuffer = new CapturingVertexBuffer(chunkPos);
        BlockFluidRenderer fluidRenderer = blockRendererDispatcher.getFluidRenderer();
        BlockRendererDispatcher blockrendererdispatcherm = Minecraft.getMinecraft().getBlockRendererDispatcher();

        int blockIndexCounter = 0;
        for(BlockPos blockPos : positions) {
            IBlockState blockState = world.getBlockState(blockPos);
            blockState = blockState.getActualState(world, blockPos);
            Block block = blockState.getBlock();

            if(blockState.getRenderType() == EnumBlockRenderType.MODEL) {

                IBakedModel blockModel = blockrendererdispatcherm.getModelForState(blockState);
              //  IBakedModel ibakedmodel = blockRendererDispatcherm.getModelForState(blockState);//this.blockModelShapes.getModelForState(state);
              //  IBakedModel ibakedmodel1 = (new SimpleBakedModel.Builder(blockState, ibakedmodel, texture, BlockPos)).makeBakedModel();

                int colorMultiplier = blockColors.colorMultiplier(blockState, null, null, 0);

                List<EnumFacing> actuallyAllValuesOfEnumFacing = new ArrayList<>();
                Collections.addAll(actuallyAllValuesOfEnumFacing, EnumFacing.values());

                // FUCK YOU NULL
                // AND FUCK WHOEVER DECIDED THAT NULL WAS A MEMBER OF ENUMFACING
                actuallyAllValuesOfEnumFacing.add(null);

                int faceIndexCounter = 0;
                for(EnumFacing facing : actuallyAllValuesOfEnumFacing) {
                    List<BakedQuad> quads = blockModel.getQuads(blockState, facing, 0);
                    boolean shouldSideBeRendered = true;
                    if(facing != null) {
                        // When Nova explodes and I get invited to Sweden to visit Mojang, the absolute first thing I'm
                        // going to do it find whoever decided to use `null` rather than ADDING ANOTHER FUCKING ENUM
                        // VALUE and I'm going to make them regret everything they've ever done
                        shouldSideBeRendered = blockState.shouldSideBeRendered(world, blockPos, facing);
                    }
                    boolean hasQuads = !quads.isEmpty();
                    if(shouldSideBeRendered && hasQuads) {
                        int lmCoords;
                        if(facing == null) {
                            // This logic would be reasonable to write and simple to maintain IF THEY HAD JUST ADDED
                            // ANOTHER FUCKING VALUE TO THEIR STUPID FUCKING ENUM
                            lmCoords = blockState.getPackedLightmapCoords(world, blockPos.offset(EnumFacing.UP));
                        } else {
                            lmCoords = blockState.getPackedLightmapCoords(world, blockPos.offset(facing));
                        }

                        for(BakedQuad quad : quads) {
                            if(quad.hasTintIndex()) {
                                colorMultiplier = blockColors.colorMultiplier(blockState, world, blockPos, quad.getTintIndex());
                            }
                            int[] quadVertexData = addPosition(quad, blockPos.subtract(chunkPos));
                            setVertexColor(quadVertexData, colorMultiplier);
                            setLightmapCoord(quadVertexData, lmCoords);

                            for(int data : quadVertexData) {
                                vertexData.add(data);
                            }

                            indices.addIndicesForFace(faceIndexCounter, blockIndexCounter);
                            faceIndexCounter += 4;
                        }
                    }
                }

                blockIndexCounter += faceIndexCounter;

            } else if(blockState.getRenderType() == EnumBlockRenderType.LIQUID) {
                // Why do liquids have to be different? :(
                fluidRenderer.renderFluid(world, blockState, blockPos, capturingVertexBuffer);
            }
        }

        vertexData.addAll(capturingVertexBuffer.getData());

        int offset = indices.size();
        for(int i = 0; i < capturingVertexBuffer.getVertexCount() / 4; i++) {
            indices.addIndicesForFace(offset, 0);

            offset += 4;
        }

        if(vertexData.isEmpty()) {
            return Optional.empty();
        }

        chunk_render_object.setVertex_data(vertexData);
        chunk_render_object.setIndices(indices);
        chunk_render_object.format = NovaNative.NovaVertexFormat.POS_UV_LIGHTMAPUV_NORMAL_TANGENT.ordinal();

        return Optional.of(chunk_render_object);
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
