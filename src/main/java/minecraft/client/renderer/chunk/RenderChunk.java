package net.minecraft.client.renderer.chunk;

import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import java.nio.FloatBuffer;
import java.util.EnumMap;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.locks.ReentrantLock;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.client.renderer.BlockRendererDispatcher;
import net.minecraft.client.renderer.GLAllocation;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.OpenGlHelper;
import net.minecraft.client.renderer.RegionRenderCache;
import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.client.renderer.tileentity.TileEntityRendererDispatcher;
import net.minecraft.client.renderer.tileentity.TileEntitySpecialRenderer;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.client.renderer.vertex.VertexBuffer;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class RenderChunk
{
    private World world;
    private final RenderGlobal renderGlobal;
    public static int renderChunksUpdated;
    private BlockPos position;
    public CompiledChunk compiledChunk = CompiledChunk.DUMMY;
    private final ReentrantLock lockCompileTask = new ReentrantLock();
    private final ReentrantLock lockCompiledChunk = new ReentrantLock();
    private ChunkCompileTaskGenerator compileTask = null;
    private final Set<TileEntity> setTileEntities = Sets.<TileEntity>newHashSet();
    private final int index;
    private final FloatBuffer modelviewMatrix = GLAllocation.createDirectFloatBuffer(16);
    private final VertexBuffer[] vertexBuffers = new VertexBuffer[BlockRenderLayer.values().length];
    public AxisAlignedBB boundingBox;
    private int frameIndex = -1;
    private boolean needsUpdate = true;
    private EnumMap<EnumFacing, BlockPos> mapEnumFacing = Maps.newEnumMap(EnumFacing.class);
    private boolean field_188284_q = false;

    public RenderChunk(World worldIn, RenderGlobal renderGlobalIn, BlockPos blockPosIn, int indexIn)
    {
        this.world = worldIn;
        this.renderGlobal = renderGlobalIn;
        this.index = indexIn;

        if (!blockPosIn.equals(this.getPosition()))
        {
            this.setPosition(blockPosIn);
        }

        if (OpenGlHelper.useVbo())
        {
            for (int i = 0; i < BlockRenderLayer.values().length; ++i)
            {
                this.vertexBuffers[i] = new VertexBuffer(DefaultVertexFormats.BLOCK);
            }
        }
    }

    public boolean setFrameIndex(int frameIndexIn)
    {
        if (this.frameIndex == frameIndexIn)
        {
            return false;
        }
        else
        {
            this.frameIndex = frameIndexIn;
            return true;
        }
    }

    public VertexBuffer getVertexBufferByLayer(int layer)
    {
        return this.vertexBuffers[layer];
    }

    public void setPosition(BlockPos pos)
    {
        this.stopCompileTask();
        this.position = pos;
        this.boundingBox = new AxisAlignedBB(pos, pos.add(16, 16, 16));

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            this.mapEnumFacing.put(enumfacing, pos.offset(enumfacing, 16));
        }

        this.initModelviewMatrix();
    }

    public void resortTransparency(float x, float y, float z, ChunkCompileTaskGenerator generator)
    {
        CompiledChunk compiledchunk = generator.getCompiledChunk();

        if (compiledchunk.getState() != null && !compiledchunk.isLayerEmpty(BlockRenderLayer.TRANSLUCENT))
        {
            this.preRenderBlocks(generator.getRegionRenderCacheBuilder().getWorldRendererByLayer(BlockRenderLayer.TRANSLUCENT), this.position);
            generator.getRegionRenderCacheBuilder().getWorldRendererByLayer(BlockRenderLayer.TRANSLUCENT).setVertexState(compiledchunk.getState());
            this.postRenderBlocks(BlockRenderLayer.TRANSLUCENT, x, y, z, generator.getRegionRenderCacheBuilder().getWorldRendererByLayer(BlockRenderLayer.TRANSLUCENT), compiledchunk);
        }
    }

    public void rebuildChunk(float x, float y, float z, ChunkCompileTaskGenerator generator)
    {
        CompiledChunk compiledchunk = new CompiledChunk();
        int i = 1;
        BlockPos blockpos = this.position;
        BlockPos blockpos1 = blockpos.add(15, 15, 15);
        generator.getLock().lock();
        IBlockAccess iblockaccess;

        try
        {
            if (generator.getStatus() != ChunkCompileTaskGenerator.Status.COMPILING)
            {
                return;
            }

            iblockaccess = new RegionRenderCache(this.world, blockpos.add(-1, -1, -1), blockpos1.add(1, 1, 1), 1);
            generator.setCompiledChunk(compiledchunk);
        }
        finally
        {
            generator.getLock().unlock();
        }

        VisGraph lvt_10_1_ = new VisGraph();
        HashSet lvt_11_1_ = Sets.newHashSet();

        if (!iblockaccess.extendedLevelsInChunkCache())
        {
            ++renderChunksUpdated;
            boolean[] aboolean = new boolean[BlockRenderLayer.values().length];
            BlockRendererDispatcher blockrendererdispatcher = Minecraft.getMinecraft().getBlockRendererDispatcher();

            for (BlockPos.MutableBlockPos blockpos$mutableblockpos : BlockPos.getAllInBoxMutable(blockpos, blockpos1))
            {
                IBlockState iblockstate = iblockaccess.getBlockState(blockpos$mutableblockpos);
                Block block = iblockstate.getBlock();

                if (iblockstate.isOpaqueCube())
                {
                    lvt_10_1_.func_178606_a(blockpos$mutableblockpos);
                }

                if (block.hasTileEntity())
                {
                    TileEntity tileentity = iblockaccess.getTileEntity(new BlockPos(blockpos$mutableblockpos));
                    TileEntitySpecialRenderer<TileEntity> tileentityspecialrenderer = TileEntityRendererDispatcher.instance.<TileEntity>getSpecialRenderer(tileentity);

                    if (tileentity != null && tileentityspecialrenderer != null)
                    {
                        compiledchunk.addTileEntity(tileentity);

                        if (tileentityspecialrenderer.func_188185_a(tileentity))
                        {
                            lvt_11_1_.add(tileentity);
                        }
                    }
                }

                BlockRenderLayer blockrenderlayer1 = block.getBlockLayer();
                int j = blockrenderlayer1.ordinal();

                if (block.getDefaultState().getRenderType() != EnumBlockRenderType.INVISIBLE)
                {
                    net.minecraft.client.renderer.VertexBuffer vertexbuffer = generator.getRegionRenderCacheBuilder().getWorldRendererByLayerId(j);

                    if (!compiledchunk.isLayerStarted(blockrenderlayer1))
                    {
                        compiledchunk.setLayerStarted(blockrenderlayer1);
                        this.preRenderBlocks(vertexbuffer, blockpos);
                    }

                    aboolean[j] |= blockrendererdispatcher.renderBlock(iblockstate, blockpos$mutableblockpos, iblockaccess, vertexbuffer);
                }
            }

            for (BlockRenderLayer blockrenderlayer : BlockRenderLayer.values())
            {
                if (aboolean[blockrenderlayer.ordinal()])
                {
                    compiledchunk.setLayerUsed(blockrenderlayer);
                }

                if (compiledchunk.isLayerStarted(blockrenderlayer))
                {
                    this.postRenderBlocks(blockrenderlayer, x, y, z, generator.getRegionRenderCacheBuilder().getWorldRendererByLayer(blockrenderlayer), compiledchunk);
                }
            }
        }

        compiledchunk.setVisibility(lvt_10_1_.computeVisibility());
        this.lockCompileTask.lock();

        try
        {
            Set<TileEntity> set = Sets.newHashSet(lvt_11_1_);
            Set<TileEntity> set1 = Sets.newHashSet(this.setTileEntities);
            set.removeAll(this.setTileEntities);
            set1.removeAll(lvt_11_1_);
            this.setTileEntities.clear();
            this.setTileEntities.addAll(lvt_11_1_);
            this.renderGlobal.updateTileEntities(set1, set);
        }
        finally
        {
            this.lockCompileTask.unlock();
        }
    }

    protected void finishCompileTask()
    {
        this.lockCompileTask.lock();

        try
        {
            if (this.compileTask != null && this.compileTask.getStatus() != ChunkCompileTaskGenerator.Status.DONE)
            {
                this.compileTask.finish();
                this.compileTask = null;
            }
        }
        finally
        {
            this.lockCompileTask.unlock();
        }
    }

    public ReentrantLock getLockCompileTask()
    {
        return this.lockCompileTask;
    }

    public ChunkCompileTaskGenerator makeCompileTaskChunk()
    {
        this.lockCompileTask.lock();
        ChunkCompileTaskGenerator chunkcompiletaskgenerator;

        try
        {
            this.finishCompileTask();
            this.compileTask = new ChunkCompileTaskGenerator(this, ChunkCompileTaskGenerator.Type.REBUILD_CHUNK, this.func_188280_f());
            chunkcompiletaskgenerator = this.compileTask;
        }
        finally
        {
            this.lockCompileTask.unlock();
        }

        return chunkcompiletaskgenerator;
    }

    public ChunkCompileTaskGenerator makeCompileTaskTransparency()
    {
        this.lockCompileTask.lock();
        ChunkCompileTaskGenerator chunkcompiletaskgenerator;

        try
        {
            if (this.compileTask == null || this.compileTask.getStatus() != ChunkCompileTaskGenerator.Status.PENDING)
            {
                if (this.compileTask != null && this.compileTask.getStatus() != ChunkCompileTaskGenerator.Status.DONE)
                {
                    this.compileTask.finish();
                    this.compileTask = null;
                }

                this.compileTask = new ChunkCompileTaskGenerator(this, ChunkCompileTaskGenerator.Type.RESORT_TRANSPARENCY, this.func_188280_f());
                this.compileTask.setCompiledChunk(this.compiledChunk);
                chunkcompiletaskgenerator = this.compileTask;
                return chunkcompiletaskgenerator;
            }

            chunkcompiletaskgenerator = null;
        }
        finally
        {
            this.lockCompileTask.unlock();
        }

        return chunkcompiletaskgenerator;
    }

    protected double func_188280_f()
    {
        EntityPlayerSP entityplayersp = Minecraft.getMinecraft().thePlayer;
        double d0 = this.boundingBox.minX + 8.0D - entityplayersp.posX;
        double d1 = this.boundingBox.minY + 8.0D - entityplayersp.posY;
        double d2 = this.boundingBox.minZ + 8.0D - entityplayersp.posZ;
        return d0 * d0 + d1 * d1 + d2 * d2;
    }

    private void preRenderBlocks(net.minecraft.client.renderer.VertexBuffer worldRendererIn, BlockPos pos)
    {
        worldRendererIn.begin(7, DefaultVertexFormats.BLOCK);
        worldRendererIn.setTranslation((double)(-pos.getX()), (double)(-pos.getY()), (double)(-pos.getZ()));
    }

    private void postRenderBlocks(BlockRenderLayer layer, float x, float y, float z, net.minecraft.client.renderer.VertexBuffer worldRendererIn, CompiledChunk compiledChunkIn)
    {
        if (layer == BlockRenderLayer.TRANSLUCENT && !compiledChunkIn.isLayerEmpty(layer))
        {
            worldRendererIn.sortVertexData(x, y, z);
            compiledChunkIn.setState(worldRendererIn.getVertexState());
        }

        worldRendererIn.finishDrawing();
    }

    private void initModelviewMatrix()
    {
        GlStateManager.pushMatrix();
        GlStateManager.loadIdentity();
        float f = 1.000001F;
        GlStateManager.translate(-8.0F, -8.0F, -8.0F);
        GlStateManager.scale(f, f, f);
        GlStateManager.translate(8.0F, 8.0F, 8.0F);
        GlStateManager.getFloat(2982, this.modelviewMatrix);
        GlStateManager.popMatrix();
    }

    public void multModelviewMatrix()
    {
        GlStateManager.multMatrix(this.modelviewMatrix);
    }

    public CompiledChunk getCompiledChunk()
    {
        return this.compiledChunk;
    }

    public void setCompiledChunk(CompiledChunk compiledChunkIn)
    {
        this.lockCompiledChunk.lock();

        try
        {
            this.compiledChunk = compiledChunkIn;
        }
        finally
        {
            this.lockCompiledChunk.unlock();
        }
    }

    public void stopCompileTask()
    {
        this.finishCompileTask();
        this.compiledChunk = CompiledChunk.DUMMY;
    }

    public void deleteGlResources()
    {
        this.stopCompileTask();
        this.world = null;

        for (int i = 0; i < BlockRenderLayer.values().length; ++i)
        {
            if (this.vertexBuffers[i] != null)
            {
                this.vertexBuffers[i].deleteGlBuffers();
            }
        }
    }

    public BlockPos getPosition()
    {
        return this.position;
    }

    public void setNeedsUpdate(boolean needsUpdateIn)
    {
        if (this.needsUpdate)
        {
            needsUpdateIn |= this.field_188284_q;
        }

        this.needsUpdate = true;
        this.field_188284_q = needsUpdateIn;
    }

    public void func_188282_m()
    {
        this.needsUpdate = false;
        this.field_188284_q = false;
    }

    public boolean isNeedsUpdate()
    {
        return this.needsUpdate;
    }

    public boolean func_188281_o()
    {
        return this.needsUpdate && this.field_188284_q;
    }

    public BlockPos getBlockPosOffset16(EnumFacing p_181701_1_)
    {
        return (BlockPos)this.mapEnumFacing.get(p_181701_1_);
    }

    public World func_188283_p()
    {
        return this.world;
    }
}
