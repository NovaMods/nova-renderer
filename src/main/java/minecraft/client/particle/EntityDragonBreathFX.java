package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityDragonBreathFX extends EntityFX
{
    private final float field_187143_a;
    private boolean field_187144_G;

    protected EntityDragonBreathFX(World p_i46581_1_, double p_i46581_2_, double p_i46581_4_, double p_i46581_6_, double p_i46581_8_, double p_i46581_10_, double p_i46581_12_)
    {
        super(p_i46581_1_, p_i46581_2_, p_i46581_4_, p_i46581_6_, p_i46581_8_, p_i46581_10_, p_i46581_12_);
        this.field_187129_i = p_i46581_8_;
        this.field_187130_j = p_i46581_10_;
        this.field_187131_k = p_i46581_12_;
        this.particleRed = MathHelper.randomFloatClamp(this.field_187136_p, 0.7176471F, 0.8745098F);
        this.particleGreen = MathHelper.randomFloatClamp(this.field_187136_p, 0.0F, 0.0F);
        this.particleBlue = MathHelper.randomFloatClamp(this.field_187136_p, 0.8235294F, 0.9764706F);
        this.particleScale *= 0.75F;
        this.field_187143_a = this.particleScale;
        this.particleMaxAge = (int)(20.0D / ((double)this.field_187136_p.nextFloat() * 0.8D + 0.2D));
        this.field_187144_G = false;
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
        else
        {
            this.setParticleTextureIndex(3 * this.particleAge / this.particleMaxAge + 5);

            if (this.field_187132_l)
            {
                this.field_187130_j = 0.0D;
                this.field_187144_G = true;
            }

            if (this.field_187144_G)
            {
                this.field_187130_j += 0.002D;
            }

            this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);

            if (this.field_187127_g == this.field_187124_d)
            {
                this.field_187129_i *= 1.1D;
                this.field_187131_k *= 1.1D;
            }

            this.field_187129_i *= 0.9599999785423279D;
            this.field_187131_k *= 0.9599999785423279D;

            if (this.field_187144_G)
            {
                this.field_187130_j *= 0.9599999785423279D;
            }
        }
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        this.particleScale = this.field_187143_a * MathHelper.clamp_float(((float)this.particleAge + partialTicks) / (float)this.particleMaxAge * 32.0F, 0.0F, 1.0F);
        super.renderParticle(worldRendererIn, entityIn, partialTicks, rotationX, rotationZ, rotationYZ, rotationXY, rotationXZ);
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityDragonBreathFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
