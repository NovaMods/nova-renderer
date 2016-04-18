package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.gen.feature.WorldGenEndPodium;

public class PhaseDying extends PhaseBase
{
    private Vec3d field_188672_b;
    private int field_188673_c;

    public PhaseDying(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188657_b()
    {
        if (this.field_188673_c++ % 10 == 0)
        {
            float f = (this.dragon.getRNG().nextFloat() - 0.5F) * 8.0F;
            float f1 = (this.dragon.getRNG().nextFloat() - 0.5F) * 4.0F;
            float f2 = (this.dragon.getRNG().nextFloat() - 0.5F) * 8.0F;
            this.dragon.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_HUGE, this.dragon.posX + (double)f, this.dragon.posY + 2.0D + (double)f1, this.dragon.posZ + (double)f2, 0.0D, 0.0D, 0.0D, new int[0]);
        }
    }

    public void func_188659_c()
    {
        ++this.field_188673_c;

        if (this.field_188672_b == null)
        {
            BlockPos blockpos = this.dragon.worldObj.getHeight(WorldGenEndPodium.field_186139_a);
            this.field_188672_b = new Vec3d((double)blockpos.getX(), (double)blockpos.getY(), (double)blockpos.getZ());
        }

        double d0 = this.field_188672_b.func_186679_c(this.dragon.posX, this.dragon.posY, this.dragon.posZ);

        if (d0 >= 100.0D && d0 <= 22500.0D && !this.dragon.isCollidedHorizontally && !this.dragon.isCollidedVertically)
        {
            this.dragon.setHealth(1.0F);
        }
        else
        {
            this.dragon.setHealth(0.0F);
        }
    }

    public void func_188660_d()
    {
        this.field_188672_b = null;
        this.field_188673_c = 0;
    }

    public float func_188651_f()
    {
        return 3.0F;
    }

    public Vec3d func_188650_g()
    {
        return this.field_188672_b;
    }

    public PhaseList<PhaseDying> func_188652_i()
    {
        return PhaseList.DYING;
    }
}
