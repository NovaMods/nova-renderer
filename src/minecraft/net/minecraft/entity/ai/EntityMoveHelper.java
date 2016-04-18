package net.minecraft.entity.ai;

import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.pathfinding.PathNodeType;
import net.minecraft.pathfinding.WalkNodeProcessor;
import net.minecraft.util.math.MathHelper;

public class EntityMoveHelper
{
    /** The EntityLiving that is being moved */
    protected final EntityLiving entity;
    protected double posX;
    protected double posY;
    protected double posZ;

    /** The speed at which the entity should move */
    protected double speed;
    protected float field_188489_f;
    protected float field_188490_g;
    protected EntityMoveHelper.Action field_188491_h = EntityMoveHelper.Action.WAIT;

    public EntityMoveHelper(EntityLiving entitylivingIn)
    {
        this.entity = entitylivingIn;
    }

    public boolean isUpdating()
    {
        return this.field_188491_h == EntityMoveHelper.Action.MOVE_TO;
    }

    public double getSpeed()
    {
        return this.speed;
    }

    /**
     * Sets the speed and location to move to
     */
    public void setMoveTo(double x, double y, double z, double speedIn)
    {
        this.posX = x;
        this.posY = y;
        this.posZ = z;
        this.speed = speedIn;
        this.field_188491_h = EntityMoveHelper.Action.MOVE_TO;
    }

    public void func_188488_a(float p_188488_1_, float p_188488_2_)
    {
        this.field_188491_h = EntityMoveHelper.Action.STRAFE;
        this.field_188489_f = p_188488_1_;
        this.field_188490_g = p_188488_2_;
        this.speed = 0.25D;
    }

    public void func_188487_a(EntityMoveHelper p_188487_1_)
    {
        this.field_188491_h = p_188487_1_.field_188491_h;
        this.posX = p_188487_1_.posX;
        this.posY = p_188487_1_.posY;
        this.posZ = p_188487_1_.posZ;
        this.speed = Math.max(p_188487_1_.speed, 1.0D);
        this.field_188489_f = p_188487_1_.field_188489_f;
        this.field_188490_g = p_188487_1_.field_188490_g;
    }

    public void onUpdateMoveHelper()
    {
        if (this.field_188491_h == EntityMoveHelper.Action.STRAFE)
        {
            float f = (float)this.entity.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).getAttributeValue();
            float f1 = (float)this.speed * f;
            float f2 = this.field_188489_f;
            float f3 = this.field_188490_g;
            float f4 = MathHelper.sqrt_float(f2 * f2 + f3 * f3);

            if (f4 < 1.0F)
            {
                f4 = 1.0F;
            }

            f4 = f1 / f4;
            f2 = f2 * f4;
            f3 = f3 * f4;
            float f5 = MathHelper.sin(this.entity.rotationYaw * 0.017453292F);
            float f6 = MathHelper.cos(this.entity.rotationYaw * 0.017453292F);
            float f7 = f2 * f6 - f3 * f5;
            float f8 = f3 * f6 + f2 * f5;

            if (WalkNodeProcessor.func_186330_a(this.entity.worldObj, MathHelper.floor_double(this.entity.posX + (double)f7), MathHelper.floor_double(this.entity.posY), MathHelper.floor_double(this.entity.posZ + (double)f8)) != PathNodeType.WALKABLE)
            {
                this.field_188489_f = 1.0F;
                this.field_188490_g = 0.0F;
                f1 = f;
            }

            this.entity.setAIMoveSpeed(f1);
            this.entity.setMoveForward(this.field_188489_f);
            this.entity.func_184646_p(this.field_188490_g);
            this.field_188491_h = EntityMoveHelper.Action.WAIT;
        }
        else if (this.field_188491_h == EntityMoveHelper.Action.MOVE_TO)
        {
            this.field_188491_h = EntityMoveHelper.Action.WAIT;
            double d0 = this.posX - this.entity.posX;
            double d1 = this.posZ - this.entity.posZ;
            double d2 = this.posY - this.entity.posY;
            double d3 = d0 * d0 + d2 * d2 + d1 * d1;

            if (d3 < 2.500000277905201E-7D)
            {
                this.entity.setMoveForward(0.0F);
                return;
            }

            float f9 = (float)(MathHelper.atan2(d1, d0) * (180D / Math.PI)) - 90.0F;
            this.entity.rotationYaw = this.limitAngle(this.entity.rotationYaw, f9, 90.0F);
            this.entity.setAIMoveSpeed((float)(this.speed * this.entity.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).getAttributeValue()));

            if (d2 > (double)this.entity.stepHeight && d0 * d0 + d1 * d1 < 1.0D)
            {
                this.entity.getJumpHelper().setJumping();
            }
        }
        else
        {
            this.entity.setMoveForward(0.0F);
        }
    }

    /**
     * Limits the given angle to a upper and lower limit.
     */
    protected float limitAngle(float p_75639_1_, float p_75639_2_, float p_75639_3_)
    {
        float f = MathHelper.wrapAngleTo180_float(p_75639_2_ - p_75639_1_);

        if (f > p_75639_3_)
        {
            f = p_75639_3_;
        }

        if (f < -p_75639_3_)
        {
            f = -p_75639_3_;
        }

        float f1 = p_75639_1_ + f;

        if (f1 < 0.0F)
        {
            f1 += 360.0F;
        }
        else if (f1 > 360.0F)
        {
            f1 -= 360.0F;
        }

        return f1;
    }

    public double getX()
    {
        return this.posX;
    }

    public double getY()
    {
        return this.posY;
    }

    public double getZ()
    {
        return this.posZ;
    }

    public static enum Action
    {
        WAIT,
        MOVE_TO,
        STRAFE;
    }
}
