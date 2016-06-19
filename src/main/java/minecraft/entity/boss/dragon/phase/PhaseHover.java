package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.util.math.Vec3d;

public class PhaseHover extends PhaseBase
{
    private Vec3d field_188680_b;

    public PhaseHover(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188659_c()
    {
        if (this.field_188680_b == null)
        {
            this.field_188680_b = new Vec3d(this.dragon.posX, this.dragon.posY, this.dragon.posZ);
        }
    }

    public boolean func_188654_a()
    {
        return true;
    }

    public void func_188660_d()
    {
        this.field_188680_b = null;
    }

    public float func_188651_f()
    {
        return 1.0F;
    }

    public Vec3d func_188650_g()
    {
        return this.field_188680_b;
    }

    public PhaseList<PhaseHover> func_188652_i()
    {
        return PhaseList.HOVER;
    }
}
