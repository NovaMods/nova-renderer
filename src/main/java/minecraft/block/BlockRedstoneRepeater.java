package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockRedstoneRepeater extends BlockRedstoneDiode
{
    public static final PropertyBool LOCKED = PropertyBool.create("locked");
    public static final PropertyInteger DELAY = PropertyInteger.create("delay", 1, 4);

    protected BlockRedstoneRepeater(boolean powered)
    {
        super(powered);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(DELAY, Integer.valueOf(1)).withProperty(LOCKED, Boolean.valueOf(false)));
    }

    /**
     * Gets the localized name of this block. Used for the statistics page.
     */
    public String getLocalizedName()
    {
        return I18n.translateToLocal("item.diode.name");
    }

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        return state.withProperty(LOCKED, Boolean.valueOf(this.isLocked(worldIn, pos, state)));
    }

    /**
     * Returns the blockstate with the given rotation from the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withRotation(IBlockState state, Rotation rot)
    {
        return state.withProperty(FACING, rot.func_185831_a((EnumFacing)state.getValue(FACING)));
    }

    /**
     * Returns the blockstate with the given mirror of the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
    {
        return state.withRotation(mirrorIn.func_185800_a((EnumFacing)state.getValue(FACING)));
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (!playerIn.capabilities.allowEdit)
        {
            return false;
        }
        else
        {
            worldIn.setBlockState(pos, state.cycleProperty(DELAY), 3);
            return true;
        }
    }

    protected int getDelay(IBlockState state)
    {
        return ((Integer)state.getValue(DELAY)).intValue() * 2;
    }

    protected IBlockState getPoweredState(IBlockState unpoweredState)
    {
        Integer integer = (Integer)unpoweredState.getValue(DELAY);
        Boolean obool = (Boolean)unpoweredState.getValue(LOCKED);
        EnumFacing enumfacing = (EnumFacing)unpoweredState.getValue(FACING);
        return Blocks.powered_repeater.getDefaultState().withProperty(FACING, enumfacing).withProperty(DELAY, integer).withProperty(LOCKED, obool);
    }

    protected IBlockState getUnpoweredState(IBlockState poweredState)
    {
        Integer integer = (Integer)poweredState.getValue(DELAY);
        Boolean obool = (Boolean)poweredState.getValue(LOCKED);
        EnumFacing enumfacing = (EnumFacing)poweredState.getValue(FACING);
        return Blocks.unpowered_repeater.getDefaultState().withProperty(FACING, enumfacing).withProperty(DELAY, integer).withProperty(LOCKED, obool);
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.repeater;
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return new ItemStack(Items.repeater);
    }

    public boolean isLocked(IBlockAccess worldIn, BlockPos pos, IBlockState state)
    {
        return this.getPowerOnSides(worldIn, pos, state) > 0;
    }

    protected boolean func_185545_A(IBlockState p_185545_1_)
    {
        return func_185546_B(p_185545_1_);
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        if (this.isRepeaterPowered)
        {
            EnumFacing enumfacing = (EnumFacing)worldIn.getValue(FACING);
            double d0 = (double)((float)state.getX() + 0.5F) + (double)(rand.nextFloat() - 0.5F) * 0.2D;
            double d1 = (double)((float)state.getY() + 0.4F) + (double)(rand.nextFloat() - 0.5F) * 0.2D;
            double d2 = (double)((float)state.getZ() + 0.5F) + (double)(rand.nextFloat() - 0.5F) * 0.2D;
            float f = -5.0F;

            if (rand.nextBoolean())
            {
                f = (float)(((Integer)worldIn.getValue(DELAY)).intValue() * 2 - 1);
            }

            f = f / 16.0F;
            double d3 = (double)(f * (float)enumfacing.getFrontOffsetX());
            double d4 = (double)(f * (float)enumfacing.getFrontOffsetZ());
            pos.spawnParticle(EnumParticleTypes.REDSTONE, d0 + d3, d1, d2 + d4, 0.0D, 0.0D, 0.0D, new int[0]);
        }
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        super.breakBlock(worldIn, pos, state);
        this.notifyNeighbors(worldIn, pos, state);
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(FACING, EnumFacing.getHorizontal(meta)).withProperty(LOCKED, Boolean.valueOf(false)).withProperty(DELAY, Integer.valueOf(1 + (meta >> 2)));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;
        i = i | ((EnumFacing)state.getValue(FACING)).getHorizontalIndex();
        i = i | ((Integer)state.getValue(DELAY)).intValue() - 1 << 2;
        return i;
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {FACING, DELAY, LOCKED});
    }
}
