package net.minecraft.client.particle;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.init.Blocks;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class EntityDiggingFX extends EntityFX
{
    private IBlockState sourceState;
    private BlockPos sourcePos;

    protected EntityDiggingFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, IBlockState state)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        this.sourceState = state;
        this.setParticleTexture(Minecraft.getMinecraft().getBlockRendererDispatcher().getBlockModelShapes().getTexture(state));
        this.particleGravity = state.getBlock().blockParticleGravity;
        this.particleRed = this.particleGreen = this.particleBlue = 0.6F;
        this.particleScale /= 2.0F;
    }

    /**
     * Sets the position of the block that this particle came from. Used for calculating texture and color multiplier.
     */
    public EntityDiggingFX setBlockPos(BlockPos pos)
    {
        this.sourcePos = pos;

        if (this.sourceState.getBlock() == Blocks.grass)
        {
            return this;
        }
        else
        {
            this.func_187154_b(pos);
            return this;
        }
    }

    public EntityDiggingFX func_174845_l()
    {
        this.sourcePos = new BlockPos(this.field_187126_f, this.field_187127_g, this.field_187128_h);
        Block block = this.sourceState.getBlock();

        if (block == Blocks.grass)
        {
            return this;
        }
        else
        {
            this.func_187154_b((BlockPos)null);
            return this;
        }
    }

    protected void func_187154_b(BlockPos p_187154_1_)
    {
        int i = Minecraft.getMinecraft().func_184125_al().func_186724_a(this.sourceState, this.field_187122_b, p_187154_1_, 0);
        this.particleRed *= (float)(i >> 16 & 255) / 255.0F;
        this.particleGreen *= (float)(i >> 8 & 255) / 255.0F;
        this.particleBlue *= (float)(i & 255) / 255.0F;
    }

    public int getFXLayer()
    {
        return 1;
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
        float f = ((float)this.particleTextureIndexX + this.particleTextureJitterX / 4.0F) / 16.0F;
        float f1 = f + 0.015609375F;
        float f2 = ((float)this.particleTextureIndexY + this.particleTextureJitterY / 4.0F) / 16.0F;
        float f3 = f2 + 0.015609375F;
        float f4 = 0.1F * this.particleScale;

        if (this.particleTexture != null)
        {
            f = this.particleTexture.getInterpolatedU((double)(this.particleTextureJitterX / 4.0F * 16.0F));
            f1 = this.particleTexture.getInterpolatedU((double)((this.particleTextureJitterX + 1.0F) / 4.0F * 16.0F));
            f2 = this.particleTexture.getInterpolatedV((double)(this.particleTextureJitterY / 4.0F * 16.0F));
            f3 = this.particleTexture.getInterpolatedV((double)((this.particleTextureJitterY + 1.0F) / 4.0F * 16.0F));
        }

        float f5 = (float)(this.field_187123_c + (this.field_187126_f - this.field_187123_c) * (double)partialTicks - interpPosX);
        float f6 = (float)(this.field_187124_d + (this.field_187127_g - this.field_187124_d) * (double)partialTicks - interpPosY);
        float f7 = (float)(this.field_187125_e + (this.field_187128_h - this.field_187125_e) * (double)partialTicks - interpPosZ);
        int i = this.func_189214_a(partialTicks);
        int j = i >> 16 & 65535;
        int k = i & 65535;
        worldRendererIn.pos((double)(f5 - rotationX * f4 - rotationXY * f4), (double)(f6 - rotationZ * f4), (double)(f7 - rotationYZ * f4 - rotationXZ * f4)).tex((double)f, (double)f3).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(j, k).endVertex();
        worldRendererIn.pos((double)(f5 - rotationX * f4 + rotationXY * f4), (double)(f6 + rotationZ * f4), (double)(f7 - rotationYZ * f4 + rotationXZ * f4)).tex((double)f, (double)f2).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(j, k).endVertex();
        worldRendererIn.pos((double)(f5 + rotationX * f4 + rotationXY * f4), (double)(f6 + rotationZ * f4), (double)(f7 + rotationYZ * f4 + rotationXZ * f4)).tex((double)f1, (double)f2).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(j, k).endVertex();
        worldRendererIn.pos((double)(f5 + rotationX * f4 - rotationXY * f4), (double)(f6 - rotationZ * f4), (double)(f7 + rotationYZ * f4 - rotationXZ * f4)).tex((double)f1, (double)f3).color(this.particleRed, this.particleGreen, this.particleBlue, 1.0F).lightmap(j, k).endVertex();
    }

    public int func_189214_a(float p_189214_1_)
    {
        int i = super.func_189214_a(p_189214_1_);
        int j = 0;

        if (this.field_187122_b.isBlockLoaded(this.sourcePos))
        {
            j = this.field_187122_b.getCombinedLight(this.sourcePos, 0);
        }

        return i == 0 ? j : i;
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return (new EntityDiggingFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn, Block.getStateById(p_178902_15_[0]))).func_174845_l();
        }
    }
}
