package net.minecraft.client.audio;

import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;

public abstract class PositionedSound implements ISound
{
    protected Sound field_184367_a;
    private SoundEventAccessor field_184369_l;
    protected SoundCategory field_184368_b;
    protected ResourceLocation positionedSoundLocation;
    protected float volume;
    protected float pitch;
    protected float xPosF;
    protected float yPosF;
    protected float zPosF;
    protected boolean repeat;

    /** The number of ticks between repeating the sound */
    protected int repeatDelay;
    protected ISound.AttenuationType attenuationType;

    protected PositionedSound(SoundEvent p_i46533_1_, SoundCategory p_i46533_2_)
    {
        this(p_i46533_1_.func_187503_a(), p_i46533_2_);
    }

    protected PositionedSound(ResourceLocation p_i46534_1_, SoundCategory p_i46534_2_)
    {
        this.volume = 1.0F;
        this.pitch = 1.0F;
        this.attenuationType = ISound.AttenuationType.LINEAR;
        this.positionedSoundLocation = p_i46534_1_;
        this.field_184368_b = p_i46534_2_;
    }

    public ResourceLocation getSoundLocation()
    {
        return this.positionedSoundLocation;
    }

    public SoundEventAccessor func_184366_a(SoundHandler p_184366_1_)
    {
        this.field_184369_l = p_184366_1_.func_184398_a(this.positionedSoundLocation);

        if (this.field_184369_l == null)
        {
            this.field_184367_a = SoundHandler.missing_sound;
        }
        else
        {
            this.field_184367_a = this.field_184369_l.cloneEntry();
        }

        return this.field_184369_l;
    }

    public Sound func_184364_b()
    {
        return this.field_184367_a;
    }

    public SoundCategory func_184365_d()
    {
        return this.field_184368_b;
    }

    public boolean canRepeat()
    {
        return this.repeat;
    }

    public int getRepeatDelay()
    {
        return this.repeatDelay;
    }

    public float getVolume()
    {
        return this.volume * this.field_184367_a.func_188724_c();
    }

    public float getPitch()
    {
        return this.pitch * this.field_184367_a.func_188725_d();
    }

    public float getXPosF()
    {
        return this.xPosF;
    }

    public float getYPosF()
    {
        return this.yPosF;
    }

    public float getZPosF()
    {
        return this.zPosF;
    }

    public ISound.AttenuationType getAttenuationType()
    {
        return this.attenuationType;
    }
}
