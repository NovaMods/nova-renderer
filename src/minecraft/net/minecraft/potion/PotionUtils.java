package net.minecraft.potion;

import com.google.common.base.Objects;
import com.google.common.collect.Lists;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttribute;
import net.minecraft.init.PotionTypes;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.Tuple;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.util.text.translation.I18n;

public class PotionUtils
{
    public static List<PotionEffect> getEffectsFromStack(ItemStack stack)
    {
        return getEffectsFromTag(stack.getTagCompound());
    }

    public static List<PotionEffect> func_185186_a(PotionType p_185186_0_, Collection<PotionEffect> p_185186_1_)
    {
        List<PotionEffect> list = Lists.<PotionEffect>newArrayList();
        list.addAll(p_185186_0_.getEffects());
        list.addAll(p_185186_1_);
        return list;
    }

    public static List<PotionEffect> getEffectsFromTag(NBTTagCompound tag)
    {
        List<PotionEffect> list = Lists.<PotionEffect>newArrayList();
        list.addAll(func_185187_c(tag).getEffects());
        func_185193_a(tag, list);
        return list;
    }

    public static List<PotionEffect> func_185190_b(ItemStack p_185190_0_)
    {
        return func_185192_b(p_185190_0_.getTagCompound());
    }

    public static List<PotionEffect> func_185192_b(NBTTagCompound p_185192_0_)
    {
        List<PotionEffect> list = Lists.<PotionEffect>newArrayList();
        func_185193_a(p_185192_0_, list);
        return list;
    }

