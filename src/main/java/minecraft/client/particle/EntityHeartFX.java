package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityHeartFX extends EntityFX
{
    float particleScaleOverTime;

    protected EntityHeartFX(World worldIn, double p_i1211_2_, double p_i1211_4_, double p_i1211_6_, double p_i1211_8_, double p_i1211_10_, double p_i1211_12_)
    {
        this(worldIn, p_i1211_2_, p_i1211_4_, p_i1211_6_, p_i1211_8_, p_i1211_10_, p_i1211_12_, 2.0F);
    }

    protected EntityHeartFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i46354_8_, double p_i46354_10_, double p_i46354_12_, float scale)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        this.field_187129_i *= 0.009999999776482582D;
        this.field_187130_j *= 0.009999999776482582D;
        this.field_187131_k *= 0.009999999776482582D;
        this.field_187130_j += 0.1D;
        this.particleScale *= 0.75F;
        this.particleScale *= scale;
        this.particleScaleOverTime = this.particleScale;
        this.particleMaxAge = 16;
        this.setParticleTextureIndex(80);
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge * 32.0F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        this.particleScale = this.particleScaleOverTime * f;
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

        this.field_187129_i *= 0.8600000143051147D;
        this.field_187130_j *= 0.8600000143051147D;
        this.field_187131_k *= 0.8600000143051147D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    public static class AngryVillagerFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntityHeartFX(worldIn, xCoordIn, yCoordIn + 0.5D, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            entityfx.setParticleTextureIndex(81);
            entityfx.setRBGColorF(1.0F, 1.0F, 1.0F);
            return entityfx;
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityHeartFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
