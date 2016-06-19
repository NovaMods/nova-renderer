package net.minecraft.client.particle;

import java.util.Random;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntitySpellParticleFX extends EntityFX
{
    private static final Random RANDOM = new Random();

    /** Base spell texture index */
    private int baseSpellTextureIndex = 128;

    protected EntitySpellParticleFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i1229_8_, double p_i1229_10_, double p_i1229_12_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.5D - RANDOM.nextDouble(), p_i1229_10_, 0.5D - RANDOM.nextDouble());
        this.field_187130_j *= 0.20000000298023224D;

        if (p_i1229_8_ == 0.0D && p_i1229_12_ == 0.0D)
        {
            this.field_187129_i *= 0.10000000149011612D;
            this.field_187131_k *= 0.10000000149011612D;
        }

        this.particleScale *= 0.75F;
        this.particleMaxAge = (int)(8.0D / (Math.random() * 0.8D + 0.2D));
    }

    public boolean func_187111_c()
    {
        return true;
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge * 32.0F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
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

        this.setParticleTextureIndex(this.baseSpellTextureIndex + (7 - this.particleAge * 8 / this.particleMaxAge));
        this.field_187130_j += 0.004D;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);

        if (this.field_187127_g == this.field_187124_d)
        {
            this.field_187129_i *= 1.1D;
            this.field_187131_k *= 1.1D;
        }

        this.field_187129_i *= 0.9599999785423279D;
        this.field_187130_j *= 0.9599999785423279D;
        this.field_187131_k *= 0.9599999785423279D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    /**
     * Sets the base spell texture index
     */
    public void setBaseSpellTextureIndex(int baseSpellTextureIndexIn)
    {
        this.baseSpellTextureIndex = baseSpellTextureIndexIn;
    }

    public static class AmbientMobFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntitySpellParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            entityfx.setAlphaF(0.15F);
            entityfx.setRBGColorF((float)xSpeedIn, (float)ySpeedIn, (float)zSpeedIn);
            return entityfx;
        }
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntitySpellParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }

    public static class InstantFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntitySpellParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            ((EntitySpellParticleFX)entityfx).setBaseSpellTextureIndex(144);
            return entityfx;
        }
    }

    public static class MobFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntitySpellParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            entityfx.setRBGColorF((float)xSpeedIn, (float)ySpeedIn, (float)zSpeedIn);
            return entityfx;
        }
    }

    public static class WitchFactory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            EntityFX entityfx = new EntitySpellParticleFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
            ((EntitySpellParticleFX)entityfx).setBaseSpellTextureIndex(144);
            float f = worldIn.rand.nextFloat() * 0.5F + 0.35F;
            entityfx.setRBGColorF(1.0F * f, 0.0F * f, 1.0F * f);
            return entityfx;
        }
    }
}
