package net.minecraft.client.audio;

import com.google.common.collect.Lists;
import java.util.List;
import java.util.Random;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentTranslation;

public class SoundEventAccessor implements ISoundEventAccessor<Sound>
{
    private final List<ISoundEventAccessor<Sound>> field_188716_a = Lists.<ISoundEventAccessor<Sound>>newArrayList();
    private final Random rnd = new Random();
    private final ResourceLocation field_188717_c;
    private final ITextComponent field_188718_d;

    public SoundEventAccessor(ResourceLocation p_i46521_1_, String p_i46521_2_)
    {
        this.field_188717_c = p_i46521_1_;
        this.field_188718_d = p_i46521_2_ == null ? null : new TextComponentTranslation(p_i46521_2_, new Object[0]);
    }

    public int getWeight()
    {
        int i = 0;

        for (ISoundEventAccessor<Sound> isoundeventaccessor : this.field_188716_a)
        {
            i += isoundeventaccessor.getWeight();
        }

        return i;
    }

    public Sound cloneEntry()
    {
        int i = this.getWeight();

        if (!this.field_188716_a.isEmpty() && i != 0)
        {
            int j = this.rnd.nextInt(i);

            for (ISoundEventAccessor<Sound> isoundeventaccessor : this.field_188716_a)
            {
                j -= isoundeventaccessor.getWeight();

                if (j < 0)
                {
                    return (Sound)isoundeventaccessor.cloneEntry();
                }
            }

            return SoundHandler.missing_sound;
        }
        else
        {
            return SoundHandler.missing_sound;
        }
    }

    public void func_188715_a(ISoundEventAccessor<Sound> p_188715_1_)
    {
        this.field_188716_a.add(p_188715_1_);
    }

    public ResourceLocation func_188714_b()
    {
        return this.field_188717_c;
    }

    public ITextComponent func_188712_c()
    {
        return this.field_188718_d;
    }
}
