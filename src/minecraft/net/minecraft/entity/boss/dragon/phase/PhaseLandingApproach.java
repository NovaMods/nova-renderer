package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.pathfinding.PathEntity;
import net.minecraft.pathfinding.PathPoint;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.gen.feature.WorldGenEndPodium;

public class PhaseLandingApproach extends PhaseBase
{
    private PathEntity field_188683_b;
    private Vec3d field_188684_c;

    public PhaseLandingApproach(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public PhaseList<PhaseLandingApproach> func_188652_i()
    {
        return PhaseList.LANDING_APPROACH;
    }

    public void func_188660_d()
    {
        this.field_188683_b = null;
        this.field_188684_c = null;
    }

    public void func_188659_c()
    {
        double d0 = this.field_188684_c == null ? 0.0D : this.field_188684_c.func_186679_c(this.dragon.posX, this.dragon.posY, this.dragon.posZ);

        if (d0 < 100.0D || d0 > 22500.0D || this.dragon.isCollidedHorizontally || this.dragon.isCollidedVertically)
        {
            this.func_188681_j();
        }
    }

    public Vec3d func_188650_g()
    {
        return this.field_188684_c;
    }

    private void func_188681_j()
    {
        if (this.field_188683_b == null || this.field_188683_b.isFinished())
        {
            int i = this.dragon.func_184671_o();
            BlockPos blockpos = this.dragon.worldObj.getTopSolidOrLiquidBlock(WorldGenEndPodium.field_186139_a);
            EntityPlayer entityplayer = this.dragon.worldObj.func_184139_a(blockpos, 128.0D, 128.0D);
            int j;

            if (entityplayer != null)
            {
                Vec3d vec3d = (new Vec3d(entityplayer.posX, 0.0D, entityplayer.posZ)).normalize();
                j = this.dragon.func_184663_l(-vec3d.xCoord * 40.0D, 105.0D, -vec3d.zCoord * 40.0D);
            }
            else
            {
                j = this.dragon.func_184663_l(40.0D, (double)blockpos.getY(), 0.0D);
            }

            PathPoint pathpoint = new PathPoint(blockpos.getX(), blockpos.getY(), blockpos.getZ());
            this.field_188683_b = this.dragon.func_184666_a(i, j, pathpoint);

            if (this.field_188683_b != null)
            {
                this.field_188683_b.incrementPathIndex();
            }
        }

        this.func_188682_k();

        if (this.field_188683_b != null && this.field_188683_b.isFinished())
        {
            this.dragon.getPhaseManager().func_188758_a(PhaseList.LANDING);
        }
    }

    private void func_188682_k()
    {
        if (this.field_188683_b != null && !this.field_188683_b.isFinished())
        {
            Vec3d vec3d = this.field_188683_b.func_186310_f();
            this.field_188683_b.incrementPathIndex();
            double d0 = vec3d.xCoord;
            double d1 = vec3d.zCoord;
            double d2;

            while (true)
            {
                d2 = vec3d.yCoord + (double)(this.dragon.getRNG().nextFloat() * 20.0F);

                if (d2 >= vec3d.yCoord)
                {
                    break;
                }
            }

            this.field_188684_c = new Vec3d(d0, d2, d1);
        }
    }
}
