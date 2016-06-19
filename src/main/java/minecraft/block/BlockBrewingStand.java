package net.minecraft.block;

import java.util.List;
import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.InventoryHelper;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityBrewingStand;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockBrewingStand extends BlockContainer
{
    public static final PropertyBool[] HAS_BOTTLE = new PropertyBool[] {PropertyBool.create("has_bottle_0"), PropertyBool.create("has_bottle_1"), PropertyBool.create("has_bottle_2")};
    protected static final AxisAlignedBB field_185555_b = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.125D, 1.0D);
    protected static final AxisAlignedBB field_185556_c = new AxisAlignedBB(0.4375D, 0.0D, 0.4375D, 0.5625D, 0.875D, 0.5625D);

    public BlockBrewingStand()
    {
        super(Material.iron);
        this.setDefaultState(this.blockState.getBaseState().withProperty(HAS_BOTTLE[0], Boolean.valueOf(false)).withProperty(HAS_BOTTLE[1], Boolean.valueOf(false)).withProperty(HAS_BOTTLE[2], Boolean.valueOf(false)));
    }

    /**
     * Gets the localized name of this block. Used for the statistics page.
     */
    public String getLocalizedName()
    {
        return I18n.translateToLocal("item.brewingStand.name");
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    /**
     * The type of render function called. 3 for standard block models, 2 for TESR's, 1 for liquids, -1 is no render
     */
    public EnumBlockRenderType getRenderType(IBlockState state)
    {
        return EnumBlockRenderType.MODEL;
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        return new TileEntityBrewingStand();
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185556_c);
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185555_b);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185555_b;
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (worldIn.isRemote)
        {
            return true;
        }
        else
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityBrewingStand)
            {
                playerIn.displayGUIChest((TileEntityBrewingStand)tileentity);
                playerIn.triggerAchievement(StatList.field_188081_O);
            }

            return true;
        }
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
        if (stack.hasDisplayName())
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityBrewingStand)
            {
                ((TileEntityBrewingStand)tileentity).setName(stack.getDisplayName());
            }
        }
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        double d0 = (double)((float)state.getX() + 0.4F + rand.nextFloat() * 0.2F);
        double d1 = (double)((float)state.getY() + 0.7F + rand.nextFloat() * 0.3F);
        double d2 = (double)((float)state.getZ() + 0.4F + rand.nextFloat() * 0.2F);
        pos.spawnParticle(EnumParticleTypes.SMOKE_NORMAL, d0, d1, d2, 0.0D, 0.0D, 0.0D, new int[0]);
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        TileEntity tileentity = worldIn.getTileEntity(pos);

        if (tileentity instanceof TileEntityBrewingStand)
        {
            InventoryHelper.dropInventoryItems(worldIn, pos, (TileEntityBrewingStand)tileentity);
        }

        super.breakBlock(worldIn, pos, state);
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.brewing_stand;
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return new ItemStack(Items.brewing_stand);
    }

    public boolean hasComparatorInputOverride(IBlockState state)
    {
        return true;
    }

    public int getComparatorInputOverride(IBlockState worldIn, World pos, BlockPos p_180641_3_)
    {
        return Container.calcRedstone(pos.getTileEntity(p_180641_3_));
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        IBlockState iblockstate = this.getDefaultState();

        for (int i = 0; i < 3; ++i)
        {
            iblockstate = iblockstate.withProperty(HAS_BOTTLE[i], Boolean.valueOf((meta & 1 << i) > 0));
        }

        return iblockstate;
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        int i = 0;

        for (int j = 0; j < 3; ++j)
        {
            if (((Boolean)state.getValue(HAS_BOTTLE[j])).booleanValue())
            {
                i |= 1 << j;
            }
        }

        return i;
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {HAS_BOTTLE[0], HAS_BOTTLE[1], HAS_BOTTLE[2]});
    }
}