    public static void func_185193_a(NBTTagCompound p_185193_0_, List<PotionEffect> p_185193_1_)
    {
        if (p_185193_0_ != null && p_185193_0_.hasKey("CustomPotionEffects", 9))
        {
            NBTTagList nbttaglist = p_185193_0_.getTagList("CustomPotionEffects", 10);

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                NBTTagCompound nbttagcompound = nbttaglist.getCompoundTagAt(i);
                PotionEffect potioneffect = PotionEffect.readCustomPotionEffectFromNBT(nbttagcompound);

                if (potioneffect != null)
                {
                    p_185193_1_.add(potioneffect);
                }
            }
        }
    }

    public static int func_185183_a(PotionType p_185183_0_)
    {
        return func_185181_a(p_185183_0_.getEffects());
    }

    public static int func_185181_a(Collection<PotionEffect> p_185181_0_)
    {
        int i = 3694022;

        if (p_185181_0_.isEmpty())
        {
            return 3694022;
        }
        else
        {
            float f = 0.0F;
            float f1 = 0.0F;
            float f2 = 0.0F;
            int j = 0;

            for (PotionEffect potioneffect : p_185181_0_)
            {
                if (potioneffect.func_188418_e())
                {
                    int k = potioneffect.func_188419_a().getLiquidColor();
                    int l = potioneffect.getAmplifier() + 1;
                    f += (float)(l * (k >> 16 & 255)) / 255.0F;
                    f1 += (float)(l * (k >> 8 & 255)) / 255.0F;
                    f2 += (float)(l * (k >> 0 & 255)) / 255.0F;
                    j += l;
                }
            }

            if (j == 0)
            {
                return 0;
            }
            else
            {
                f = f / (float)j * 255.0F;
                f1 = f1 / (float)j * 255.0F;
                f2 = f2 / (float)j * 255.0F;
                return (int)f << 16 | (int)f1 << 8 | (int)f2;
            }
        }
    }

    public static PotionType func_185191_c(ItemStack p_185191_0_)
    {
        return func_185187_c(p_185191_0_.getTagCompound());
    }

    public static PotionType func_185187_c(NBTTagCompound p_185187_0_)
    {
        return p_185187_0_ == null ? PotionTypes.water : PotionType.getPotionTypeForName(p_185187_0_.getString("Potion"));
    }

    public static ItemStack func_185188_a(ItemStack p_185188_0_, PotionType p_185188_1_)
    {
        ResourceLocation resourcelocation = (ResourceLocation)PotionType.REGISTRY.getNameForObject(p_185188_1_);

        if (resourcelocation != null)
        {
            NBTTagCompound nbttagcompound = p_185188_0_.hasTagCompound() ? p_185188_0_.getTagCompound() : new NBTTagCompound();
            nbttagcompound.setString("Potion", resourcelocation.toString());
            p_185188_0_.setTagCompound(nbttagcompound);
        }

        return p_185188_0_;
    }

    public static ItemStack func_185184_a(ItemStack p_185184_0_, Collection<PotionEffect> p_185184_1_)
    {
        if (p_185184_1_.isEmpty())
        {
            return p_185184_0_;
        }
        else
        {
            NBTTagCompound nbttagcompound = (NBTTagCompound)Objects.firstNonNull(p_185184_0_.getTagCompound(), new NBTTagCompound());
            NBTTagList nbttaglist = nbttagcompound.getTagList("CustomPotionEffects", 9);

            for (PotionEffect potioneffect : p_185184_1_)
            {
                nbttaglist.appendTag(potioneffect.writeCustomPotionEffectToNBT(new NBTTagCompound()));
            }

            nbttagcompound.setTag("CustomPotionEffects", nbttaglist);
            p_185184_0_.setTagCompound(nbttagcompound);
            return p_185184_0_;
        }
    }

    public static void addPotionTooltip(ItemStack p_185182_0_, List<String> p_185182_1_, float p_185182_2_)
    {
        List<PotionEffect> list = getEffectsFromStack(p_185182_0_);
        List<Tuple<String, AttributeModifier>> list1 = Lists.<Tuple<String, AttributeModifier>>newArrayList();

        if (list.isEmpty())
        {
            String s = I18n.translateToLocal("effect.none").trim();
            p_185182_1_.add(TextFormatting.GRAY + s);
        }
        else
        {
            for (PotionEffect potioneffect : list)
            {
                String s1 = I18n.translateToLocal(potioneffect.getEffectName()).trim();
                Potion potion = potioneffect.func_188419_a();
                Map<IAttribute, AttributeModifier> map = potion.getAttributeModifierMap();

                if (!map.isEmpty())
                {
                    for (Entry<IAttribute, AttributeModifier> entry : map.entrySet())
                    {
                        AttributeModifier attributemodifier = (AttributeModifier)entry.getValue();
                        AttributeModifier attributemodifier1 = new AttributeModifier(attributemodifier.getName(), potion.getAttributeModifierAmount(potioneffect.getAmplifier(), attributemodifier), attributemodifier.getOperation());
                        list1.add(new Tuple(((IAttribute)entry.getKey()).getAttributeUnlocalizedName(), attributemodifier1));
                    }
                }

                if (potioneffect.getAmplifier() > 0)
                {
                    s1 = s1 + " " + I18n.translateToLocal("potion.potency." + potioneffect.getAmplifier()).trim();
                }

                if (potioneffect.getDuration() > 20)
                {
                    s1 = s1 + " (" + Potion.func_188410_a(potioneffect, p_185182_2_) + ")";
                }

                if (potion.isBadEffect())
                {
                    p_185182_1_.add(TextFormatting.RED + s1);
                }
                else
                {
                    p_185182_1_.add(TextFormatting.BLUE + s1);
                }
            }
        }

        if (!list1.isEmpty())
        {
            p_185182_1_.add("");
            p_185182_1_.add(TextFormatting.DARK_PURPLE + I18n.translateToLocal("potion.whenDrank"));

            for (Tuple<String, AttributeModifier> tuple : list1)
            {
                AttributeModifier attributemodifier2 = (AttributeModifier)tuple.getSecond();
                double d0 = attributemodifier2.getAmount();
                double d1;

                if (attributemodifier2.getOperation() != 1 && attributemodifier2.getOperation() != 2)
                {
                    d1 = attributemodifier2.getAmount();
                }
                else
                {
                    d1 = attributemodifier2.getAmount() * 100.0D;
                }

                if (d0 > 0.0D)
                {
                    p_185182_1_.add(TextFormatting.BLUE + I18n.translateToLocalFormatted("attribute.modifier.plus." + attributemodifier2.getOperation(), new Object[] {ItemStack.DECIMALFORMAT.format(d1), I18n.translateToLocal("attribute.name." + (String)tuple.getFirst())}));
                }
                else if (d0 < 0.0D)
                {
                    d1 = d1 * -1.0D;
                    p_185182_1_.add(TextFormatting.RED + I18n.translateToLocalFormatted("attribute.modifier.take." + attributemodifier2.getOperation(), new Object[] {ItemStack.DECIMALFORMAT.format(d1), I18n.translateToLocal("attribute.name." + (String)tuple.getFirst())}));
                }
            }
        }
    }
}
