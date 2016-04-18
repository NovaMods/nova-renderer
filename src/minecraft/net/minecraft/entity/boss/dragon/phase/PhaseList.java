package net.minecraft.entity.boss.dragon.phase;

import java.lang.reflect.Constructor;
import java.util.Arrays;
import net.minecraft.entity.boss.EntityDragon;

public class PhaseList<T extends IPhase>
{
    private static PhaseList<?>[] phases = new PhaseList[0];
    public static final PhaseList<PhaseHoldingPattern> HOLDING_PATTERN = func_188735_a(PhaseHoldingPattern.class, "HoldingPattern");
    public static final PhaseList<PhaseStrafePlayer> STRAFE_PLAYER = func_188735_a(PhaseStrafePlayer.class, "StrafePlayer");
    public static final PhaseList<PhaseLandingApproach> LANDING_APPROACH = func_188735_a(PhaseLandingApproach.class, "LandingApproach");
    public static final PhaseList<PhaseLanding> LANDING = func_188735_a(PhaseLanding.class, "Landing");
    public static final PhaseList<PhaseTakeoff> TAKEOFF = func_188735_a(PhaseTakeoff.class, "Takeoff");
    public static final PhaseList<PhaseSittingFlaming> SITTING_FLAMING = func_188735_a(PhaseSittingFlaming.class, "SittingFlaming");
    public static final PhaseList<PhaseSittingScanning> SITTING_SCANNING = func_188735_a(PhaseSittingScanning.class, "SittingScanning");
    public static final PhaseList<PhaseSittingAttacking> SITTING_ATTACKING = func_188735_a(PhaseSittingAttacking.class, "SittingAttacking");
    public static final PhaseList<PhaseChargingPlayer> CHARGING_PLAYER = func_188735_a(PhaseChargingPlayer.class, "ChargingPlayer");
    public static final PhaseList<PhaseDying> DYING = func_188735_a(PhaseDying.class, "Dying");
    public static final PhaseList<PhaseHover> HOVER = func_188735_a(PhaseHover.class, "Hover");
    private final Class <? extends IPhase > clazz;
    private final int id;
    private final String name;

    private PhaseList(int idIn, Class <? extends IPhase > clazzIn, String nameIn)
    {
        this.id = idIn;
        this.clazz = clazzIn;
        this.name = nameIn;
    }

    public IPhase func_188736_a(EntityDragon p_188736_1_)
    {
        try
        {
            Constructor <? extends IPhase > constructor = this.func_188737_a();
            return (IPhase)constructor.newInstance(new Object[] {p_188736_1_});
        }
        catch (Exception exception)
        {
            throw new Error(exception);
        }
    }

    protected Constructor <? extends IPhase > func_188737_a() throws NoSuchMethodException
    {
        return this.clazz.getConstructor(new Class[] {EntityDragon.class});
    }

    public int getId()
    {
        return this.id;
    }

    public String toString()
    {
        return this.name + " (#" + this.id + ")";
    }

    public static PhaseList<?> func_188738_a(int p_188738_0_)
    {
        return p_188738_0_ >= 0 && p_188738_0_ < phases.length ? phases[p_188738_0_] : HOLDING_PATTERN;
    }

    public static int func_188739_c()
    {
        return phases.length;
    }

    private static <T extends IPhase> PhaseList<T> func_188735_a(Class<T> p_188735_0_, String p_188735_1_)
    {
        PhaseList<T> phaselist = new PhaseList(phases.length, p_188735_0_, p_188735_1_);
        phases = (PhaseList[])Arrays.copyOf(phases, phases.length + 1);
        phases[phaselist.getId()] = phaselist;
        return phaselist;
    }
}
