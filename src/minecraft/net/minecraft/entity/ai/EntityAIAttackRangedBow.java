package net.minecraft.entity.ai;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.monster.EntitySkeleton;
import net.minecraft.init.Items;
import net.minecraft.item.ItemBow;
import net.minecraft.util.EnumHand;

public class EntityAIAttackRangedBow extends EntityAIBase
{
    private final EntitySkeleton field_188499_a;
    private final double field_188500_b;
    private final int field_188501_c;
    private final float field_188502_d;
    private int field_188503_e = -1;
    private int field_188504_f;
    private boolean field_188505_g;
    private boolean field_188506_h;
    private int field_188507_i = -1;

    public EntityAIAttackRangedBow(EntitySkeleton p_i46805_1_, double p_i46805_2_, int p_i46805_4_, float p_i46805_5_)
    {
        this.field_188499_a = p_i46805_1_;
        this.field_188500_b = p_i46805_2_;
        this.field_188501_c = p_i46805_4_;
        this.field_188502_d = p_i46805_5_ * p_i46805_5_;
        this.setMutexBits(3);
    }

    /**
     * Returns whether the EntityAIBase should begin execution.
     */
    public boolean shouldExecute()
    {
        return this.field_188499_a.getAttackTarget() == null ? false : this.func_188498_f();
    }

    protected boolean func_188498_f()
    {
        return this.field_188499_a.getHeldItemMainhand() != null && this.field_188499_a.getHeldItemMainhand().getItem() == Items.bow;
    }

    /**
     * Returns whether an in-progress EntityAIBase should continue executing
     */
    public boolean continueExecuting()
    {
        return (this.shouldExecute() || !this.field_188499_a.getNavigator().noPath()) && this.func_188498_f();
    }

    /**
     * Execute a one shot task or start executing a continuous task
     */
    public void startExecuting()
    {
        super.startExecuting();
        this.field_188499_a.func_184724_a(true);
    }

    /**
     * Resets the task
     */
    public void resetTask()
    {
        super.startExecuting();
        this.field_188499_a.func_184724_a(false);
        this.field_188504_f = 0;
        this.field_188503_e = -1;
        this.field_188499_a.func_184602_cy();
    }

    /**
     * Updates the task
     */
    public void updateTask()
    {
        EntityLivingBase entitylivingbase = this.field_188499_a.getAttackTarget();

        if (entitylivingbase != null)
        {
            double d0 = this.field_188499_a.getDistanceSq(entitylivingbase.posX, entitylivingbase.getEntityBoundingBox().minY, entitylivingbase.posZ);
            boolean flag = this.field_188499_a.getEntitySenses().canSee(entitylivingbase);
            boolean flag1 = this.field_188504_f > 0;

            if (flag != flag1)
            {
                this.field_188504_f = 0;
            }

            if (flag)
            {
                ++this.field_188504_f;
            }
            else
            {
                --this.field_188504_f;
            }

            if (d0 <= (double)this.field_188502_d && this.field_188504_f >= 20)
            {
                this.field_188499_a.getNavigator().clearPathEntity();
                ++this.field_188507_i;
            }
            else
            {
                this.field_188499_a.getNavigator().tryMoveToEntityLiving(entitylivingbase, this.field_188500_b);
                this.field_188507_i = -1;
            }

            if (this.field_188507_i >= 20)
            {
                if ((double)this.field_188499_a.getRNG().nextFloat() < 0.3D)
                {
                    this.field_188505_g = !this.field_188505_g;
                }

                if ((double)this.field_188499_a.getRNG().nextFloat() < 0.3D)
                {
                    this.field_188506_h = !this.field_188506_h;
                }

                this.field_188507_i = 0;
            }

            if (this.field_188507_i > -1)
            {
                if (d0 > (double)(this.field_188502_d * 0.75F))
                {
                    this.field_188506_h = false;
                }
                else if (d0 < (double)(this.field_188502_d * 0.25F))
                {
                    this.field_188506_h = true;
                }

                this.field_188499_a.getMoveHelper().func_188488_a(this.field_188506_h ? -0.5F : 0.5F, this.field_188505_g ? 0.5F : -0.5F);
                this.field_188499_a.faceEntity(entitylivingbase, 30.0F, 30.0F);
            }
            else
            {
                this.field_188499_a.getLookHelper().setLookPositionWithEntity(entitylivingbase, 30.0F, 30.0F);
            }

            if (this.field_188499_a.func_184587_cr())
            {
                if (!flag && this.field_188504_f < -60)
                {
                    this.field_188499_a.func_184602_cy();
                }
                else if (flag)
                {
                    int i = this.field_188499_a.func_184612_cw();

                    if (i >= 20)
                    {
                        this.field_188499_a.func_184602_cy();
                        this.field_188499_a.attackEntityWithRangedAttack(entitylivingbase, ItemBow.func_185059_b(i));
                        this.field_188503_e = this.field_188501_c;
                    }
                }
            }
            else if (--this.field_188503_e <= 0 && this.field_188504_f >= -60)
            {
                this.field_188499_a.func_184598_c(EnumHand.MAIN_HAND);
            }
        }
    }
}
