package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.pathfinding.PathEntity;
import net.minecraft.pathfinding.PathPoint;
import net.minecraft.util.DamageSource;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.gen.feature.WorldGenEndPodium;

public class PhaseHoldingPattern extends PhaseBase
{
    private PathEntity field_188677_b;
    private Vec3d field_188678_c;
    private boolean field_188679_d;

    public PhaseHoldingPattern(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public PhaseList<PhaseHoldingPattern> func_188652_i()
    {
        return PhaseList.HOLDING_PATTERN;
    }

    public void func_188659_c()
    {
        double d0 = this.field_188678_c == null ? 0.0D : this.field_188678_c.func_186679_c(this.dragon.posX, this.dragon.posY, this.dragon.posZ);

        if (d0 < 100.0D || d0 > 22500.0D || this.dragon.isCollidedHorizontally || this.dragon.isCollidedVertically)
        {
            this.func_188675_j();
        }
    }

    public void func_188660_d()
    {
        this.field_188677_b = null;
        this.field_188678_c = null;
    }

    public Vec3d func_188650_g()
    {
        return this.field_188678_c;
    }

    private void func_188675_j()
    {
        if (this.field_188677_b != null && this.field_188677_b.isFinished())
        {
            BlockPos blockpos = this.dragon.worldObj.getTopSolidOrLiquidBlock(new BlockPos(WorldGenEndPodium.field_186139_a));
            int i = this.dragon.getFightManager() == null ? 0 : this.dragon.getFightManager().func_186092_c();

            if (this.dragon.getRNG().nextInt(i + 3) == 0)
            {
                this.dragon.getPhaseManager().func_188758_a(PhaseList.LANDING_APPROACH);
                return;
            }

            double d0 = 64.0D;
            EntityPlayer entityplayer = this.dragon.worldObj.func_184139_a(blockpos, d0, d0);

            if (entityplayer != null)
            {
                d0 = entityplayer.getDistanceSqToCenter(blockpos) / 512.0D;
            }

            if (entityplayer != null && (this.dragon.getRNG().nextInt(MathHelper.abs_int((int)d0) + 2) == 0 || this.dragon.getRNG().nextInt(i + 2) == 0))
            {
                this.func_188674_a(entityplayer);
                return;
            }
        }

        if (this.field_188677_b == null || this.field_188677_b.isFinished())
        {
            int j = this.dragon.func_184671_o();
            int k = j;

            if (this.dragon.getRNG().nextInt(8) == 0)
            {
                this.field_188679_d = !this.field_188679_d;
                k = j + 6;
            }

            if (this.field_188679_d)
            {
                ++k;
            }
            else
            {
                --k;
            }

            if (this.dragon.getFightManager() != null && this.dragon.getFightManager().func_186092_c() >= 0)
            {
                k = k % 12;

                if (k < 0)
                {
                    k += 12;
                }
            }
            else
            {
                k = k - 12;
                k = k & 7;
                k = k + 12;
            }

            this.field_188677_b = this.dragon.func_184666_a(j, k, (PathPoint)null);

            if (this.field_188677_b != null)
            {
                this.field_188677_b.incrementPathIndex();
            }
        }

        this.func_188676_k();
    }

    private void func_188674_a(EntityPlayer p_188674_1_)
    {
        this.dragon.getPhaseManager().func_188758_a(PhaseList.STRAFE_PLAYER);
        ((PhaseStrafePlayer)this.dragon.getPhaseManager().func_188757_b(PhaseList.STRAFE_PLAYER)).func_188686_a(p_188674_1_);
    }

    private void func_188676_k()
    {
        if (this.field_188677_b != null && !this.field_188677_b.isFinished())
        {
            Vec3d vec3d = this.field_188677_b.func_186310_f();
            this.field_188677_b.incrementPathIndex();
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

            this.field_188678_c = new Vec3d(d0, d2, d1);
        }
    }

    public void func_188655_a(EntityEnderCrystal p_188655_1_, BlockPos p_188655_2_, DamageSource p_188655_3_, EntityPlayer p_188655_4_)
    {
        if (p_188655_4_ != null)
        {
            this.func_188674_a(p_188655_4_);
        }
    }
}
