package net.minecraft.client.particle;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.entity.Entity;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.world.World;

public class EntityHugeExplodeFX extends EntityFX
{
    private int timeSinceStart;

    /** the maximum time for the explosion */
    private int maximumTime = 8;

    protected EntityHugeExplodeFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double p_i1214_8_, double p_i1214_10_, double p_i1214_12_)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, 0.0D, 0.0D, 0.0D);
    }

    /**
     * Renders the particle
     */
    public void renderParticle(VertexBuffer worldRendererIn, Entity entityIn, float partialTicks, float rotationX, float rotationZ, float rotationYZ, float rotationXY, float rotationXZ)
    {
    }

    public void func_189213_a()
    {
        for (int i = 0; i < 6; ++i)
        {
            double d0 = this.field_187126_f + (this.field_187136_p.nextDouble() - this.field_187136_p.nextDouble()) * 4.0D;
            double d1 = this.field_187127_g + (this.field_187136_p.nextDouble() - this.field_187136_p.nextDouble()) * 4.0D;
            double d2 = this.field_187128_h + (this.field_187136_p.nextDouble() - this.field_187136_p.nextDouble()) * 4.0D;
            this.field_187122_b.spawnParticle(EnumParticleTypes.EXPLOSION_LARGE, d0, d1, d2, (double)((float)this.timeSinceStart / (float)this.maximumTime), 0.0D, 0.0D, new int[0]);
        }

        ++this.timeSinceStart;

        if (this.timeSinceStart == this.maximumTime)
        {
            this.func_187112_i();
        }
    }

    public int getFXLayer()
    {
        return 1;
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            return new EntityHugeExplodeFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn);
        }
    }
}
