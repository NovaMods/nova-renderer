package net.minecraft.client.audio;

import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;

public interface ISound
{
    ResourceLocation getSoundLocation();

    SoundEventAccessor func_184366_a(SoundHandler p_184366_1_);

    Sound func_184364_b();

    SoundCategory func_184365_d();

    boolean canRepeat();

    int getRepeatDelay();

    float getVolume();

    float getPitch();

    float getXPosF();

    float getYPosF();

    float getZPosF();

    ISound.AttenuationType getAttenuationType();

    public static enum AttenuationType
    {
        NONE(0),
        LINEAR(2);

        private final int type;

        private AttenuationType(int typeIn)
        {
            this.type = typeIn;
        }

        public int getTypeInt()
        {
            return this.type;
        }
    }
}
