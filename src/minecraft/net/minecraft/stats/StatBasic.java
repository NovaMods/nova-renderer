package net.minecraft.stats;

import net.minecraft.util.text.ITextComponent;

public class StatBasic extends StatBase
{
    public StatBasic(String statIdIn, ITextComponent statNameIn, IStatType typeIn)
    {
        super(statIdIn, statNameIn, typeIn);
    }

    public StatBasic(String statIdIn, ITextComponent statNameIn)
    {
        super(statIdIn, statNameIn);
    }

    /**
     * Register the stat into StatList.
     */
    public StatBase registerStat()
    {
        super.registerStat();
        StatList.field_188094_c.add(this);
        return this;
    }
}
