package net.minecraft.world;

import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.network.play.server.SPacketBlockBreakAnim;
import net.minecraft.network.play.server.SPacketEffect;
import net.minecraft.network.play.server.SPacketSoundEffect;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;

public class WorldManager implements IWorldEventListener
{
    /** Reference to the MinecraftServer object. */
    private MinecraftServer mcServer;

    /** The WorldServer object. */
    private WorldServer theWorldServer;

    public WorldManager(MinecraftServer mcServerIn, WorldServer worldServerIn)
    {
        this.mcServer = mcServerIn;
        this.theWorldServer = worldServerIn;
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
        this.theWorldServer.getEntityTracker().trackEntity(entityIn);

        if (entityIn instanceof EntityPlayerMP)
        {
            this.theWorldServer.provider.func_186061_a((EntityPlayerMP)entityIn);
        }
    }

    /**
     * Called on all IWorldAccesses when an entity is unloaded or destroyed. On client worlds, releases any downloaded
     * textures. On server worlds, removes the entity from the entity tracker.
     */
    public void onEntityRemoved(Entity entityIn)
    {
        this.theWorldServer.getEntityTracker().untrackEntity(entityIn);
        this.theWorldServer.getScoreboard().func_181140_a(entityIn);

        if (entityIn instanceof EntityPlayerMP)
        {
            this.theWorldServer.provider.func_186062_b((EntityPlayerMP)entityIn);
        }
    }

    public void func_184375_a(EntityPlayer player, SoundEvent soundIn, SoundCategory category, double x, double y, double z, float volume, float pitch)
    {
        this.mcServer.getPlayerList().sendToAllNearExcept(player, x, y, z, volume > 1.0F ? (double)(16.0F * volume) : 16.0D, this.theWorldServer.provider.getDimensionType().getId(), new SPacketSoundEffect(soundIn, category, x, y, z, volume, pitch));
    }

    /**
     * On the client, re-renders all blocks in this range, inclusive. On the server, does nothing. Args: min x, min y,
     * min z, max x, max y, max z
     */
    public void markBlockRangeForRenderUpdate(int x1, int y1, int z1, int x2, int y2, int z2)
    {
    }

    public void notifyBlockUpdate(World worldIn, BlockPos pos, IBlockState oldState, IBlockState newState, int flags)
    {
        this.theWorldServer.getPlayerChunkManager().markBlockForUpdate(pos);
    }

    public void notifyLightSet(BlockPos pos)
    {
    }

    public void func_184377_a(SoundEvent soundIn, BlockPos pos)
    {
    }

    public void playAuxSFX(EntityPlayer player, int sfxType, BlockPos blockPosIn, int data)
    {
        this.mcServer.getPlayerList().sendToAllNearExcept(player, (double)blockPosIn.getX(), (double)blockPosIn.getY(), (double)blockPosIn.getZ(), 64.0D, this.theWorldServer.provider.getDimensionType().getId(), new SPacketEffect(sfxType, blockPosIn, data, false));
    }

    public void broadcastSound(int soundID, BlockPos pos, int data)
    {
        this.mcServer.getPlayerList().sendPacketToAllPlayers(new SPacketEffect(soundID, pos, data, true));
    }

    public void sendBlockBreakProgress(int breakerId, BlockPos pos, int progress)
    {
        for (EntityPlayerMP entityplayermp : this.mcServer.getPlayerList().getPlayerList())
        {
            if (entityplayermp != null && entityplayermp.worldObj == this.theWorldServer && entityplayermp.getEntityId() != breakerId)
            {
                double d0 = (double)pos.getX() - entityplayermp.posX;
                double d1 = (double)pos.getY() - entityplayermp.posY;
                double d2 = (double)pos.getZ() - entityplayermp.posZ;

                if (d0 * d0 + d1 * d1 + d2 * d2 < 1024.0D)
                {
                    entityplayermp.playerNetServerHandler.sendPacket(new SPacketBlockBreakAnim(breakerId, pos, progress));
                }
            }
        }
    }
}
