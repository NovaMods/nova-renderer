package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityCloudFX extends EntityFX
{
    float field_70569_a;

    protected EntityCloudFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i1221_8_, double p_i1221_10_, double p_i1221_12_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
        float f = 2.5F;
        this.field_187129_i *= 0.10000000149011612D;
        this.field_187130_j *= 0.10000000149011612D;
        this.field_187131_k *= 0.10000000149011612D;
        this.field_187129_i += p_i1221_8_;
        this.field_187130_j += p_i1221_10_;
        this.field_187131_k += p_i1221_12_;
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F - (float)(Math.random() * 0.30000001192092896D);
        this.particleScale *= 0.75F;
        this.particleScale *= f;
        this.field_70569_a = this.particleScale;
        this.particleMaxAge = (int)(8.0D / (Math.random() * 0.8D + 0.3D));
        this.particleMaxAge = (int)((float)this.particleMaxAge * f);
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleAge + partialTicks) / (float)this.particleMaxAge * 32.0F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        this.particleScale = this.field_70569_a * f;
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

        this.setParticleTextureIndex(7 - this.particleAge * 8 / this.particleMaxAge);
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.9599999785423279D;
        this.field_187130_j *= 0.9599999785423279D;
        this.field_187131_k *= 0.9599999785423279D;
        EntityPlayer entityplayer = this.field_187122_b.func_184137_a(this.field_187126_f, this.field_187127_g, this.field_187128_h, 2.0D, false);

        if (entityplayer != null)
        {
            AxisAlignedBB axisalignedbb = entityplayer.getEntityBoundingBox();

            if (this.field_187127_g > axisalignedbb.minY)
            {
                this.field_187127_g += (axisalignedbb.minY - this.field_187127_g) * 0.2D;
                this.field_187130_j += (entityplayer.motionY - this.field_187130_j) * 0.2D;
                this.func_187109_b(this.field_187126_f, this.field_187127_g, this.field_187128_h);
            }
        }

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
            return new EntityCloudFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
