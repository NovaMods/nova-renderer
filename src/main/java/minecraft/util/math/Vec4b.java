package net.minecraft.util.math;

public class Vec4b
{
    private byte x;
    private byte y;
    private byte z;
    private byte w;

    public Vec4b(byte xIn, byte yIn, byte zIn, byte wIn)
    {
        this.x = xIn;
        this.y = yIn;
        this.z = zIn;
        this.w = wIn;
    }

    public Vec4b(Vec4b vec)
    {
        this.x = vec.x;
        this.y = vec.y;
        this.z = vec.z;
        this.w = vec.w;
    }

    public byte getX()
    {
        return this.x;
    }

    public byte getY()
    {
        return this.y;
    }

    public byte getZ()
    {
        return this.z;
    }

    public byte getW()
    {
        return this.w;
    }

    public boolean equals(Object p_equals_1_)
    {
        if (this == p_equals_1_)
        {
            return true;
        }
        else if (!(p_equals_1_ instanceof Vec4b))
        {
            return false;
        }
        else
        {
            Vec4b vec4b = (Vec4b)p_equals_1_;
            return this.x != vec4b.x ? false : (this.w != vec4b.w ? false : (this.y != vec4b.y ? false : this.z == vec4b.z));
        }
    }

    public int hashCode()
    {
        int i = this.x;
        i = 31 * i + this.y;
        i = 31 * i + this.z;
        i = 31 * i + this.w;
        return i;
    }
}
