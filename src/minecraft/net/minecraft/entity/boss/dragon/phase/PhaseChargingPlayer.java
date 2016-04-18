package net.minecraft.entity.boss.dragon.phase;

import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.util.math.Vec3d;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class PhaseChargingPlayer extends PhaseBase
{
    private static final Logger field_188669_b = LogManager.getLogger();
    private Vec3d field_188670_c;
    private int field_188671_d = 0;

    public PhaseChargingPlayer(EntityDragon dragonIn)
    {
        super(dragonIn);
    }

    public void func_188659_c()
    {
        if (this.field_188670_c == null)
        {
            field_188669_b.warn("Aborting charge player as no target was set.");
            this.dragon.getPhaseManager().func_188758_a(PhaseList.HOLDING_PATTERN);
        }
        else if (this.field_188671_d > 0 && this.field_188671_d++ >= 10)
        {
            this.dragon.getPhaseManager().func_188758_a(PhaseList.HOLDING_PATTERN);
        }
        else
        {
            double d0 = this.field_188670_c.func_186679_c(this.dragon.posX, this.dragon.posY, this.dragon.posZ);

            if (d0 < 100.0D || d0 > 22500.0D || this.dragon.isCollidedHorizontally || this.dragon.isCollidedVertically)
            {
                ++this.field_188671_d;
            }
        }
    }

    public void func_188660_d()
    {
        this.field_188670_c = null;
        this.field_188671_d = 0;
    }

    public void func_188668_a(Vec3d p_188668_1_)
    {
        this.field_188670_c = p_188668_1_;
    }

    public float func_188651_f()
    {
        return 3.0F;
    }

    public Vec3d func_188650_g()
    {
        return this.field_188670_c;
    }

    public PhaseList<PhaseChargingPlayer> func_188652_i()
    {
        return PhaseList.CHARGING_PLAYER;
    }
}
