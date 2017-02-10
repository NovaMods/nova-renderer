package com.continuum.nova.chunks;

import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IWorldEventListener;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author ddubois
 */
public class ChunkUpdateListener implements IWorldEventListener {
    private static final Logger LOG = LogManager.getLogger(ChunkUpdateListener.class);
    private World world;

    public void setWorld(World world) {
        this.world = world;
    }

    @Override
    public void notifyBlockUpdate(World worldIn, BlockPos pos, IBlockState oldState, IBlockState newState, int flags) {
        LOG.debug("Update block at " + pos);
    }

    @Override
    public void notifyLightSet(BlockPos pos) {

    }

    @Override
    public void markBlockRangeForRenderUpdate(int x1, int y1, int z1, int x2, int y2, int z2) {
        LOG.debug("Marking blocks in range (" + x1 + ", " + y1 + ", " + z1 + ") to (" + x2 + ", " + y2 + ", " + z2 + ") for render update");
        int xDist = x2 - x1 + 1;
        int yDist = y2 - y1 + 1;
        int zDist = z2 - z1 + 1;
        Block[][][] blocks = new Block[xDist][yDist][zDist];
        for(int x = x1; x <= x2; x++) {
            for(int y = y1; y <= y2; y++) {
                for(int z = z1; z <= z2; z++) {
                    IBlockState blockState = world.getBlockState(new BlockPos(x, y, z));
                    blocks[x - x1][y - y1][z - z1] = blockState.getBlock();
                }
            }
        }
    }

    @Override
    public void playSoundToAllNearExcept(EntityPlayer player, SoundEvent soundIn, SoundCategory category, double x, double y, double z, float volume, float pitch) {

    }

    @Override
    public void playRecord(SoundEvent soundIn, BlockPos pos) {

    }

    @Override
    public void spawnParticle(int particleID, boolean ignoreRange, double xCoord, double yCoord, double zCoord, double xSpeed, double ySpeed, double zSpeed, int... parameters) {

    }

    @Override
    public void onEntityAdded(Entity entityIn) {

    }

    @Override
    public void onEntityRemoved(Entity entityIn) {

    }

    @Override
    public void broadcastSound(int soundID, BlockPos pos, int data) {

    }

    @Override
    public void playEvent(EntityPlayer player, int type, BlockPos blockPosIn, int data) {

    }

    @Override
    public void sendBlockBreakProgress(int breakerId, BlockPos pos, int progress) {

    }
}
