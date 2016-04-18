package net.minecraft.entity.ai;

import net.minecraft.entity.monster.EntityZombie;

public class EntityAIZombieAttack extends EntityAIAttackMelee
{
    private final EntityZombie field_188494_h;
    private int field_188495_i;

    public EntityAIZombieAttack(EntityZombie p_i46803_1_, double p_i46803_2_, boolean p_i46803_4_)
    {
        super(p_i46803_1_, p_i46803_2_, p_i46803_4_);
        this.field_188494_h = p_i46803_1_;
    }

    /**
     * Execute a one shot task or start executing a continuous task
     */
    public void startExecuting()
    {
        super.startExecuting();
        this.field_188495_i = 0;
    }

    /**
     * Resets the task
     */
    public void resetTask()
    {
        super.resetTask();
        this.field_188494_h.setArmsRaised(false);
    }

    /**
     * Updates the task
     */
    public void updateTask()
    {
        super.updateTask();
        ++this.field_188495_i;

        if (this.field_188495_i >= 5 && this.attackTick < 10)
        {
            this.field_188494_h.setArmsRaised(true);
        }
        else
        {
            this.field_188494_h.setArmsRaised(false);
        }
    }
}
