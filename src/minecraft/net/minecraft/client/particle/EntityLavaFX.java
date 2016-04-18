package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityLavaFX extends EntityFX
{
    private float lavaParticleScale;

    protected EntityLavaFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i *= 0.800000011920929D;
        this.field_187130_j *= 0.800000011920929D;
        this.field_187131_k *= 0.800000011920929D;
        this.field_187130_j = (double)(this.field_187136_p.nextFloat() * 0.4F + 0.05F);
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F;
        this.particleScale *= this.field_187136_p.nextFloat() * 2.0F + 0.2F;
        this.lavaParticleScale = this.particleScale;
        this.particleMaxAge = (int)(16.0D / (Math.random() * 0.8D + 0.2D));
        this.setParticleTextureIndex(49);
    }

    public int func_189214_a(float p_189214_1_)
    {
        float f = ((float)this.particleAge + p_189214_1_) / (float)this.particleMaxAge;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        int i = super.func_189214_a(p_189214_1_);
        int j = 240;
        int k = i >> 16 & 255;
        return j | k << 16;
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge;
        this.particleScale = this.lavaParticleScale * (1.0F - f * f);
        super.renderParticle(worldRendererIn, entityIn, partialTicks, rotationX, rotationZ, rotationYZ, rotationXY, rotationXZ);
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

        float f = (float)this.particleAge / (float)this.particleMaxAge;

        if (this.field_187136_p.nextFloat() > f)
        {
            this.field_187122_b.spawnParticle(EnumParticleTypes.SMOKE_NORMAL, this.field_187126_f, this.field_187127_g, this.field_187128_h, this.field_187129_i, this.field_187130_j, this.field_187131_k, new int[0]);
        }

        this.field_187130_j -= 0.03D;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.9990000128746033D;
        this.field_187130_j *= 0.9990000128746033D;
        this.field_187131_k *= 0.9990000128746033D;

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
            return new EntityLavaFX(worldIn, xCoordIn, yCoordIn, zCoordIn);
        }
    }
}
