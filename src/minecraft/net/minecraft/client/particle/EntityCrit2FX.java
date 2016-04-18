package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityCrit2FX extends EntityFX
{
    float field_174839_a;

    protected EntityCrit2FX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i46284_8_, double p_i46284_10_, double p_i46284_12_)
    {
        this(worldIn, xCoordIn, yCoordIn, zCoordIn, p_i46284_8_, p_i46284_10_, p_i46284_12_, 1.0F);
    }

    protected EntityCrit2FX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i46285_8_, double p_i46285_10_, double p_i46285_12_, float p_i46285_14_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i *= 0.10000000149011612D;
        this.field_187130_j *= 0.10000000149011612D;
        this.field_187131_k *= 0.10000000149011612D;
        this.field_187129_i += p_i46285_8_ * 0.4D;
        this.field_187130_j += p_i46285_10_ * 0.4D;
        this.field_187131_k += p_i46285_12_ * 0.4D;
        this.particleRed = this.particleGreen = this.particleBlue = (float)(Math.random() * 0.30000001192092896D + 0.6000000238418579D);
        this.particleScale *= 0.75F;
        this.particleScale *= p_i46285_14_;
        this.field_174839_a = this.particleScale;
        this.particleMaxAge = (int)(6.0D / (Math.random() * 0.8D + 0.6D));
        this.particleMaxAge = (int)((float)this.particleMaxAge * p_i46285_14_);
        this.setParticleTextureIndex(65);
        this.func_189213_a();
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge * 32.0F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        this.particleScale = this.field_174839_a * f;
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
        this.particleGreen = (float)((double)this.particleGreen * 0.96D);
        this.particleBlue = (float)((double)this.particleBlue * 0.9D);
        this.field_187129_i *= 0.699999988079071D;
        this.field_187130_j *= 0.699999988079071D;
        this.field_187131_k *= 0.699999988079071D;
        this.field_187130_j -= 0.019999999552965164D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    public static class DamageIndicatorFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntityCrit2FX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn + 1.0D, zSpeedIn, 1.0F);
            entityfx.func_187114_a(20);
            entityfx.setParticleTextureIndex(67);
            return entityfx;
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityCrit2FX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }

    public static class MagicFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntityCrit2FX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            entityfx.setRBGColorF(entityfx.getRedColorF() * 0.3F, entityfx.getGreenColorF() * 0.8F, entityfx.getBlueColorF());
            entityfx.nextTextureIndexX();
            return entityfx;
        }
    }
}
