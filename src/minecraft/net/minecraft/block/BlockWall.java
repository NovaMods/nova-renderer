package net.minecraft.block;

import java.util.List;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyEnum;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.init.Blocks;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.IStringSerializable;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockWall extends Block
{
    public static final PropertyBool UP = PropertyBool.create("up");
    public static final PropertyBool NORTH = PropertyBool.create("north");
    public static final PropertyBool EAST = PropertyBool.create("east");
    public static final PropertyBool SOUTH = PropertyBool.create("south");
    public static final PropertyBool WEST = PropertyBool.create("west");
    public static final PropertyEnum<BlockWall.EnumType> VARIANT = PropertyEnum.<BlockWall.EnumType>create("variant", BlockWall.EnumType.class);
    protected static final AxisAlignedBB[] field_185751_g = new AxisAlignedBB[] {new AxisAlignedBB(0.25D, 0.0D, 0.25D, 0.75D, 1.0D, 0.75D), new AxisAlignedBB(0.25D, 0.0D, 0.25D, 0.75D, 1.0D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.25D, 0.75D, 1.0D, 0.75D), new AxisAlignedBB(0.0D, 0.0D, 0.25D, 0.75D, 1.0D, 1.0D), new AxisAlignedBB(0.25D, 0.0D, 0.0D, 0.75D, 1.0D, 0.75D), new AxisAlignedBB(0.3125D, 0.0D, 0.0D, 0.6875D, 0.875D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.75D, 1.0D, 0.75D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.75D, 1.0D, 1.0D), new AxisAlignedBB(0.25D, 0.0D, 0.25D, 1.0D, 1.0D, 0.75D), new AxisAlignedBB(0.25D, 0.0D, 0.25D, 1.0D, 1.0D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.3125D, 1.0D, 0.875D, 0.6875D), new AxisAlignedBB(0.0D, 0.0D, 0.25D, 1.0D, 1.0D, 1.0D), new AxisAlignedBB(0.25D, 0.0D, 0.0D, 1.0D, 1.0D, 0.75D), new AxisAlignedBB(0.25D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 0.75D), new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D)};
    protected static final AxisAlignedBB[] field_185750_B = new AxisAlignedBB[] {field_185751_g[0].func_186666_e(1.5D), field_185751_g[1].func_186666_e(1.5D), field_185751_g[2].func_186666_e(1.5D), field_185751_g[3].func_186666_e(1.5D), field_185751_g[4].func_186666_e(1.5D), field_185751_g[5].func_186666_e(1.5D), field_185751_g[6].func_186666_e(1.5D), field_185751_g[7].func_186666_e(1.5D), field_185751_g[8].func_186666_e(1.5D), field_185751_g[9].func_186666_e(1.5D), field_185751_g[10].func_186666_e(1.5D), field_185751_g[11].func_186666_e(1.5D), field_185751_g[12].func_186666_e(1.5D), field_185751_g[13].func_186666_e(1.5D), field_185751_g[14].func_186666_e(1.5D), field_185751_g[15].func_186666_e(1.5D)};

    public BlockWall(Block modelBlock)
    {
        super(modelBlock.blockMaterial);
        this.setDefaultState(this.blockState.getBaseState().withProperty(UP, Boolean.valueOf(false)).withProperty(NORTH, Boolean.valueOf(false)).withProperty(EAST, Boolean.valueOf(false)).withProperty(SOUTH, Boolean.valueOf(false)).withProperty(WEST, Boolean.valueOf(false)).withProperty(VARIANT, BlockWall.EnumType.NORMAL));
        this.setHardness(modelBlock.blockHardness);
        this.setResistance(modelBlock.blockResistance / 3.0F);
        this.setStepSound(modelBlock.stepSound);
        this.setCreativeTab(CreativeTabs.tabBlock);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        state = this.getActualState(state, source, pos);
        return field_185751_g[func_185749_i(state)];
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        worldIn = this.getActualState(worldIn, pos, p_180646_3_);
        return field_185750_B[func_185749_i(worldIn)];
    }

    private static int func_185749_i(IBlockState p_185749_0_)
    {
        int i = 0;

        if (((Boolean)p_185749_0_.getValue(NORTH)).booleanValue())
        {
            i |= 1 << EnumFacing.NORTH.getHorizontalIndex();
        }

        if (((Boolean)p_185749_0_.getValue(EAST)).booleanValue())
        {
            i |= 1 << EnumFacing.EAST.getHorizontalIndex();
        }

        if (((Boolean)p_185749_0_.getValue(SOUTH)).booleanValue())
        {
            i |= 1 << EnumFacing.SOUTH.getHorizontalIndex();
        }

        if (((Boolean)p_185749_0_.getValue(WEST)).booleanValue())
        {
            i |= 1 << EnumFacing.WEST.getHorizontalIndex();
        }

        return i;
    }

    /**
     * Gets the localized name of this block. Used for the statistics page.
     */
    public String getLocalizedName()
    {
        return I18n.translateToLocal(this.getUnlocalizedName() + "." + BlockWall.EnumType.NORMAL.getUnlocalizedName() + ".name");
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return false;
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    private boolean canConnectTo(IBlockAccess worldIn, BlockPos pos)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos);
        Block block = iblockstate.getBlock();
        return block == Blocks.barrier ? false : (block != this && !(block instanceof BlockFenceGate) ? (block.blockMaterial.isOpaque() && iblockstate.isFullCube() ? block.blockMaterial != Material.gourd : false) : true);
    }

    /**
     * returns a list of blocks with the same ID, but different meta (eg: wood returns 4 blocks)
     */
    public void getSubBlocks(Item itemIn, CreativeTabs tab, List<ItemStack> list)
    {
        for (BlockWall.EnumType blockwall$enumtype : BlockWall.EnumType.values())
        {
            list.add(new ItemStack(itemIn, 1, blockwall$enumtype.getMetadata()));
        }
    }

    /**
     * Gets the metadata of the item this Block can drop. This method is called when the block gets destroyed. It
     * returns the metadata of the dropped item based on the old metadata of the block.
     */
    public int damageDropped(IBlockState state)
    {
        return ((BlockWall.EnumType)state.getValue(VARIANT)).getMetadata();
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        return facing == EnumFacing.DOWN ? super.shouldSideBeRendered(worldIn, pos, side, facing) : true;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(VARIANT, BlockWall.EnumType.byMetadata(meta));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((BlockWall.EnumType)state.getValue(VARIANT)).getMetadata();
    }

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        boolean flag = this.canConnectTo(worldIn, pos.north());
        boolean flag1 = this.canConnectTo(worldIn, pos.east());
        boolean flag2 = this.canConnectTo(worldIn, pos.south());
        boolean flag3 = this.canConnectTo(worldIn, pos.west());
        boolean flag4 = flag && !flag1 && flag2 && !flag3 || !flag && flag1 && !flag2 && flag3;
        return state.withProperty(UP, Boolean.valueOf(!flag4 || !worldIn.isAirBlock(pos.up()))).withProperty(NORTH, Boolean.valueOf(flag)).withProperty(EAST, Boolean.valueOf(flag1)).withProperty(SOUTH, Boolean.valueOf(flag2)).withProperty(WEST, Boolean.valueOf(flag3));
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {UP, NORTH, EAST, WEST, SOUTH, VARIANT});
    }

    public static enum EnumType implements IStringSerializable
    {
        NORMAL(0, "cobblestone", "normal"),
        MOSSY(1, "mossy_cobblestone", "mossy");

        private static final BlockWall.EnumType[] META_LOOKUP = new BlockWall.EnumType[values().length];
        private final int meta;
        private final String name;
        private String unlocalizedName;

        private EnumType(int meta, String name, String unlocalizedName)
        {
            this.meta = meta;
            this.name = name;
            this.unlocalizedName = unlocalizedName;
        }

        public int getMetadata()
        {
            return this.meta;
        }

        public String toString()
        {
            return this.name;
        }

        public static BlockWall.EnumType byMetadata(int meta)
        {
            if (meta < 0 || meta >= META_LOOKUP.length)
            {
                meta = 0;
            }

            return META_LOOKUP[meta];
        }

        public String getName()
        {
            return this.name;
        }

        public String getUnlocalizedName()
        {
            return this.unlocalizedName;
        }

        static {
            for (BlockWall.EnumType blockwall$enumtype : values())
            {
                META_LOOKUP[blockwall$enumtype.getMetadata()] = blockwall$enumtype;
            }
        }
    }
}
