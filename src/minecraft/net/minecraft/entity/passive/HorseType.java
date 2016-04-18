package net.minecraft.entity.passive;

import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;

public enum HorseType
{
    NONE(0),
    IRON(5, "iron", "meo"),
    GOLD(7, "gold", "goo"),
    DIAMOND(11, "diamond", "dio");

    private final String field_188586_e;
    private final String field_188587_f;
    private final int field_188588_g;

    private HorseType(int p_i46799_3_)
    {
        this.field_188588_g = p_i46799_3_;
        this.field_188586_e = null;
        this.field_188587_f = "";
    }

    private HorseType(int p_i46800_3_, String p_i46800_4_, String p_i46800_5_)
    {
        this.field_188588_g = p_i46800_3_;
        this.field_188586_e = "textures/entity/horse/armor/horse_armor_" + p_i46800_4_ + ".png";
        this.field_188587_f = p_i46800_5_;
    }

    public int func_188579_a()
    {
        return this.ordinal();
    }

    public String func_188573_b()
    {
        return this.field_188587_f;
    }

    public int func_188578_c()
    {
        return this.field_188588_g;
    }

    public String func_188574_d()
    {
        return this.field_188586_e;
    }

    public static HorseType func_188575_a(int p_188575_0_)
    {
        return values()[p_188575_0_];
    }

    public static HorseType func_188580_a(ItemStack p_188580_0_)
    {
        return p_188580_0_ == null ? NONE : func_188576_a(p_188580_0_.getItem());
    }

    public static HorseType func_188576_a(Item p_188576_0_)
    {
        return p_188576_0_ == Items.iron_horse_armor ? IRON : (p_188576_0_ == Items.golden_horse_armor ? GOLD : (p_188576_0_ == Items.diamond_horse_armor ? DIAMOND : NONE));
    }

    public static boolean func_188577_b(Item p_188577_0_)
    {
        return func_188576_a(p_188577_0_) != NONE;
    }
}
