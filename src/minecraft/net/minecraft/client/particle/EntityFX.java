package net.minecraft.client.particle;

import java.util.List;
import java.util.Random;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityFX
{
    private static final AxisAlignedBB field_187121_a = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.0D, 0.0D, 0.0D);
    protected World field_187122_b;
    protected double field_187123_c;
    protected double field_187124_d;
    protected double field_187125_e;
    protected double field_187126_f;
    protected double field_187127_g;
    protected double field_187128_h;
    protected double field_187129_i;
    protected double field_187130_j;
    protected double field_187131_k;
    private AxisAlignedBB field_187120_G;
    protected boolean field_187132_l;
    protected boolean field_187133_m;
    protected float field_187134_n;
    protected float field_187135_o;
    protected Random field_187136_p;
    protected int particleTextureIndexX;
    protected int particleTextureIndexY;
    protected float particleTextureJitterX;
    protected float particleTextureJitterY;
    protected int particleAge;
    protected int particleMaxAge;
    protected float particleScale;
    protected float particleGravity;

    /** The red amount of color. Used as a percentage, 1.0 = 255 and 0.0 = 0. */
    protected float particleRed;

    /**
     * The green amount of color. Used as a percentage, 1.0 = 255 and 0.0 = 0.
     */
    protected float particleGreen;

    /**
     * The blue amount of color. Used as a percentage, 1.0 = 255 and 0.0 = 0.
     */
    protected float particleBlue;

    /** Particle alpha */
    protected float particleAlpha;
    protected TextureAtlasSprite particleTexture;
    public static double interpPosX;
    public static double interpPosY;
    public static double interpPosZ;

    protected EntityFX(World worldIn, double posXIn, double posYIn, double posZIn)
    {
        this.field_187120_G = field_187121_a;
        this.field_187134_n = 0.6F;
        this.field_187135_o = 1.8F;
        this.field_187136_p = new Random();
        this.particleAlpha = 1.0F;
        this.field_187122_b = worldIn;
        this.func_187115_a(0.2F, 0.2F);
        this.func_187109_b(posXIn, posYIn, posZIn);
        this.field_187123_c = posXIn;
        this.field_187124_d = posYIn;
        this.field_187125_e = posZIn;
        this.particleRed = this.particleGreen = this.particleBlue = 1.0F;
        this.particleTextureJitterX = this.field_187136_p.nextFloat() * 3.0F;
        this.particleTextureJitterY = this.field_187136_p.nextFloat() * 3.0F;
        this.particleScale = (this.field_187136_p.nextFloat() * 0.5F + 0.5F) * 2.0F;
        this.particleMaxAge = (int)(4.0F / (this.field_187136_p.nextFloat() * 0.9F + 0.1F));
        this.particleAge = 0;
    }

    public EntityFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn)
    {
        this(worldIn, xCoordIn, yCoordIn, zCoordIn);
        this.field_187129_i = xSpeedIn + (Math.random() * 2.0D - 1.0D) * 0.4000000059604645D;
        this.field_187130_j = ySpeedIn + (Math.random() * 2.0D - 1.0D) * 0.4000000059604645D;
        this.field_187131_k = zSpeedIn + (Math.random() * 2.0D - 1.0D) * 0.4000000059604645D;
        float f = (float)(Math.random() + Math.random() + 1.0D) * 0.15F;
        float f1 = MathHelper.sqrt_double(this.field_187129_i * this.field_187129_i + this.field_187130_j * this.field_187130_j + this.field_187131_k * this.field_187131_k);
        this.field_187129_i = this.field_187129_i / (double)f1 * (double)f * 0.4000000059604645D;
        this.field_187130_j = this.field_187130_j / (double)f1 * (double)f * 0.4000000059604645D + 0.10000000149011612D;
        this.field_187131_k = this.field_187131_k / (double)f1 * (double)f * 0.4000000059604645D;
    }

    public EntityFX multiplyVelocity(float multiplier)
    {
        this.field_187129_i *= (double)multiplier;
        this.field_187130_j = (this.field_187130_j - 0.10000000149011612D) * (double)multiplier + 0.10000000149011612D;
        this.field_187131_k *= (double)multiplier;
        return this;
    }

    public EntityFX multipleParticleScaleBy(float scale)
    {
        this.func_187115_a(0.2F * scale, 0.2F * scale);
        this.particleScale *= scale;
        return this;
    }

    public void setRBGColorF(float particleRedIn, float particleGreenIn, float particleBlueIn)
    {
        this.particleRed = particleRedIn;
        this.particleGreen = particleGreenIn;
        this.particleBlue = particleBlueIn;
    }

    /**
     * Sets the particle alpha (float)
     */
    public void setAlphaF(float alpha)
    {
        this.particleAlpha = alpha;
    }

    public boolean func_187111_c()
    {
        return false;
    }

    public float getRedColorF()
    {
        return this.particleRed;
    }

    public float getGreenColorF()
    {
        return this.particleGreen;
    }

    public float getBlueColorF()
    {
        return this.particleBlue;
    }

    public void func_187114_a(int p_187114_1_)
    {
        this.particleMaxAge = p_187114_1_;
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

        this.field_187130_j -= 0.04D * (double)this.particleGravity;
        this.func_187110_a(this.field_187129_i, this.field_187130_j, this.field_187131_k);
        this.field_187129_i *= 0.9800000190734863D;
        this.field_187130_j *= 0.9800000190734863D;
        this.field_187131_k *= 0.9800000190734863D;

        if (this.field_187132_l)
        {
            this.field_187129_i *= 0.699999988079071D;
            this.field_187131_k *= 0.699999988079071D;
        }
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = (float)this.particleTextureIndexX / 16.0F;
        float f1 = f + 0.0624375F;
        float f2 = (float)this.particleTextureIndexY / 16.0F;
        float f3 = f2 + 0.0624375F;
        float f4 = 0.1F * this.particleScale;

        if (this.particleTexture != null)
        {
            f = this.particleTexture.getMinU();
            f1 = this.particleTexture.getMaxU();
            f2 = this.particleTexture.getMinV();
            f3 = this.particleTexture.getMaxV();
        }

        float f5 = (float)(this.field_187123_c + (this.field_187126_f - this.field_187123_c) * (double)partialTicks - interpPosX);
        float f6 = (float)(this.field_187124_d + (this.field_187127_g - this.field_187124_d) * (double)partialTicks - interpPosY);
        float f7 = (float)(this.field_187125_e + (this.field_187128_h - this.field_187125_e) * (double)partialTicks - interpPosZ);
        int i = this.func_189214_a(partialTicks);
        int j = i >> 16 & 65535;
        int k = i & 65535;
        worldRendererIn.pos((double)(f5 - rotationX * f4 - rotationXY * f4), (double)(f6 - rotationZ * f4), (double)(f7 - rotationYZ * f4 - rotationXZ * f4)).tex((double)f1, (double)f3).color(this.particleRed, this.particleGreen, this.particleBlue, this.particleAlpha).lightmap(j, k).endVertex();
        worldRendererIn.pos((double)(f5 - rotationX * f4 + rotationXY * f4), (double)(f6 + rotationZ * f4), (double)(f7 - rotationYZ * f4 + rotationXZ * f4)).tex((double)f1, (double)f2).color(this.particleRed, this.particleGreen, this.particleBlue, this.particleAlpha).lightmap(j, k).endVertex();
        worldRendererIn.pos((double)(f5 + rotationX * f4 + rotationXY * f4), (double)(f6 + rotationZ * f4), (double)(f7 + rotationYZ * f4 + rotationXZ * f4)).tex((double)f, (double)f2).color(this.particleRed, this.particleGreen, this.particleBlue, this.particleAlpha).lightmap(j, k).endVertex();
        worldRendererIn.pos((double)(f5 + rotationX * f4 - rotationXY * f4), (double)(f6 - rotationZ * f4), (double)(f7 + rotationYZ * f4 - rotationXZ * f4)).tex((double)f, (double)f3).color(this.particleRed, this.particleGreen, this.particleBlue, this.particleAlpha).lightmap(j, k).endVertex();
    }

    public int getFXLayer()
    {
        return 0;
    }

    /**
     * Sets the texture used by the particle.
     *  
     * @param texture texture atlas to use for the particle to use.
     */
    public void setParticleTexture(TextureAtlasSprite texture)
    {
        int i = this.getFXLayer();

        if (i == 1)
        {
            this.particleTexture = texture;
        }
        else
        {
            throw new RuntimeException("Invalid call to Particle.setTex, use coordinate methods");
        }
    }

    /**
     * Public method to set private field particleTextureIndex.
     */
    public void setParticleTextureIndex(int particleTextureIndex)
    {
        if (this.getFXLayer() != 0)
        {
            throw new RuntimeException("Invalid call to Particle.setMiscTex");
        }
        else
        {
            this.particleTextureIndexX = particleTextureIndex % 16;
            this.particleTextureIndexY = particleTextureIndex / 16;
        }
    }

    public void nextTextureIndexX()
    {
        ++this.particleTextureIndexX;
    }

    public String toString()
    {
        return this.getClass().getSimpleName() + ", Pos (" + this.field_187126_f + "," + this.field_187127_g + "," + this.field_187128_h + "), RGBA (" + this.particleRed + "," + this.particleGreen + "," + this.particleBlue + "," + this.particleAlpha + "), Age " + this.particleAge;
    }

    public void func_187112_i()
    {
        this.field_187133_m = true;
    }

    protected void func_187115_a(float p_187115_1_, float p_187115_2_)
    {
        if (p_187115_1_ != this.field_187134_n || p_187115_2_ != this.field_187135_o)
        {
            this.field_187134_n = p_187115_1_;
            this.field_187135_o = p_187115_2_;
            AxisAlignedBB axisalignedbb = this.func_187116_l();
            this.func_187108_a(new AxisAlignedBB(axisalignedbb.minX, axisalignedbb.minY, axisalignedbb.minZ, axisalignedbb.minX + (double)this.field_187134_n, axisalignedbb.minY + (double)this.field_187135_o, axisalignedbb.minZ + (double)this.field_187134_n));
        }
    }

    public void func_187109_b(double p_187109_1_, double p_187109_3_, double p_187109_5_)
    {
        this.field_187126_f = p_187109_1_;
        this.field_187127_g = p_187109_3_;
        this.field_187128_h = p_187109_5_;
        float f = this.field_187134_n / 2.0F;
        float f1 = this.field_187135_o;
        this.func_187108_a(new AxisAlignedBB(p_187109_1_ - (double)f, p_187109_3_, p_187109_5_ - (double)f, p_187109_1_ + (double)f, p_187109_3_ + (double)f1, p_187109_5_ + (double)f));
    }

    public void func_187110_a(double p_187110_1_, double p_187110_3_, double p_187110_5_)
    {
        double d0 = p_187110_1_;
        double d1 = p_187110_3_;
        double d2 = p_187110_5_;
        List<AxisAlignedBB> list = this.field_187122_b.func_184144_a((Entity)null, this.func_187116_l().addCoord(p_187110_1_, p_187110_3_, p_187110_5_));

        for (AxisAlignedBB axisalignedbb : list)
        {
            p_187110_3_ = axisalignedbb.calculateYOffset(this.func_187116_l(), p_187110_3_);
        }

        this.func_187108_a(this.func_187116_l().offset(0.0D, p_187110_3_, 0.0D));

        for (AxisAlignedBB axisalignedbb1 : list)
        {
            p_187110_1_ = axisalignedbb1.calculateXOffset(this.func_187116_l(), p_187110_1_);
        }

        this.func_187108_a(this.func_187116_l().offset(p_187110_1_, 0.0D, 0.0D));

        for (AxisAlignedBB axisalignedbb2 : list)
        {
            p_187110_5_ = axisalignedbb2.calculateZOffset(this.func_187116_l(), p_187110_5_);
        }

        this.func_187108_a(this.func_187116_l().offset(0.0D, 0.0D, p_187110_5_));
        this.func_187118_j();
        this.field_187132_l = d1 != p_187110_3_ && d1 < 0.0D;

        if (d0 != p_187110_1_)
        {
            this.field_187129_i = 0.0D;
        }

        if (d2 != p_187110_5_)
        {
            this.field_187131_k = 0.0D;
        }
    }

    protected void func_187118_j()
    {
        AxisAlignedBB axisalignedbb = this.func_187116_l();
        this.field_187126_f = (axisalignedbb.minX + axisalignedbb.maxX) / 2.0D;
        this.field_187127_g = axisalignedbb.minY;
        this.field_187128_h = (axisalignedbb.minZ + axisalignedbb.maxZ) / 2.0D;
    }

    public int func_189214_a(float p_189214_1_)
    {
        BlockPos blockpos = new BlockPos(this.field_187126_f, this.field_187127_g, this.field_187128_h);
        return this.field_187122_b.isBlockLoaded(blockpos) ? this.field_187122_b.getCombinedLight(blockpos, 0) : 0;
    }

    public boolean func_187113_k()
    {
        return !this.field_187133_m;
    }

    public AxisAlignedBB func_187116_l()
    {
        return this.field_187120_G;
    }

    public void func_187108_a(AxisAlignedBB p_187108_1_)
    {
        this.field_187120_G = p_187108_1_;
    }
}
