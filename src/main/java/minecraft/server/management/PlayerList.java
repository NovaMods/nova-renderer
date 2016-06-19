package net.minecraft.server.management;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import com.mojang.authlib.GameProfile;
import io.netty.buffer.Unpooled;
import java.io.File;
import java.net.SocketAddress;
import java.text.SimpleDateFormat;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.NetHandlerPlayServer;
import net.minecraft.network.NetworkManager;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.server.SPacketChangeGameState;
import net.minecraft.network.play.server.SPacketChat;
import net.minecraft.network.play.server.SPacketCustomPayload;
import net.minecraft.network.play.server.SPacketEntityEffect;
import net.minecraft.network.play.server.SPacketEntityStatus;
import net.minecraft.network.play.server.SPacketHeldItemChange;
import net.minecraft.network.play.server.SPacketJoinGame;
import net.minecraft.network.play.server.SPacketPlayerAbilities;
import net.minecraft.network.play.server.SPacketPlayerListItem;
import net.minecraft.network.play.server.SPacketRespawn;
import net.minecraft.network.play.server.SPacketServerDifficulty;
import net.minecraft.network.play.server.SPacketSetExperience;
import net.minecraft.network.play.server.SPacketSpawnPosition;
import net.minecraft.network.play.server.SPacketTeams;
import net.minecraft.network.play.server.SPacketTimeUpdate;
import net.minecraft.network.play.server.SPacketWorldBorder;
import net.minecraft.potion.PotionEffect;
import net.minecraft.scoreboard.ScoreObjective;
import net.minecraft.scoreboard.ScorePlayerTeam;
import net.minecraft.scoreboard.ServerScoreboard;
import net.minecraft.scoreboard.Team;
import net.minecraft.server.MinecraftServer;
import net.minecraft.stats.StatList;
import net.minecraft.stats.StatisticsFile;
import net.minecraft.util.datafix.FixTypes;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;
import net.minecraft.world.WorldSettings;
import net.minecraft.world.border.IBorderListener;
import net.minecraft.world.border.WorldBorder;
import net.minecraft.world.chunk.storage.AnvilChunkLoader;
import net.minecraft.world.demo.DemoWorldManager;
import net.minecraft.world.storage.IPlayerFileData;
import net.minecraft.world.storage.WorldInfo;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class PlayerList
{
    public static final File FILE_PLAYERBANS = new File("banned-players.json");
    public static final File FILE_IPBANS = new File("banned-ips.json");
    public static final File FILE_OPS = new File("ops.json");
    public static final File FILE_WHITELIST = new File("whitelist.json");
    private static final Logger logger = LogManager.getLogger();
    private static final SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd \'at\' HH:mm:ss z");

    /** Reference to the MinecraftServer object. */
    private final MinecraftServer mcServer;
    private final List<EntityPlayerMP> playerEntityList = Lists.<EntityPlayerMP>newArrayList();
    private final Map<UUID, EntityPlayerMP> uuidToPlayerMap = Maps.<UUID, EntityPlayerMP>newHashMap();
    private final UserListBans bannedPlayers;
    private final UserListIPBans bannedIPs;

    /** A set containing the OPs. */
    private final UserListOps ops;

    /** The Set of all whitelisted players. */
    private final UserListWhitelist whiteListedPlayers;
    private final Map<UUID, StatisticsFile> playerStatFiles;

    /** Reference to the PlayerNBTManager object. */
    private IPlayerFileData playerNBTManagerObj;

    /**
     * Server setting to only allow OPs and whitelisted players to join the server.
     */
    private boolean whiteListEnforced;

    /** The maximum number of players that can be connected at a time. */
    protected int maxPlayers;
    private int viewDistance;
    private WorldSettings.GameType gameType;

    /** True if all players are allowed to use commands (cheats). */
    private boolean commandsAllowedForAll;

    /**
     * index into playerEntities of player to ping, updated every tick; currently hardcoded to max at 200 players
     */
    private int playerPingIndex;

    public PlayerList(MinecraftServer server)
    {
        this.bannedPlayers = new UserListBans(FILE_PLAYERBANS);
        this.bannedIPs = new UserListIPBans(FILE_IPBANS);
        this.ops = new UserListOps(FILE_OPS);
        this.whiteListedPlayers = new UserListWhitelist(FILE_WHITELIST);
        this.playerStatFiles = Maps.<UUID, StatisticsFile>newHashMap();
        this.mcServer = server;
        this.bannedPlayers.setLanServer(false);
        this.bannedIPs.setLanServer(false);
        this.maxPlayers = 8;
    }

    public void initializeConnectionToPlayer(NetworkManager netManager, EntityPlayerMP playerIn)
    {
        GameProfile gameprofile = playerIn.getGameProfile();
        PlayerProfileCache playerprofilecache = this.mcServer.getPlayerProfileCache();
        GameProfile gameprofile1 = playerprofilecache.getProfileByUUID(gameprofile.getId());
        String s = gameprofile1 == null ? gameprofile.getName() : gameprofile1.getName();
        playerprofilecache.addEntry(gameprofile);
        NBTTagCompound nbttagcompound = this.readPlayerDataFromFile(playerIn);
        playerIn.setWorld(this.mcServer.worldServerForDimension(playerIn.dimension));
        playerIn.theItemInWorldManager.setWorld((WorldServer)playerIn.worldObj);
        String s1 = "local";

        if (netManager.getRemoteAddress() != null)
        {
            s1 = netManager.getRemoteAddress().toString();
        }

        logger.info(playerIn.getName() + "[" + s1 + "] logged in with entity id " + playerIn.getEntityId() + " at (" + playerIn.posX + ", " + playerIn.posY + ", " + playerIn.posZ + ")");
        WorldServer worldserver = this.mcServer.worldServerForDimension(playerIn.dimension);
        WorldInfo worldinfo = worldserver.getWorldInfo();
        BlockPos blockpos = worldserver.getSpawnPoint();
        this.setPlayerGameTypeBasedOnOther(playerIn, (EntityPlayerMP)null, worldserver);
        NetHandlerPlayServer nethandlerplayserver = new NetHandlerPlayServer(this.mcServer, netManager, playerIn);
        nethandlerplayserver.sendPacket(new SPacketJoinGame(playerIn.getEntityId(), playerIn.theItemInWorldManager.getGameType(), worldinfo.isHardcoreModeEnabled(), worldserver.provider.getDimensionType().getId(), worldserver.getDifficulty(), this.getMaxPlayers(), worldinfo.getTerrainType(), worldserver.getGameRules().getBoolean("reducedDebugInfo")));
        nethandlerplayserver.sendPacket(new SPacketCustomPayload("MC|Brand", (new PacketBuffer(Unpooled.buffer())).writeString(this.getServerInstance().getServerModName())));
        nethandlerplayserver.sendPacket(new SPacketServerDifficulty(worldinfo.getDifficulty(), worldinfo.isDifficultyLocked()));
        nethandlerplayserver.sendPacket(new SPacketSpawnPosition(blockpos));
        nethandlerplayserver.sendPacket(new SPacketPlayerAbilities(playerIn.capabilities));
        nethandlerplayserver.sendPacket(new SPacketHeldItemChange(playerIn.inventory.currentItem));
        this.func_187243_f(playerIn);
        playerIn.getStatFile().func_150877_d();
        playerIn.getStatFile().sendAchievements(playerIn);
        this.sendScoreboard((ServerScoreboard)worldserver.getScoreboard(), playerIn);
        this.mcServer.refreshStatusNextTick();
        TextComponentTranslation textcomponenttranslation;

        if (!playerIn.getName().equalsIgnoreCase(s))
        {
            textcomponenttranslation = new TextComponentTranslation("multiplayer.player.joined.renamed", new Object[] {playerIn.getDisplayName(), s});
        }
        else
        {
            textcomponenttranslation = new TextComponentTranslation("multiplayer.player.joined", new Object[] {playerIn.getDisplayName()});
        }

        textcomponenttranslation.getChatStyle().setColor(TextFormatting.YELLOW);
        this.sendChatMsg(textcomponenttranslation);
        this.playerLoggedIn(playerIn);
        nethandlerplayserver.setPlayerLocation(playerIn.posX, playerIn.posY, playerIn.posZ, playerIn.rotationYaw, playerIn.rotationPitch);
        this.updateTimeAndWeatherForPlayer(playerIn, worldserver);

        if (!this.mcServer.getResourcePackUrl().isEmpty())
        {
            playerIn.loadResourcePack(this.mcServer.getResourcePackUrl(), this.mcServer.getResourcePackHash());
        }

        for (PotionEffect potioneffect : playerIn.getActivePotionEffects())
        {
            nethandlerplayserver.sendPacket(new SPacketEntityEffect(playerIn.getEntityId(), potioneffect));
        }

        if (nbttagcompound != null)
        {
            if (nbttagcompound.hasKey("RootVehicle", 10))
            {
                NBTTagCompound nbttagcompound1 = nbttagcompound.getCompoundTag("RootVehicle");
                Entity entity2 = AnvilChunkLoader.func_186051_a(nbttagcompound1.getCompoundTag("Entity"), worldserver, true);

                if (entity2 != null)
                {
                    UUID uuid = nbttagcompound1.getUniqueId("Attach");

                    if (entity2.getUniqueID().equals(uuid))
                    {
                        playerIn.startRiding(entity2, true);
                    }
                    else
                    {
                        for (Entity entity : entity2.func_184182_bu())
                        {
                            if (entity.getUniqueID().equals(uuid))
                            {
                                playerIn.startRiding(entity, true);
                                break;
                            }
                        }
                    }

                    if (!playerIn.isRiding())
                    {
                        logger.warn("Couldn\'t reattach entity to player");
                        worldserver.removePlayerEntityDangerously(entity2);

                        for (Entity entity3 : entity2.func_184182_bu())
                        {
                            worldserver.removePlayerEntityDangerously(entity3);
                        }
                    }
                }
            }
            else if (nbttagcompound.hasKey("Riding", 10))
            {
                Entity entity1 = AnvilChunkLoader.func_186051_a(nbttagcompound.getCompoundTag("Riding"), worldserver, true);

                if (entity1 != null)
                {
                    playerIn.startRiding(entity1, true);
                }
            }
        }

        playerIn.addSelfToInternalCraftingInventory();
    }

    protected void sendScoreboard(ServerScoreboard scoreboardIn, EntityPlayerMP playerIn)
    {
        Set<ScoreObjective> set = Sets.<ScoreObjective>newHashSet();

        for (ScorePlayerTeam scoreplayerteam : scoreboardIn.getTeams())
        {
            playerIn.playerNetServerHandler.sendPacket(new SPacketTeams(scoreplayerteam, 0));
        }

        for (int i = 0; i < 19; ++i)
        {
            ScoreObjective scoreobjective = scoreboardIn.getObjectiveInDisplaySlot(i);

            if (scoreobjective != null && !set.contains(scoreobjective))
            {
                for (Packet<?> packet : scoreboardIn.func_96550_d(scoreobjective))
                {
                    playerIn.playerNetServerHandler.sendPacket(packet);
                }

                set.add(scoreobjective);
            }
        }
    }

    /**
     * Sets the NBT manager to the one for the WorldServer given.
     */
    public void setPlayerManager(WorldServer[] worldServers)
    {
        this.playerNBTManagerObj = worldServers[0].getSaveHandler().getPlayerNBTManager();
        worldServers[0].getWorldBorder().addListener(new IBorderListener()
        {
            public void onSizeChanged(WorldBorder border, double newSize)
            {
                PlayerList.this.sendPacketToAllPlayers(new SPacketWorldBorder(border, SPacketWorldBorder.Action.SET_SIZE));
            }
            public void onTransitionStarted(WorldBorder border, double oldSize, double newSize, long time)
            {
                PlayerList.this.sendPacketToAllPlayers(new SPacketWorldBorder(border, SPacketWorldBorder.Action.LERP_SIZE));
            }
            public void onCenterChanged(WorldBorder border, double x, double z)
            {
                PlayerList.this.sendPacketToAllPlayers(new SPacketWorldBorder(border, SPacketWorldBorder.Action.SET_CENTER));
            }
            public void onWarningTimeChanged(WorldBorder border, int newTime)
            {
                PlayerList.this.sendPacketToAllPlayers(new SPacketWorldBorder(border, SPacketWorldBorder.Action.SET_WARNING_TIME));
            }
            public void onWarningDistanceChanged(WorldBorder border, int newDistance)
            {
                PlayerList.this.sendPacketToAllPlayers(new SPacketWorldBorder(border, SPacketWorldBorder.Action.SET_WARNING_BLOCKS));
            }
            public void onDamageAmountChanged(WorldBorder border, double newAmount)
            {
            }
            public void onDamageBufferChanged(WorldBorder border, double newSize)
            {
            }
        });
    }

    public void preparePlayer(EntityPlayerMP playerIn, WorldServer worldIn)
    {
        WorldServer worldserver = playerIn.getServerForPlayer();

        if (worldIn != null)
        {
            worldIn.getPlayerChunkManager().removePlayer(playerIn);
        }

        worldserver.getPlayerChunkManager().addPlayer(playerIn);
        worldserver.getChunkProvider().func_186025_d((int)playerIn.posX >> 4, (int)playerIn.posZ >> 4);
    }

    public int getEntityViewDistance()
    {
        return PlayerManager.getFurthestViewableBlock(this.getViewDistance());
    }

    /**
     * called during player login. reads the player information from disk.
     */
    public NBTTagCompound readPlayerDataFromFile(EntityPlayerMP playerIn)
    {
        NBTTagCompound nbttagcompound = this.mcServer.worldServers[0].getWorldInfo().getPlayerNBTTagCompound();
        NBTTagCompound nbttagcompound1;

        if (playerIn.getName().equals(this.mcServer.getServerOwner()) && nbttagcompound != null)
        {
            nbttagcompound1 = this.mcServer.getDataFixer().func_188257_a(FixTypes.PLAYER, nbttagcompound);
            playerIn.readFromNBT(nbttagcompound1);
            logger.debug("loading single player");
        }
        else
        {
            nbttagcompound1 = this.playerNBTManagerObj.readPlayerData(playerIn);
        }

        return nbttagcompound1;
    }

    /**
     * also stores the NBTTags if this is an intergratedPlayerList
     */
    protected void writePlayerData(EntityPlayerMP playerIn)
    {
        this.playerNBTManagerObj.writePlayerData(playerIn);
        StatisticsFile statisticsfile = (StatisticsFile)this.playerStatFiles.get(playerIn.getUniqueID());

        if (statisticsfile != null)
        {
            statisticsfile.saveStatFile();
        }
    }

    /**
     * Called when a player successfully logs in. Reads player data from disk and inserts the player into the world.
     */
    public void playerLoggedIn(EntityPlayerMP playerIn)
    {
        this.playerEntityList.add(playerIn);
        this.uuidToPlayerMap.put(playerIn.getUniqueID(), playerIn);
        this.sendPacketToAllPlayers(new SPacketPlayerListItem(SPacketPlayerListItem.Action.ADD_PLAYER, new EntityPlayerMP[] {playerIn}));
        WorldServer worldserver = this.mcServer.worldServerForDimension(playerIn.dimension);

        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            playerIn.playerNetServerHandler.sendPacket(new SPacketPlayerListItem(SPacketPlayerListItem.Action.ADD_PLAYER, new EntityPlayerMP[] {(EntityPlayerMP)this.playerEntityList.get(i)}));
        }

        worldserver.spawnEntityInWorld(playerIn);
        this.preparePlayer(playerIn, (WorldServer)null);
    }

    /**
     * using player's dimension, update their movement when in a vehicle (e.g. cart, boat)
     */
    public void serverUpdateMountedMovingPlayer(EntityPlayerMP playerIn)
    {
        playerIn.getServerForPlayer().getPlayerChunkManager().updateMountedMovingPlayer(playerIn);
    }

    /**
     * Called when a player disconnects from the game. Writes player data to disk and removes them from the world.
     */
    public void playerLoggedOut(EntityPlayerMP playerIn)
    {
        WorldServer worldserver = playerIn.getServerForPlayer();
        playerIn.triggerAchievement(StatList.leaveGameStat);
        this.writePlayerData(playerIn);

        if (playerIn.isRiding())
        {
            Entity entity = playerIn.getLowestRidingEntity();

            if (entity.func_184180_b(EntityPlayerMP.class).size() == 1)
            {
                logger.debug("Removing player mount");
                playerIn.dismountRidingEntity();
                worldserver.removePlayerEntityDangerously(entity);

                for (Entity entity1 : entity.func_184182_bu())
                {
                    worldserver.removePlayerEntityDangerously(entity1);
                }

                worldserver.getChunkFromChunkCoords(playerIn.chunkCoordX, playerIn.chunkCoordZ).setChunkModified();
            }
        }

        worldserver.removeEntity(playerIn);
        worldserver.getPlayerChunkManager().removePlayer(playerIn);
        this.playerEntityList.remove(playerIn);
        UUID uuid = playerIn.getUniqueID();
        EntityPlayerMP entityplayermp = (EntityPlayerMP)this.uuidToPlayerMap.get(uuid);

        if (entityplayermp == playerIn)
        {
            this.uuidToPlayerMap.remove(uuid);
            this.playerStatFiles.remove(uuid);
        }

        this.sendPacketToAllPlayers(new SPacketPlayerListItem(SPacketPlayerListItem.Action.REMOVE_PLAYER, new EntityPlayerMP[] {playerIn}));
    }

    /**
     * checks ban-lists, then white-lists, then space for the server. Returns null on success, or an error message
     */
    public String allowUserToConnect(SocketAddress address, GameProfile profile)
    {
        if (this.bannedPlayers.isBanned(profile))
        {
            UserListBansEntry userlistbansentry = (UserListBansEntry)this.bannedPlayers.getEntry(profile);
            String s1 = "You are banned from this server!\nReason: " + userlistbansentry.getBanReason();

            if (userlistbansentry.getBanEndDate() != null)
            {
                s1 = s1 + "\nYour ban will be removed on " + dateFormat.format(userlistbansentry.getBanEndDate());
            }

            return s1;
        }
        else if (!this.canJoin(profile))
        {
            return "You are not white-listed on this server!";
        }
        else if (this.bannedIPs.isBanned(address))
        {
            UserListIPBansEntry userlistipbansentry = this.bannedIPs.getBanEntry(address);
            String s = "Your IP address is banned from this server!\nReason: " + userlistipbansentry.getBanReason();

            if (userlistipbansentry.getBanEndDate() != null)
            {
                s = s + "\nYour ban will be removed on " + dateFormat.format(userlistipbansentry.getBanEndDate());
            }

            return s;
        }
        else
        {
            return this.playerEntityList.size() >= this.maxPlayers && !this.bypassesPlayerLimit(profile) ? "The server is full!" : null;
        }
    }

    /**
     * also checks for multiple logins across servers
     */
    public EntityPlayerMP createPlayerForUser(GameProfile profile)
    {
        UUID uuid = EntityPlayer.getUUID(profile);
        List<EntityPlayerMP> list = Lists.<EntityPlayerMP>newArrayList();

        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            EntityPlayerMP entityplayermp = (EntityPlayerMP)this.playerEntityList.get(i);

            if (entityplayermp.getUniqueID().equals(uuid))
            {
                list.add(entityplayermp);
            }
        }

        EntityPlayerMP entityplayermp2 = (EntityPlayerMP)this.uuidToPlayerMap.get(profile.getId());

        if (entityplayermp2 != null && !list.contains(entityplayermp2))
        {
            list.add(entityplayermp2);
        }

        for (EntityPlayerMP entityplayermp1 : list)
        {
            entityplayermp1.playerNetServerHandler.kickPlayerFromServer("You logged in from another location");
        }

        PlayerInteractionManager playerinteractionmanager;

        if (this.mcServer.isDemo())
        {
            playerinteractionmanager = new DemoWorldManager(this.mcServer.worldServerForDimension(0));
        }
        else
        {
            playerinteractionmanager = new PlayerInteractionManager(this.mcServer.worldServerForDimension(0));
        }

        return new EntityPlayerMP(this.mcServer, this.mcServer.worldServerForDimension(0), profile, playerinteractionmanager);
    }

    /**
     * Called on respawn
     */
    public EntityPlayerMP recreatePlayerEntity(EntityPlayerMP playerIn, int dimension, boolean conqueredEnd)
    {
        playerIn.getServerForPlayer().getEntityTracker().removePlayerFromTrackers(playerIn);
        playerIn.getServerForPlayer().getEntityTracker().untrackEntity(playerIn);
        playerIn.getServerForPlayer().getPlayerChunkManager().removePlayer(playerIn);
        this.playerEntityList.remove(playerIn);
        this.mcServer.worldServerForDimension(playerIn.dimension).removePlayerEntityDangerously(playerIn);
        BlockPos blockpos = playerIn.getBedLocation();
        boolean flag = playerIn.isSpawnForced();
        playerIn.dimension = dimension;
        PlayerInteractionManager playerinteractionmanager;

        if (this.mcServer.isDemo())
        {
            playerinteractionmanager = new DemoWorldManager(this.mcServer.worldServerForDimension(playerIn.dimension));
        }
        else
        {
            playerinteractionmanager = new PlayerInteractionManager(this.mcServer.worldServerForDimension(playerIn.dimension));
        }

        EntityPlayerMP entityplayermp = new EntityPlayerMP(this.mcServer, this.mcServer.worldServerForDimension(playerIn.dimension), playerIn.getGameProfile(), playerinteractionmanager);
        entityplayermp.playerNetServerHandler = playerIn.playerNetServerHandler;
        entityplayermp.clonePlayer(playerIn, conqueredEnd);
        entityplayermp.setEntityId(playerIn.getEntityId());
        entityplayermp.setCommandStats(playerIn);
        entityplayermp.func_184819_a(playerIn.getPrimaryHand());

        for (String s : playerIn.getTags())
        {
            entityplayermp.addTag(s);
        }

        WorldServer worldserver = this.mcServer.worldServerForDimension(playerIn.dimension);
        this.setPlayerGameTypeBasedOnOther(entityplayermp, playerIn, worldserver);

        if (blockpos != null)
        {
            BlockPos blockpos1 = EntityPlayer.getBedSpawnLocation(this.mcServer.worldServerForDimension(playerIn.dimension), blockpos, flag);

            if (blockpos1 != null)
            {
                entityplayermp.setLocationAndAngles((double)((float)blockpos1.getX() + 0.5F), (double)((float)blockpos1.getY() + 0.1F), (double)((float)blockpos1.getZ() + 0.5F), 0.0F, 0.0F);
                entityplayermp.setSpawnPoint(blockpos, flag);
            }
            else
            {
                entityplayermp.playerNetServerHandler.sendPacket(new SPacketChangeGameState(0, 0.0F));
            }
        }

        worldserver.getChunkProvider().func_186025_d((int)entityplayermp.posX >> 4, (int)entityplayermp.posZ >> 4);

        while (!worldserver.func_184144_a(entityplayermp, entityplayermp.getEntityBoundingBox()).isEmpty() && entityplayermp.posY < 256.0D)
        {
            entityplayermp.setPosition(entityplayermp.posX, entityplayermp.posY + 1.0D, entityplayermp.posZ);
        }

        entityplayermp.playerNetServerHandler.sendPacket(new SPacketRespawn(entityplayermp.dimension, entityplayermp.worldObj.getDifficulty(), entityplayermp.worldObj.getWorldInfo().getTerrainType(), entityplayermp.theItemInWorldManager.getGameType()));
        BlockPos blockpos2 = worldserver.getSpawnPoint();
        entityplayermp.playerNetServerHandler.setPlayerLocation(entityplayermp.posX, entityplayermp.posY, entityplayermp.posZ, entityplayermp.rotationYaw, entityplayermp.rotationPitch);
        entityplayermp.playerNetServerHandler.sendPacket(new SPacketSpawnPosition(blockpos2));
        entityplayermp.playerNetServerHandler.sendPacket(new SPacketSetExperience(entityplayermp.experience, entityplayermp.experienceTotal, entityplayermp.experienceLevel));
        this.updateTimeAndWeatherForPlayer(entityplayermp, worldserver);
        this.func_187243_f(entityplayermp);
        worldserver.getPlayerChunkManager().addPlayer(entityplayermp);
        worldserver.spawnEntityInWorld(entityplayermp);
        this.playerEntityList.add(entityplayermp);
        this.uuidToPlayerMap.put(entityplayermp.getUniqueID(), entityplayermp);
        entityplayermp.addSelfToInternalCraftingInventory();
        entityplayermp.setHealth(entityplayermp.getHealth());
        return entityplayermp;
    }

    public void func_187243_f(EntityPlayerMP p_187243_1_)
    {
        GameProfile gameprofile = p_187243_1_.getGameProfile();
        int i = this.canSendCommands(gameprofile) ? this.ops.func_187452_a(gameprofile) : 0;
        i = this.mcServer.isSinglePlayer() && this.mcServer.worldServers[0].getWorldInfo().areCommandsAllowed() ? 4 : i;
        i = this.commandsAllowedForAll ? 4 : i;
        this.func_187245_a(p_187243_1_, i);
    }

    public void func_187242_a(EntityPlayerMP p_187242_1_, int p_187242_2_)
    {
        int i = p_187242_1_.dimension;
        WorldServer worldserver = this.mcServer.worldServerForDimension(p_187242_1_.dimension);
        p_187242_1_.dimension = p_187242_2_;
        WorldServer worldserver1 = this.mcServer.worldServerForDimension(p_187242_1_.dimension);
        p_187242_1_.playerNetServerHandler.sendPacket(new SPacketRespawn(p_187242_1_.dimension, p_187242_1_.worldObj.getDifficulty(), p_187242_1_.worldObj.getWorldInfo().getTerrainType(), p_187242_1_.theItemInWorldManager.getGameType()));
        this.func_187243_f(p_187242_1_);
        worldserver.removePlayerEntityDangerously(p_187242_1_);
        p_187242_1_.isDead = false;
        this.transferEntityToWorld(p_187242_1_, i, worldserver, worldserver1);
        this.preparePlayer(p_187242_1_, worldserver);
        p_187242_1_.playerNetServerHandler.setPlayerLocation(p_187242_1_.posX, p_187242_1_.posY, p_187242_1_.posZ, p_187242_1_.rotationYaw, p_187242_1_.rotationPitch);
        p_187242_1_.theItemInWorldManager.setWorld(worldserver1);
        p_187242_1_.playerNetServerHandler.sendPacket(new SPacketPlayerAbilities(p_187242_1_.capabilities));
        this.updateTimeAndWeatherForPlayer(p_187242_1_, worldserver1);
        this.syncPlayerInventory(p_187242_1_);

        for (PotionEffect potioneffect : p_187242_1_.getActivePotionEffects())
        {
            p_187242_1_.playerNetServerHandler.sendPacket(new SPacketEntityEffect(p_187242_1_.getEntityId(), potioneffect));
        }
    }

    /**
     * Transfers an entity from a world to another world.
     */
    public void transferEntityToWorld(Entity entityIn, int p_82448_2_, WorldServer oldWorldIn, WorldServer toWorldIn)
    {
        double d0 = entityIn.posX;
        double d1 = entityIn.posZ;
        double d2 = 8.0D;
        float f = entityIn.rotationYaw;
        oldWorldIn.theProfiler.startSection("moving");

        if (entityIn.dimension == -1)
        {
            d0 = MathHelper.clamp_double(d0 / d2, toWorldIn.getWorldBorder().minX() + 16.0D, toWorldIn.getWorldBorder().maxX() - 16.0D);
            d1 = MathHelper.clamp_double(d1 / d2, toWorldIn.getWorldBorder().minZ() + 16.0D, toWorldIn.getWorldBorder().maxZ() - 16.0D);
            entityIn.setLocationAndAngles(d0, entityIn.posY, d1, entityIn.rotationYaw, entityIn.rotationPitch);

            if (entityIn.isEntityAlive())
            {
                oldWorldIn.updateEntityWithOptionalForce(entityIn, false);
            }
        }
        else if (entityIn.dimension == 0)
        {
            d0 = MathHelper.clamp_double(d0 * d2, toWorldIn.getWorldBorder().minX() + 16.0D, toWorldIn.getWorldBorder().maxX() - 16.0D);
            d1 = MathHelper.clamp_double(d1 * d2, toWorldIn.getWorldBorder().minZ() + 16.0D, toWorldIn.getWorldBorder().maxZ() - 16.0D);
            entityIn.setLocationAndAngles(d0, entityIn.posY, d1, entityIn.rotationYaw, entityIn.rotationPitch);

            if (entityIn.isEntityAlive())
            {
                oldWorldIn.updateEntityWithOptionalForce(entityIn, false);
            }
        }
        else
        {
            BlockPos blockpos;

            if (p_82448_2_ == 1)
            {
                blockpos = toWorldIn.getSpawnPoint();
            }
            else
            {
                blockpos = toWorldIn.getSpawnCoordinate();
            }

            d0 = (double)blockpos.getX();
            entityIn.posY = (double)blockpos.getY();
            d1 = (double)blockpos.getZ();
            entityIn.setLocationAndAngles(d0, entityIn.posY, d1, 90.0F, 0.0F);

            if (entityIn.isEntityAlive())
            {
                oldWorldIn.updateEntityWithOptionalForce(entityIn, false);
            }
        }

        oldWorldIn.theProfiler.endSection();

        if (p_82448_2_ != 1)
        {
            oldWorldIn.theProfiler.startSection("placing");
            d0 = (double)MathHelper.clamp_int((int)d0, -29999872, 29999872);
            d1 = (double)MathHelper.clamp_int((int)d1, -29999872, 29999872);

            if (entityIn.isEntityAlive())
            {
                entityIn.setLocationAndAngles(d0, entityIn.posY, d1, entityIn.rotationYaw, entityIn.rotationPitch);
                toWorldIn.getDefaultTeleporter().placeInPortal(entityIn, f);
                toWorldIn.spawnEntityInWorld(entityIn);
                toWorldIn.updateEntityWithOptionalForce(entityIn, false);
            }

            oldWorldIn.theProfiler.endSection();
        }

        entityIn.setWorld(toWorldIn);
    }

    /**
     * self explanitory
     */
    public void onTick()
    {
        if (++this.playerPingIndex > 600)
        {
            this.sendPacketToAllPlayers(new SPacketPlayerListItem(SPacketPlayerListItem.Action.UPDATE_LATENCY, this.playerEntityList));
            this.playerPingIndex = 0;
        }
    }

    public void sendPacketToAllPlayers(Packet<?> packetIn)
    {
        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            ((EntityPlayerMP)this.playerEntityList.get(i)).playerNetServerHandler.sendPacket(packetIn);
        }
    }

    public void sendPacketToAllPlayersInDimension(Packet<?> packetIn, int dimension)
    {
        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            EntityPlayerMP entityplayermp = (EntityPlayerMP)this.playerEntityList.get(i);

            if (entityplayermp.dimension == dimension)
            {
                entityplayermp.playerNetServerHandler.sendPacket(packetIn);
            }
        }
    }

    public void sendMessageToAllTeamMembers(EntityPlayer player, ITextComponent message)
    {
        Team team = player.getTeam();

        if (team != null)
        {
            for (String s : team.getMembershipCollection())
            {
                EntityPlayerMP entityplayermp = this.getPlayerByUsername(s);

                if (entityplayermp != null && entityplayermp != player)
                {
                    entityplayermp.addChatMessage(message);
                }
            }
        }
    }

    public void sendMessageToTeamOrEvryPlayer(EntityPlayer player, ITextComponent message)
    {
        Team team = player.getTeam();

        if (team == null)
        {
            this.sendChatMsg(message);
        }
        else
        {
            for (int i = 0; i < this.playerEntityList.size(); ++i)
            {
                EntityPlayerMP entityplayermp = (EntityPlayerMP)this.playerEntityList.get(i);

                if (entityplayermp.getTeam() != team)
                {
                    entityplayermp.addChatMessage(message);
                }
            }
        }
    }

    public String func_181058_b(boolean p_181058_1_)
    {
        String s = "";
        List<EntityPlayerMP> list = Lists.newArrayList(this.playerEntityList);

        for (int i = 0; i < ((List)list).size(); ++i)
        {
            if (i > 0)
            {
                s = s + ", ";
            }

            s = s + ((EntityPlayerMP)list.get(i)).getName();

            if (p_181058_1_)
            {
                s = s + " (" + ((EntityPlayerMP)list.get(i)).getUniqueID().toString() + ")";
            }
        }

        return s;
    }

    /**
     * Returns an array of the usernames of all the connected players.
     */
    public String[] getAllUsernames()
    {
        String[] astring = new String[this.playerEntityList.size()];

        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            astring[i] = ((EntityPlayerMP)this.playerEntityList.get(i)).getName();
        }

        return astring;
    }

    public GameProfile[] getAllProfiles()
    {
        GameProfile[] agameprofile = new GameProfile[this.playerEntityList.size()];

        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            agameprofile[i] = ((EntityPlayerMP)this.playerEntityList.get(i)).getGameProfile();
        }

        return agameprofile;
    }

    public UserListBans getBannedPlayers()
    {
        return this.bannedPlayers;
    }

    public UserListIPBans getBannedIPs()
    {
        return this.bannedIPs;
    }

    public void addOp(GameProfile profile)
    {
        int i = this.mcServer.getOpPermissionLevel();
        this.ops.addEntry(new UserListOpsEntry(profile, this.mcServer.getOpPermissionLevel(), this.ops.bypassesPlayerLimit(profile)));
        this.func_187245_a(this.getPlayerByUUID(profile.getId()), i);
    }

    public void removeOp(GameProfile profile)
    {
        this.ops.removeEntry(profile);
        this.func_187245_a(this.getPlayerByUUID(profile.getId()), 0);
    }

    private void func_187245_a(EntityPlayerMP p_187245_1_, int p_187245_2_)
    {
        if (p_187245_1_ != null && p_187245_1_.playerNetServerHandler != null)
        {
            byte b0;

            if (p_187245_2_ <= 0)
            {
                b0 = 24;
            }
            else if (p_187245_2_ >= 4)
            {
                b0 = 28;
            }
            else
            {
                b0 = (byte)(24 + p_187245_2_);
            }

            p_187245_1_.playerNetServerHandler.sendPacket(new SPacketEntityStatus(p_187245_1_, b0));
        }
    }

    public boolean canJoin(GameProfile profile)
    {
        return !this.whiteListEnforced || this.ops.hasEntry(profile) || this.whiteListedPlayers.hasEntry(profile);
    }

    public boolean canSendCommands(GameProfile profile)
    {
        return this.ops.hasEntry(profile) || this.mcServer.isSinglePlayer() && this.mcServer.worldServers[0].getWorldInfo().areCommandsAllowed() && this.mcServer.getServerOwner().equalsIgnoreCase(profile.getName()) || this.commandsAllowedForAll;
    }

    public EntityPlayerMP getPlayerByUsername(String username)
    {
        for (EntityPlayerMP entityplayermp : this.playerEntityList)
        {
            if (entityplayermp.getName().equalsIgnoreCase(username))
            {
                return entityplayermp;
            }
        }

        return null;
    }

    /**
     * params: srcPlayer,x,y,z,r,dimension. The packet is not sent to the srcPlayer, but all other players within the
     * search radius
     */
    public void sendToAllNearExcept(EntityPlayer p_148543_1_, double x, double y, double z, double radius, int dimension, Packet<?> p_148543_11_)
    {
        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            EntityPlayerMP entityplayermp = (EntityPlayerMP)this.playerEntityList.get(i);

            if (entityplayermp != p_148543_1_ && entityplayermp.dimension == dimension)
            {
                double d0 = x - entityplayermp.posX;
                double d1 = y - entityplayermp.posY;
                double d2 = z - entityplayermp.posZ;

                if (d0 * d0 + d1 * d1 + d2 * d2 < radius * radius)
                {
                    entityplayermp.playerNetServerHandler.sendPacket(p_148543_11_);
                }
            }
        }
    }

    /**
     * Saves all of the players' current states.
     */
    public void saveAllPlayerData()
    {
        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            this.writePlayerData((EntityPlayerMP)this.playerEntityList.get(i));
        }
    }

    public void addWhitelistedPlayer(GameProfile profile)
    {
        this.whiteListedPlayers.addEntry(new UserListWhitelistEntry(profile));
    }

    public void removePlayerFromWhitelist(GameProfile profile)
    {
        this.whiteListedPlayers.removeEntry(profile);
    }

    public UserListWhitelist getWhitelistedPlayers()
    {
        return this.whiteListedPlayers;
    }

    public String[] getWhitelistedPlayerNames()
    {
        return this.whiteListedPlayers.getKeys();
    }

    public UserListOps getOppedPlayers()
    {
        return this.ops;
    }

    public String[] getOppedPlayerNames()
    {
        return this.ops.getKeys();
    }

    public void reloadWhitelist()
    {
    }

    /**
     * Updates the time and weather for the given player to those of the given world
     */
    public void updateTimeAndWeatherForPlayer(EntityPlayerMP playerIn, WorldServer worldIn)
    {
        WorldBorder worldborder = this.mcServer.worldServers[0].getWorldBorder();
        playerIn.playerNetServerHandler.sendPacket(new SPacketWorldBorder(worldborder, SPacketWorldBorder.Action.INITIALIZE));
        playerIn.playerNetServerHandler.sendPacket(new SPacketTimeUpdate(worldIn.getTotalWorldTime(), worldIn.getWorldTime(), worldIn.getGameRules().getBoolean("doDaylightCycle")));

        if (worldIn.isRaining())
        {
            playerIn.playerNetServerHandler.sendPacket(new SPacketChangeGameState(1, 0.0F));
            playerIn.playerNetServerHandler.sendPacket(new SPacketChangeGameState(7, worldIn.getRainStrength(1.0F)));
            playerIn.playerNetServerHandler.sendPacket(new SPacketChangeGameState(8, worldIn.getThunderStrength(1.0F)));
        }
    }

    /**
     * sends the players inventory to himself
     */
    public void syncPlayerInventory(EntityPlayerMP playerIn)
    {
        playerIn.sendContainerToPlayer(playerIn.inventoryContainer);
        playerIn.setPlayerHealthUpdated();
        playerIn.playerNetServerHandler.sendPacket(new SPacketHeldItemChange(playerIn.inventory.currentItem));
    }

    /**
     * Returns the number of players currently on the server.
     */
    public int getCurrentPlayerCount()
    {
        return this.playerEntityList.size();
    }

    /**
     * Returns the maximum number of players allowed on the server.
     */
    public int getMaxPlayers()
    {
        return this.maxPlayers;
    }

    /**
     * Returns an array of usernames for which player.dat exists for.
     */
    public String[] getAvailablePlayerDat()
    {
        return this.mcServer.worldServers[0].getSaveHandler().getPlayerNBTManager().getAvailablePlayerDat();
    }

    public void setWhiteListEnabled(boolean whitelistEnabled)
    {
        this.whiteListEnforced = whitelistEnabled;
    }

    public List<EntityPlayerMP> getPlayersMatchingAddress(String address)
    {
        List<EntityPlayerMP> list = Lists.<EntityPlayerMP>newArrayList();

        for (EntityPlayerMP entityplayermp : this.playerEntityList)
        {
            if (entityplayermp.getPlayerIP().equals(address))
            {
                list.add(entityplayermp);
            }
        }

        return list;
    }

    /**
     * Gets the View Distance.
     */
    public int getViewDistance()
    {
        return this.viewDistance;
    }

    public MinecraftServer getServerInstance()
    {
        return this.mcServer;
    }

    /**
     * On integrated servers, returns the host's player data to be written to level.dat.
     */
    public NBTTagCompound getHostPlayerData()
    {
        return null;
    }

    public void setGameType(WorldSettings.GameType gameModeIn)
    {
        this.gameType = gameModeIn;
    }

    private void setPlayerGameTypeBasedOnOther(EntityPlayerMP p_72381_1_, EntityPlayerMP p_72381_2_, World worldIn)
    {
        if (p_72381_2_ != null)
        {
            p_72381_1_.theItemInWorldManager.setGameType(p_72381_2_.theItemInWorldManager.getGameType());
        }
        else if (this.gameType != null)
        {
            p_72381_1_.theItemInWorldManager.setGameType(this.gameType);
        }

        p_72381_1_.theItemInWorldManager.initializeGameType(worldIn.getWorldInfo().getGameType());
    }

    /**
     * Sets whether all players are allowed to use commands (cheats) on the server.
     */
    public void setCommandsAllowedForAll(boolean p_72387_1_)
    {
        this.commandsAllowedForAll = p_72387_1_;
    }

    /**
     * Kicks everyone with "Server closed" as reason.
     */
    public void removeAllPlayers()
    {
        for (int i = 0; i < this.playerEntityList.size(); ++i)
        {
            ((EntityPlayerMP)this.playerEntityList.get(i)).playerNetServerHandler.kickPlayerFromServer("Server closed");
        }
    }

    public void sendChatMsgImpl(ITextComponent component, boolean isChat)
    {
        this.mcServer.addChatMessage(component);
        byte b0 = (byte)(isChat ? 1 : 0);
        this.sendPacketToAllPlayers(new SPacketChat(component, b0));
    }

    /**
     * Sends the given string to every player as chat message.
     */
    public void sendChatMsg(ITextComponent component)
    {
        this.sendChatMsgImpl(component, true);
    }

    public StatisticsFile getPlayerStatsFile(EntityPlayer playerIn)
    {
        UUID uuid = playerIn.getUniqueID();
        StatisticsFile statisticsfile = uuid == null ? null : (StatisticsFile)this.playerStatFiles.get(uuid);

        if (statisticsfile == null)
        {
            File file1 = new File(this.mcServer.worldServerForDimension(0).getSaveHandler().getWorldDirectory(), "stats");
            File file2 = new File(file1, uuid.toString() + ".json");

            if (!file2.exists())
            {
                File file3 = new File(file1, playerIn.getName() + ".json");

                if (file3.exists() && file3.isFile())
                {
                    file3.renameTo(file2);
                }
            }

            statisticsfile = new StatisticsFile(this.mcServer, file2);
            statisticsfile.readStatFile();
            this.playerStatFiles.put(uuid, statisticsfile);
        }

        return statisticsfile;
    }

    public void setViewDistance(int distance)
    {
        this.viewDistance = distance;

        if (this.mcServer.worldServers != null)
        {
            for (WorldServer worldserver : this.mcServer.worldServers)
            {
                if (worldserver != null)
                {
                    worldserver.getPlayerChunkManager().setPlayerViewRadius(distance);
                    worldserver.getEntityTracker().func_187252_a(distance);
                }
            }
        }
    }

    public List<EntityPlayerMP> getPlayerList()
    {
        return this.playerEntityList;
    }

    /**
     * Get's the EntityPlayerMP object representing the player with the UUID.
     */
    public EntityPlayerMP getPlayerByUUID(UUID playerUUID)
    {
        return (EntityPlayerMP)this.uuidToPlayerMap.get(playerUUID);
    }

    public boolean bypassesPlayerLimit(GameProfile profile)
    {
        return false;
    }
}
