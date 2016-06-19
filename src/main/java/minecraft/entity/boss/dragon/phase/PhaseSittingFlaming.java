package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.EntityAreaEffectCloud;
import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.init.MobEffects;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;

public class PhaseSittingFlaming extends PhaseSittingBase
{
    private int field_188664_b;
    private int field_188665_c;
    private EntityAreaEffectCloud areaEffectCloud;

    public PhaseSittingFlaming(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188657_b()
    {
        ++this.field_188664_b;

        if (this.field_188664_b % 2 == 0 && this.field_188664_b < 10)
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

                for (int j = 0; j < 6; ++j)
                {
                    this.dragon.worldObj.spawnParticle(EnumParticleTypes.DRAGON_BREATH, d3, d4, d5, -vec3d.xCoord * 0.07999999821186066D * (double)j, -vec3d.yCoord * 0.6000000238418579D, -vec3d.zCoord * 0.07999999821186066D * (double)j, new int[0]);
                }

                vec3d.rotateYaw(0.19634955F);
            }
        }
    }

    public void func_188659_c()
    {
        ++this.field_188664_b;

        if (this.field_188664_b >= 200)
        {
            if (this.field_188665_c >= 4)
            {
                this.dragon.getPhaseManager().func_188758_a(PhaseList.TAKEOFF);
            }
            else
            {
                this.dragon.getPhaseManager().func_188758_a(PhaseList.SITTING_SCANNING);
            }
        }
        else if (this.field_188664_b == 10)
        {
            Vec3d vec3d = (new Vec3d(this.dragon.dragonPartHead.posX - this.dragon.posX, 0.0D, this.dragon.dragonPartHead.posZ - this.dragon.posZ)).normalize();
            float f = 5.0F;
            double d0 = this.dragon.dragonPartHead.posX + vec3d.xCoord * (double)f / 2.0D;
            double d1 = this.dragon.dragonPartHead.posZ + vec3d.zCoord * (double)f / 2.0D;
            double d2 = this.dragon.dragonPartHead.posY + (double)(this.dragon.dragonPartHead.height / 2.0F);
            BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos(MathHelper.floor_double(d0), MathHelper.floor_double(d2), MathHelper.floor_double(d1));

            while (this.dragon.worldObj.isAirBlock(blockpos$mutableblockpos))
            {
                --d2;
                blockpos$mutableblockpos.set(MathHelper.floor_double(d0), MathHelper.floor_double(d2), MathHelper.floor_double(d1));
            }

            d2 = (double)(MathHelper.floor_double(d2) + 1);
            this.areaEffectCloud = new EntityAreaEffectCloud(this.dragon.worldObj, d0, d2, d1);
            this.areaEffectCloud.func_184481_a(this.dragon);
            this.areaEffectCloud.setRadius(f);
            this.areaEffectCloud.func_184486_b(200);
            this.areaEffectCloud.func_184491_a(EnumParticleTypes.DRAGON_BREATH);
            this.areaEffectCloud.func_184496_a(new PotionEffect(MobEffects.harm));
            this.dragon.worldObj.spawnEntityInWorld(this.areaEffectCloud);
        }
    }

    public void func_188660_d()
    {
        this.field_188664_b = 0;
        ++this.field_188665_c;
    }

    public void func_188658_e()
    {
        if (this.areaEffectCloud != null)
        {
            this.areaEffectCloud.setDead();
            this.areaEffectCloud = null;
        }
    }

    public PhaseList<PhaseSittingFlaming> func_188652_i()
    {
        return PhaseList.SITTING_FLAMING;
    }

    public void func_188663_j()
    {
        this.field_188665_c = 0;
    }
}
