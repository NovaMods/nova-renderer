package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.gen.feature.WorldGenEndPodium;

public class PhaseLanding extends PhaseBase
{
    private Vec3d field_188685_b;

    public PhaseLanding(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188657_b()
    {
        Vec3d vec3d = this.dragon.func_184665_a(1.0F).normalize();
        vec3d.rotateYaw(-((float)Math.PI / 4F));
        double d0 = this.dragon.dragonPartHead.posX;
        double d1 = this.dragon.dragonPartHead.posY + (double)(this.dragon.dragonPartHead.height / 2.0F);
        double d2 = this.dragon.dragonPartHead.posZ;

        for (int i = 0; i < 8; ++i)
        {
            double d3 = d0 + this.dragon.getRNG().nextGaussian() / 2.0D;
            double d4 = d1 + this.dragon.getRNG().nextGaussian() / 2.0D;
            double d5 = d2 + this.dragon.getRNG().nextGaussian() / 2.0D;
            this.dragon.worldObj.spawnParticle(EnumParticleTypes.DRAGON_BREATH, d3, d4, d5, -vec3d.xCoord * 0.07999999821186066D + this.dragon.motionX, -vec3d.yCoord * 0.30000001192092896D + this.dragon.motionY, -vec3d.zCoord * 0.07999999821186066D + this.dragon.motionZ, new int[0]);
            vec3d.rotateYaw(0.19634955F);
        }
    }

    public void func_188659_c()
    {
        if (this.field_188685_b == null)
        {
            this.field_188685_b = new Vec3d(this.dragon.worldObj.getTopSolidOrLiquidBlock(WorldGenEndPodium.field_186139_a));
        }

        if (this.field_188685_b.func_186679_c(this.dragon.posX, this.dragon.posY, this.dragon.posZ) < 1.0D)
        {
            ((PhaseSittingFlaming)this.dragon.getPhaseManager().func_188757_b(PhaseList.SITTING_FLAMING)).func_188663_j();
            this.dragon.getPhaseManager().func_188758_a(PhaseList.SITTING_SCANNING);
        }
    }

    public float func_188651_f()
    {
        return 1.5F;
    }

    public float func_188653_h()
    {
        float f = MathHelper.sqrt_double(this.dragon.motionX * this.dragon.motionX + this.dragon.motionZ * this.dragon.motionZ) + 1.0F;
        float f1 = Math.min(f, 40.0F);
        return f1 / f;
    }

    public void func_188660_d()
    {
        this.field_188685_b = null;
    }

    public Vec3d func_188650_g()
    {
        return this.field_188685_b;
    }

    public PhaseList<PhaseLanding> func_188652_i()
    {
        return PhaseList.LANDING;
    }
}
