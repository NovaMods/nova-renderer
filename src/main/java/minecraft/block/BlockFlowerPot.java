package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyEnum;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemBlock;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityFlowerPot;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.IStringSerializable;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockFlowerPot extends BlockContainer
{
    public static final PropertyInteger LEGACY_DATA = PropertyInteger.create("legacy_data", 0, 15);
    public static final PropertyEnum<BlockFlowerPot.EnumFlowerType> CONTENTS = PropertyEnum.<BlockFlowerPot.EnumFlowerType>create("contents", BlockFlowerPot.EnumFlowerType.class);
    protected static final AxisAlignedBB field_185570_c = new AxisAlignedBB(0.3125D, 0.0D, 0.3125D, 0.6875D, 0.375D, 0.6875D);

    public BlockFlowerPot()
    {
        super(Material.circuits);
        this.setDefaultState(this.blockState.getBaseState().withProperty(CONTENTS, BlockFlowerPot.EnumFlowerType.EMPTY).withProperty(LEGACY_DATA, Integer.valueOf(0)));
    }

    /**
     * Gets the localized name of this block. Used for the statistics page.
     */
    public String getLocalizedName()
    {
        return I18n.translateToLocal("item.flowerPot.name");
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return field_185570_c;
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

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (hitX != null && hitX.getItem() instanceof ItemBlock)
        {
            TileEntityFlowerPot tileentityflowerpot = this.getTileEntity(worldIn, pos);

            if (tileentityflowerpot == null)
            {
                return false;
            }
            else if (tileentityflowerpot.getFlowerPotItem() != null)
            {
                return false;
            }
            else
            {
                Block block = Block.getBlockFromItem(hitX.getItem());

                if (!this.canNotContain(block, hitX.getMetadata()))
                {
                    return false;
                }
                else
                {
                    tileentityflowerpot.setFlowerPotData(hitX.getItem(), hitX.getMetadata());
                    tileentityflowerpot.markDirty();
                    worldIn.notifyBlockUpdate(pos, state, state, 3);
                    playerIn.triggerAchievement(StatList.field_188088_V);

                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        --hitX.stackSize;
                    }

                    return true;
                }
            }
        }
        else
        {
            return false;
        }
    }

    private boolean canNotContain(Block blockIn, int meta)
    {
        return blockIn != Blocks.yellow_flower && blockIn != Blocks.red_flower && blockIn != Blocks.cactus && blockIn != Blocks.brown_mushroom && blockIn != Blocks.red_mushroom && blockIn != Blocks.sapling && blockIn != Blocks.deadbush ? blockIn == Blocks.tallgrass && meta == BlockTallGrass.EnumType.FERN.getMeta() : true;
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        TileEntityFlowerPot tileentityflowerpot = this.getTileEntity(worldIn, pos);

        if (tileentityflowerpot != null)
        {
            ItemStack itemstack = tileentityflowerpot.func_184403_b();

            if (itemstack != null)
            {
                return itemstack;
            }
        }

        return new ItemStack(Items.flower_pot);
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return super.canPlaceBlockAt(worldIn, pos) && worldIn.getBlockState(pos.down()).func_185896_q();
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (!worldIn.getBlockState(pos.down()).func_185896_q())
        {
            this.dropBlockAsItem(worldIn, pos, state, 0);
            worldIn.setBlockToAir(pos);
        }
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
        TileEntityFlowerPot tileentityflowerpot = this.getTileEntity(worldIn, pos);

        if (tileentityflowerpot != null && tileentityflowerpot.getFlowerPotItem() != null)
        {
            spawnAsEntity(worldIn, pos, new ItemStack(tileentityflowerpot.getFlowerPotItem(), 1, tileentityflowerpot.getFlowerPotData()));
        }

        super.breakBlock(worldIn, pos, state);
    }

    public void onBlockHarvested(World worldIn, BlockPos pos, IBlockState state, EntityPlayer player)
    {
        super.onBlockHarvested(worldIn, pos, state, player);

        if (player.capabilities.isCreativeMode)
        {
            TileEntityFlowerPot tileentityflowerpot = this.getTileEntity(worldIn, pos);

            if (tileentityflowerpot != null)
            {
                tileentityflowerpot.setFlowerPotData((Item)null, 0);
            }
        }
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.flower_pot;
    }

    private TileEntityFlowerPot getTileEntity(World worldIn, BlockPos pos)
    {
        TileEntity tileentity = worldIn.getTileEntity(pos);
        return tileentity instanceof TileEntityFlowerPot ? (TileEntityFlowerPot)tileentity : null;
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        Block block = null;
        int i = 0;

        switch (meta)
        {
            case 1:
                block = Blocks.red_flower;
                i = BlockFlower.EnumFlowerType.POPPY.getMeta();
                break;

            case 2:
                block = Blocks.yellow_flower;
                break;

            case 3:
                block = Blocks.sapling;
                i = BlockPlanks.EnumType.OAK.getMetadata();
                break;

            case 4:
                block = Blocks.sapling;
                i = BlockPlanks.EnumType.SPRUCE.getMetadata();
                break;

            case 5:
                block = Blocks.sapling;
                i = BlockPlanks.EnumType.BIRCH.getMetadata();
                break;

            case 6:
                block = Blocks.sapling;
                i = BlockPlanks.EnumType.JUNGLE.getMetadata();
                break;

            case 7:
                block = Blocks.red_mushroom;
                break;

            case 8:
                block = Blocks.brown_mushroom;
                break;

            case 9:
                block = Blocks.cactus;
                break;

            case 10:
                block = Blocks.deadbush;
                break;

            case 11:
                block = Blocks.tallgrass;
                i = BlockTallGrass.EnumType.FERN.getMeta();
                break;

            case 12:
                block = Blocks.sapling;
                i = BlockPlanks.EnumType.ACACIA.getMetadata();
                break;

            case 13:
                block = Blocks.sapling;
                i = BlockPlanks.EnumType.DARK_OAK.getMetadata();
        }

        return new TileEntityFlowerPot(Item.getItemFromBlock(block), i);
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {CONTENTS, LEGACY_DATA});
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((Integer)state.getValue(LEGACY_DATA)).intValue();
    }

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        BlockFlowerPot.EnumFlowerType blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.EMPTY;
        TileEntity tileentity = worldIn.getTileEntity(pos);

        if (tileentity instanceof TileEntityFlowerPot)
        {
            TileEntityFlowerPot tileentityflowerpot = (TileEntityFlowerPot)tileentity;
            Item item = tileentityflowerpot.getFlowerPotItem();

            if (item instanceof ItemBlock)
            {
                int i = tileentityflowerpot.getFlowerPotData();
                Block block = Block.getBlockFromItem(item);

                if (block == Blocks.sapling)
                {
                    switch (BlockPlanks.EnumType.byMetadata(i))
                    {
                        case OAK:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.OAK_SAPLING;
                            break;

                        case SPRUCE:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.SPRUCE_SAPLING;
                            break;

                        case BIRCH:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.BIRCH_SAPLING;
                            break;

                        case JUNGLE:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.JUNGLE_SAPLING;
                            break;

                        case ACACIA:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.ACACIA_SAPLING;
                            break;

                        case DARK_OAK:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.DARK_OAK_SAPLING;
                            break;

                        default:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.EMPTY;
                    }
                }
                else if (block == Blocks.tallgrass)
                {
                    switch (i)
                    {
                        case 0:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.DEAD_BUSH;
                            break;

                        case 2:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.FERN;
                            break;

                        default:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.EMPTY;
                    }
                }
                else if (block == Blocks.yellow_flower)
                {
                    blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.DANDELION;
                }
                else if (block == Blocks.red_flower)
                {
                    switch (BlockFlower.EnumFlowerType.getType(BlockFlower.EnumFlowerColor.RED, i))
                    {
                        case POPPY:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.POPPY;
                            break;

                        case BLUE_ORCHID:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.BLUE_ORCHID;
                            break;

                        case ALLIUM:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.ALLIUM;
                            break;

                        case HOUSTONIA:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.HOUSTONIA;
                            break;

                        case RED_TULIP:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.RED_TULIP;
                            break;

                        case ORANGE_TULIP:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.ORANGE_TULIP;
                            break;

                        case WHITE_TULIP:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.WHITE_TULIP;
                            break;

                        case PINK_TULIP:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.PINK_TULIP;
                            break;

                        case OXEYE_DAISY:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.OXEYE_DAISY;
                            break;

                        default:
                            blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.EMPTY;
                    }
                }
                else if (block == Blocks.red_mushroom)
                {
                    blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.MUSHROOM_RED;
                }
                else if (block == Blocks.brown_mushroom)
                {
                    blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.MUSHROOM_BROWN;
                }
                else if (block == Blocks.deadbush)
                {
                    blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.DEAD_BUSH;
                }
                else if (block == Blocks.cactus)
                {
                    blockflowerpot$enumflowertype = BlockFlowerPot.EnumFlowerType.CACTUS;
                }
            }
        }

        return state.withProperty(CONTENTS, blockflowerpot$enumflowertype);
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    public static enum EnumFlowerType implements IStringSerializable
    {
        EMPTY("empty"),
        POPPY("rose"),
        BLUE_ORCHID("blue_orchid"),
        ALLIUM("allium"),
        HOUSTONIA("houstonia"),
        RED_TULIP("red_tulip"),
        ORANGE_TULIP("orange_tulip"),
        WHITE_TULIP("white_tulip"),
        PINK_TULIP("pink_tulip"),
        OXEYE_DAISY("oxeye_daisy"),
        DANDELION("dandelion"),
        OAK_SAPLING("oak_sapling"),
        SPRUCE_SAPLING("spruce_sapling"),
        BIRCH_SAPLING("birch_sapling"),
        JUNGLE_SAPLING("jungle_sapling"),
        ACACIA_SAPLING("acacia_sapling"),
        DARK_OAK_SAPLING("dark_oak_sapling"),
        MUSHROOM_RED("mushroom_red"),
        MUSHROOM_BROWN("mushroom_brown"),
        DEAD_BUSH("dead_bush"),
        FERN("fern"),
        CACTUS("cactus");

        private final String name;

        private EnumFlowerType(String name)
        {
            this.name = name;
        }

        public String toString()
        {
            return this.name;
        }

        public String getName()
        {
            return this.name;
        }
    }
}
