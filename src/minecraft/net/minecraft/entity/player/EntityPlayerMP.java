package net.minecraft.entity.player;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import com.mojang.authlib.GameProfile;
import io.netty.buffer.Unpooled;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.BlockFence;
import net.minecraft.block.BlockFenceGate;
import net.minecraft.block.BlockWall;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityList;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IMerchant;
import net.minecraft.entity.passive.EntityHorse;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.ContainerChest;
import net.minecraft.inventory.ContainerHorseInventory;
import net.minecraft.inventory.ContainerMerchant;
import net.minecraft.inventory.ICrafting;
import net.minecraft.inventory.IInventory;
import net.minecraft.inventory.SlotCrafting;
import net.minecraft.item.Item;
import net.minecraft.item.ItemMapBase;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.NetHandlerPlayServer;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.client.CPacketClientSettings;
import net.minecraft.network.play.server.SPacketAnimation;
import net.minecraft.network.play.server.SPacketCamera;
import net.minecraft.network.play.server.SPacketChangeGameState;
import net.minecraft.network.play.server.SPacketChat;
import net.minecraft.network.play.server.SPacketCloseWindow;
import net.minecraft.network.play.server.SPacketCombatEvent;
import net.minecraft.network.play.server.SPacketCustomPayload;
import net.minecraft.network.play.server.SPacketDestroyEntities;
import net.minecraft.network.play.server.SPacketEffect;
import net.minecraft.network.play.server.SPacketEntityEffect;
import net.minecraft.network.play.server.SPacketEntityStatus;
import net.minecraft.network.play.server.SPacketOpenWindow;
import net.minecraft.network.play.server.SPacketPlayerAbilities;
import net.minecraft.network.play.server.SPacketRemoveEntityEffect;
import net.minecraft.network.play.server.SPacketResourcePackSend;
import net.minecraft.network.play.server.SPacketSetExperience;
import net.minecraft.network.play.server.SPacketSetSlot;
import net.minecraft.network.play.server.SPacketSignEditorOpen;
import net.minecraft.network.play.server.SPacketSoundEffect;
import net.minecraft.network.play.server.SPacketUpdateHealth;
import net.minecraft.network.play.server.SPacketUseBed;
import net.minecraft.network.play.server.SPacketWindowItems;
import net.minecraft.network.play.server.SPacketWindowProperty;
import net.minecraft.potion.PotionEffect;
import net.minecraft.scoreboard.IScoreCriteria;
import net.minecraft.scoreboard.Score;
import net.minecraft.scoreboard.ScoreObjective;
import net.minecraft.scoreboard.Team;
import net.minecraft.server.MinecraftServer;
import net.minecraft.server.management.PlayerInteractionManager;
import net.minecraft.server.management.UserListOpsEntry;
import net.minecraft.stats.Achievement;
import net.minecraft.stats.AchievementList;
import net.minecraft.stats.StatBase;
import net.minecraft.stats.StatList;
import net.minecraft.stats.StatisticsFile;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityCommandBlock;
import net.minecraft.tileentity.TileEntitySign;
import net.minecraft.util.CooldownTracker;
import net.minecraft.util.CooldownTrackerServer;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntityDamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumHandSide;
import net.minecraft.util.JsonSerializableSet;
import net.minecraft.util.ReportedException;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.Style;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.village.MerchantRecipeList;
import net.minecraft.world.IInteractionObject;
import net.minecraft.world.ILockableContainer;
import net.minecraft.world.WorldServer;
import net.minecraft.world.WorldSettings;
import net.minecraft.world.biome.BiomeGenBase;
import net.minecraft.world.storage.loot.ILootContainer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class EntityPlayerMP extends EntityPlayer implements ICrafting
{
    private static final Logger logger = LogManager.getLogger();
    private String translator = "en_US";

    /**
     * The NetServerHandler assigned to this player by the ServerConfigurationManager.
     */
    public NetHandlerPlayServer playerNetServerHandler;

    /** Reference to the MinecraftServer object. */
    public final MinecraftServer mcServer;

    /** The ItemInWorldManager belonging to this player */
    public final PlayerInteractionManager theItemInWorldManager;

    /** player X position as seen by PlayerManager */
    public double managedPosX;

    /** player Z position as seen by PlayerManager */
    public double managedPosZ;
    private final List<Integer> destroyedItemsNetCache = Lists.<Integer>newLinkedList();
    private final StatisticsFile statsFile;

    /**
     * the total health of the player, includes actual health and absorption health. Updated every tick.
     */
    private float combinedHealth = Float.MIN_VALUE;
    private int field_184852_bV = Integer.MIN_VALUE;
    private int field_184853_bW = Integer.MIN_VALUE;
    private int field_184854_bX = Integer.MIN_VALUE;
    private int field_184855_bY = Integer.MIN_VALUE;
    private int field_184856_bZ = Integer.MIN_VALUE;

    /** amount of health the client was last set to */
    private float lastHealth = -1.0E8F;

    /** set to foodStats.GetFoodLevel */
    private int lastFoodLevel = -99999999;

    /** set to foodStats.getSaturationLevel() == 0.0F each tick */
    private boolean wasHungry = true;

    /** Amount of experience the client was last set to */
    private int lastExperience = -99999999;
    private int respawnInvulnerabilityTicks = 60;
    private EntityPlayer.EnumChatVisibility chatVisibility;
    private boolean chatColours = true;
    private long playerLastActiveTime = System.currentTimeMillis();

    /** The entity the player is currently spectating through. */
    private Entity spectatingEntity = null;
    private boolean field_184851_cj;

    /**
     * The currently in use window ID. Incremented every time a window is opened.
     */
    private int currentWindowId;

    /**
     * set to true when player is moving quantity of items from one inventory to another(crafting) but item in either
     * slot is not changed
     */
    public boolean isChangingQuantityOnly;
    public int ping;

    /**
     * Set when a player beats the ender dragon, used to respawn the player at the spawn point while retaining inventory
     * and XP
     */
    public boolean playerConqueredTheEnd;

    public EntityPlayerMP(MinecraftServer server, WorldServer worldIn, GameProfile profile, PlayerInteractionManager interactionManager)
    {
        super(worldIn, profile);
        interactionManager.thisPlayerMP = this;
        this.theItemInWorldManager = interactionManager;
        BlockPos blockpos = worldIn.getSpawnPoint();

        if (!worldIn.provider.getHasNoSky() && worldIn.getWorldInfo().getGameType() != WorldSettings.GameType.ADVENTURE)
        {
            int i = Math.max(0, server.getSpawnRadius(worldIn));
            int j = MathHelper.floor_double(worldIn.getWorldBorder().getClosestDistance((double)blockpos.getX(), (double)blockpos.getZ()));

            if (j < i)
            {
                i = j;
            }

            if (j <= 1)
            {
                i = 1;
            }

            blockpos = worldIn.getTopSolidOrLiquidBlock(blockpos.add(this.rand.nextInt(i * 2 + 1) - i, 0, this.rand.nextInt(i * 2 + 1) - i));
        }

        this.mcServer = server;
        this.statsFile = server.getPlayerList().getPlayerStatsFile(this);
        this.stepHeight = 0.0F;
        this.moveToBlockPosAndAngles(blockpos, 0.0F, 0.0F);

        while (!worldIn.func_184144_a(this, this.getEntityBoundingBox()).isEmpty() && this.posY < 255.0D)
        {
            this.setPosition(this.posX, this.posY + 1.0D, this.posZ);
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.hasKey("playerGameType", 99))
        {
            if (this.getServer().getForceGamemode())
            {
                this.theItemInWorldManager.setGameType(this.getServer().getGameType());
            }
            else
            {
                this.theItemInWorldManager.setGameType(WorldSettings.GameType.getByID(tagCompund.getInteger("playerGameType")));
            }
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setInteger("playerGameType", this.theItemInWorldManager.getGameType().getID());
        Entity entity = this.getLowestRidingEntity();

        if (this.getRidingEntity() != null && entity != this & entity.func_184180_b(EntityPlayerMP.class).size() == 1)
        {
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            NBTTagCompound nbttagcompound1 = new NBTTagCompound();
            entity.writeToNBTOptional(nbttagcompound1);
            nbttagcompound.setUniqueId("Attach", this.getRidingEntity().getUniqueID());
            nbttagcompound.setTag("Entity", nbttagcompound1);
            tagCompound.setTag("RootVehicle", nbttagcompound);
        }
    }

    /**
     * Add experience levels to this player.
     */
    public void addExperienceLevel(int levels)
    {
        super.addExperienceLevel(levels);
        this.lastExperience = -1;
    }

    public void removeExperienceLevel(int levels)
    {
        super.removeExperienceLevel(levels);
        this.lastExperience = -1;
    }

    public void addSelfToInternalCraftingInventory()
    {
        this.openContainer.onCraftGuiOpened(this);
    }

    /**
     * Sends an ENTER_COMBAT packet to the client
     */
    public void sendEnterCombat()
    {
        super.sendEnterCombat();
        this.playerNetServerHandler.sendPacket(new SPacketCombatEvent(this.getCombatTracker(), SPacketCombatEvent.Event.ENTER_COMBAT));
    }

    /**
     * Sends an END_COMBAT packet to the client
     */
    public void sendEndCombat()
    {
        super.sendEndCombat();
        this.playerNetServerHandler.sendPacket(new SPacketCombatEvent(this.getCombatTracker(), SPacketCombatEvent.Event.END_COMBAT));
    }

    protected CooldownTracker createCooldownTracker()
    {
        return new CooldownTrackerServer(this);
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        this.theItemInWorldManager.updateBlockRemoving();
        --this.respawnInvulnerabilityTicks;

        if (this.hurtResistantTime > 0)
        {
            --this.hurtResistantTime;
        }

        this.openContainer.detectAndSendChanges();

        if (!this.worldObj.isRemote && !this.openContainer.canInteractWith(this))
        {
            this.closeScreen();
            this.openContainer = this.inventoryContainer;
        }

        while (!this.destroyedItemsNetCache.isEmpty())
        {
            int i = Math.min(this.destroyedItemsNetCache.size(), Integer.MAX_VALUE);
            int[] aint = new int[i];
            Iterator<Integer> iterator = this.destroyedItemsNetCache.iterator();
            int j = 0;

            while (iterator.hasNext() && j < i)
            {
                aint[j++] = ((Integer)iterator.next()).intValue();
                iterator.remove();
            }

            this.playerNetServerHandler.sendPacket(new SPacketDestroyEntities(aint));
        }

        Entity entity = this.getSpectatingEntity();

        if (entity != this)
        {
            if (!entity.isEntityAlive())
            {
                this.setSpectatingEntity(this);
            }
            else
            {
                this.setPositionAndRotation(entity.posX, entity.posY, entity.posZ, entity.rotationYaw, entity.rotationPitch);
                this.mcServer.getPlayerList().serverUpdateMountedMovingPlayer(this);

                if (this.isSneaking())
                {
                    this.setSpectatingEntity(this);
                }
            }
        }
    }

    public void onUpdateEntity()
    {
        try
        {
            super.onUpdate();

            for (int i = 0; i < this.inventory.getSizeInventory(); ++i)
            {
                ItemStack itemstack = this.inventory.getStackInSlot(i);

                if (itemstack != null && itemstack.getItem().isMap())
                {
                    Packet<?> packet = ((ItemMapBase)itemstack.getItem()).createMapDataPacket(itemstack, this.worldObj, this);

                    if (packet != null)
                    {
                        this.playerNetServerHandler.sendPacket(packet);
                    }
                }
            }

            if (this.getHealth() != this.lastHealth || this.lastFoodLevel != this.foodStats.getFoodLevel() || this.foodStats.getSaturationLevel() == 0.0F != this.wasHungry)
            {
                this.playerNetServerHandler.sendPacket(new SPacketUpdateHealth(this.getHealth(), this.foodStats.getFoodLevel(), this.foodStats.getSaturationLevel()));
                this.lastHealth = this.getHealth();
                this.lastFoodLevel = this.foodStats.getFoodLevel();
                this.wasHungry = this.foodStats.getSaturationLevel() == 0.0F;
            }

            if (this.getHealth() + this.getAbsorptionAmount() != this.combinedHealth)
            {
                this.combinedHealth = this.getHealth() + this.getAbsorptionAmount();
                this.func_184849_a(IScoreCriteria.health, MathHelper.ceiling_float_int(this.combinedHealth));
            }

            if (this.foodStats.getFoodLevel() != this.field_184852_bV)
            {
                this.field_184852_bV = this.foodStats.getFoodLevel();
                this.func_184849_a(IScoreCriteria.FOOD, MathHelper.ceiling_float_int((float)this.field_184852_bV));
            }

            if (this.getAir() != this.field_184853_bW)
            {
                this.field_184853_bW = this.getAir();
                this.func_184849_a(IScoreCriteria.AIR, MathHelper.ceiling_float_int((float)this.field_184853_bW));
            }

            if (this.getTotalArmorValue() != this.field_184854_bX)
            {
                this.field_184854_bX = this.getTotalArmorValue();
                this.func_184849_a(IScoreCriteria.ARMOR, MathHelper.ceiling_float_int((float)this.field_184854_bX));
            }

            if (this.experienceTotal != this.field_184856_bZ)
            {
                this.field_184856_bZ = this.experienceTotal;
                this.func_184849_a(IScoreCriteria.XP, MathHelper.ceiling_float_int((float)this.field_184856_bZ));
            }

            if (this.experienceLevel != this.field_184855_bY)
            {
                this.field_184855_bY = this.experienceLevel;
                this.func_184849_a(IScoreCriteria.LEVEL, MathHelper.ceiling_float_int((float)this.field_184855_bY));
            }

            if (this.experienceTotal != this.lastExperience)
            {
                this.lastExperience = this.experienceTotal;
                this.playerNetServerHandler.sendPacket(new SPacketSetExperience(this.experience, this.experienceTotal, this.experienceLevel));
            }

            if (this.ticksExisted % 20 * 5 == 0 && !this.getStatFile().hasAchievementUnlocked(AchievementList.field_187979_L))
            {
                this.updateBiomesExplored();
            }
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Ticking player");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Player being ticked");
            this.addEntityCrashInfo(crashreportcategory);
            throw new ReportedException(crashreport);
        }
    }

    private void func_184849_a(IScoreCriteria p_184849_1_, int p_184849_2_)
    {
        for (ScoreObjective scoreobjective : this.getWorldScoreboard().getObjectivesFromCriteria(p_184849_1_))
        {
            Score score = this.getWorldScoreboard().getValueFromObjective(this.getName(), scoreobjective);
            score.setScorePoints(p_184849_2_);
        }
    }

    /**
     * Updates all biomes that have been explored by this player and triggers Adventuring Time if player qualifies.
     */
    protected void updateBiomesExplored()
    {
        BiomeGenBase biomegenbase = this.worldObj.getBiomeGenForCoords(new BlockPos(MathHelper.floor_double(this.posX), 0, MathHelper.floor_double(this.posZ)));
        String s = biomegenbase.getBiomeName();
        JsonSerializableSet jsonserializableset = (JsonSerializableSet)this.getStatFile().func_150870_b(AchievementList.field_187979_L);

        if (jsonserializableset == null)
        {
            jsonserializableset = (JsonSerializableSet)this.getStatFile().func_150872_a(AchievementList.field_187979_L, new JsonSerializableSet());
        }

        jsonserializableset.add(s);

        if (this.getStatFile().canUnlockAchievement(AchievementList.field_187979_L) && jsonserializableset.size() >= BiomeGenBase.explorationBiomesList.size())
        {
            Set<BiomeGenBase> set = Sets.newHashSet(BiomeGenBase.explorationBiomesList);

            for (String s1 : jsonserializableset)
            {
                Iterator<BiomeGenBase> iterator = set.iterator();

                while (iterator.hasNext())
                {
                    BiomeGenBase biomegenbase1 = (BiomeGenBase)iterator.next();

                    if (biomegenbase1.getBiomeName().equals(s1))
                    {
                        iterator.remove();
                    }
                }

                if (set.isEmpty())
                {
                    break;
                }
            }

            if (set.isEmpty())
            {
                this.triggerAchievement(AchievementList.field_187979_L);
            }
        }
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        boolean flag = this.worldObj.getGameRules().getBoolean("showDeathMessages");
        this.playerNetServerHandler.sendPacket(new SPacketCombatEvent(this.getCombatTracker(), SPacketCombatEvent.Event.ENTITY_DIED, flag));

        if (flag)
        {
            Team team = this.getTeam();

            if (team != null && team.getDeathMessageVisibility() != Team.EnumVisible.ALWAYS)
            {
                if (team.getDeathMessageVisibility() == Team.EnumVisible.HIDE_FOR_OTHER_TEAMS)
                {
                    this.mcServer.getPlayerList().sendMessageToAllTeamMembers(this, this.getCombatTracker().getDeathMessage());
                }
                else if (team.getDeathMessageVisibility() == Team.EnumVisible.HIDE_FOR_OWN_TEAM)
                {
                    this.mcServer.getPlayerList().sendMessageToTeamOrEvryPlayer(this, this.getCombatTracker().getDeathMessage());
                }
            }
            else
            {
                this.mcServer.getPlayerList().sendChatMsg(this.getCombatTracker().getDeathMessage());
            }
        }

        if (!this.worldObj.getGameRules().getBoolean("keepInventory") && !this.isSpectator())
        {
            this.inventory.dropAllItems();
        }

        for (ScoreObjective scoreobjective : this.worldObj.getScoreboard().getObjectivesFromCriteria(IScoreCriteria.deathCount))
        {
            Score score = this.getWorldScoreboard().getValueFromObjective(this.getName(), scoreobjective);
            score.func_96648_a();
        }

        EntityLivingBase entitylivingbase = this.getAttackingEntity();

        if (entitylivingbase != null)
        {
            EntityList.EntityEggInfo entitylist$entityegginfo = (EntityList.EntityEggInfo)EntityList.entityEggs.get(EntityList.getEntityString(entitylivingbase));

            if (entitylist$entityegginfo != null)
            {
                this.triggerAchievement(entitylist$entityegginfo.field_151513_e);
            }

            entitylivingbase.addToPlayerScore(this, this.scoreValue);
        }

        this.triggerAchievement(StatList.field_188069_A);
        this.func_175145_a(StatList.field_188098_h);
        this.getCombatTracker().reset();
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        if (this.isEntityInvulnerable(source))
        {
            return false;
        }
        else
        {
            boolean flag = this.mcServer.isDedicatedServer() && this.canPlayersAttack() && "fall".equals(source.damageType);

            if (!flag && this.respawnInvulnerabilityTicks > 0 && source != DamageSource.outOfWorld)
            {
                return false;
            }
            else
            {
                if (source instanceof EntityDamageSource)
                {
                    Entity entity = source.getEntity();

                    if (entity instanceof EntityPlayer && !this.canAttackPlayer((EntityPlayer)entity))
                    {
                        return false;
                    }

                    if (entity instanceof EntityArrow)
                    {
                        EntityArrow entityarrow = (EntityArrow)entity;

                        if (entityarrow.shootingEntity instanceof EntityPlayer && !this.canAttackPlayer((EntityPlayer)entityarrow.shootingEntity))
                        {
                            return false;
                        }
                    }
                }

                return super.attackEntityFrom(source, amount);
            }
        }
    }

    public boolean canAttackPlayer(EntityPlayer other)
    {
        return !this.canPlayersAttack() ? false : super.canAttackPlayer(other);
    }

    /**
     * Returns if other players can attack this player
     */
    private boolean canPlayersAttack()
    {
        return this.mcServer.isPVPEnabled();
    }

    public Entity changeDimension(int dimensionIn)
    {
        this.field_184851_cj = true;

        if (this.dimension == 1 && dimensionIn == 1)
        {
            this.worldObj.removeEntity(this);

            if (!this.playerConqueredTheEnd)
            {
                this.playerConqueredTheEnd = true;

                if (this.func_189102_a(AchievementList.field_187971_D))
                {
                    this.playerNetServerHandler.sendPacket(new SPacketChangeGameState(4, 0.0F));
                }
                else
                {
                    this.triggerAchievement(AchievementList.field_187971_D);
                    this.playerNetServerHandler.sendPacket(new SPacketChangeGameState(4, 1.0F));
                }
            }

            return this;
        }
        else
        {
            if (this.dimension == 0 && dimensionIn == 1)
            {
                this.triggerAchievement(AchievementList.theEnd);
                dimensionIn = 1;
            }
            else
            {
                this.triggerAchievement(AchievementList.field_187997_y);
            }

            this.mcServer.getPlayerList().func_187242_a(this, dimensionIn);
            this.playerNetServerHandler.sendPacket(new SPacketEffect(1032, BlockPos.ORIGIN, 0, false));
            this.lastExperience = -1;
            this.lastHealth = -1.0F;
            this.lastFoodLevel = -1;
            return this;
        }
    }

    public boolean isSpectatedByPlayer(EntityPlayerMP player)
    {
        return player.isSpectator() ? this.getSpectatingEntity() == this : (this.isSpectator() ? false : super.isSpectatedByPlayer(player));
    }

    private void sendTileEntityUpdate(TileEntity p_147097_1_)
    {
        if (p_147097_1_ != null)
        {
            Packet<?> packet = p_147097_1_.getDescriptionPacket();

            if (packet != null)
            {
                this.playerNetServerHandler.sendPacket(packet);
            }
        }
    }

    /**
     * Called whenever an item is picked up from walking over it. Args: pickedUpEntity, stackSize
     */
    public void onItemPickup(Entity entityIn, int quantity)
    {
        super.onItemPickup(entityIn, quantity);
        this.openContainer.detectAndSendChanges();
    }

    public EntityPlayer.EnumStatus trySleep(BlockPos bedLocation)
    {
        EntityPlayer.EnumStatus entityplayer$enumstatus = super.trySleep(bedLocation);

        if (entityplayer$enumstatus == EntityPlayer.EnumStatus.OK)
        {
            this.triggerAchievement(StatList.field_188064_ad);
            Packet<?> packet = new SPacketUseBed(this, bedLocation);
            this.getServerForPlayer().getEntityTracker().sendToAllTrackingEntity(this, packet);
            this.playerNetServerHandler.setPlayerLocation(this.posX, this.posY, this.posZ, this.rotationYaw, this.rotationPitch);
            this.playerNetServerHandler.sendPacket(packet);
        }

        return entityplayer$enumstatus;
    }

    /**
     * Wake up the player if they're sleeping.
     */
    public void wakeUpPlayer(boolean immediately, boolean updateWorldFlag, boolean setSpawn)
    {
        if (this.isPlayerSleeping())
        {
            this.getServerForPlayer().getEntityTracker().func_151248_b(this, new SPacketAnimation(this, 2));
        }

        super.wakeUpPlayer(immediately, updateWorldFlag, setSpawn);

        if (this.playerNetServerHandler != null)
        {
            this.playerNetServerHandler.setPlayerLocation(this.posX, this.posY, this.posZ, this.rotationYaw, this.rotationPitch);
        }
    }

    public boolean startRiding(Entity entityIn, boolean force)
    {
        Entity entity = this.getRidingEntity();

        if (!super.startRiding(entityIn, force))
        {
            return false;
        }
        else
        {
            Entity entity1 = this.getRidingEntity();

            if (entity1 != entity && this.playerNetServerHandler != null)
            {
                this.playerNetServerHandler.setPlayerLocation(this.posX, this.posY, this.posZ, this.rotationYaw, this.rotationPitch);
            }

            return true;
        }
    }

    public void dismountRidingEntity()
    {
        Entity entity = this.getRidingEntity();
        super.dismountRidingEntity();
        Entity entity1 = this.getRidingEntity();

        if (entity1 != entity && this.playerNetServerHandler != null)
        {
            this.playerNetServerHandler.setPlayerLocation(this.posX, this.posY, this.posZ, this.rotationYaw, this.rotationPitch);
        }
    }

    public boolean isEntityInvulnerable(DamageSource source)
    {
        return super.isEntityInvulnerable(source) || this.func_184850_K();
    }

    protected void updateFallState(double y, boolean onGroundIn, IBlockState state, BlockPos pos)
    {
    }

    protected void func_184594_b(BlockPos p_184594_1_)
    {
        if (!this.isSpectator())
        {
            super.func_184594_b(p_184594_1_);
        }
    }

    /**
     * process player falling based on movement packet
     */
    public void handleFalling(double p_71122_1_, boolean p_71122_3_)
    {
        int i = MathHelper.floor_double(this.posX);
        int j = MathHelper.floor_double(this.posY - 0.20000000298023224D);
        int k = MathHelper.floor_double(this.posZ);
        BlockPos blockpos = new BlockPos(i, j, k);
        IBlockState iblockstate = this.worldObj.getBlockState(blockpos);

        if (iblockstate.getMaterial() == Material.air)
        {
            BlockPos blockpos1 = blockpos.down();
            IBlockState iblockstate1 = this.worldObj.getBlockState(blockpos1);
            Block block = iblockstate1.getBlock();

            if (block instanceof BlockFence || block instanceof BlockWall || block instanceof BlockFenceGate)
            {
                blockpos = blockpos1;
                iblockstate = iblockstate1;
            }
        }

        super.updateFallState(p_71122_1_, p_71122_3_, iblockstate, blockpos);
    }

    public void openEditSign(TileEntitySign signTile)
    {
        signTile.setPlayer(this);
        this.playerNetServerHandler.sendPacket(new SPacketSignEditorOpen(signTile.getPos()));
    }

    /**
     * get the next window id to use
     */
    private void getNextWindowId()
    {
        this.currentWindowId = this.currentWindowId % 100 + 1;
    }

    public void displayGui(IInteractionObject guiOwner)
    {
        if (guiOwner instanceof ILootContainer && ((ILootContainer)guiOwner).getLootTable() != null && this.isSpectator())
        {
            this.addChatMessage((new TextComponentTranslation("container.spectatorCantOpen", new Object[0])).setChatStyle((new Style()).setColor(TextFormatting.RED)));
        }
        else
        {
            this.getNextWindowId();
            this.playerNetServerHandler.sendPacket(new SPacketOpenWindow(this.currentWindowId, guiOwner.getGuiID(), guiOwner.getDisplayName()));
            this.openContainer = guiOwner.createContainer(this.inventory, this);
            this.openContainer.windowId = this.currentWindowId;
            this.openContainer.onCraftGuiOpened(this);
        }
    }

    /**
     * Displays the GUI for interacting with a chest inventory. Args: chestInventory
     */
    public void displayGUIChest(IInventory chestInventory)
    {
        if (chestInventory instanceof ILootContainer && ((ILootContainer)chestInventory).getLootTable() != null && this.isSpectator())
        {
            this.addChatMessage((new TextComponentTranslation("container.spectatorCantOpen", new Object[0])).setChatStyle((new Style()).setColor(TextFormatting.RED)));
        }
        else
        {
            if (this.openContainer != this.inventoryContainer)
            {
                this.closeScreen();
            }

            if (chestInventory instanceof ILockableContainer)
            {
                ILockableContainer ilockablecontainer = (ILockableContainer)chestInventory;

                if (ilockablecontainer.isLocked() && !this.canOpen(ilockablecontainer.getLockCode()) && !this.isSpectator())
                {
                    this.playerNetServerHandler.sendPacket(new SPacketChat(new TextComponentTranslation("container.isLocked", new Object[] {chestInventory.getDisplayName()}), (byte)2));
                    this.playerNetServerHandler.sendPacket(new SPacketSoundEffect(SoundEvents.block_chest_locked, SoundCategory.BLOCKS, this.posX, this.posY, this.posZ, 1.0F, 1.0F));
                    return;
                }
            }

            this.getNextWindowId();

            if (chestInventory instanceof IInteractionObject)
            {
                this.playerNetServerHandler.sendPacket(new SPacketOpenWindow(this.currentWindowId, ((IInteractionObject)chestInventory).getGuiID(), chestInventory.getDisplayName(), chestInventory.getSizeInventory()));
                this.openContainer = ((IInteractionObject)chestInventory).createContainer(this.inventory, this);
            }
            else
            {
                this.playerNetServerHandler.sendPacket(new SPacketOpenWindow(this.currentWindowId, "minecraft:container", chestInventory.getDisplayName(), chestInventory.getSizeInventory()));
                this.openContainer = new ContainerChest(this.inventory, chestInventory, this);
            }

            this.openContainer.windowId = this.currentWindowId;
            this.openContainer.onCraftGuiOpened(this);
        }
    }

    public void displayVillagerTradeGui(IMerchant villager)
    {
        this.getNextWindowId();
        this.openContainer = new ContainerMerchant(this.inventory, villager, this.worldObj);
        this.openContainer.windowId = this.currentWindowId;
        this.openContainer.onCraftGuiOpened(this);
        IInventory iinventory = ((ContainerMerchant)this.openContainer).getMerchantInventory();
        ITextComponent itextcomponent = villager.getDisplayName();
        this.playerNetServerHandler.sendPacket(new SPacketOpenWindow(this.currentWindowId, "minecraft:villager", itextcomponent, iinventory.getSizeInventory()));
        MerchantRecipeList merchantrecipelist = villager.getRecipes(this);

        if (merchantrecipelist != null)
        {
            PacketBuffer packetbuffer = new PacketBuffer(Unpooled.buffer());
            packetbuffer.writeInt(this.currentWindowId);
            merchantrecipelist.writeToBuf(packetbuffer);
            this.playerNetServerHandler.sendPacket(new SPacketCustomPayload("MC|TrList", packetbuffer));
        }
    }

    public void func_184826_a(EntityHorse p_184826_1_, IInventory p_184826_2_)
    {
        if (this.openContainer != this.inventoryContainer)
        {
            this.closeScreen();
        }

        this.getNextWindowId();
        this.playerNetServerHandler.sendPacket(new SPacketOpenWindow(this.currentWindowId, "EntityHorse", p_184826_2_.getDisplayName(), p_184826_2_.getSizeInventory(), p_184826_1_.getEntityId()));
        this.openContainer = new ContainerHorseInventory(this.inventory, p_184826_2_, p_184826_1_, this);
        this.openContainer.windowId = this.currentWindowId;
        this.openContainer.onCraftGuiOpened(this);
    }

    public void func_184814_a(ItemStack p_184814_1_, EnumHand p_184814_2_)
    {
        Item item = p_184814_1_.getItem();

        if (item == Items.written_book)
        {
            PacketBuffer packetbuffer = new PacketBuffer(Unpooled.buffer());
            packetbuffer.writeEnumValue(p_184814_2_);
            this.playerNetServerHandler.sendPacket(new SPacketCustomPayload("MC|BOpen", packetbuffer));
        }
    }

    public void func_184824_a(TileEntityCommandBlock p_184824_1_)
    {
        if (this.canCommandSenderUseCommand(2, ""))
        {
            p_184824_1_.func_184252_d(true);
            this.sendTileEntityUpdate(p_184824_1_);
        }
    }

    /**
     * Sends the contents of an inventory slot to the client-side Container. This doesn't have to match the actual
     * contents of that slot. Args: Container, slot number, slot contents
     */
    public void sendSlotContents(Container containerToSend, int slotInd, ItemStack stack)
    {
        if (!(containerToSend.getSlot(slotInd) instanceof SlotCrafting))
        {
            if (!this.isChangingQuantityOnly)
            {
                this.playerNetServerHandler.sendPacket(new SPacketSetSlot(containerToSend.windowId, slotInd, stack));
            }
        }
    }

    public void sendContainerToPlayer(Container p_71120_1_)
    {
        this.updateCraftingInventory(p_71120_1_, p_71120_1_.getInventory());
    }

    /**
     * update the crafting window inventory with the items in the list
     */
    public void updateCraftingInventory(Container containerToSend, List<ItemStack> itemsList)
    {
        this.playerNetServerHandler.sendPacket(new SPacketWindowItems(containerToSend.windowId, itemsList));
        this.playerNetServerHandler.sendPacket(new SPacketSetSlot(-1, -1, this.inventory.getItemStack()));
    }

    /**
     * Sends two ints to the client-side Container. Used for furnace burning time, smelting progress, brewing progress,
     * and enchanting level. Normally the first int identifies which variable to update, and the second contains the new
     * value. Both are truncated to shorts in non-local SMP.
     */
    public void sendProgressBarUpdate(Container containerIn, int varToUpdate, int newValue)
    {
        this.playerNetServerHandler.sendPacket(new SPacketWindowProperty(containerIn.windowId, varToUpdate, newValue));
    }

    public void sendAllWindowProperties(Container p_175173_1_, IInventory p_175173_2_)
    {
        for (int i = 0; i < p_175173_2_.getFieldCount(); ++i)
        {
            this.playerNetServerHandler.sendPacket(new SPacketWindowProperty(p_175173_1_.windowId, i, p_175173_2_.getField(i)));
        }
    }

    /**
     * set current crafting inventory back to the 2x2 square
     */
    public void closeScreen()
    {
        this.playerNetServerHandler.sendPacket(new SPacketCloseWindow(this.openContainer.windowId));
        this.closeContainer();
    }

    /**
     * updates item held by mouse
     */
    public void updateHeldItem()
    {
        if (!this.isChangingQuantityOnly)
        {
            this.playerNetServerHandler.sendPacket(new SPacketSetSlot(-1, -1, this.inventory.getItemStack()));
        }
    }

    /**
     * Closes the container the player currently has open.
     */
    public void closeContainer()
    {
        this.openContainer.onContainerClosed(this);
        this.openContainer = this.inventoryContainer;
    }

    public void setEntityActionState(float p_110430_1_, float p_110430_2_, boolean p_110430_3_, boolean sneaking)
    {
        if (this.isRiding())
        {
            if (p_110430_1_ >= -1.0F && p_110430_1_ <= 1.0F)
            {
                this.moveStrafing = p_110430_1_;
            }

            if (p_110430_2_ >= -1.0F && p_110430_2_ <= 1.0F)
            {
                this.moveForward = p_110430_2_;
            }

            this.isJumping = p_110430_3_;
            this.setSneaking(sneaking);
        }
    }

    public boolean func_189102_a(Achievement p_189102_1_)
    {
        return this.statsFile.hasAchievementUnlocked(p_189102_1_);
    }

    /**
     * Adds a value to a statistic field.
     */
    public void addStat(StatBase stat, int amount)
    {
        if (stat != null)
        {
            this.statsFile.increaseStat(this, stat, amount);

            for (ScoreObjective scoreobjective : this.getWorldScoreboard().getObjectivesFromCriteria(stat.getCriteria()))
            {
                this.getWorldScoreboard().getValueFromObjective(this.getName(), scoreobjective).increseScore(amount);
            }

            if (this.statsFile.func_150879_e())
            {
                this.statsFile.func_150876_a(this);
            }
        }
    }

    public void func_175145_a(StatBase p_175145_1_)
    {
        if (p_175145_1_ != null)
        {
            this.statsFile.unlockAchievement(this, p_175145_1_, 0);

            for (ScoreObjective scoreobjective : this.getWorldScoreboard().getObjectivesFromCriteria(p_175145_1_.getCriteria()))
            {
                this.getWorldScoreboard().getValueFromObjective(this.getName(), scoreobjective).setScorePoints(0);
            }

            if (this.statsFile.func_150879_e())
            {
                this.statsFile.func_150876_a(this);
            }
        }
    }

    public void mountEntityAndWakeUp()
    {
        this.removePassengers();

        if (this.sleeping)
        {
            this.wakeUpPlayer(true, false, false);
        }
    }

    /**
     * this function is called when a players inventory is sent to him, lastHealth is updated on any dimension
     * transitions, then reset.
     */
    public void setPlayerHealthUpdated()
    {
        this.lastHealth = -1.0E8F;
    }

    public void addChatComponentMessage(ITextComponent chatComponent)
    {
        this.playerNetServerHandler.sendPacket(new SPacketChat(chatComponent));
    }

    /**
     * Used for when item use count runs out, ie: eating completed
     */
    protected void onItemUseFinish()
    {
        if (this.field_184627_bm != null && this.func_184587_cr())
        {
            this.playerNetServerHandler.sendPacket(new SPacketEntityStatus(this, (byte)9));
            super.onItemUseFinish();
        }
    }

    /**
     * Copies the values from the given player into this player if boolean par2 is true. Always clones Ender Chest
     * Inventory.
     */
    public void clonePlayer(EntityPlayer oldPlayer, boolean respawnFromEnd)
    {
        super.clonePlayer(oldPlayer, respawnFromEnd);
        this.lastExperience = -1;
        this.lastHealth = -1.0F;
        this.lastFoodLevel = -1;
        this.destroyedItemsNetCache.addAll(((EntityPlayerMP)oldPlayer).destroyedItemsNetCache);
    }

    protected void onNewPotionEffect(PotionEffect id)
    {
        super.onNewPotionEffect(id);
        this.playerNetServerHandler.sendPacket(new SPacketEntityEffect(this.getEntityId(), id));
    }

    protected void onChangedPotionEffect(PotionEffect id, boolean p_70695_2_)
    {
        super.onChangedPotionEffect(id, p_70695_2_);
        this.playerNetServerHandler.sendPacket(new SPacketEntityEffect(this.getEntityId(), id));
    }

    protected void onFinishedPotionEffect(PotionEffect effect)
    {
        super.onFinishedPotionEffect(effect);
        this.playerNetServerHandler.sendPacket(new SPacketRemoveEntityEffect(this.getEntityId(), effect.func_188419_a()));
    }

    /**
     * Sets the position of the entity and updates the 'last' variables
     */
    public void setPositionAndUpdate(double x, double y, double z)
    {
        this.playerNetServerHandler.setPlayerLocation(x, y, z, this.rotationYaw, this.rotationPitch);
    }

    /**
     * Called when the player performs a critical hit on the Entity. Args: entity that was hit critically
     */
    public void onCriticalHit(Entity entityHit)
    {
        this.getServerForPlayer().getEntityTracker().func_151248_b(this, new SPacketAnimation(entityHit, 4));
    }

    public void onEnchantmentCritical(Entity entityHit)
    {
        this.getServerForPlayer().getEntityTracker().func_151248_b(this, new SPacketAnimation(entityHit, 5));
    }

    /**
     * Sends the player's abilities to the server (if there is one).
     */
    public void sendPlayerAbilities()
    {
        if (this.playerNetServerHandler != null)
        {
            this.playerNetServerHandler.sendPacket(new SPacketPlayerAbilities(this.capabilities));
            this.updatePotionMetadata();
        }
    }

    public WorldServer getServerForPlayer()
    {
        return (WorldServer)this.worldObj;
    }

    /**
     * Sets the player's game mode and sends it to them.
     */
    public void setGameType(WorldSettings.GameType gameType)
    {
        this.theItemInWorldManager.setGameType(gameType);
        this.playerNetServerHandler.sendPacket(new SPacketChangeGameState(3, (float)gameType.getID()));

        if (gameType == WorldSettings.GameType.SPECTATOR)
        {
            this.dismountRidingEntity();
        }
        else
        {
            this.setSpectatingEntity(this);
        }

        this.sendPlayerAbilities();
        this.markPotionsDirty();
    }

    /**
     * Returns true if the player is in spectator mode.
     */
    public boolean isSpectator()
    {
        return this.theItemInWorldManager.getGameType() == WorldSettings.GameType.SPECTATOR;
    }

    public boolean isCreative()
    {
        return this.theItemInWorldManager.getGameType() == WorldSettings.GameType.CREATIVE;
    }

    /**
     * Send a chat message to the CommandSender
     */
    public void addChatMessage(ITextComponent component)
    {
        this.playerNetServerHandler.sendPacket(new SPacketChat(component));
    }

    /**
     * Returns {@code true} if the CommandSender is allowed to execute the command, {@code false} if not
     */
    public boolean canCommandSenderUseCommand(int permLevel, String commandName)
    {
        if ("seed".equals(commandName) && !this.mcServer.isDedicatedServer())
        {
            return true;
        }
        else if (!"tell".equals(commandName) && !"help".equals(commandName) && !"me".equals(commandName) && !"trigger".equals(commandName))
        {
            if (this.mcServer.getPlayerList().canSendCommands(this.getGameProfile()))
            {
                UserListOpsEntry userlistopsentry = (UserListOpsEntry)this.mcServer.getPlayerList().getOppedPlayers().getEntry(this.getGameProfile());
                return userlistopsentry != null ? userlistopsentry.getPermissionLevel() >= permLevel : this.mcServer.getOpPermissionLevel() >= permLevel;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    }

    /**
     * Gets the player's IP address. Used in /banip.
     */
    public String getPlayerIP()
    {
        String s = this.playerNetServerHandler.netManager.getRemoteAddress().toString();
        s = s.substring(s.indexOf("/") + 1);
        s = s.substring(0, s.indexOf(":"));
        return s;
    }

    public void handleClientSettings(CPacketClientSettings packetIn)
    {
        this.translator = packetIn.getLang();
        this.chatVisibility = packetIn.getChatVisibility();
        this.chatColours = packetIn.isColorsEnabled();
        this.getDataManager().set(PLAYER_MODEL_FLAG, Byte.valueOf((byte)packetIn.getModelPartFlags()));
        this.getDataManager().set(MAIN_HAND, Byte.valueOf((byte)(packetIn.func_186991_f() == EnumHandSide.LEFT ? 0 : 1)));
    }

    public EntityPlayer.EnumChatVisibility getChatVisibility()
    {
        return this.chatVisibility;
    }

    public void loadResourcePack(String url, String hash)
    {
        this.playerNetServerHandler.sendPacket(new SPacketResourcePackSend(url, hash));
    }

    /**
     * Get the position in the world. <b>{@code null} is not allowed!</b> If you are not an entity in the world, return
     * the coordinates 0, 0, 0
     */
    public BlockPos getPosition()
    {
        return new BlockPos(this.posX, this.posY + 0.5D, this.posZ);
    }

    public void markPlayerActive()
    {
        this.playerLastActiveTime = MinecraftServer.getCurrentTimeMillis();
    }

    /**
     * Gets the stats file for reading achievements
     */
    public StatisticsFile getStatFile()
    {
        return this.statsFile;
    }

    /**
     * Sends a packet to the player to remove an entity.
     */
    public void removeEntity(Entity p_152339_1_)
    {
        if (p_152339_1_ instanceof EntityPlayer)
        {
            this.playerNetServerHandler.sendPacket(new SPacketDestroyEntities(new int[] {p_152339_1_.getEntityId()}));
        }
        else
        {
            this.destroyedItemsNetCache.add(Integer.valueOf(p_152339_1_.getEntityId()));
        }
    }

    public void func_184848_d(Entity p_184848_1_)
    {
        this.destroyedItemsNetCache.remove(Integer.valueOf(p_184848_1_.getEntityId()));
    }

    /**
     * Clears potion metadata values if the entity has no potion effects. Otherwise, updates potion effect color,
     * ambience, and invisibility metadata values
     */
    protected void updatePotionMetadata()
    {
        if (this.isSpectator())
        {
            this.resetPotionEffectMetadata();
            this.setInvisible(true);
        }
        else
        {
            super.updatePotionMetadata();
        }

        this.getServerForPlayer().getEntityTracker().func_180245_a(this);
    }

    public Entity getSpectatingEntity()
    {
        return (Entity)(this.spectatingEntity == null ? this : this.spectatingEntity);
    }

    public void setSpectatingEntity(Entity entityToSpectate)
    {
        Entity entity = this.getSpectatingEntity();
        this.spectatingEntity = (Entity)(entityToSpectate == null ? this : entityToSpectate);

        if (entity != this.spectatingEntity)
        {
            this.playerNetServerHandler.sendPacket(new SPacketCamera(this.spectatingEntity));
            this.setPositionAndUpdate(this.spectatingEntity.posX, this.spectatingEntity.posY, this.spectatingEntity.posZ);
        }
    }

    protected void func_184173_H()
    {
        if (this.timeUntilPortal > 0 && !this.field_184851_cj)
        {
            --this.timeUntilPortal;
        }
    }

    /**
     * Attacks for the player the targeted entity with the currently equipped item.  The equipped item has hitEntity
     * called on it. Args: targetEntity
     */
    public void attackTargetEntityWithCurrentItem(Entity targetEntity)
    {
        if (this.theItemInWorldManager.getGameType() == WorldSettings.GameType.SPECTATOR)
        {
            this.setSpectatingEntity(targetEntity);
        }
        else
        {
            super.attackTargetEntityWithCurrentItem(targetEntity);
        }
    }

    public long getLastActiveTime()
    {
        return this.playerLastActiveTime;
    }

    /**
     * Returns null which indicates the tab list should just display the player's name, return a different value to
     * display the specified text instead of the player's name
     */
    public ITextComponent getTabListDisplayName()
    {
        return null;
    }

    public void swingArm(EnumHand hand)
    {
        super.swingArm(hand);
        this.func_184821_cY();
    }

    public boolean func_184850_K()
    {
        return this.field_184851_cj;
    }

    public void func_184846_L()
    {
        this.field_184851_cj = false;
    }

    public void func_184847_M()
    {
        this.setFlag(7, true);
    }

    public void func_189103_N()
    {
        this.setFlag(7, true);
        this.setFlag(7, false);
    }
}
