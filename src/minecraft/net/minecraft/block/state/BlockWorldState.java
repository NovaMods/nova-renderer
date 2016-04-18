package net.minecraft.block.state;

import com.google.common.base.Predicate;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class BlockWorldState
{
    private final World world;
    private final BlockPos pos;
    private final boolean field_181628_c;
    private IBlockState state;
    private TileEntity tileEntity;
    private boolean tileEntityInitialized;

    public BlockWorldState(World worldIn, BlockPos posIn, boolean p_i46451_3_)
    {
        this.world = worldIn;
        this.pos = posIn;
        this.field_181628_c = p_i46451_3_;
    }

    public IBlockState getBlockState()
    {
        if (this.state == null && (this.field_181628_c || this.world.isBlockLoaded(this.pos)))
        {
            this.state = this.world.getBlockState(this.pos);
        }

        return this.state;
    }

    public TileEntity getTileEntity()
    {
        if (this.tileEntity == null && !this.tileEntityInitialized)
        {
            this.tileEntity = this.world.getTileEntity(this.pos);
            this.tileEntityInitialized = true;
        }

        return this.tileEntity;
    }

    public BlockPos getPos()
    {
        return this.pos;
    }

    public static Predicate<BlockWorldState> hasState(final Predicate<IBlockState> predicatesIn)
    {
        return new Predicate<BlockWorldState>()
        {
            public boolean apply(BlockWorldState p_apply_1_)
            {
                return p_apply_1_ != null && predicatesIn.apply(p_apply_1_.getBlockState());
            }
        };
    }

    public static Predicate<BlockWorldState> func_185925_a(final IBlockState p_185925_0_)
    {
        return new Predicate<BlockWorldState>()
        {
            public boolean apply(BlockWorldState p_apply_1_)
            {
                return p_apply_1_ != null && p_apply_1_.getBlockState().equals(p_185925_0_);
            }
        };
    }
}
