package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityDragonFireball;
import net.minecraft.pathfinding.PathEntity;
import net.minecraft.pathfinding.PathPoint;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class PhaseStrafePlayer extends PhaseBase
{
    private static final Logger field_188689_b = LogManager.getLogger();
    private int field_188690_c;
    private PathEntity field_188691_d;
    private Vec3d field_188692_e;
    private EntityLivingBase field_188693_f;
    private boolean field_188694_g;

    public PhaseStrafePlayer(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188659_c()
    {
        if (this.field_188693_f == null)
        {
            field_188689_b.warn("Skipping player strafe phase because no player was found");
            this.dragon.getPhaseManager().func_188758_a(PhaseList.HOLDING_PATTERN);
        }
        else
        {
            if (this.field_188691_d != null && this.field_188691_d.isFinished())
            {
                double d0 = this.field_188693_f.posX;
                double d1 = this.field_188693_f.posZ;
                double d2 = d0 - this.dragon.posX;
                double d3 = d1 - this.dragon.posZ;
                double d4 = (double)MathHelper.sqrt_double(d2 * d2 + d3 * d3);
                double d5 = Math.min(0.4000000059604645D + d4 / 80.0D - 1.0D, 10.0D);
                this.field_188692_e = new Vec3d(d0, this.field_188693_f.posY + d5, d1);
            }

            double d12 = this.field_188692_e == null ? 0.0D : this.field_188692_e.func_186679_c(this.dragon.posX, this.dragon.posY, this.dragon.posZ);

            if (d12 < 100.0D || d12 > 22500.0D)
            {
                this.func_188687_j();
            }

            double d13 = 64.0D;

            if (this.field_188693_f.getDistanceSqToEntity(this.dragon) < d13 * d13)
            {
                if (this.dragon.canEntityBeSeen(this.field_188693_f))
                {
                    ++this.field_188690_c;
                    Vec3d vec3d1 = (new Vec3d(this.field_188693_f.posX - this.dragon.posX, 0.0D, this.field_188693_f.posZ - this.dragon.posZ)).normalize();
                    Vec3d vec3d = (new Vec3d((double)MathHelper.sin(this.dragon.rotationYaw * 0.017453292F), 0.0D, (double)(-MathHelper.cos(this.dragon.rotationYaw * 0.017453292F)))).normalize();
                    float f1 = (float)vec3d.dotProduct(vec3d1);
                    float f = (float)(Math.acos((double)f1) * (180D / Math.PI));
                    f = f + 0.5F;

                    if (this.field_188690_c >= 5 && f >= 0.0F && f < 10.0F)
                    {
                        double d14 = 1.0D;
                        Vec3d vec3d2 = this.dragon.getLook(1.0F);
                        double d6 = this.dragon.dragonPartHead.posX - vec3d2.xCoord * d14;
                        double d7 = this.dragon.dragonPartHead.posY + (double)(this.dragon.dragonPartHead.height / 2.0F) + 0.5D;
                        double d8 = this.dragon.dragonPartHead.posZ - vec3d2.zCoord * d14;
                        double d9 = this.field_188693_f.posX - d6;
                        double d10 = this.field_188693_f.posY + (double)(this.field_188693_f.height / 2.0F) - (d7 + (double)(this.dragon.dragonPartHead.height / 2.0F));
                        double d11 = this.field_188693_f.posZ - d8;
                        this.dragon.worldObj.playAuxSFXAtEntity((EntityPlayer)null, 1017, new BlockPos(this.dragon), 0);
                        EntityDragonFireball entitydragonfireball = new EntityDragonFireball(this.dragon.worldObj, this.dragon, d9, d10, d11);
                        entitydragonfireball.posX = d6;
                        entitydragonfireball.posY = d7;
                        entitydragonfireball.posZ = d8;
                        this.dragon.worldObj.spawnEntityInWorld(entitydragonfireball);
                        this.field_188690_c = 0;

                        if (this.field_188691_d != null)
                        {
                            while (!this.field_188691_d.isFinished())
                            {
                                this.field_188691_d.incrementPathIndex();
                            }
                        }

                        this.dragon.getPhaseManager().func_188758_a(PhaseList.HOLDING_PATTERN);
                    }
                }
                else if (this.field_188690_c > 0)
                {
                    --this.field_188690_c;
                }
            }
            else if (this.field_188690_c > 0)
            {
                --this.field_188690_c;
            }
        }
    }

    private void func_188687_j()
    {
        if (this.field_188691_d == null || this.field_188691_d.isFinished())
        {
            int i = this.dragon.func_184671_o();
            int j = i;

            if (this.dragon.getRNG().nextInt(8) == 0)
            {
                this.field_188694_g = !this.field_188694_g;
                j = i + 6;
            }

            if (this.field_188694_g)
            {
                ++j;
            }
            else
            {
                --j;
            }

            if (this.dragon.getFightManager() != null && this.dragon.getFightManager().func_186092_c() >= 0)
            {
                j = j % 12;

                if (j < 0)
                {
                    j += 12;
                }
            }
            else
            {
                j = j - 12;
                j = j & 7;
                j = j + 12;
            }

            this.field_188691_d = this.dragon.func_184666_a(i, j, (PathPoint)null);

            if (this.field_188691_d != null)
            {
                this.field_188691_d.incrementPathIndex();
            }
        }

        this.func_188688_k();
    }

    private void func_188688_k()
    {
        if (this.field_188691_d != null && !this.field_188691_d.isFinished())
        {
            Vec3d vec3d = this.field_188691_d.func_186310_f();
            this.field_188691_d.incrementPathIndex();
            double d0 = vec3d.xCoord;
            double d2 = vec3d.zCoord;
            double d1;

            while (true)
            {
                d1 = vec3d.yCoord + (double)(this.dragon.getRNG().nextFloat() * 20.0F);

                if (d1 >= vec3d.yCoord)
                {
                    break;
                }
            }

            this.field_188692_e = new Vec3d(d0, d1, d2);
        }
    }

    public void func_188660_d()
    {
        this.field_188690_c = 0;
        this.field_188692_e = null;
        this.field_188691_d = null;
        this.field_188693_f = null;
    }

    public void func_188686_a(EntityLivingBase p_188686_1_)
    {
        this.field_188693_f = p_188686_1_;
        int i = this.dragon.func_184671_o();
        int j = this.dragon.func_184663_l(this.field_188693_f.posX, this.field_188693_f.posY, this.field_188693_f.posZ);
        int k = MathHelper.floor_double(this.field_188693_f.posX);
        int l = MathHelper.floor_double(this.field_188693_f.posZ);
        double d0 = (double)k - this.dragon.posX;
        double d1 = (double)l - this.dragon.posZ;
        double d2 = (double)MathHelper.sqrt_double(d0 * d0 + d1 * d1);
        double d3 = Math.min(0.4000000059604645D + d2 / 80.0D - 1.0D, 10.0D);
        int i1 = MathHelper.floor_double(this.field_188693_f.posY + d3);
        PathPoint pathpoint = new PathPoint(k, i1, l);
        this.field_188691_d = this.dragon.func_184666_a(i, j, pathpoint);

        if (this.field_188691_d != null)
        {
            this.field_188691_d.incrementPathIndex();
            this.func_188688_k();
        }
    }

    public Vec3d func_188650_g()
    {
        return this.field_188692_e;
    }

    public PhaseList<PhaseStrafePlayer> func_188652_i()
    {
        return PhaseList.STRAFE_PLAYER;
    }
}
