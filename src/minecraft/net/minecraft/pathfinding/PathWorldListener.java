package net.minecraft.pathfinding;

import java.util.WeakHashMap;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IWorldEventListener;
import net.minecraft.world.World;

public class PathWorldListener implements IWorldEventListener
{
    private static final Object field_184380_a = new Object();
    private final WeakHashMap<PathNavigate, Object> field_184381_b = new WeakHashMap();

    public void func_184379_a(PathNavigate p_184379_1_)
    {
        this.field_184381_b.put(p_184379_1_, field_184380_a);
    }

    public void notifyBlockUpdate(World worldIn, BlockPos pos, IBlockState oldState, IBlockState newState, int flags)
    {
        if (this.func_184378_a(worldIn, pos, oldState, newState))
        {
            PathNavigate[] apathnavigate = (PathNavigate[])this.field_184381_b.keySet().toArray(new PathNavigate[0]);

            for (PathNavigate pathnavigate : apathnavigate)
            {
                if (pathnavigate != null && !pathnavigate.func_188553_i())
                {
                    PathEntity pathentity = pathnavigate.getPath();

                    if (pathentity != null && !pathentity.isFinished() && pathentity.getCurrentPathLength() != 0)
                    {
                        PathPoint pathpoint = pathnavigate.currentPath.getFinalPathPoint();
                        double d0 = pos.distanceSq(((double)pathpoint.xCoord + pathnavigate.theEntity.posX) / 2.0D, ((double)pathpoint.yCoord + pathnavigate.theEntity.posY) / 2.0D, ((double)pathpoint.zCoord + pathnavigate.theEntity.posZ) / 2.0D);
                        int i = (pathentity.getCurrentPathLength() - pathentity.getCurrentPathIndex()) * (pathentity.getCurrentPathLength() - pathentity.getCurrentPathIndex());

                        if (d0 < (double)i)
                        {
                            pathnavigate.func_188554_j();
                        }
                    }
                }
            }
        }
    }

    protected boolean func_184378_a(World worldIn, BlockPos pos, IBlockState oldState, IBlockState newState)
    {
        AxisAlignedBB axisalignedbb = oldState.getSelectedBoundingBox(worldIn, pos);
        AxisAlignedBB axisalignedbb1 = newState.getSelectedBoundingBox(worldIn, pos);
        return axisalignedbb != axisalignedbb1 && (axisalignedbb == null || !axisalignedbb.equals(axisalignedbb1));
    }

    public void notifyLightSet(BlockPos pos)
    {
    }

    /**
     * On the client, re-renders all blocks in this range, inclusive. On the server, does nothing. Args: min x, min y,
     * min z, max x, max y, max z
     */
    public void markBlockRangeForRenderUpdate(int x1, int y1, int z1, int x2, int y2, int z2)
    {
    }

    public void func_184375_a(EntityPlayer player, SoundEvent soundIn, SoundCategory category, double x, double y, double z, float volume, float pitch)
    {
    }

    public void spawnParticle(int particleID, boolean ignoreRange, double xCoord, double yCoord, double zCoord, double xOffset, double yOffset, double zOffset, int... parameters)
    {
    }

    /**
     * Called on all IWorldAccesses when an entity is created or loaded. On client worlds, starts downloading any
     * necessary textures. On server worlds, adds the entity to the entity tracker.
     */
    public void onEntityAdded(Entity entityIn)
    {
    }

    /**
     * Called on all IWorldAccesses when an entity is unloaded or destroyed. On client worlds, releases any downloaded
     * textures. On server worlds, removes the entity from the entity tracker.
     */
    public void onEntityRemoved(Entity entityIn)
    {
    }

    public void func_184377_a(SoundEvent soundIn, BlockPos pos)
    {
    }

    public void broadcastSound(int soundID, BlockPos pos, int data)
    {
    }

    public void playAuxSFX(EntityPlayer player, int sfxType, BlockPos blockPosIn, int data)
    {
    }

    public void sendBlockBreakProgress(int breakerId, BlockPos pos, int progress)
    {
    }
}
