package net.minecraft.util;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.Set;

public enum SoundCategory
{
    MASTER("master"),
    MUSIC("music"),
    RECORDS("record"),
    WEATHER("weather"),
    BLOCKS("block"),
    HOSTILE("hostile"),
    NEUTRAL("neutral"),
    PLAYERS("player"),
    AMBIENT("ambient"),
    VOICE("voice");

    private static final Map<String, SoundCategory> field_187961_k = Maps.<String, SoundCategory>newHashMap();
    private final String name;

    private SoundCategory(String nameIn)
    {
        this.name = nameIn;
    }

    public String getName()
    {
        return this.name;
    }

    public static SoundCategory func_187950_a(String p_187950_0_)
    {
        return (SoundCategory)field_187961_k.get(p_187950_0_);
    }

    public static Set<String> func_187949_b()
    {
        return field_187961_k.keySet();
    }

    static {
        for (SoundCategory soundcategory : values())
        {
            if (field_187961_k.containsKey(soundcategory.getName()))
            {
                throw new Error("Clash in Sound Category name pools! Cannot insert " + soundcategory);
            }

            field_187961_k.put(soundcategory.getName(), soundcategory);
        }
    }
}
