package net.minecraft.block;

import com.google.common.cache.LoadingCache;
import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyEnum;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.BlockWorldState;
import net.minecraft.block.state.IBlockState;
import net.minecraft.block.state.pattern.BlockPattern;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityList;
import net.minecraft.entity.monster.EntityPigZombie;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemMonsterPlacer;
import net.minecraft.item.ItemStack;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.Rotation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockPortal extends BlockBreakable
{
    public static final PropertyEnum<EnumFacing.Axis> AXIS = PropertyEnum.<EnumFacing.Axis>create("axis", EnumFacing.Axis.class, new EnumFacing.Axis[] {EnumFacing.Axis.X, EnumFacing.Axis.Z});
    protected static final AxisAlignedBB field_185683_b = new AxisAlignedBB(0.0D, 0.0D, 0.375D, 1.0D, 1.0D, 0.625D);
    protected static final AxisAlignedBB field_185684_c = new AxisAlignedBB(0.375D, 0.0D, 0.0D, 0.625D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185685_d = new AxisAlignedBB(0.375D, 0.0D, 0.375D, 0.625D, 1.0D, 0.625D);

    public BlockPortal()
    {
        super(Material.portal, false);
        this.setDefaultState(this.blockState.getBaseState().withProperty(AXIS, EnumFacing.Axis.X));
        this.setTickRandomly(true);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        switch ((EnumFacing.Axis)state.getValue(AXIS))
        {
            case X:
                return field_185683_b;

            case Y:
            default:
                return field_185685_d;

            case Z:
                return field_185684_c;
        }
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        super.updateTick(worldIn, pos, state, rand);

        if (worldIn.provider.isSurfaceWorld() && worldIn.getGameRules().getBoolean("doMobSpawning") && rand.nextInt(2000) < worldIn.getDifficulty().getDifficultyId())
        {
            int i = pos.getY();
            BlockPos blockpos;

            for (blockpos = pos; !worldIn.getBlockState(blockpos).func_185896_q() && blockpos.getY() > 0; blockpos = blockpos.down())
            {
                ;
            }

            if (i > 0 && !worldIn.getBlockState(blockpos.up()).isNormalCube())
            {
                Entity entity = ItemMonsterPlacer.spawnCreature(worldIn, EntityList.func_188430_a(EntityPigZombie.class), (double)blockpos.getX() + 0.5D, (double)blockpos.getY() + 1.1D, (double)blockpos.getZ() + 0.5D);

                if (entity != null)
                {
                    entity.timeUntilPortal = entity.getPortalCooldown();
                }
            }
        }
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return NULL_AABB;
    }

    public static int getMetaForAxis(EnumFacing.Axis axis)
    {
        return axis == EnumFacing.Axis.X ? 1 : (axis == EnumFacing.Axis.Z ? 2 : 0);
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public boolean func_176548_d(World worldIn, BlockPos p_176548_2_)
    {
        BlockPortal.Size blockportal$size = new BlockPortal.Size(worldIn, p_176548_2_, EnumFacing.Axis.X);

        if (blockportal$size.func_150860_b() && blockportal$size.field_150864_e == 0)
        {
            blockportal$size.func_150859_c();
            return true;
        }
        else
        {
            BlockPortal.Size blockportal$size1 = new BlockPortal.Size(worldIn, p_176548_2_, EnumFacing.Axis.Z);

            if (blockportal$size1.func_150860_b() && blockportal$size1.field_150864_e == 0)
            {
                blockportal$size1.func_150859_c();
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        EnumFacing.Axis enumfacing$axis = (EnumFacing.Axis)state.getValue(AXIS);

        if (enumfacing$axis == EnumFacing.Axis.X)
        {
            BlockPortal.Size blockportal$size = new BlockPortal.Size(worldIn, pos, EnumFacing.Axis.X);

            if (!blockportal$size.func_150860_b() || blockportal$size.field_150864_e < blockportal$size.field_150868_h * blockportal$size.field_150862_g)
            {
                worldIn.setBlockState(pos, Blocks.air.getDefaultState());
            }
        }
        else if (enumfacing$axis == EnumFacing.Axis.Z)
        {
            BlockPortal.Size blockportal$size1 = new BlockPortal.Size(worldIn, pos, EnumFacing.Axis.Z);

            if (!blockportal$size1.func_150860_b() || blockportal$size1.field_150864_e < blockportal$size1.field_150868_h * blockportal$size1.field_150862_g)
            {
                worldIn.setBlockState(pos, Blocks.air.getDefaultState());
            }
        }
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        side = side.offset(facing);
        EnumFacing.Axis enumfacing$axis = null;

        if (worldIn.getBlock() == this)
        {
            enumfacing$axis = (EnumFacing.Axis)worldIn.getValue(AXIS);

            if (enumfacing$axis == null)
            {
                return false;
            }

            if (enumfacing$axis == EnumFacing.Axis.Z && facing != EnumFacing.EAST && facing != EnumFacing.WEST)
            {
                return false;
            }

            if (enumfacing$axis == EnumFacing.Axis.X && facing != EnumFacing.SOUTH && facing != EnumFacing.NORTH)
            {
                return false;
            }
        }

        boolean flag = pos.getBlockState(side.west()).getBlock() == this && pos.getBlockState(side.west(2)).getBlock() != this;
        boolean flag1 = pos.getBlockState(side.east()).getBlock() == this && pos.getBlockState(side.east(2)).getBlock() != this;
        boolean flag2 = pos.getBlockState(side.north()).getBlock() == this && pos.getBlockState(side.north(2)).getBlock() != this;
        boolean flag3 = pos.getBlockState(side.south()).getBlock() == this && pos.getBlockState(side.south(2)).getBlock() != this;
        boolean flag4 = flag || flag1 || enumfacing$axis == EnumFacing.Axis.X;
        boolean flag5 = flag2 || flag3 || enumfacing$axis == EnumFacing.Axis.Z;
        return flag4 && facing == EnumFacing.WEST ? true : (flag4 && facing == EnumFacing.EAST ? true : (flag5 && facing == EnumFacing.NORTH ? true : flag5 && facing == EnumFacing.SOUTH));
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 0;
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.TRANSLUCENT;
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        if (!entityIn.isRiding() && !entityIn.isBeingRidden() && entityIn.func_184222_aU())
        {
            entityIn.setPortal(pos);
        }
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        if (rand.nextInt(100) == 0)
        {
            pos.func_184134_a((double)state.getX() + 0.5D, (double)state.getY() + 0.5D, (double)state.getZ() + 0.5D, SoundEvents.block_portal_ambient, SoundCategory.BLOCKS, 0.5F, rand.nextFloat() * 0.4F + 0.8F, false);
        }

        for (int i = 0; i < 4; ++i)
        {
            double d0 = (double)((float)state.getX() + rand.nextFloat());
            double d1 = (double)((float)state.getY() + rand.nextFloat());
            double d2 = (double)((float)state.getZ() + rand.nextFloat());
            double d3 = ((double)rand.nextFloat() - 0.5D) * 0.5D;
            double d4 = ((double)rand.nextFloat() - 0.5D) * 0.5D;
            double d5 = ((double)rand.nextFloat() - 0.5D) * 0.5D;
            int j = rand.nextInt(2) * 2 - 1;

            if (pos.getBlockState(state.west()).getBlock() != this && pos.getBlockState(state.east()).getBlock() != this)
            {
                d0 = (double)state.getX() + 0.5D + 0.25D * (double)j;
                d3 = (double)(rand.nextFloat() * 2.0F * (float)j);
            }
            else
            {
                d2 = (double)state.getZ() + 0.5D + 0.25D * (double)j;
                d5 = (double)(rand.nextFloat() * 2.0F * (float)j);
            }

            pos.spawnParticle(EnumParticleTypes.PORTAL, d0, d1, d2, d3, d4, d5, new int[0]);
        }
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return null;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(AXIS, (meta & 3) == 2 ? EnumFacing.Axis.Z : EnumFacing.Axis.X);
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return getMetaForAxis((EnumFacing.Axis)state.getValue(AXIS));
    }

    /**
     * Returns the blockstate with the given rotation from the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withRotation(IBlockState state, Rotation rot)
    {
        switch (rot)
        {
            case COUNTERCLOCKWISE_90:
            case CLOCKWISE_90:
                switch ((EnumFacing.Axis)state.getValue(AXIS))
                {
                    case X:
                        return state.withProperty(AXIS, EnumFacing.Axis.Z);

                    case Z:
                        return state.withProperty(AXIS, EnumFacing.Axis.X);

                    default:
                        return state;
                }

            default:
                return state;
        }
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {AXIS});
    }

    public BlockPattern.PatternHelper func_181089_f(World p_181089_1_, BlockPos p_181089_2_)
    {
        EnumFacing.Axis enumfacing$axis = EnumFacing.Axis.Z;
        BlockPortal.Size blockportal$size = new BlockPortal.Size(p_181089_1_, p_181089_2_, EnumFacing.Axis.X);
        LoadingCache<BlockPos, BlockWorldState> loadingcache = BlockPattern.func_181627_a(p_181089_1_, true);

        if (!blockportal$size.func_150860_b())
        {
            enumfacing$axis = EnumFacing.Axis.X;
            blockportal$size = new BlockPortal.Size(p_181089_1_, p_181089_2_, EnumFacing.Axis.Z);
        }

        if (!blockportal$size.func_150860_b())
        {
            return new BlockPattern.PatternHelper(p_181089_2_, EnumFacing.NORTH, EnumFacing.UP, loadingcache, 1, 1, 1);
        }
        else
        {
            int[] aint = new int[EnumFacing.AxisDirection.values().length];
            EnumFacing enumfacing = blockportal$size.field_150866_c.rotateYCCW();
            BlockPos blockpos = blockportal$size.field_150861_f.up(blockportal$size.func_181100_a() - 1);

            for (EnumFacing.AxisDirection enumfacing$axisdirection : EnumFacing.AxisDirection.values())
            {
                BlockPattern.PatternHelper blockpattern$patternhelper = new BlockPattern.PatternHelper(enumfacing.getAxisDirection() == enumfacing$axisdirection ? blockpos : blockpos.offset(blockportal$size.field_150866_c, blockportal$size.func_181101_b() - 1), EnumFacing.getFacingFromAxis(enumfacing$axisdirection, enumfacing$axis), EnumFacing.UP, loadingcache, blockportal$size.func_181101_b(), blockportal$size.func_181100_a(), 1);

                for (int i = 0; i < blockportal$size.func_181101_b(); ++i)
                {
                    for (int j = 0; j < blockportal$size.func_181100_a(); ++j)
                    {
                        BlockWorldState blockworldstate = blockpattern$patternhelper.translateOffset(i, j, 1);

                        if (blockworldstate.getBlockState() != null && blockworldstate.getBlockState().getMaterial() != Material.air)
                        {
                            ++aint[enumfacing$axisdirection.ordinal()];
                        }
                    }
                }
            }

            EnumFacing.AxisDirection enumfacing$axisdirection1 = EnumFacing.AxisDirection.POSITIVE;

            for (EnumFacing.AxisDirection enumfacing$axisdirection2 : EnumFacing.AxisDirection.values())
            {
                if (aint[enumfacing$axisdirection2.ordinal()] < aint[enumfacing$axisdirection1.ordinal()])
                {
                    enumfacing$axisdirection1 = enumfacing$axisdirection2;
                }
            }

            return new BlockPattern.PatternHelper(enumfacing.getAxisDirection() == enumfacing$axisdirection1 ? blockpos : blockpos.offset(blockportal$size.field_150866_c, blockportal$size.func_181101_b() - 1), EnumFacing.getFacingFromAxis(enumfacing$axisdirection1, enumfacing$axis), EnumFacing.UP, loadingcache, blockportal$size.func_181101_b(), blockportal$size.func_181100_a(), 1);
        }
    }

    public static class Size
    {
        private final World world;
        private final EnumFacing.Axis axis;
        private final EnumFacing field_150866_c;
        private final EnumFacing field_150863_d;
        private int field_150864_e = 0;
        private BlockPos field_150861_f;
        private int field_150862_g;
        private int field_150868_h;

        public Size(World worldIn, BlockPos p_i45694_2_, EnumFacing.Axis p_i45694_3_)
        {
            this.world = worldIn;
            this.axis = p_i45694_3_;

            if (p_i45694_3_ == EnumFacing.Axis.X)
            {
                this.field_150863_d = EnumFacing.EAST;
                this.field_150866_c = EnumFacing.WEST;
            }
            else
            {
                this.field_150863_d = EnumFacing.NORTH;
                this.field_150866_c = EnumFacing.SOUTH;
            }

            for (BlockPos blockpos = p_i45694_2_; p_i45694_2_.getY() > blockpos.getY() - 21 && p_i45694_2_.getY() > 0 && this.func_150857_a(worldIn.getBlockState(p_i45694_2_.down()).getBlock()); p_i45694_2_ = p_i45694_2_.down())
            {
                ;
            }

            int i = this.func_180120_a(p_i45694_2_, this.field_150863_d) - 1;

            if (i >= 0)
            {
                this.field_150861_f = p_i45694_2_.offset(this.field_150863_d, i);
                this.field_150868_h = this.func_180120_a(this.field_150861_f, this.field_150866_c);

                if (this.field_150868_h < 2 || this.field_150868_h > 21)
                {
                    this.field_150861_f = null;
                    this.field_150868_h = 0;
                }
            }

            if (this.field_150861_f != null)
            {
                this.field_150862_g = this.func_150858_a();
            }
        }

        protected int func_180120_a(BlockPos p_180120_1_, EnumFacing p_180120_2_)
        {
            int i;

            for (i = 0; i < 22; ++i)
            {
                BlockPos blockpos = p_180120_1_.offset(p_180120_2_, i);

                if (!this.func_150857_a(this.world.getBlockState(blockpos).getBlock()) || this.world.getBlockState(blockpos.down()).getBlock() != Blocks.obsidian)
                {
                    break;
                }
            }

            Block block = this.world.getBlockState(p_180120_1_.offset(p_180120_2_, i)).getBlock();
            return block == Blocks.obsidian ? i : 0;
        }

        public int func_181100_a()
        {
            return this.field_150862_g;
        }

        public int func_181101_b()
        {
            return this.field_150868_h;
        }

        protected int func_150858_a()
        {
            label24:

            for (this.field_150862_g = 0; this.field_150862_g < 21; ++this.field_150862_g)
            {
                for (int i = 0; i < this.field_150868_h; ++i)
                {
                    BlockPos blockpos = this.field_150861_f.offset(this.field_150866_c, i).up(this.field_150862_g);
                    Block block = this.world.getBlockState(blockpos).getBlock();

                    if (!this.func_150857_a(block))
                    {
                        break label24;
                    }

                    if (block == Blocks.portal)
                    {
                        ++this.field_150864_e;
                    }

                    if (i == 0)
                    {
                        block = this.world.getBlockState(blockpos.offset(this.field_150863_d)).getBlock();

                        if (block != Blocks.obsidian)
                        {
                            break label24;
                        }
                    }
                    else if (i == this.field_150868_h - 1)
                    {
                        block = this.world.getBlockState(blockpos.offset(this.field_150866_c)).getBlock();

                        if (block != Blocks.obsidian)
                        {
                            break label24;
                        }
                    }
                }
            }

            for (int j = 0; j < this.field_150868_h; ++j)
            {
                if (this.world.getBlockState(this.field_150861_f.offset(this.field_150866_c, j).up(this.field_150862_g)).getBlock() != Blocks.obsidian)
                {
                    this.field_150862_g = 0;
                    break;
                }
            }

            if (this.field_150862_g <= 21 && this.field_150862_g >= 3)
            {
                return this.field_150862_g;
            }
            else
            {
                this.field_150861_f = null;
                this.field_150868_h = 0;
                this.field_150862_g = 0;
                return 0;
            }
        }

        protected boolean func_150857_a(Block p_150857_1_)
        {
            return p_150857_1_.blockMaterial == Material.air || p_150857_1_ == Blocks.fire || p_150857_1_ == Blocks.portal;
        }

        public boolean func_150860_b()
        {
            return this.field_150861_f != null && this.field_150868_h >= 2 && this.field_150868_h <= 21 && this.field_150862_g >= 3 && this.field_150862_g <= 21;
        }

        public void func_150859_c()
        {
            for (int i = 0; i < this.field_150868_h; ++i)
            {
                BlockPos blockpos = this.field_150861_f.offset(this.field_150866_c, i);

                for (int j = 0; j < this.field_150862_g; ++j)
                {
                    this.world.setBlockState(blockpos.up(j), Blocks.portal.getDefaultState().withProperty(BlockPortal.AXIS, this.axis), 2);
                }
            }
        }
    }
}
