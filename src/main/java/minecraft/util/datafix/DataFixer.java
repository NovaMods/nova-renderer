package net.minecraft.util.datafix;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import java.util.List;
import java.util.Map;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.Util;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class DataFixer implements IDataFixer
{
    private static final Logger LOGGER = LogManager.getLogger();
    private final Map<IFixType, List<IDataWalker>> field_188260_b = Maps.<IFixType, List<IDataWalker>>newHashMap();
    private final Map<IFixType, List<IFixableData>> field_188261_c = Maps.<IFixType, List<IFixableData>>newHashMap();
    private final int field_188262_d;

    public DataFixer(int p_i46828_1_)
    {
        this.field_188262_d = p_i46828_1_;
    }

    public NBTTagCompound func_188257_a(IFixType type, NBTTagCompound compound)
    {
        int i = compound.hasKey("DataVersion", 99) ? compound.getInteger("DataVersion") : -1;
        return i >= 169 ? compound : this.func_188251_a(type, compound, i);
    }

    public NBTTagCompound func_188251_a(IFixType type, NBTTagCompound compound, int p_188251_3_)
    {
        if (p_188251_3_ < this.field_188262_d)
        {
            compound = this.func_188252_b(type, compound, p_188251_3_);
            compound = this.func_188253_c(type, compound, p_188251_3_);
        }

        return compound;
    }

    private NBTTagCompound func_188252_b(IFixType type, NBTTagCompound compound, int p_188252_3_)
    {
        List<IFixableData> list = (List)this.field_188261_c.get(type);

        if (list != null)
        {
            for (int i = 0; i < list.size(); ++i)
            {
                IFixableData ifixabledata = (IFixableData)list.get(i);

                if (ifixabledata.getFixVersion() > p_188252_3_)
                {
                    compound = ifixabledata.fixTagCompound(compound);
                }
            }
        }

        return compound;
    }

    private NBTTagCompound func_188253_c(IFixType type, NBTTagCompound compound, int p_188253_3_)
    {
        List<IDataWalker> list = (List)this.field_188260_b.get(type);

        if (list != null)
        {
            for (int i = 0; i < list.size(); ++i)
            {
                compound = ((IDataWalker)list.get(i)).func_188266_a(this, compound, p_188253_3_);
            }
        }

        return compound;
    }

    public void func_188258_a(FixTypes type, IDataWalker walker)
    {
        this.func_188255_a(type, walker);
    }

    public void func_188255_a(IFixType type, IDataWalker walker)
    {
        this.func_188254_a(this.field_188260_b, type).add(walker);
    }

    public void func_188256_a(IFixType type, IFixableData fixable)
    {
        List<IFixableData> list = this.<IFixableData>func_188254_a(this.field_188261_c, type);
        int i = fixable.getFixVersion();

        if (i > this.field_188262_d)
        {
            LOGGER.warn("Ignored fix registered for version: {} as the DataVersion of the game is: {}", new Object[] {Integer.valueOf(i), Integer.valueOf(this.field_188262_d)});
        }
        else
        {
            if (!list.isEmpty() && ((IFixableData)Util.getLastElement(list)).getFixVersion() > i)
            {
                for (int j = 0; j < list.size(); ++j)
                {
                    if (((IFixableData)list.get(j)).getFixVersion() > i)
                    {
                        list.add(j, fixable);
                        break;
                    }
                }
            }
            else
            {
                list.add(fixable);
            }
        }
    }

    private <V> List<V> func_188254_a(Map<IFixType, List<V>> p_188254_1_, IFixType type)
    {
        List<V> list = (List)p_188254_1_.get(type);

        if (list == null)
        {
            list = Lists.<V>newArrayList();
            p_188254_1_.put(type, list);
        }

        return list;
    }
}
