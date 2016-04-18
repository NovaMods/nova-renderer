package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.init.SoundEvents;

public class PhaseSittingAttacking extends PhaseSittingBase
{
    private int field_188662_b;

    public PhaseSittingAttacking(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188657_b()
    {
        this.dragon.worldObj.func_184134_a(this.dragon.posX, this.dragon.posY, this.dragon.posZ, SoundEvents.entity_enderdragon_growl, this.dragon.getSoundCategory(), 2.5F, 0.8F + this.dragon.getRNG().nextFloat() * 0.3F, false);
    }

    public void func_188659_c()
    {
        if (this.field_188662_b++ >= 40)
        {
            this.dragon.getPhaseManager().func_188758_a(PhaseList.SITTING_FLAMING);
        }
    }

    public void func_188660_d()
    {
        this.field_188662_b = 0;
    }

    public PhaseList<PhaseSittingAttacking> func_188652_i()
    {
        return PhaseList.SITTING_ATTACKING;
    }
}
