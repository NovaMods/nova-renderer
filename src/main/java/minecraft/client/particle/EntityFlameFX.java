package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityFlameFX extends EntityFX
{
    /** the scale of the flame FX */
    private float flameScale;

    protected EntityFlameFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.field_187129_i = this.field_187129_i * 0.009999999776482582D + xSpeedIn;
        this.field_187130_j = this.field_187130_j * 0.009999999776482582D + ySpeedIn;
        this.field_187131_k = this.field_187131_k * 0.009999999776482582D + zSpeedIn;
        this.field_187126_f += (double)((this.field_187136_p.nextFloat() - this.field_187136_p.nextFloat()) * 0.05F);
        this.field_187127_g += (double)((this.field_187136_p.nextFloat() - this.field_187136_p.nextFloat()) * 0.05F);
        this.field_187128_h += (double)((this.field_187136_p.nextFloat() - this.field_187136_p.nextFloat()) * 0.05F);
        this.flameScale = this.particleScale;
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F;
        this.particleMaxAge = (int)(8.0D / (Math.random() * 0.8D + 0.2D)) + 4;
        this.setParticleTextureIndex(48);
    }

    public void func_187110_a(double p_187110_1_, double p_187110_3_, double p_187110_5_)
    {
        this.func_187108_a(this.func_187116_l().offset(p_187110_1_, p_187110_3_, p_187110_5_));
        this.func_187118_j();
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge;
        this.particleScale = this.flameScale * (1.0F - f * f * 0.5F);
        super.renderParticle(worldRendererIn, entityIn, partialTicks, rotationX, rotationZ, rotationYZ, rotationXY, rotationXZ);
    }

    public int func_189214_a(float p_189214_1_)
    {
        float f = ((float)this.particleAge + p_189214_1_) / (float)this.particleMaxAge;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        int i = super.func_189214_a(p_189214_1_);
        int j = i & 255;
        int k = i >> 16 & 255;
        j = j + (int)(f * 15.0F * 16.0F);

        if (j > 240)
        {
            j = 240;
        }

        return j | k << 16;
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

        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.9599999785423279D;
        this.field_187130_j *= 0.9599999785423279D;
        this.field_187131_k *= 0.9599999785423279D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityFlameFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
