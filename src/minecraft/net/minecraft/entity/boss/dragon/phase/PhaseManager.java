package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class PhaseManager
{
    private static final Logger LOGGER = LogManager.getLogger();
    private final EntityDragon dragon;
    private final IPhase[] field_188761_c = new IPhase[PhaseList.func_188739_c()];
    private IPhase field_188762_d;

    public PhaseManager(EntityDragon dragonIn)
    {
        this.dragon = dragonIn;
        this.func_188758_a(PhaseList.HOVER);
    }

    public void func_188758_a(PhaseList<?> p_188758_1_)
    {
        if (this.field_188762_d == null || p_188758_1_ != this.field_188762_d.func_188652_i())
        {
            if (this.field_188762_d != null)
            {
                this.field_188762_d.func_188658_e();
            }

            this.field_188762_d = this.func_188757_b(p_188758_1_);

            if (!this.dragon.worldObj.isRemote)
            {
                this.dragon.getDataManager().set(EntityDragon.PHASE, Integer.valueOf(p_188758_1_.getId()));
            }

            LOGGER.debug("Dragon is now in phase {} on the {}", new Object[] {p_188758_1_, this.dragon.worldObj.isRemote ? "client" : "server"});
            this.field_188762_d.func_188660_d();
        }
    }

    public IPhase func_188756_a()
    {
        return this.field_188762_d;
    }

    public <T extends IPhase> T func_188757_b(PhaseList<T> p_188757_1_)
    {
        int i = p_188757_1_.getId();

        if (this.field_188761_c[i] == null)
        {
            this.field_188761_c[i] = p_188757_1_.func_188736_a(this.dragon);
        }

        return (T)this.field_188761_c[i];
    }
}
