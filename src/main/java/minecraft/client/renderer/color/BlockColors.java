package net.minecraft.client.renderer.color;

import net.minecraft.block.Block;
import net.minecraft.block.BlockDoublePlant;
import net.minecraft.block.BlockOldLeaf;
import net.minecraft.block.BlockPlanks;
import net.minecraft.block.BlockRedstoneWire;
import net.minecraft.block.BlockStem;
import net.minecraft.block.BlockTallGrass;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;
import net.minecraft.item.Item;
import net.minecraft.item.ItemBlock;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityFlowerPot;
import net.minecraft.util.ObjectIntIdentityMap;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.ColorizerFoliage;
import net.minecraft.world.ColorizerGrass;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.biome.BiomeColorHelper;

public class BlockColors
{
    private final ObjectIntIdentityMap<IBlockColor> field_186725_a = new ObjectIntIdentityMap(32);

    public static BlockColors func_186723_a()
    {
        final BlockColors blockcolors = new BlockColors();
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                BlockDoublePlant.EnumPlantType blockdoubleplant$enumplanttype = (BlockDoublePlant.EnumPlantType)p_186720_1_.getValue(BlockDoublePlant.VARIANT);
                return p_186720_2_ == null || p_186720_3_ == null || blockdoubleplant$enumplanttype != BlockDoublePlant.EnumPlantType.GRASS && blockdoubleplant$enumplanttype != BlockDoublePlant.EnumPlantType.FERN ? -1 : BiomeColorHelper.getGrassColorAtPos(p_186720_2_, p_186720_3_);
            }
        }, new Block[] {Blocks.double_plant});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                if (p_186720_2_ != null && p_186720_3_ != null)
                {
                    TileEntity tileentity = p_186720_2_.getTileEntity(p_186720_3_);

                    if (tileentity instanceof TileEntityFlowerPot)
                    {
                        Item item = ((TileEntityFlowerPot)tileentity).getFlowerPotItem();

                        if (item instanceof ItemBlock)
                        {
                            IBlockState iblockstate = Block.getBlockFromItem(item).getDefaultState();
                            return blockcolors.func_186724_a(iblockstate, p_186720_2_, p_186720_3_, p_186720_4_);
                        }
                    }

                    return -1;
                }
                else
                {
                    return -1;
                }
            }
        }, new Block[] {Blocks.flower_pot});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getGrassColorAtPos(p_186720_2_, p_186720_3_) : ColorizerGrass.getGrassColor(0.5D, 1.0D);
            }
        }, new Block[] {Blocks.grass});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                BlockPlanks.EnumType blockplanks$enumtype = (BlockPlanks.EnumType)p_186720_1_.getValue(BlockOldLeaf.VARIANT);
                return blockplanks$enumtype == BlockPlanks.EnumType.SPRUCE ? ColorizerFoliage.getFoliageColorPine() : (blockplanks$enumtype == BlockPlanks.EnumType.BIRCH ? ColorizerFoliage.getFoliageColorBirch() : (p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getFoliageColorAtPos(p_186720_2_, p_186720_3_) : ColorizerFoliage.getFoliageColorBasic()));
            }
        }, new Block[] {Blocks.leaves});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getFoliageColorAtPos(p_186720_2_, p_186720_3_) : ColorizerFoliage.getFoliageColorBasic();
            }
        }, new Block[] {Blocks.leaves2});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getWaterColorAtPos(p_186720_2_, p_186720_3_) : -1;
            }
        }, new Block[] {Blocks.water, Blocks.flowing_water});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return BlockRedstoneWire.colorMultiplier(((Integer)p_186720_1_.getValue(BlockRedstoneWire.POWER)).intValue());
            }
        }, new Block[] {Blocks.redstone_wire});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getGrassColorAtPos(p_186720_2_, p_186720_3_) : -1;
            }
        }, new Block[] {Blocks.reeds});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                int i = ((Integer)p_186720_1_.getValue(BlockStem.AGE)).intValue();
                int j = i * 32;
                int k = 255 - i * 8;
                int l = i * 4;
                return j << 16 | k << 8 | l;
            }
        }, new Block[] {Blocks.melon_stem, Blocks.pumpkin_stem});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getGrassColorAtPos(p_186720_2_, p_186720_3_) : (p_186720_1_.getValue(BlockTallGrass.TYPE) == BlockTallGrass.EnumType.DEAD_BUSH ? 16777215 : ColorizerGrass.getGrassColor(0.5D, 1.0D));
            }
        }, new Block[] {Blocks.tallgrass});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? BiomeColorHelper.getFoliageColorAtPos(p_186720_2_, p_186720_3_) : ColorizerFoliage.getFoliageColorBasic();
            }
        }, new Block[] {Blocks.vine});
        blockcolors.func_186722_a(new IBlockColor()
        {
            public int func_186720_a(IBlockState p_186720_1_, IBlockAccess p_186720_2_, BlockPos p_186720_3_, int p_186720_4_)
            {
                return p_186720_2_ != null && p_186720_3_ != null ? 2129968 : 7455580;
            }
        }, new Block[] {Blocks.waterlily});
        return blockcolors;
    }

    public int func_186724_a(IBlockState p_186724_1_, IBlockAccess p_186724_2_, BlockPos p_186724_3_, int p_186724_4_)
    {
        IBlockColor iblockcolor = (IBlockColor)this.field_186725_a.getByValue(Block.getIdFromBlock(p_186724_1_.getBlock()));
        return iblockcolor == null ? -1 : iblockcolor.func_186720_a(p_186724_1_, p_186724_2_, p_186724_3_, p_186724_4_);
    }

    public void func_186722_a(IBlockColor p_186722_1_, Block... p_186722_2_)
    {
        int i = 0;

        for (int j = p_186722_2_.length; i < j; ++i)
        {
            this.field_186725_a.put(p_186722_1_, Block.getIdFromBlock(p_186722_2_[i]));
        }
    }
}
