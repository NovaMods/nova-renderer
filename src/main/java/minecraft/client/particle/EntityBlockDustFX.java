package net.minecraft.client.particle;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.world.World;

public class EntityBlockDustFX extends EntityDiggingFX
{
    protected EntityBlockDustFX(World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, IBlockState state)
    {
        super(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn, state);
        this.field_187129_i = xSpeedIn;
        this.field_187130_j = ySpeedIn;
        this.field_187131_k = zSpeedIn;
    }

    public static class Factory implements IParticleFactory
    {
        public EntityFX getEntityFX(int particleID, World worldIn, double xCoordIn, double yCoordIn, double zCoordIn, double xSpeedIn, double ySpeedIn, double zSpeedIn, int... p_178902_15_)
        {
            IBlockState iblockstate = Block.getStateById(p_178902_15_[0]);
            return iblockstate.getRenderType() == EnumBlockRenderType.INVISIBLE ? null : (new EntityBlockDustFX(worldIn, xCoordIn, yCoordIn, zCoordIn, xSpeedIn, ySpeedIn, zSpeedIn, iblockstate)).func_174845_l();
        }
    }
}
