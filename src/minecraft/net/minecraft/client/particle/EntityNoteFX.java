package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityNoteFX extends EntityFX
{
    float noteParticleScale;

    protected EntityNoteFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i46353_8_, double p_i46353_10_, double p_i46353_12_)
    {
        this(worldIn, xCoordIn, yCoordIn, zCoordIn, p_i46353_8_, p_i46353_10_, p_i46353_12_, 2.0F);
    }

    protected EntityNoteFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i1217_8_, double p_i1217_10_, double p_i1217_12_, float p_i1217_14_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i *= 0.009999999776482582D;
        this.field_187130_j *= 0.009999999776482582D;
        this.field_187131_k *= 0.009999999776482582D;
        this.field_187130_j += 0.2D;
        this.particleRed = MathHelper.sin(((float)p_i1217_8_ + 0.0F) * ((float)Math.PI * 2F)) * 0.65F + 0.35F;
        this.particleGreen = MathHelper.sin(((float)p_i1217_8_ + 0.33333334F) * ((float)Math.PI * 2F)) * 0.65F + 0.35F;
        this.particleBlue = MathHelper.sin(((float)p_i1217_8_ + 0.6666667F) * ((float)Math.PI * 2F)) * 0.65F + 0.35F;
        this.particleScale *= 0.75F;
        this.particleScale *= p_i1217_14_;
        this.noteParticleScale = this.particleScale;
        this.particleMaxAge = 6;
        this.setParticleTextureIndex(64);
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge * 32.0F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        this.particleScale = this.noteParticleScale * f;
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

        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);

        if (this.field_187127_g == this.field_187124_d)
        {
            this.field_187129_i *= 1.1D;
            this.field_187131_k *= 1.1D;
        }

        this.field_187129_i *= 0.6600000262260437D;
        this.field_187130_j *= 0.6600000262260437D;
        this.field_187131_k *= 0.6600000262260437D;

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
            return new EntityNoteFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
