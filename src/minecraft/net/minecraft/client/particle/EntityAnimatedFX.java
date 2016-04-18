package net.minecraft.client.particle;

import net.minecraft.world.World;

public class EntityAnimatedFX extends EntityFX
{
    private final int field_187147_a;
    private final int field_187148_G;
    private final float field_187149_H;
    private float field_187150_I;
    private float field_187151_J;
    private float field_187152_K;
    private boolean field_187153_L;

    public EntityAnimatedFX(World p_i46578_1_, double p_i46578_2_, double p_i46578_4_, double p_i46578_6_, int p_i46578_8_, int p_i46578_9_, float p_i46578_10_)
    {
        super(p_i46578_1_, p_i46578_2_, p_i46578_4_, p_i46578_6_);
        this.field_187147_a = p_i46578_8_;
        this.field_187148_G = p_i46578_9_;
        this.field_187149_H = p_i46578_10_;
    }

    public void func_187146_c(int p_187146_1_)
    {
        float f = (float)((p_187146_1_ & 16711680) >> 16) / 255.0F;
        float f1 = (float)((p_187146_1_ & 65280) >> 8) / 255.0F;
        float f2 = (float)((p_187146_1_ & 255) >> 0) / 255.0F;
        float f3 = 1.0F;
        this.setRBGColorF(f * f3, f1 * f3, f2 * f3);
    }

    public void func_187145_d(int p_187145_1_)
    {
        this.field_187150_I = (float)((p_187145_1_ & 16711680) >> 16) / 255.0F;
        this.field_187151_J = (float)((p_187145_1_ & 65280) >> 8) / 255.0F;
        this.field_187152_K = (float)((p_187145_1_ & 255) >> 0) / 255.0F;
        this.field_187153_L = true;
    }

    public boolean func_187111_c()
    {
        return true;
    }

    public void func_189213_a()
    {
        this.field_187123_c = this.field_187126_f;
        this.field_187124_d = this.field_187127_g;
        this.field_187125_e = this.field_187128_h;

        if (this.particleAge++ >= this.particleMaxAge)
        {
            this.func_187112_i();
        }

        if (this.particleAge > this.particleMaxAge / 2)
        {
            this.setAlphaF(1.0F - ((float)this.particleAge - (float)(this.particleMaxAge / 2)) / (float)this.particleMaxAge);

            if (this.field_187153_L)
            {
                this.particleRed += (this.field_187150_I - this.particleRed) * 0.2F;
                this.particleGreen += (this.field_187151_J - this.particleGreen) * 0.2F;
                this.particleBlue += (this.field_187152_K - this.particleBlue) * 0.2F;
            }
        }

        this.setParticleTextureIndex(this.field_187147_a + (this.field_187148_G - 1 - this.particleAge * this.field_187148_G / this.particleMaxAge));
        this.field_187130_j += (double)this.field_187149_H;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.9100000262260437D;
        this.field_187130_j *= 0.9100000262260437D;
        this.field_187131_k *= 0.9100000262260437D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    public int func_189214_a(float p_189214_1_)
    {
        return 15728880;
    }
}
