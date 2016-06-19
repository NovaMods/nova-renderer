package net.minecraft.entity.ai;

import net.minecraft.entity.EntityCreature;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.passive.EntityTameable;
import net.minecraft.util.math.AxisAlignedBB;

public class EntityAIHurtByTarget extends EntityAITarget
{
    private final boolean entityCallsForHelp;

    /** Store the previous revengeTimer value */
    private int revengeTimerOld;
    private final Class<?>[] targetClasses;

    public EntityAIHurtByTarget(EntityCreature creatureIn, boolean entityCallsForHelpIn, Class<?>... targetClassesIn)
    {
        super(creatureIn, true);
        this.entityCallsForHelp = entityCallsForHelpIn;
        this.targetClasses = targetClassesIn;
        this.setMutexBits(1);
    }

    /**
     * Returns whether the EntityAIBase should begin execution.
     */
    public boolean shouldExecute()
    {
        int i = this.taskOwner.getRevengeTimer();
        EntityLivingBase entitylivingbase = this.taskOwner.getAITarget();
        return i != this.revengeTimerOld && entitylivingbase != null && this.isSuitableTarget(entitylivingbase, false);
    }

    /**
     * Execute a one shot task or start executing a continuous task
     */
    public void startExecuting()
    {
        this.taskOwner.setAttackTarget(this.taskOwner.getAITarget());
        this.field_188509_g = this.taskOwner.getAttackTarget();
        this.revengeTimerOld = this.taskOwner.getRevengeTimer();
        this.field_188510_h = 300;

        if (this.entityCallsForHelp)
        {
            double d0 = this.getTargetDistance();

            for (EntityCreature entitycreature : this.taskOwner.worldObj.getEntitiesWithinAABB(this.taskOwner.getClass(), (new AxisAlignedBB(this.taskOwner.posX, this.taskOwner.posY, this.taskOwner.posZ, this.taskOwner.posX + 1.0D, this.taskOwner.posY + 1.0D, this.taskOwner.posZ + 1.0D)).expand(d0, 10.0D, d0)))
            {
                if (this.taskOwner != entitycreature && entitycreature.getAttackTarget() == null && (!(this.taskOwner instanceof EntityTameable) || ((EntityTameable)this.taskOwner).getOwner() == ((EntityTameable)entitycreature).getOwner()) && !entitycreature.func_184191_r(this.taskOwner.getAITarget()))
                {
                    boolean flag = false;

                    for (Class<?> oclass : this.targetClasses)
                    {
                        if (entitycreature.getClass() == oclass)
                        {
                            flag = true;
                            break;
                        }
                    }

                    if (!flag)
                    {
                        this.setEntityAttackTarget(entitycreature, this.taskOwner.getAITarget());
                    }
                }
            }
        }

        super.startExecuting();
    }

    protected void setEntityAttackTarget(EntityCreature creatureIn, EntityLivingBase entityLivingBaseIn)
    {
        creatureIn.setAttackTarget(entityLivingBaseIn);
    }
}
