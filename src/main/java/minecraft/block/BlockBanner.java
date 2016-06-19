package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyDirection;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityBanner;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockBanner extends BlockContainer
{
    public static final PropertyDirection FACING = BlockHorizontal.FACING;
    public static final PropertyInteger ROTATION = PropertyInteger.create("rotation", 0, 15);
    protected static final AxisAlignedBB field_185550_c = new AxisAlignedBB(0.25D, 0.0D, 0.25D, 0.75D, 1.0D, 0.75D);

    protected BlockBanner()
    {
        super(Material.wood);
    }

    /**
     * Gets the localized name of this block. Used for the statistics page.
     */
    public String getLocalizedName()
    {
        return I18n.translateToLocal("item.banner.white.name");
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return NULL_AABB;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return true;
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    /**
     * Return true if an entity can be spawned inside the block (used to get the player's bed spawn location)
     */
    public boolean canSpawnInBlock()
    {
        return true;
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        return new TileEntityBanner();
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.banner;
    }

    private ItemStack func_185549_e(World p_185549_1_, BlockPos p_185549_2_, IBlockState p_185549_3_)
    {
        TileEntity tileentity = p_185549_1_.getTileEntity(p_185549_2_);

        if (tileentity instanceof TileEntityBanner)
        {
            ItemStack itemstack = new ItemStack(Items.banner, 1, ((TileEntityBanner)tileentity).getBaseColor());
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            tileentity.writeToNBT(nbttagcompound);
            nbttagcompound.removeTag("x");
            nbttagcompound.removeTag("y");
            nbttagcompound.removeTag("z");
            nbttagcompound.removeTag("id");
            itemstack.setTagInfo("BlockEntityTag", nbttagcompound);
            return itemstack;
        }
        else
        {
            return null;
        }
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        ItemStack itemstack = this.func_185549_e(worldIn, pos, state);
        return itemstack != null ? itemstack : new ItemStack(Items.banner);
    }

    /**
     * Spawns this Block's drops into the World as EntityItems.
     */
    public void dropBlockAsItemWithChance(World worldIn, BlockPos pos, IBlockState state, float chance, int fortune)
    {
        ItemStack itemstack = this.func_185549_e(worldIn, pos, state);

        if (itemstack != null)
        {
            spawnAsEntity(worldIn, pos, itemstack);
        }
        else
        {
            super.dropBlockAsItemWithChance(worldIn, pos, state, chance, fortune);
        }
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return !this.hasInvalidNeighbor(worldIn, pos) && super.canPlaceBlockAt(worldIn, pos);
    }

    public void harvestBlock(World worldIn, EntityPlayer player, BlockPos pos, IBlockState state, TileEntity te, ItemStack p_180657_6_)
    {
        if (te instanceof TileEntityBanner)
        {
            TileEntityBanner tileentitybanner = (TileEntityBanner)te;
            ItemStack itemstack = new ItemStack(Items.banner, 1, ((TileEntityBanner)te).getBaseColor());
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            TileEntityBanner.setBaseColorAndPatterns(nbttagcompound, tileentitybanner.getBaseColor(), tileentitybanner.getPatterns());
            itemstack.setTagInfo("BlockEntityTag", nbttagcompound);
            spawnAsEntity(worldIn, pos, itemstack);
        }
        else
        {
            super.harvestBlock(worldIn, player, pos, state, (TileEntity)null, p_180657_6_);
        }
    }

    public static class BlockBannerHanging extends BlockBanner
    {
        protected static final AxisAlignedBB field_185551_d = new AxisAlignedBB(0.0D, 0.0D, 0.875D, 1.0D, 0.78125D, 1.0D);
        protected static final AxisAlignedBB field_185552_e = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.78125D, 0.125D);
        protected static final AxisAlignedBB field_185553_f = new AxisAlignedBB(0.875D, 0.0D, 0.0D, 1.0D, 0.78125D, 1.0D);
        protected static final AxisAlignedBB field_185554_g = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.125D, 0.78125D, 1.0D);

        public BlockBannerHanging()
        {
            this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH));
        }

        public IBlockState withRotation(IBlockState state, Rotation rot)
        {
            return state.withProperty(FACING, rot.func_185831_a((EnumFacing)state.getValue(FACING)));
        }

        public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
        {
            return state.withRotation(mirrorIn.func_185800_a((EnumFacing)state.getValue(FACING)));
        }

        public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
        {
            switch ((EnumFacing)state.getValue(FACING))
            {
                case NORTH:
                default:
                    return field_185551_d;

                case SOUTH:
                    return field_185552_e;

                case WEST:
                    return field_185553_f;

                case EAST:
                    return field_185554_g;
            }
        }

        public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
        {
            EnumFacing enumfacing = (EnumFacing)state.getValue(FACING);

            if (!worldIn.getBlockState(pos.offset(enumfacing.getOpposite())).getMaterial().isSolid())
            {
                this.dropBlockAsItem(worldIn, pos, state, 0);
                worldIn.setBlockToAir(pos);
            }

            super.onNeighborBlockChange(worldIn, pos, state, neighborBlock);
        }

        public IBlockState getStateFromMeta(int meta)
        {
            EnumFacing enumfacing = EnumFacing.getFront(meta);

            if (enumfacing.getAxis() == EnumFacing.Axis.Y)
            {
                enumfacing = EnumFacing.NORTH;
            }

            return this.getDefaultState().withProperty(FACING, enumfacing);
        }

        public int getMetaFromState(IBlockState state)
        {
            return ((EnumFacing)state.getValue(FACING)).getIndex();
        }

        protected BlockStateContainer createBlockState()
        {
            return new BlockStateContainer(this, new IProperty[] {FACING});
        }
    }

    public static class BlockBannerStanding extends BlockBanner
    {
        public BlockBannerStanding()
        {
            this.setDefaultState(this.blockState.getBaseState().withProperty(ROTATION, Integer.valueOf(0)));
        }

        public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
        {
            return field_185550_c;
        }

        public IBlockState withRotation(IBlockState state, Rotation rot)
        {
            return state.withProperty(ROTATION, Integer.valueOf(rot.func_185833_a(((Integer)state.getValue(ROTATION)).intValue(), 16)));
        }

        public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
        {
            return state.withProperty(ROTATION, Integer.valueOf(mirrorIn.func_185802_a(((Integer)state.getValue(ROTATION)).intValue(), 16)));
        }

        public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
        {
            if (!worldIn.getBlockState(pos.down()).getMaterial().isSolid())
            {
                this.dropBlockAsItem(worldIn, pos, state, 0);
                worldIn.setBlockToAir(pos);
            }

            super.onNeighborBlockChange(worldIn, pos, state, neighborBlock);
        }

        public IBlockState getStateFromMeta(int meta)
        {
            return this.getDefaultState().withProperty(ROTATION, Integer.valueOf(meta));
        }

        public int getMetaFromState(IBlockState state)
        {
            return ((Integer)state.getValue(ROTATION)).intValue();
        }

        protected BlockStateContainer createBlockState()
        {
            return new BlockStateContainer(this, new IProperty[] {ROTATION});
        }
    }
}
