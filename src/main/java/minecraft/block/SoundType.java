package net.minecraft.block;

import net.minecraft.init.SoundEvents;
import net.minecraft.util.SoundEvent;

public class SoundType
{
    public static final SoundType WOOD = new SoundType(1.0F, 1.0F, SoundEvents.block_wood_break, SoundEvents.block_wood_step, SoundEvents.block_wood_place, SoundEvents.block_wood_hit, SoundEvents.block_wood_fall);
    public static final SoundType GROUND = new SoundType(1.0F, 1.0F, SoundEvents.block_gravel_break, SoundEvents.block_gravel_step, SoundEvents.block_gravel_place, SoundEvents.block_gravel_hit, SoundEvents.block_gravel_fall);
    public static final SoundType PLANT = new SoundType(1.0F, 1.0F, SoundEvents.block_grass_break, SoundEvents.block_grass_step, SoundEvents.block_grass_place, SoundEvents.block_grass_hit, SoundEvents.block_grass_fall);
    public static final SoundType STONE = new SoundType(1.0F, 1.0F, SoundEvents.block_stone_break, SoundEvents.block_stone_step, SoundEvents.block_stone_place, SoundEvents.block_stone_hit, SoundEvents.block_stone_fall);
    public static final SoundType METAL = new SoundType(1.0F, 1.5F, SoundEvents.block_metal_break, SoundEvents.block_metal_step, SoundEvents.block_metal_place, SoundEvents.block_metal_hit, SoundEvents.block_metal_fall);
    public static final SoundType GLASS = new SoundType(1.0F, 1.0F, SoundEvents.block_glass_break, SoundEvents.block_glass_step, SoundEvents.block_glass_place, SoundEvents.block_glass_hit, SoundEvents.block_glass_fall);
    public static final SoundType CLOTH = new SoundType(1.0F, 1.0F, SoundEvents.block_cloth_break, SoundEvents.block_cloth_step, SoundEvents.block_cloth_place, SoundEvents.block_cloth_hit, SoundEvents.block_cloth_fall);
    public static final SoundType SAND = new SoundType(1.0F, 1.0F, SoundEvents.block_sand_break, SoundEvents.block_sand_step, SoundEvents.block_sand_place, SoundEvents.block_sand_hit, SoundEvents.block_sand_fall);
    public static final SoundType SNOW = new SoundType(1.0F, 1.0F, SoundEvents.block_snow_break, SoundEvents.block_snow_step, SoundEvents.block_snow_place, SoundEvents.block_snow_hit, SoundEvents.block_snow_fall);
    public static final SoundType LADDER = new SoundType(1.0F, 1.0F, SoundEvents.block_ladder_break, SoundEvents.block_ladder_step, SoundEvents.block_ladder_place, SoundEvents.block_ladder_hit, SoundEvents.block_ladder_fall);
    public static final SoundType ANVIL = new SoundType(0.3F, 1.0F, SoundEvents.block_anvil_break, SoundEvents.block_anvil_step, SoundEvents.block_anvil_place, SoundEvents.block_anvil_hit, SoundEvents.block_anvil_fall);
    public static final SoundType SLIME = new SoundType(1.0F, 1.0F, SoundEvents.block_slime_break, SoundEvents.block_slime_step, SoundEvents.block_slime_place, SoundEvents.block_slime_hit, SoundEvents.block_slime_fall);
    public final float field_185860_m;
    public final float field_185861_n;
    private final SoundEvent field_185862_o;
    private final SoundEvent field_185863_p;
    private final SoundEvent field_185864_q;
    private final SoundEvent field_185865_r;
    private final SoundEvent field_185866_s;

    public SoundType(float p_i46679_1_, float p_i46679_2_, SoundEvent p_i46679_3_, SoundEvent p_i46679_4_, SoundEvent p_i46679_5_, SoundEvent p_i46679_6_, SoundEvent p_i46679_7_)
    {
        this.field_185860_m = p_i46679_1_;
        this.field_185861_n = p_i46679_2_;
        this.field_185862_o = p_i46679_3_;
        this.field_185863_p = p_i46679_4_;
        this.field_185864_q = p_i46679_5_;
        this.field_185865_r = p_i46679_6_;
        this.field_185866_s = p_i46679_7_;
    }

    public float func_185843_a()
    {
        return this.field_185860_m;
    }

    public float func_185847_b()
    {
        return this.field_185861_n;
    }

    public SoundEvent func_185845_c()
    {
        return this.field_185862_o;
    }

    public SoundEvent func_185844_d()
    {
        return this.field_185863_p;
    }

    public SoundEvent func_185841_e()
    {
        return this.field_185864_q;
    }

    public SoundEvent func_185846_f()
    {
        return this.field_185865_r;
    }

    public SoundEvent func_185842_g()
    {
        return this.field_185866_s;
    }
}
