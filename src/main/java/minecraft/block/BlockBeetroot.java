package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockBeetroot extends BlockCrops
{
    public static final PropertyInteger field_185531_a = PropertyInteger.create("age", 0, 3);
    private static final AxisAlignedBB[] field_185532_d = new AxisAlignedBB[] {new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.125D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.25D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.375D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.5D, 1.0D)};

    protected PropertyInteger func_185524_e()
    {
        return field_185531_a;
    }

    public int func_185526_g()
    {
        return 3;
    }

    protected Item getSeed()
    {
        return Items.beetroot_seeds;
    }

    protected Item getCrop()
    {
        return Items.beetroot;
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (rand.nextInt(3) == 0)
        {
            this.checkAndDropBlock(worldIn, pos, state);
        }
        else
        {
            super.updateTick(worldIn, pos, state, rand);
        }
    }

    protected int func_185529_b(World p_185529_1_)
    {
        return super.func_185529_b(p_185529_1_) / 3;
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {field_185531_a});
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185532_d[((Integer)state.getValue(this.func_185524_e())).intValue()];
    }
}
