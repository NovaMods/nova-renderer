package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;

public class PhaseSittingScanning extends PhaseSittingBase
{
    private int field_188667_b;

    public PhaseSittingScanning(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188659_c()
    {
        ++this.field_188667_b;
        EntityLivingBase entitylivingbase = this.dragon.worldObj.func_184142_a(this.dragon, 20.0D, 10.0D);

        if (entitylivingbase != null)
        {
            if (this.field_188667_b > 25)
            {
                this.dragon.getPhaseManager().func_188758_a(PhaseList.SITTING_ATTACKING);
            }
            else
            {
                Vec3d vec3d = (new Vec3d(entitylivingbase.posX - this.dragon.posX, 0.0D, entitylivingbase.posZ - this.dragon.posZ)).normalize();
                Vec3d vec3d1 = (new Vec3d((double)MathHelper.sin(this.dragon.rotationYaw * 0.017453292F), 0.0D, (double)(-MathHelper.cos(this.dragon.rotationYaw * 0.017453292F)))).normalize();
                float f = (float)vec3d1.dotProduct(vec3d);
                float f1 = (float)(Math.acos((double)f) * (180D / Math.PI)) + 0.5F;

                if (f1 < 0.0F || f1 > 10.0F)
                {
                    double d0 = entitylivingbase.posX - this.dragon.dragonPartHead.posX;
                    double d1 = entitylivingbase.posZ - this.dragon.dragonPartHead.posZ;
                    double d2 = MathHelper.clamp_double(MathHelper.wrapAngleTo180_double(180.0D - MathHelper.atan2(d0, d1) * (180D / Math.PI) - (double)this.dragon.rotationYaw), -100.0D, 100.0D);
                    this.dragon.randomYawVelocity *= 0.8F;
                    float f2 = MathHelper.sqrt_double(d0 * d0 + d1 * d1) + 1.0F;
                    float f3 = f2;

                    if (f2 > 40.0F)
                    {
                        f2 = 40.0F;
                    }

                    this.dragon.randomYawVelocity = (float)((double)this.dragon.randomYawVelocity + d2 * (double)(0.7F / f2 / f3));
                    this.dragon.rotationYaw += this.dragon.randomYawVelocity;
                }
            }
        }
        else if (this.field_188667_b >= 100)
        {
            entitylivingbase = this.dragon.worldObj.func_184142_a(this.dragon, 150.0D, 150.0D);
            this.dragon.getPhaseManager().func_188758_a(PhaseList.TAKEOFF);

            if (entitylivingbase != null)
            {
                this.dragon.getPhaseManager().func_188758_a(PhaseList.CHARGING_PLAYER);
                ((PhaseChargingPlayer)this.dragon.getPhaseManager().func_188757_b(PhaseList.CHARGING_PLAYER)).func_188668_a(new Vec3d(entitylivingbase.posX, entitylivingbase.posY, entitylivingbase.posZ));
            }
        }
    }

    public void func_188660_d()
    {
        this.field_188667_b = 0;
    }

    public PhaseList<PhaseSittingScanning> func_188652_i()
    {
        return PhaseList.SITTING_SCANNING;
    }
}
