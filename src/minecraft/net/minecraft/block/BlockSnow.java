package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.EnumSkyBlock;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockSnow extends Block
{
    public static final PropertyInteger LAYERS = PropertyInteger.create("layers", 1, 8);
    protected static final AxisAlignedBB[] field_185702_b = new AxisAlignedBB[] {new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.0D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.125D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.25D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.375D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.5D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.625D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.75D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.875D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D)};

    protected BlockSnow()
    {
        super(Material.snow);
        this.setDefaultState(this.blockState.getBaseState().withProperty(LAYERS, Integer.valueOf(1)));
        this.setTickRandomly(true);
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185702_b[((Integer)state.getValue(LAYERS)).intValue()];
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return ((Integer)worldIn.getBlockState(pos).getValue(LAYERS)).intValue() < 5;
    }

    /**
     * Checks if an IBlockState represents a block that is opaque and a full cube.
     *  
     * @param state The block state to check.
     */
    public boolean isFullyOpaque(IBlockState state)
    {
        return ((Integer)state.getValue(LAYERS)).intValue() == 7;
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        int i = ((Integer)worldIn.getValue(LAYERS)).intValue() - 1;
        float f = 0.125F;
        AxisAlignedBB axisalignedbb = worldIn.func_185900_c(pos, p_180646_3_);
        return new AxisAlignedBB(axisalignedbb.minX, axisalignedbb.minY, axisalignedbb.minZ, axisalignedbb.maxX, (double)((float)i * f), axisalignedbb.maxZ);
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos.down());
        Block block = iblockstate.getBlock();
        return block != Blocks.ice && block != Blocks.packed_ice ? (iblockstate.getMaterial() == Material.leaves ? true : (block == this && ((Integer)iblockstate.getValue(LAYERS)).intValue() >= 7 ? true : iblockstate.isOpaqueCube() && iblockstate.getMaterial().blocksMovement())) : false;
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        this.checkAndDropBlock(worldIn, pos, state);
    }

    private boolean checkAndDropBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        if (!this.canPlaceBlockAt(worldIn, pos))
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
            return false;
        }
        else
        {
            return true;
        }
    }

    public void harvestBlock(World worldIn, EntityPlayer player, BlockPos pos, IBlockState state, TileEntity te, ItemStack p_180657_6_)
    {
        spawnAsEntity(worldIn, pos, new ItemStack(Items.snowball, ((Integer)state.getValue(LAYERS)).intValue() + 1, 0));
        worldIn.setBlockToAir(pos);
        player.triggerAchievement(StatList.func_188055_a(this));
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.snowball;
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 0;
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (worldIn.getLightFor(EnumSkyBlock.BLOCK, pos) > 11)
        {
            this.dropBlockAsItem(worldIn, pos, worldIn.getBlockState(pos), 0);
            worldIn.setBlockToAir(pos);
        }
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        if (facing == EnumFacing.UP)
        {
            return true;
        }
        else
        {
            IBlockState iblockstate = pos.getBlockState(side.offset(facing));
            return iblockstate.getBlock() == this && ((Integer)iblockstate.getValue(LAYERS)).intValue() >= ((Integer)worldIn.getValue(LAYERS)).intValue() ? true : super.shouldSideBeRendered(worldIn, pos, side, facing);
        }
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(LAYERS, Integer.valueOf((meta & 7) + 1));
    }

    /**
     * Whether this Block can be replaced directly by other blocks (true for e.g. tall grass)
     */
    public boolean isReplaceable(IBlockAccess worldIn, BlockPos pos)
    {
        return ((Integer)worldIn.getBlockState(pos).getValue(LAYERS)).intValue() == 1;
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((Integer)state.getValue(LAYERS)).intValue() - 1;
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {LAYERS});
    }
}
