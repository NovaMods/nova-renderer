package net.minecraft.client.renderer.color;

import net.minecraft.block.Block;
import net.minecraft.block.BlockDoublePlant;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EntityList;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemBanner;
import net.minecraft.item.ItemBlock;
import net.minecraft.item.ItemFireworkCharge;
import net.minecraft.item.ItemMonsterPlacer;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTBase;
import net.minecraft.nbt.NBTTagIntArray;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.ObjectIntIdentityMap;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.ColorizerGrass;
import net.minecraft.world.IBlockAccess;

public class ItemColors
{
    private final ObjectIntIdentityMap<IItemColor> field_186732_a = new ObjectIntIdentityMap(32);

    public static ItemColors func_186729_a(final BlockColors p_186729_0_)
    {
        ItemColors itemcolors = new ItemColors();
        itemcolors.func_186730_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                return p_186726_2_ > 0 ? -1 : ((ItemArmor)p_186726_1_.getItem()).getColor(p_186726_1_);
            }
        }, new Item[] {Items.leather_helmet, Items.leather_chestplate, Items.leather_leggings, Items.leather_boots});
        itemcolors.func_186730_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                return p_186726_2_ > 0 ? -1 : ItemBanner.getBaseColor(p_186726_1_).getMapColor().colorValue;
            }
        }, new Item[] {Items.banner, Items.shield});
        itemcolors.func_186731_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                BlockDoublePlant.EnumPlantType blockdoubleplant$enumplanttype = BlockDoublePlant.EnumPlantType.byMetadata(p_186726_1_.getMetadata());
                return blockdoubleplant$enumplanttype != BlockDoublePlant.EnumPlantType.GRASS && blockdoubleplant$enumplanttype != BlockDoublePlant.EnumPlantType.FERN ? -1 : ColorizerGrass.getGrassColor(0.5D, 1.0D);
            }
        }, new Block[] {Blocks.double_plant});
        itemcolors.func_186730_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                if (p_186726_2_ != 1)
                {
                    return -1;
                }
                else
                {
                    NBTBase nbtbase = ItemFireworkCharge.getExplosionTag(p_186726_1_, "Colors");

                    if (!(nbtbase instanceof NBTTagIntArray))
                    {
                        return 9079434;
                    }
                    else
                    {
                        int[] aint = ((NBTTagIntArray)nbtbase).getIntArray();

                        if (aint.length == 1)
                        {
                            return aint[0];
                        }
                        else
                        {
                            int i = 0;
                            int j = 0;
                            int k = 0;
                            int l = 0;

                            for (int i1 = aint.length; l < i1; ++l)
                            {
                                int j1 = aint[l];
                                i += (j1 & 16711680) >> 16;
                                j += (j1 & 65280) >> 8;
                                k += (j1 & 255) >> 0;
                            }

                            i = i / aint.length;
                            j = j / aint.length;
                            k = k / aint.length;
                            return i << 16 | j << 8 | k;
                        }
                    }
                }
            }
        }, new Item[] {Items.firework_charge});
        itemcolors.func_186730_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                return p_186726_2_ > 0 ? -1 : PotionUtils.func_185181_a(PotionUtils.getEffectsFromStack(p_186726_1_));
            }
        }, new Item[] {Items.potionitem, Items.splash_potion, Items.lingering_potion});
        itemcolors.func_186730_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                EntityList.EntityEggInfo entitylist$entityegginfo = (EntityList.EntityEggInfo)EntityList.entityEggs.get(ItemMonsterPlacer.func_185080_h(p_186726_1_));
                return entitylist$entityegginfo == null ? -1 : (p_186726_2_ == 0 ? entitylist$entityegginfo.primaryColor : entitylist$entityegginfo.secondaryColor);
            }
        }, new Item[] {Items.spawn_egg});
        itemcolors.func_186731_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                IBlockState iblockstate = ((ItemBlock)p_186726_1_.getItem()).getBlock().getStateFromMeta(p_186726_1_.getMetadata());
                return p_186729_0_.func_186724_a(iblockstate, (IBlockAccess)null, (BlockPos)null, p_186726_2_);
            }
        }, new Block[] {Blocks.grass, Blocks.tallgrass, Blocks.vine, Blocks.leaves, Blocks.leaves2, Blocks.waterlily});
        itemcolors.func_186730_a(new IItemColor()
        {
            public int func_186726_a(ItemStack p_186726_1_, int p_186726_2_)
            {
                return p_186726_2_ == 0 ? PotionUtils.func_185181_a(PotionUtils.getEffectsFromStack(p_186726_1_)) : -1;
            }
        }, new Item[] {Items.tipped_arrow});
        return itemcolors;
    }

    public int func_186728_a(ItemStack p_186728_1_, int p_186728_2_)
    {
        IItemColor iitemcolor = (IItemColor)this.field_186732_a.getByValue(Item.itemRegistry.getIDForObject(p_186728_1_.getItem()));
        return iitemcolor == null ? -1 : iitemcolor.func_186726_a(p_186728_1_, p_186728_2_);
    }

    public void func_186731_a(IItemColor p_186731_1_, Block... p_186731_2_)
    {
        int i = 0;

        for (int j = p_186731_2_.length; i < j; ++i)
        {
            this.field_186732_a.put(p_186731_1_, Item.getIdFromItem(Item.getItemFromBlock(p_186731_2_[i])));
        }
    }

    public void func_186730_a(IItemColor p_186730_1_, Item... p_186730_2_)
    {
        int i = 0;

        for (int j = p_186730_2_.length; i < j; ++i)
        {
            this.field_186732_a.put(p_186730_1_, Item.getIdFromItem(p_186730_2_[i]));
        }
    }
}
