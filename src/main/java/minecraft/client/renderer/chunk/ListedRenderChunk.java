package net.minecraft.client.renderer.chunk;

import net.minecraft.client.renderer.GLAllocation;
import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ListedRenderChunk extends RenderChunk
{
    private final int baseDisplayList = GLAllocation.generateDisplayLists(BlockRenderLayer.values().length);

    public ListedRenderChunk(World worldIn, RenderGlobal renderGlobalIn, BlockPos pos, int indexIn)
    {
        super(worldIn, renderGlobalIn, pos, indexIn);
    }

    public int getDisplayList(BlockRenderLayer layer, CompiledChunk p_178600_2_)
    {
        return !p_178600_2_.isLayerEmpty(layer) ? this.baseDisplayList + layer.ordinal() : -1;
    }

    public void deleteGlResources()
    {
        super.deleteGlResources();
        GLAllocation.deleteDisplayLists(this.baseDisplayList, BlockRenderLayer.values().length);
    }
}
