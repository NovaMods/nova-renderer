package net.minecraft.server.management;

import com.google.common.base.Predicate;
import com.google.common.collect.AbstractIterator;
import com.google.common.collect.ComparisonChain;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketBlockChange;
import net.minecraft.network.play.server.SPacketChunkData;
import net.minecraft.network.play.server.SPacketMultiBlockChange;
import net.minecraft.network.play.server.SPacketUnloadChunk;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.LongHashMap;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.WorldProvider;
import net.minecraft.world.WorldServer;
import net.minecraft.world.chunk.Chunk;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class PlayerManager
{
    private static final Predicate<EntityPlayerMP> field_187308_a = new Predicate<EntityPlayerMP>()
    {
        public boolean apply(EntityPlayerMP p_apply_1_)
        {
            return p_apply_1_ != null && !p_apply_1_.isSpectator();
        }
    };
    private static final Predicate<EntityPlayerMP> field_187309_b = new Predicate<EntityPlayerMP>()
    {
        public boolean apply(EntityPlayerMP p_apply_1_)
        {
            return p_apply_1_ != null && (!p_apply_1_.isSpectator() || p_apply_1_.getServerForPlayer().getGameRules().getBoolean("spectatorsGenerateChunks"));
        }
    };
    private final WorldServer theWorldServer;
    private final List<EntityPlayerMP> players = Lists.<EntityPlayerMP>newArrayList();
    private final LongHashMap<PlayerManager.PlayerInstance> playerInstances = new LongHashMap();
    private final Set<PlayerManager.PlayerInstance> playerInstancesToUpdate = Sets.<PlayerManager.PlayerInstance>newHashSet();
    private final List<PlayerManager.PlayerInstance> field_187310_g = Lists.<PlayerManager.PlayerInstance>newLinkedList();
    private final List<PlayerManager.PlayerInstance> field_187311_h = Lists.<PlayerManager.PlayerInstance>newLinkedList();
    private final List<PlayerManager.PlayerInstance> playerInstanceList = Lists.<PlayerManager.PlayerInstance>newArrayList();

    /**
     * Number of chunks the server sends to the client. Valid 3<=x<=15. In server.properties.
     */
    private int playerViewRadius;

    /** time what is using to check if InhabitedTime should be calculated */
    private long previousTotalWorldTime;
    private boolean field_187312_l = true;
    private boolean field_187313_m = true;

    public PlayerManager(WorldServer serverWorld)
    {
        this.theWorldServer = serverWorld;
        this.setPlayerViewRadius(serverWorld.getMinecraftServer().getPlayerList().getViewDistance());
    }

    /**
     * Returns the WorldServer associated with this PlayerManager
     */
    public WorldServer getWorldServer()
    {
        return this.theWorldServer;
    }

    public Iterator<Chunk> func_187300_b()
    {
        final Iterator<PlayerManager.PlayerInstance> iterator = this.playerInstanceList.iterator();
        return new AbstractIterator<Chunk>()
        {
            protected Chunk computeNext()
            {
                while (true)
                {
                    if (iterator.hasNext())
                    {
                        PlayerManager.PlayerInstance playermanager$playerinstance = (PlayerManager.PlayerInstance)iterator.next();
                        Chunk chunk = playermanager$playerinstance.func_187266_f();

                        if (chunk == null)
                        {
                            continue;
                        }

                        if (!chunk.isLightPopulated() && chunk.isTerrainPopulated())
                        {
                            return chunk;
                        }

                        if (!chunk.func_186035_j())
                        {
                            return chunk;
                        }

                        if (!playermanager$playerinstance.func_187271_a(128.0D, PlayerManager.field_187308_a))
                        {
                            continue;
                        }

                        return chunk;
                    }

                    return (Chunk)this.endOfData();
                }
            }
        };
    }

    /**
     * updates all the player instances that need to be updated
     */
    public void updatePlayerInstances()
    {
        long i = this.theWorldServer.getTotalWorldTime();

        if (i - this.previousTotalWorldTime > 8000L)
        {
            this.previousTotalWorldTime = i;

            for (int j = 0; j < this.playerInstanceList.size(); ++j)
            {
                PlayerManager.PlayerInstance playermanager$playerinstance = (PlayerManager.PlayerInstance)this.playerInstanceList.get(j);
                playermanager$playerinstance.func_187280_d();
                playermanager$playerinstance.func_187279_c();
            }
        }

        if (!this.playerInstancesToUpdate.isEmpty())
        {
            for (PlayerManager.PlayerInstance playermanager$playerinstance2 : this.playerInstancesToUpdate)
            {
                playermanager$playerinstance2.func_187280_d();
            }

            this.playerInstancesToUpdate.clear();
        }

        if (this.field_187312_l && i % 4L == 0L)
        {
            this.field_187312_l = false;
            Collections.sort(this.field_187311_h, new Comparator<PlayerManager.PlayerInstance>()
            {
                public int compare(PlayerManager.PlayerInstance p_compare_1_, PlayerManager.PlayerInstance p_compare_2_)
                {
                    return ComparisonChain.start().compare(p_compare_1_.func_187270_g(), p_compare_2_.func_187270_g()).result();
                }
            });
        }

        if (this.field_187313_m && i % 4L == 2L)
        {
            this.field_187313_m = false;
            Collections.sort(this.field_187310_g, new Comparator<PlayerManager.PlayerInstance>()
            {
                public int compare(PlayerManager.PlayerInstance p_compare_1_, PlayerManager.PlayerInstance p_compare_2_)
                {
                    return ComparisonChain.start().compare(p_compare_1_.func_187270_g(), p_compare_2_.func_187270_g()).result();
                }
            });
        }

        if (!this.field_187311_h.isEmpty())
        {
            long l = System.nanoTime() + 50000000L;
            int k = 49;
            Iterator<PlayerManager.PlayerInstance> iterator = this.field_187311_h.iterator();

            while (iterator.hasNext())
            {
                PlayerManager.PlayerInstance playermanager$playerinstance1 = (PlayerManager.PlayerInstance)iterator.next();

                if (playermanager$playerinstance1.func_187266_f() == null)
                {
                    boolean flag = playermanager$playerinstance1.func_187269_a(field_187309_b);

                    if (playermanager$playerinstance1.func_187268_a(flag))
                    {
                        iterator.remove();

                        if (playermanager$playerinstance1.func_187272_b())
                        {
                            this.field_187310_g.remove(playermanager$playerinstance1);
                        }

                        --k;

                        if (k < 0 || System.nanoTime() > l)
                        {
                            break;
                        }
                    }
                }
            }
        }

        if (!this.field_187310_g.isEmpty())
        {
            int i1 = 81;
            Iterator<PlayerManager.PlayerInstance> iterator1 = this.field_187310_g.iterator();

            while (iterator1.hasNext())
            {
                PlayerManager.PlayerInstance playermanager$playerinstance3 = (PlayerManager.PlayerInstance)iterator1.next();

                if (playermanager$playerinstance3.func_187272_b())
                {
                    iterator1.remove();
                    --i1;

                    if (i1 < 0)
                    {
                        break;
                    }
                }
            }
        }

        if (this.players.isEmpty())
        {
            WorldProvider worldprovider = this.theWorldServer.provider;

            if (!worldprovider.canRespawnHere())
            {
                this.theWorldServer.getChunkProvider().unloadAllChunks();
            }
        }
    }

    public boolean hasPlayerInstance(int chunkX, int chunkZ)
    {
        long i = func_187307_d(chunkX, chunkZ);
        return this.playerInstances.getValueByKey(i) != null;
    }

    public PlayerManager.PlayerInstance func_187301_b(int p_187301_1_, int p_187301_2_)
    {
        return (PlayerManager.PlayerInstance)this.playerInstances.getValueByKey(func_187307_d(p_187301_1_, p_187301_2_));
    }

    private PlayerManager.PlayerInstance func_187302_c(int p_187302_1_, int p_187302_2_)
    {
        long i = func_187307_d(p_187302_1_, p_187302_2_);
        PlayerManager.PlayerInstance playermanager$playerinstance = (PlayerManager.PlayerInstance)this.playerInstances.getValueByKey(i);

        if (playermanager$playerinstance == null)
        {
            playermanager$playerinstance = new PlayerManager.PlayerInstance(p_187302_1_, p_187302_2_);
            this.playerInstances.add(i, playermanager$playerinstance);
            this.playerInstanceList.add(playermanager$playerinstance);

            if (playermanager$playerinstance.func_187266_f() == null)
            {
                this.field_187311_h.add(playermanager$playerinstance);
            }

            if (!playermanager$playerinstance.func_187272_b())
            {
                this.field_187310_g.add(playermanager$playerinstance);
            }
        }

        return playermanager$playerinstance;
    }

    public void markBlockForUpdate(BlockPos pos)
    {
        int i = pos.getX() >> 4;
        int j = pos.getZ() >> 4;
        PlayerManager.PlayerInstance playermanager$playerinstance = this.func_187301_b(i, j);

        if (playermanager$playerinstance != null)
        {
            playermanager$playerinstance.func_187265_a(pos.getX() & 15, pos.getY(), pos.getZ() & 15);
        }
    }

    /**
     * Adds an EntityPlayerMP to the PlayerManager and to all player instances within player visibility
     */
    public void addPlayer(EntityPlayerMP player)
    {
        int i = (int)player.posX >> 4;
        int j = (int)player.posZ >> 4;
        player.managedPosX = player.posX;
        player.managedPosZ = player.posZ;

        for (int k = i - this.playerViewRadius; k <= i + this.playerViewRadius; ++k)
        {
            for (int l = j - this.playerViewRadius; l <= j + this.playerViewRadius; ++l)
            {
                this.func_187302_c(k, l).addPlayer(player);
            }
        }

        this.players.add(player);
        this.func_187306_e();
    }

    /**
     * Removes an EntityPlayerMP from the PlayerManager.
     */
    public void removePlayer(EntityPlayerMP player)
    {
        int i = (int)player.managedPosX >> 4;
        int j = (int)player.managedPosZ >> 4;

        for (int k = i - this.playerViewRadius; k <= i + this.playerViewRadius; ++k)
        {
            for (int l = j - this.playerViewRadius; l <= j + this.playerViewRadius; ++l)
            {
                PlayerManager.PlayerInstance playermanager$playerinstance = this.func_187301_b(k, l);

                if (playermanager$playerinstance != null)
                {
                    playermanager$playerinstance.removePlayer(player);
                }
            }
        }

        this.players.remove(player);
        this.func_187306_e();
    }

    /**
     * Determine if two rectangles centered at the given points overlap for the provided radius. Arguments: x1, z1, x2,
     * z2, radius.
     */
    private boolean overlaps(int x1, int z1, int x2, int z2, int radius)
    {
        int i = x1 - x2;
        int j = z1 - z2;
        return i >= -radius && i <= radius ? j >= -radius && j <= radius : false;
    }

    /**
     * update chunks around a player being moved by server logic (e.g. cart, boat)
     */
    public void updateMountedMovingPlayer(EntityPlayerMP player)
    {
        int i = (int)player.posX >> 4;
        int j = (int)player.posZ >> 4;
        double d0 = player.managedPosX - player.posX;
        double d1 = player.managedPosZ - player.posZ;
        double d2 = d0 * d0 + d1 * d1;

        if (d2 >= 64.0D)
        {
            int k = (int)player.managedPosX >> 4;
            int l = (int)player.managedPosZ >> 4;
            int i1 = this.playerViewRadius;
            int j1 = i - k;
            int k1 = j - l;

            if (j1 != 0 || k1 != 0)
            {
                for (int l1 = i - i1; l1 <= i + i1; ++l1)
                {
                    for (int i2 = j - i1; i2 <= j + i1; ++i2)
                    {
                        if (!this.overlaps(l1, i2, k, l, i1))
                        {
                            this.func_187302_c(l1, i2).addPlayer(player);
                        }

                        if (!this.overlaps(l1 - j1, i2 - k1, i, j, i1))
                        {
                            PlayerManager.PlayerInstance playermanager$playerinstance = this.func_187301_b(l1 - j1, i2 - k1);

                            if (playermanager$playerinstance != null)
                            {
                                playermanager$playerinstance.removePlayer(player);
                            }
                        }
                    }
                }

                player.managedPosX = player.posX;
                player.managedPosZ = player.posZ;
                this.func_187306_e();
            }
        }
    }

    public boolean isPlayerWatchingChunk(EntityPlayerMP player, int chunkX, int chunkZ)
    {
        PlayerManager.PlayerInstance playermanager$playerinstance = this.func_187301_b(chunkX, chunkZ);
        return playermanager$playerinstance != null && playermanager$playerinstance.func_187275_d(player) && playermanager$playerinstance.func_187274_e();
    }

    public void setPlayerViewRadius(int radius)
    {
        radius = MathHelper.clamp_int(radius, 3, 32);

        if (radius != this.playerViewRadius)
        {
            int i = radius - this.playerViewRadius;

            for (EntityPlayerMP entityplayermp : Lists.newArrayList(this.players))
            {
                int j = (int)entityplayermp.posX >> 4;
                int k = (int)entityplayermp.posZ >> 4;

                if (i > 0)
                {
                    for (int j1 = j - radius; j1 <= j + radius; ++j1)
                    {
                        for (int k1 = k - radius; k1 <= k + radius; ++k1)
                        {
                            PlayerManager.PlayerInstance playermanager$playerinstance = this.func_187302_c(j1, k1);

                            if (!playermanager$playerinstance.func_187275_d(entityplayermp))
                            {
                                playermanager$playerinstance.addPlayer(entityplayermp);
                            }
                        }
                    }
                }
                else
                {
                    for (int l = j - this.playerViewRadius; l <= j + this.playerViewRadius; ++l)
                    {
                        for (int i1 = k - this.playerViewRadius; i1 <= k + this.playerViewRadius; ++i1)
                        {
                            if (!this.overlaps(l, i1, j, k, radius))
                            {
                                this.func_187302_c(l, i1).removePlayer(entityplayermp);
                            }
                        }
                    }
                }
            }

            this.playerViewRadius = radius;
            this.func_187306_e();
        }
    }

    private void func_187306_e()
    {
        this.field_187312_l = true;
        this.field_187313_m = true;
    }

    /**
     * Get the furthest viewable block given player's view distance
     */
    public static int getFurthestViewableBlock(int distance)
    {
        return distance * 16 - 16;
    }

    private static long func_187307_d(int p_187307_0_, int p_187307_1_)
    {
        return (long)p_187307_0_ + 2147483647L | (long)p_187307_1_ + 2147483647L << 32;
    }

    public void func_187304_a(PlayerManager.PlayerInstance p_187304_1_)
    {
        this.playerInstancesToUpdate.add(p_187304_1_);
    }

    public void func_187305_b(PlayerManager.PlayerInstance p_187305_1_)
    {
        ChunkCoordIntPair chunkcoordintpair = p_187305_1_.func_187264_a();
        long i = func_187307_d(chunkcoordintpair.chunkXPos, chunkcoordintpair.chunkZPos);
        p_187305_1_.func_187279_c();
        this.playerInstances.remove(i);
        this.playerInstanceList.remove(p_187305_1_);
        this.playerInstancesToUpdate.remove(p_187305_1_);
        this.field_187310_g.remove(p_187305_1_);
        this.field_187311_h.remove(p_187305_1_);
        this.getWorldServer().getChunkProvider().dropChunk(chunkcoordintpair.chunkXPos, chunkcoordintpair.chunkZPos);
    }

    public class PlayerInstance
    {
        private final Logger logger = LogManager.getLogger();
        private final List<EntityPlayerMP> players = Lists.<EntityPlayerMP>newArrayList();
        private final ChunkCoordIntPair field_187284_d;
        private final short[] field_187285_e = new short[64];
        private Chunk field_187286_f;
        private int field_187287_g;
        private int field_187288_h;
        private long field_187289_i;
        private boolean field_187290_j;

        public PlayerInstance(int chunkX, int chunkZ)
        {
            this.field_187284_d = new ChunkCoordIntPair(chunkX, chunkZ);
            this.field_187286_f = PlayerManager.this.getWorldServer().getChunkProvider().func_186028_c(chunkX, chunkZ);
        }

        public ChunkCoordIntPair func_187264_a()
        {
            return this.field_187284_d;
        }

        public void addPlayer(EntityPlayerMP player)
        {
            if (this.players.contains(player))
            {
                logger.debug("Failed to add player. {} already is in chunk {}, {}", new Object[] {player, Integer.valueOf(this.field_187284_d.chunkXPos), Integer.valueOf(this.field_187284_d.chunkZPos)});
            }
            else
            {
                if (this.players.isEmpty())
                {
                    this.field_187289_i = PlayerManager.this.getWorldServer().getTotalWorldTime();
                }

                this.players.add(player);

                if (this.field_187290_j)
                {
                    this.func_187278_c(player);
                }
            }
        }

        public void removePlayer(EntityPlayerMP player)
        {
            if (this.players.contains(player))
            {
                if (this.field_187290_j)
                {
                    player.playerNetServerHandler.sendPacket(new SPacketUnloadChunk(this.field_187284_d.chunkXPos, this.field_187284_d.chunkZPos));
                }

                this.players.remove(player);

                if (this.players.isEmpty())
                {
                    PlayerManager.this.func_187305_b(this);
                }
            }
        }

        public boolean func_187268_a(boolean p_187268_1_)
        {
            if (this.field_187286_f != null)
            {
                return true;
            }
            else
            {
                if (p_187268_1_)
                {
                    this.field_187286_f = PlayerManager.this.getWorldServer().getChunkProvider().func_186025_d(this.field_187284_d.chunkXPos, this.field_187284_d.chunkZPos);
                }
                else
                {
                    this.field_187286_f = PlayerManager.this.getWorldServer().getChunkProvider().func_186028_c(this.field_187284_d.chunkXPos, this.field_187284_d.chunkZPos);
                }

                return this.field_187286_f != null;
            }
        }

        public boolean func_187272_b()
        {
            if (this.field_187290_j)
            {
                return true;
            }
            else if (this.field_187286_f == null)
            {
                return false;
            }
            else if (!this.field_187286_f.isPopulated())
            {
                return false;
            }
            else
            {
                this.field_187287_g = 0;
                this.field_187288_h = 0;
                this.field_187290_j = true;
                List<TileEntity> list = Lists.newArrayList(PlayerManager.this.getWorldServer().getTileEntitiesIn(this.field_187284_d.chunkXPos * 16, 0, this.field_187284_d.chunkZPos * 16, this.field_187284_d.chunkXPos * 16 + 16, 256, this.field_187284_d.chunkZPos * 16 + 16));
                SPacketChunkData spacketchunkdata = new SPacketChunkData(this.field_187286_f, true, 65535);

                for (EntityPlayerMP entityplayermp : this.players)
                {
                    entityplayermp.playerNetServerHandler.sendPacket(spacketchunkdata);

                    for (TileEntity tileentity : list)
                    {
                        Packet<?> packet = tileentity.getDescriptionPacket();

                        if (packet != null)
                        {
                            entityplayermp.playerNetServerHandler.sendPacket(packet);
                        }
                    }

                    PlayerManager.this.getWorldServer().getEntityTracker().func_85172_a(entityplayermp, this.field_187286_f);
                }

                return true;
            }
        }

        public void func_187278_c(EntityPlayerMP p_187278_1_)
        {
            if (this.field_187290_j)
            {
                p_187278_1_.playerNetServerHandler.sendPacket(new SPacketChunkData(this.field_187286_f, true, 65535));

                for (TileEntity tileentity : PlayerManager.this.getWorldServer().getTileEntitiesIn(this.field_187284_d.chunkXPos * 16, 0, this.field_187284_d.chunkZPos * 16, this.field_187284_d.chunkXPos * 16 + 16, 256, this.field_187284_d.chunkZPos * 16 + 16))
                {
                    Packet<?> packet = tileentity.getDescriptionPacket();

                    if (packet != null)
                    {
                        p_187278_1_.playerNetServerHandler.sendPacket(packet);
                    }
                }

                PlayerManager.this.getWorldServer().getEntityTracker().func_85172_a(p_187278_1_, this.field_187286_f);
            }
        }

        public void func_187279_c()
        {
            if (this.field_187286_f != null)
            {
                this.field_187286_f.setInhabitedTime(this.field_187286_f.getInhabitedTime() + PlayerManager.this.getWorldServer().getTotalWorldTime() - this.field_187289_i);
            }

            this.field_187289_i = PlayerManager.this.getWorldServer().getTotalWorldTime();
        }

        public void func_187265_a(int p_187265_1_, int p_187265_2_, int p_187265_3_)
        {
            if (this.field_187290_j)
            {
                if (this.field_187287_g == 0)
                {
                    PlayerManager.this.func_187304_a(this);
                }

                this.field_187288_h |= 1 << (p_187265_2_ >> 4);

                if (this.field_187287_g < 64)
                {
                    short short1 = (short)(p_187265_1_ << 12 | p_187265_3_ << 8 | p_187265_2_);

                    for (int i = 0; i < this.field_187287_g; ++i)
                    {
                        if (this.field_187285_e[i] == short1)
                        {
                            return;
                        }
                    }

                    this.field_187285_e[this.field_187287_g++] = short1;
                }
            }
        }

        public void func_187267_a(Packet<?> p_187267_1_)
        {
            if (this.field_187290_j)
            {
                for (int i = 0; i < this.players.size(); ++i)
                {
                    ((EntityPlayerMP)this.players.get(i)).playerNetServerHandler.sendPacket(p_187267_1_);
                }
            }
        }

        public void func_187280_d()
        {
            if (this.field_187290_j && this.field_187286_f != null)
            {
                if (this.field_187287_g != 0)
                {
                    if (this.field_187287_g == 1)
                    {
                        int i = (this.field_187285_e[0] >> 12 & 15) + this.field_187284_d.chunkXPos * 16;
                        int j = this.field_187285_e[0] & 255;
                        int k = (this.field_187285_e[0] >> 8 & 15) + this.field_187284_d.chunkZPos * 16;
                        BlockPos blockpos = new BlockPos(i, j, k);
                        this.func_187267_a(new SPacketBlockChange(PlayerManager.this.getWorldServer(), blockpos));

                        if (PlayerManager.this.getWorldServer().getBlockState(blockpos).getBlock().hasTileEntity())
                        {
                            this.func_187273_a(PlayerManager.this.getWorldServer().getTileEntity(blockpos));
                        }
                    }
                    else if (this.field_187287_g == 64)
                    {
                        int i1 = this.field_187284_d.chunkXPos * 16;
                        int k1 = this.field_187284_d.chunkZPos * 16;
                        this.func_187267_a(new SPacketChunkData(this.field_187286_f, false, this.field_187288_h));

                        for (int i2 = 0; i2 < 16; ++i2)
                        {
                            if ((this.field_187288_h & 1 << i2) != 0)
                            {
                                int k2 = i2 << 4;
                                List<TileEntity> list = PlayerManager.this.getWorldServer().getTileEntitiesIn(i1, k2, k1, i1 + 16, k2 + 16, k1 + 16);

                                for (int l = 0; l < list.size(); ++l)
                                {
                                    this.func_187273_a((TileEntity)list.get(l));
                                }
                            }
                        }
                    }
                    else
                    {
                        this.func_187267_a(new SPacketMultiBlockChange(this.field_187287_g, this.field_187285_e, this.field_187286_f));

                        for (int j1 = 0; j1 < this.field_187287_g; ++j1)
                        {
                            int l1 = (this.field_187285_e[j1] >> 12 & 15) + this.field_187284_d.chunkXPos * 16;
                            int j2 = this.field_187285_e[j1] & 255;
                            int l2 = (this.field_187285_e[j1] >> 8 & 15) + this.field_187284_d.chunkZPos * 16;
                            BlockPos blockpos1 = new BlockPos(l1, j2, l2);

                            if (PlayerManager.this.getWorldServer().getBlockState(blockpos1).getBlock().hasTileEntity())
                            {
                                this.func_187273_a(PlayerManager.this.getWorldServer().getTileEntity(blockpos1));
                            }
                        }
                    }

                    this.field_187287_g = 0;
                    this.field_187288_h = 0;
                }
            }
        }

        private void func_187273_a(TileEntity p_187273_1_)
        {
            if (p_187273_1_ != null)
            {
                Packet<?> packet = p_187273_1_.getDescriptionPacket();

                if (packet != null)
                {
                    this.func_187267_a(packet);
                }
            }
        }

        public boolean func_187275_d(EntityPlayerMP p_187275_1_)
        {
            return this.players.contains(p_187275_1_);
        }

        public boolean func_187269_a(Predicate<EntityPlayerMP> p_187269_1_)
        {
            return Iterables.tryFind(this.players, p_187269_1_).isPresent();
        }

        public boolean func_187271_a(double p_187271_1_, Predicate<EntityPlayerMP> p_187271_3_)
        {
            int i = 0;

            for (int j = this.players.size(); i < j; ++i)
            {
                EntityPlayerMP entityplayermp = (EntityPlayerMP)this.players.get(i);

                if (p_187271_3_.apply(entityplayermp) && this.field_187284_d.func_185327_a(entityplayermp) < p_187271_1_ * p_187271_1_)
                {
                    return true;
                }
            }

            return false;
        }

        public boolean func_187274_e()
        {
            return this.field_187290_j;
        }

        public Chunk func_187266_f()
        {
            return this.field_187286_f;
        }

        public double func_187270_g()
        {
            double d0 = Double.MAX_VALUE;

            for (EntityPlayerMP entityplayermp : this.players)
            {
                double d1 = this.field_187284_d.func_185327_a(entityplayermp);

                if (d1 < d0)
                {
                    d0 = d1;
                }
            }

            return d0;
        }
    }
}
