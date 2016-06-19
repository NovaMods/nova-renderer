package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockCrops extends BlockBush implements IGrowable
{
    public static final PropertyInteger AGE = PropertyInteger.create("age", 0, 7);
    private static final AxisAlignedBB[] field_185530_a = new AxisAlignedBB[] {new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.125D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.25D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.375D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.5D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.625D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.75D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.875D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D)};

    protected BlockCrops()
    {
        this.setDefaultState(this.blockState.getBaseState().withProperty(this.func_185524_e(), Integer.valueOf(0)));
        this.setTickRandomly(true);
        this.setCreativeTab((CreativeTabs)null);
        this.setHardness(0.0F);
        this.setStepSound(SoundType.PLANT);
        this.disableStats();
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185530_a[((Integer)state.getValue(this.func_185524_e())).intValue()];
    }

    protected boolean func_185514_i(IBlockState p_185514_1_)
    {
        return p_185514_1_.getBlock() == Blocks.farmland;
    }

    protected PropertyInteger func_185524_e()
    {
        return AGE;
    }

    public int func_185526_g()
    {
        return 7;
    }

    protected int func_185527_x(IBlockState p_185527_1_)
    {
        return ((Integer)p_185527_1_.getValue(this.func_185524_e())).intValue();
    }

    public IBlockState func_185528_e(int p_185528_1_)
    {
        return this.getDefaultState().withProperty(this.func_185524_e(), Integer.valueOf(p_185528_1_));
    }

    public boolean func_185525_y(IBlockState p_185525_1_)
    {
        return ((Integer)p_185525_1_.getValue(this.func_185524_e())).intValue() >= this.func_185526_g();
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        super.updateTick(worldIn, pos, state, rand);

        if (worldIn.getLightFromNeighbors(pos.up()) >= 9)
        {
            int i = this.func_185527_x(state);

            if (i < this.func_185526_g())
            {
                float f = getGrowthChance(this, worldIn, pos);

                if (rand.nextInt((int)(25.0F / f) + 1) == 0)
                {
                    worldIn.setBlockState(pos, this.func_185528_e(i + 1), 2);
                }
            }
        }
    }

    public void grow(World worldIn, BlockPos pos, IBlockState state)
    {
        int i = this.func_185527_x(state) + this.func_185529_b(worldIn);
        int j = this.func_185526_g();

        if (i > j)
        {
            i = j;
        }

        worldIn.setBlockState(pos, this.func_185528_e(i), 2);
    }

    protected int func_185529_b(World p_185529_1_)
    {
        return MathHelper.getRandomIntegerInRange(p_185529_1_.rand, 2, 5);
    }

    protected static float getGrowthChance(Block blockIn, World worldIn, BlockPos pos)
    {
        float f = 1.0F;
        BlockPos blockpos = pos.down();

        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                float f1 = 0.0F;
                IBlockState iblockstate = worldIn.getBlockState(blockpos.add(i, 0, j));

                if (iblockstate.getBlock() == Blocks.farmland)
                {
                    f1 = 1.0F;

                    if (((Integer)iblockstate.getValue(BlockFarmland.MOISTURE)).intValue() > 0)
                    {
                        f1 = 3.0F;
                    }
                }

                if (i != 0 || j != 0)
                {
                    f1 /= 4.0F;
                }

                f += f1;
            }
        }

        BlockPos blockpos1 = pos.north();
        BlockPos blockpos2 = pos.south();
        BlockPos blockpos3 = pos.west();
        BlockPos blockpos4 = pos.east();
        boolean flag = blockIn == worldIn.getBlockState(blockpos3).getBlock() || blockIn == worldIn.getBlockState(blockpos4).getBlock();
        boolean flag1 = blockIn == worldIn.getBlockState(blockpos1).getBlock() || blockIn == worldIn.getBlockState(blockpos2).getBlock();

        if (flag && flag1)
        {
            f /= 2.0F;
        }
        else
        {
            boolean flag2 = blockIn == worldIn.getBlockState(blockpos3.north()).getBlock() || blockIn == worldIn.getBlockState(blockpos4.north()).getBlock() || blockIn == worldIn.getBlockState(blockpos4.south()).getBlock() || blockIn == worldIn.getBlockState(blockpos3.south()).getBlock();

            if (flag2)
            {
                f /= 2.0F;
            }
        }

        return f;
    }

    public boolean canBlockStay(World worldIn, BlockPos pos, IBlockState state)
    {
        return (worldIn.getLight(pos) >= 8 || worldIn.canSeeSky(pos)) && this.func_185514_i(worldIn.getBlockState(pos.down()));
    }

    protected Item getSeed()
    {
        return Items.wheat_seeds;
    }

    protected Item getCrop()
    {
        return Items.wheat;
    }

    /**
     * Spawns this Block's drops into the World as EntityItems.
     */
    public void dropBlockAsItemWithChance(World worldIn, BlockPos pos, IBlockState state, float chance, int fortune)
    {
        super.dropBlockAsItemWithChance(worldIn, pos, state, chance, 0);

        if (!worldIn.isRemote)
        {
            int i = this.func_185527_x(state);

            if (i >= this.func_185526_g())
            {
                int j = 3 + fortune;

                for (int k = 0; k < j; ++k)
                {
                    if (worldIn.rand.nextInt(2 * this.func_185526_g()) <= i)
                    {
                        spawnAsEntity(worldIn, pos, new ItemStack(this.getSeed()));
                    }
                }
            }
        }
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return this.func_185525_y(state) ? this.getCrop() : this.getSeed();
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return new ItemStack(this.getSeed());
    }

    /**
     * Whether this IGrowable can grow
     */
    public boolean canGrow(World worldIn, BlockPos pos, IBlockState state, boolean isClient)
    {
        return !this.func_185525_y(state);
    }

    public boolean canUseBonemeal(World worldIn, Random rand, BlockPos pos, IBlockState state)
    {
        return true;
    }

    public void grow(World worldIn, Random rand, BlockPos pos, IBlockState state)
    {
        this.grow(worldIn, pos, state);
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.func_185528_e(meta);
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return this.func_185527_x(state);
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {AGE});
    }
}
