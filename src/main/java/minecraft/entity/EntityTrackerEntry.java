package net.minecraft.entity;

import com.google.common.collect.Sets;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.entity.ai.attributes.AttributeMap;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.item.EntityArmorStand;
import net.minecraft.entity.item.EntityBoat;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.entity.item.EntityEnderEye;
import net.minecraft.entity.item.EntityEnderPearl;
import net.minecraft.entity.item.EntityExpBottle;
import net.minecraft.entity.item.EntityFallingBlock;
import net.minecraft.entity.item.EntityFireworkRocket;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.item.EntityItemFrame;
import net.minecraft.entity.item.EntityMinecart;
import net.minecraft.entity.item.EntityPainting;
import net.minecraft.entity.item.EntityTNTPrimed;
import net.minecraft.entity.item.EntityXPOrb;
import net.minecraft.entity.passive.IAnimals;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.entity.projectile.EntityDragonFireball;
import net.minecraft.entity.projectile.EntityEgg;
import net.minecraft.entity.projectile.EntityFireball;
import net.minecraft.entity.projectile.EntityFishHook;
import net.minecraft.entity.projectile.EntityPotion;
import net.minecraft.entity.projectile.EntityShulkerBullet;
import net.minecraft.entity.projectile.EntitySmallFireball;
import net.minecraft.entity.projectile.EntitySnowball;
import net.minecraft.entity.projectile.EntitySpectralArrow;
import net.minecraft.entity.projectile.EntityTippedArrow;
import net.minecraft.entity.projectile.EntityWitherSkull;
import net.minecraft.init.Items;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemMap;
import net.minecraft.item.ItemStack;
import net.minecraft.network.Packet;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.network.play.server.SPacketEntity;
import net.minecraft.network.play.server.SPacketEntityEffect;
import net.minecraft.network.play.server.SPacketEntityEquipment;
import net.minecraft.network.play.server.SPacketEntityHeadLook;
import net.minecraft.network.play.server.SPacketEntityMetadata;
import net.minecraft.network.play.server.SPacketEntityProperties;
import net.minecraft.network.play.server.SPacketEntityTeleport;
import net.minecraft.network.play.server.SPacketEntityVelocity;
import net.minecraft.network.play.server.SPacketSetPassengers;
import net.minecraft.network.play.server.SPacketSpawnExperienceOrb;
import net.minecraft.network.play.server.SPacketSpawnMob;
import net.minecraft.network.play.server.SPacketSpawnObject;
import net.minecraft.network.play.server.SPacketSpawnPainting;
import net.minecraft.network.play.server.SPacketSpawnPlayer;
import net.minecraft.network.play.server.SPacketUseBed;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.storage.MapData;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class EntityTrackerEntry
{
    private static final Logger logger = LogManager.getLogger();

    /** The entity that this EntityTrackerEntry tracks. */
    private Entity trackedEntity;
    private int trackingDistanceThreshold;
    private int field_187262_f;

    /** check for sync when ticks % updateFrequency==0 */
    private int updateFrequency;

    /** The encoded entity X position. */
    private long encodedPosX;

    /** The encoded entity Y position. */
    private long encodedPosY;

    /** The encoded entity Z position. */
    private long encodedPosZ;

    /** The encoded entity yaw rotation. */
    private int encodedRotationYaw;

    /** The encoded entity pitch rotation. */
    private int encodedRotationPitch;
    private int lastHeadMotion;
    private double lastTrackedEntityMotionX;
    private double lastTrackedEntityMotionY;
    private double motionZ;
    public int updateCounter;
    private double lastTrackedEntityPosX;
    private double lastTrackedEntityPosY;
    private double lastTrackedEntityPosZ;
    private boolean firstUpdateDone;
    private boolean sendVelocityUpdates;

    /**
     * every 400 ticks a  full teleport packet is sent, rather than just a "move me +x" command, so that position
     * remains fully synced.
     */
    private int ticksSinceLastForcedTeleport;
    private List<Entity> field_187263_w = Collections.<Entity>emptyList();
    private boolean ridingEntity;
    private boolean onGround;
    public boolean playerEntitiesUpdated;
    private Set<EntityPlayerMP> trackingPlayers = Sets.<EntityPlayerMP>newHashSet();

    public EntityTrackerEntry(Entity entityIn, int p_i46837_2_, int p_i46837_3_, int p_i46837_4_, boolean p_i46837_5_)
    {
        this.trackedEntity = entityIn;
        this.trackingDistanceThreshold = p_i46837_2_;
        this.field_187262_f = p_i46837_3_;
        this.updateFrequency = p_i46837_4_;
        this.sendVelocityUpdates = p_i46837_5_;
        this.encodedPosX = EntityTracker.func_187253_a(entityIn.posX);
        this.encodedPosY = EntityTracker.func_187253_a(entityIn.posY);
        this.encodedPosZ = EntityTracker.func_187253_a(entityIn.posZ);
        this.encodedRotationYaw = MathHelper.floor_float(entityIn.rotationYaw * 256.0F / 360.0F);
        this.encodedRotationPitch = MathHelper.floor_float(entityIn.rotationPitch * 256.0F / 360.0F);
        this.lastHeadMotion = MathHelper.floor_float(entityIn.getRotationYawHead() * 256.0F / 360.0F);
        this.onGround = entityIn.onGround;
    }

    public boolean equals(Object p_equals_1_)
    {
        return p_equals_1_ instanceof EntityTrackerEntry ? ((EntityTrackerEntry)p_equals_1_).trackedEntity.getEntityId() == this.trackedEntity.getEntityId() : false;
    }

    public int hashCode()
    {
        return this.trackedEntity.getEntityId();
    }

    public void updatePlayerList(List<EntityPlayer> players)
    {
        this.playerEntitiesUpdated = false;

        if (!this.firstUpdateDone || this.trackedEntity.getDistanceSq(this.lastTrackedEntityPosX, this.lastTrackedEntityPosY, this.lastTrackedEntityPosZ) > 16.0D)
        {
            this.lastTrackedEntityPosX = this.trackedEntity.posX;
            this.lastTrackedEntityPosY = this.trackedEntity.posY;
            this.lastTrackedEntityPosZ = this.trackedEntity.posZ;
            this.firstUpdateDone = true;
            this.playerEntitiesUpdated = true;
            this.updatePlayerEntities(players);
        }

        List<Entity> list = this.trackedEntity.getPassengers();

        if (!list.equals(this.field_187263_w))
        {
            this.field_187263_w = list;
            this.sendPacketToTrackedPlayers(new SPacketSetPassengers(this.trackedEntity));
        }

        if (this.trackedEntity instanceof EntityItemFrame && this.updateCounter % 10 == 0)
        {
            EntityItemFrame entityitemframe = (EntityItemFrame)this.trackedEntity;
            ItemStack itemstack = entityitemframe.getDisplayedItem();

            if (itemstack != null && itemstack.getItem() instanceof ItemMap)
            {
                MapData mapdata = Items.filled_map.getMapData(itemstack, this.trackedEntity.worldObj);

                for (EntityPlayer entityplayer : players)
                {
                    EntityPlayerMP entityplayermp = (EntityPlayerMP)entityplayer;
                    mapdata.updateVisiblePlayers(entityplayermp, itemstack);
                    Packet<?> packet = Items.filled_map.createMapDataPacket(itemstack, this.trackedEntity.worldObj, entityplayermp);

                    if (packet != null)
                    {
                        entityplayermp.playerNetServerHandler.sendPacket(packet);
                    }
                }
            }

            this.sendMetadataToAllAssociatedPlayers();
        }

        if (this.updateCounter % this.updateFrequency == 0 || this.trackedEntity.isAirBorne || this.trackedEntity.getDataManager().isDirty())
        {
            if (!this.trackedEntity.isRiding())
            {
                ++this.ticksSinceLastForcedTeleport;
                long j1 = EntityTracker.func_187253_a(this.trackedEntity.posX);
                long i2 = EntityTracker.func_187253_a(this.trackedEntity.posY);
                long j2 = EntityTracker.func_187253_a(this.trackedEntity.posZ);
                int k2 = MathHelper.floor_float(this.trackedEntity.rotationYaw * 256.0F / 360.0F);
                int i = MathHelper.floor_float(this.trackedEntity.rotationPitch * 256.0F / 360.0F);
                long j = j1 - this.encodedPosX;
                long k = i2 - this.encodedPosY;
                long l = j2 - this.encodedPosZ;
                Packet<?> packet1 = null;
                boolean flag = j * j + k * k + l * l >= 128L || this.updateCounter % 60 == 0;
                boolean flag1 = Math.abs(k2 - this.encodedRotationYaw) >= 1 || Math.abs(i - this.encodedRotationPitch) >= 1;

                if (this.updateCounter > 0 || this.trackedEntity instanceof EntityArrow)
                {
                    if (j >= -32768L && j < 32768L && k >= -32768L && k < 32768L && l >= -32768L && l < 32768L && this.ticksSinceLastForcedTeleport <= 400 && !this.ridingEntity && this.onGround == this.trackedEntity.onGround)
                    {
                        if ((!flag || !flag1) && !(this.trackedEntity instanceof EntityArrow))
                        {
                            if (flag)
                            {
                                packet1 = new SPacketEntity.S15PacketEntityRelMove(this.trackedEntity.getEntityId(), j, k, l, this.trackedEntity.onGround);
                            }
                            else if (flag1)
                            {
                                packet1 = new SPacketEntity.S16PacketEntityLook(this.trackedEntity.getEntityId(), (byte)k2, (byte)i, this.trackedEntity.onGround);
                            }
                        }
                        else
                        {
                            packet1 = new SPacketEntity.S17PacketEntityLookMove(this.trackedEntity.getEntityId(), j, k, l, (byte)k2, (byte)i, this.trackedEntity.onGround);
                        }
                    }
                    else
                    {
                        this.onGround = this.trackedEntity.onGround;
                        this.ticksSinceLastForcedTeleport = 0;
                        this.func_187261_c();
                        packet1 = new SPacketEntityTeleport(this.trackedEntity);
                    }
                }

                boolean flag2 = this.sendVelocityUpdates;

                if (this.trackedEntity instanceof EntityLivingBase && ((EntityLivingBase)this.trackedEntity).func_184613_cA())
                {
                    flag2 = true;
                }

                if (flag2)
                {
                    double d0 = this.trackedEntity.motionX - this.lastTrackedEntityMotionX;
                    double d1 = this.trackedEntity.motionY - this.lastTrackedEntityMotionY;
                    double d2 = this.trackedEntity.motionZ - this.motionZ;
                    double d3 = 0.02D;
                    double d4 = d0 * d0 + d1 * d1 + d2 * d2;

                    if (d4 > 4.0E-4D || d4 > 0.0D && this.trackedEntity.motionX == 0.0D && this.trackedEntity.motionY == 0.0D && this.trackedEntity.motionZ == 0.0D)
                    {
                        this.lastTrackedEntityMotionX = this.trackedEntity.motionX;
                        this.lastTrackedEntityMotionY = this.trackedEntity.motionY;
                        this.motionZ = this.trackedEntity.motionZ;
                        this.sendPacketToTrackedPlayers(new SPacketEntityVelocity(this.trackedEntity.getEntityId(), this.lastTrackedEntityMotionX, this.lastTrackedEntityMotionY, this.motionZ));
                    }
                }

                if (packet1 != null)
                {
                    this.sendPacketToTrackedPlayers(packet1);
                }

                this.sendMetadataToAllAssociatedPlayers();

                if (flag)
                {
                    this.encodedPosX = j1;
                    this.encodedPosY = i2;
                    this.encodedPosZ = j2;
                }

                if (flag1)
                {
                    this.encodedRotationYaw = k2;
                    this.encodedRotationPitch = i;
                }

                this.ridingEntity = false;
            }
            else
            {
                int i1 = MathHelper.floor_float(this.trackedEntity.rotationYaw * 256.0F / 360.0F);
                int l1 = MathHelper.floor_float(this.trackedEntity.rotationPitch * 256.0F / 360.0F);
                boolean flag3 = Math.abs(i1 - this.encodedRotationYaw) >= 1 || Math.abs(l1 - this.encodedRotationPitch) >= 1;

                if (flag3)
                {
                    this.sendPacketToTrackedPlayers(new SPacketEntity.S16PacketEntityLook(this.trackedEntity.getEntityId(), (byte)i1, (byte)l1, this.trackedEntity.onGround));
                    this.encodedRotationYaw = i1;
                    this.encodedRotationPitch = l1;
                }

                this.encodedPosX = EntityTracker.func_187253_a(this.trackedEntity.posX);
                this.encodedPosY = EntityTracker.func_187253_a(this.trackedEntity.posY);
                this.encodedPosZ = EntityTracker.func_187253_a(this.trackedEntity.posZ);
                this.sendMetadataToAllAssociatedPlayers();
                this.ridingEntity = true;
            }

            int k1 = MathHelper.floor_float(this.trackedEntity.getRotationYawHead() * 256.0F / 360.0F);

            if (Math.abs(k1 - this.lastHeadMotion) >= 1)
            {
                this.sendPacketToTrackedPlayers(new SPacketEntityHeadLook(this.trackedEntity, (byte)k1));
                this.lastHeadMotion = k1;
            }

            this.trackedEntity.isAirBorne = false;
        }

        ++this.updateCounter;

        if (this.trackedEntity.velocityChanged)
        {
            this.func_151261_b(new SPacketEntityVelocity(this.trackedEntity));
            this.trackedEntity.velocityChanged = false;
        }
    }

    /**
     * Sends the entity metadata (DataWatcher) and attributes to all players tracking this entity, including the entity
     * itself if a player.
     */
    private void sendMetadataToAllAssociatedPlayers()
    {
        EntityDataManager entitydatamanager = this.trackedEntity.getDataManager();

        if (entitydatamanager.isDirty())
        {
            this.func_151261_b(new SPacketEntityMetadata(this.trackedEntity.getEntityId(), entitydatamanager, false));
        }

        if (this.trackedEntity instanceof EntityLivingBase)
        {
            AttributeMap attributemap = (AttributeMap)((EntityLivingBase)this.trackedEntity).getAttributeMap();
            Set<IAttributeInstance> set = attributemap.getAttributeInstanceSet();

            if (!set.isEmpty())
            {
                this.func_151261_b(new SPacketEntityProperties(this.trackedEntity.getEntityId(), set));
            }

            set.clear();
        }
    }

    /**
     * Send the given packet to all players tracking this entity.
     */
    public void sendPacketToTrackedPlayers(Packet<?> packetIn)
    {
        for (EntityPlayerMP entityplayermp : this.trackingPlayers)
        {
            entityplayermp.playerNetServerHandler.sendPacket(packetIn);
        }
    }

    public void func_151261_b(Packet<?> packetIn)
    {
        this.sendPacketToTrackedPlayers(packetIn);

        if (this.trackedEntity instanceof EntityPlayerMP)
        {
            ((EntityPlayerMP)this.trackedEntity).playerNetServerHandler.sendPacket(packetIn);
        }
    }

    public void sendDestroyEntityPacketToTrackedPlayers()
    {
        for (EntityPlayerMP entityplayermp : this.trackingPlayers)
        {
            this.trackedEntity.func_184203_c(entityplayermp);
            entityplayermp.removeEntity(this.trackedEntity);
        }
    }

    public void removeFromTrackedPlayers(EntityPlayerMP playerMP)
    {
        if (this.trackingPlayers.contains(playerMP))
        {
            this.trackedEntity.func_184203_c(playerMP);
            playerMP.removeEntity(this.trackedEntity);
            this.trackingPlayers.remove(playerMP);
        }
    }

    public void updatePlayerEntity(EntityPlayerMP playerMP)
    {
        if (playerMP != this.trackedEntity)
        {
            if (this.func_180233_c(playerMP))
            {
                if (!this.trackingPlayers.contains(playerMP) && (this.isPlayerWatchingThisChunk(playerMP) || this.trackedEntity.forceSpawn))
                {
                    this.trackingPlayers.add(playerMP);
                    Packet<?> packet = this.createSpawnPacket();
                    playerMP.playerNetServerHandler.sendPacket(packet);

                    if (!this.trackedEntity.getDataManager().isEmpty())
                    {
                        playerMP.playerNetServerHandler.sendPacket(new SPacketEntityMetadata(this.trackedEntity.getEntityId(), this.trackedEntity.getDataManager(), true));
                    }

                    boolean flag = this.sendVelocityUpdates;

                    if (this.trackedEntity instanceof EntityLivingBase)
                    {
                        AttributeMap attributemap = (AttributeMap)((EntityLivingBase)this.trackedEntity).getAttributeMap();
                        Collection<IAttributeInstance> collection = attributemap.getWatchedAttributes();

                        if (!collection.isEmpty())
                        {
                            playerMP.playerNetServerHandler.sendPacket(new SPacketEntityProperties(this.trackedEntity.getEntityId(), collection));
                        }

                        if (((EntityLivingBase)this.trackedEntity).func_184613_cA())
                        {
                            flag = true;
                        }
                    }

                    this.lastTrackedEntityMotionX = this.trackedEntity.motionX;
                    this.lastTrackedEntityMotionY = this.trackedEntity.motionY;
                    this.motionZ = this.trackedEntity.motionZ;

                    if (flag && !(packet instanceof SPacketSpawnMob))
                    {
                        playerMP.playerNetServerHandler.sendPacket(new SPacketEntityVelocity(this.trackedEntity.getEntityId(), this.trackedEntity.motionX, this.trackedEntity.motionY, this.trackedEntity.motionZ));
                    }

                    if (this.trackedEntity instanceof EntityLivingBase)
                    {
                        for (EntityEquipmentSlot entityequipmentslot : EntityEquipmentSlot.values())
                        {
                            ItemStack itemstack = ((EntityLivingBase)this.trackedEntity).getItemStackFromSlot(entityequipmentslot);

                            if (itemstack != null)
                            {
                                playerMP.playerNetServerHandler.sendPacket(new SPacketEntityEquipment(this.trackedEntity.getEntityId(), entityequipmentslot, itemstack));
                            }
                        }
                    }

                    if (this.trackedEntity instanceof EntityPlayer)
                    {
                        EntityPlayer entityplayer = (EntityPlayer)this.trackedEntity;

                        if (entityplayer.isPlayerSleeping())
                        {
                            playerMP.playerNetServerHandler.sendPacket(new SPacketUseBed(entityplayer, new BlockPos(this.trackedEntity)));
                        }
                    }

                    if (this.trackedEntity instanceof EntityLivingBase)
                    {
                        EntityLivingBase entitylivingbase = (EntityLivingBase)this.trackedEntity;

                        for (PotionEffect potioneffect : entitylivingbase.getActivePotionEffects())
                        {
                            playerMP.playerNetServerHandler.sendPacket(new SPacketEntityEffect(this.trackedEntity.getEntityId(), potioneffect));
                        }
                    }

                    this.trackedEntity.func_184178_b(playerMP);
                    playerMP.func_184848_d(this.trackedEntity);
                }
            }
            else if (this.trackingPlayers.contains(playerMP))
            {
                this.trackingPlayers.remove(playerMP);
                this.trackedEntity.func_184203_c(playerMP);
                playerMP.removeEntity(this.trackedEntity);
            }
        }
    }

    public boolean func_180233_c(EntityPlayerMP playerMP)
    {
        double d0 = playerMP.posX - (double)this.encodedPosX / 4096.0D;
        double d1 = playerMP.posZ - (double)this.encodedPosZ / 4096.0D;
        int i = Math.min(this.trackingDistanceThreshold, this.field_187262_f);
        return d0 >= (double)(-i) && d0 <= (double)i && d1 >= (double)(-i) && d1 <= (double)i && this.trackedEntity.isSpectatedByPlayer(playerMP);
    }

    private boolean isPlayerWatchingThisChunk(EntityPlayerMP playerMP)
    {
        return playerMP.getServerForPlayer().getPlayerChunkManager().isPlayerWatchingChunk(playerMP, this.trackedEntity.chunkCoordX, this.trackedEntity.chunkCoordZ);
    }

    public void updatePlayerEntities(List<EntityPlayer> players)
    {
        for (int i = 0; i < players.size(); ++i)
        {
            this.updatePlayerEntity((EntityPlayerMP)players.get(i));
        }
    }

    private Packet<?> createSpawnPacket()
    {
        if (this.trackedEntity.isDead)
        {
            logger.warn("Fetching addPacket for removed entity");
        }

        if (this.trackedEntity instanceof EntityItem)
        {
            return new SPacketSpawnObject(this.trackedEntity, 2, 1);
        }
        else if (this.trackedEntity instanceof EntityPlayerMP)
        {
            return new SPacketSpawnPlayer((EntityPlayer)this.trackedEntity);
        }
        else if (this.trackedEntity instanceof EntityMinecart)
        {
            EntityMinecart entityminecart = (EntityMinecart)this.trackedEntity;
            return new SPacketSpawnObject(this.trackedEntity, 10, entityminecart.func_184264_v().func_184956_a());
        }
        else if (this.trackedEntity instanceof EntityBoat)
        {
            return new SPacketSpawnObject(this.trackedEntity, 1);
        }
        else if (this.trackedEntity instanceof IAnimals)
        {
            this.lastHeadMotion = MathHelper.floor_float(this.trackedEntity.getRotationYawHead() * 256.0F / 360.0F);
            return new SPacketSpawnMob((EntityLivingBase)this.trackedEntity);
        }
        else if (this.trackedEntity instanceof EntityFishHook)
        {
            Entity entity2 = ((EntityFishHook)this.trackedEntity).angler;
            return new SPacketSpawnObject(this.trackedEntity, 90, entity2 != null ? entity2.getEntityId() : this.trackedEntity.getEntityId());
        }
        else if (this.trackedEntity instanceof EntitySpectralArrow)
        {
            Entity entity1 = ((EntitySpectralArrow)this.trackedEntity).shootingEntity;
            return new SPacketSpawnObject(this.trackedEntity, 91, 1 + (entity1 != null ? entity1.getEntityId() : this.trackedEntity.getEntityId()));
        }
        else if (this.trackedEntity instanceof EntityTippedArrow)
        {
            Entity entity = ((EntityArrow)this.trackedEntity).shootingEntity;
            return new SPacketSpawnObject(this.trackedEntity, 60, 1 + (entity != null ? entity.getEntityId() : this.trackedEntity.getEntityId()));
        }
        else if (this.trackedEntity instanceof EntitySnowball)
        {
            return new SPacketSpawnObject(this.trackedEntity, 61);
        }
        else if (this.trackedEntity instanceof EntityPotion)
        {
            return new SPacketSpawnObject(this.trackedEntity, 73);
        }
        else if (this.trackedEntity instanceof EntityExpBottle)
        {
            return new SPacketSpawnObject(this.trackedEntity, 75);
        }
        else if (this.trackedEntity instanceof EntityEnderPearl)
        {
            return new SPacketSpawnObject(this.trackedEntity, 65);
        }
        else if (this.trackedEntity instanceof EntityEnderEye)
        {
            return new SPacketSpawnObject(this.trackedEntity, 72);
        }
        else if (this.trackedEntity instanceof EntityFireworkRocket)
        {
            return new SPacketSpawnObject(this.trackedEntity, 76);
        }
        else if (this.trackedEntity instanceof EntityFireball)
        {
            EntityFireball entityfireball = (EntityFireball)this.trackedEntity;
            SPacketSpawnObject spacketspawnobject3 = null;
            int i = 63;

            if (this.trackedEntity instanceof EntitySmallFireball)
            {
                i = 64;
            }
            else if (this.trackedEntity instanceof EntityDragonFireball)
            {
                i = 93;
            }
            else if (this.trackedEntity instanceof EntityWitherSkull)
            {
                i = 66;
            }

            if (entityfireball.shootingEntity != null)
            {
                spacketspawnobject3 = new SPacketSpawnObject(this.trackedEntity, i, ((EntityFireball)this.trackedEntity).shootingEntity.getEntityId());
            }
            else
            {
                spacketspawnobject3 = new SPacketSpawnObject(this.trackedEntity, i, 0);
            }

            spacketspawnobject3.setSpeedX((int)(entityfireball.accelerationX * 8000.0D));
            spacketspawnobject3.setSpeedY((int)(entityfireball.accelerationY * 8000.0D));
            spacketspawnobject3.setSpeedZ((int)(entityfireball.accelerationZ * 8000.0D));
            return spacketspawnobject3;
        }
        else if (this.trackedEntity instanceof EntityShulkerBullet)
        {
            SPacketSpawnObject spacketspawnobject1 = new SPacketSpawnObject(this.trackedEntity, 67, 0);
            spacketspawnobject1.setSpeedX((int)(this.trackedEntity.motionX * 8000.0D));
            spacketspawnobject1.setSpeedY((int)(this.trackedEntity.motionY * 8000.0D));
            spacketspawnobject1.setSpeedZ((int)(this.trackedEntity.motionZ * 8000.0D));
            return spacketspawnobject1;
        }
        else if (this.trackedEntity instanceof EntityEgg)
        {
            return new SPacketSpawnObject(this.trackedEntity, 62);
        }
        else if (this.trackedEntity instanceof EntityTNTPrimed)
        {
            return new SPacketSpawnObject(this.trackedEntity, 50);
        }
        else if (this.trackedEntity instanceof EntityEnderCrystal)
        {
            return new SPacketSpawnObject(this.trackedEntity, 51);
        }
        else if (this.trackedEntity instanceof EntityFallingBlock)
        {
            EntityFallingBlock entityfallingblock = (EntityFallingBlock)this.trackedEntity;
            return new SPacketSpawnObject(this.trackedEntity, 70, Block.getStateId(entityfallingblock.getBlock()));
        }
        else if (this.trackedEntity instanceof EntityArmorStand)
        {
            return new SPacketSpawnObject(this.trackedEntity, 78);
        }
        else if (this.trackedEntity instanceof EntityPainting)
        {
            return new SPacketSpawnPainting((EntityPainting)this.trackedEntity);
        }
        else if (this.trackedEntity instanceof EntityItemFrame)
        {
            EntityItemFrame entityitemframe = (EntityItemFrame)this.trackedEntity;
            SPacketSpawnObject spacketspawnobject2 = new SPacketSpawnObject(this.trackedEntity, 71, entityitemframe.facingDirection.getHorizontalIndex(), entityitemframe.getHangingPosition());
            return spacketspawnobject2;
        }
        else if (this.trackedEntity instanceof EntityLeashKnot)
        {
            EntityLeashKnot entityleashknot = (EntityLeashKnot)this.trackedEntity;
            SPacketSpawnObject spacketspawnobject = new SPacketSpawnObject(this.trackedEntity, 77, 0, entityleashknot.getHangingPosition());
            return spacketspawnobject;
        }
        else if (this.trackedEntity instanceof EntityXPOrb)
        {
            return new SPacketSpawnExperienceOrb((EntityXPOrb)this.trackedEntity);
        }
        else if (this.trackedEntity instanceof EntityAreaEffectCloud)
        {
            return new SPacketSpawnObject(this.trackedEntity, 3);
        }
        else
        {
            throw new IllegalArgumentException("Don\'t know how to add " + this.trackedEntity.getClass() + "!");
        }
    }

    /**
     * Remove a tracked player from our list and tell the tracked player to destroy us from their world.
     */
    public void removeTrackedPlayerSymmetric(EntityPlayerMP playerMP)
    {
        if (this.trackingPlayers.contains(playerMP))
        {
            this.trackingPlayers.remove(playerMP);
            this.trackedEntity.func_184203_c(playerMP);
            playerMP.removeEntity(this.trackedEntity);
        }
    }

    public Entity func_187260_b()
    {
        return this.trackedEntity;
    }

    public void func_187259_a(int p_187259_1_)
    {
        this.field_187262_f = p_187259_1_;
    }

    public void func_187261_c()
    {
        this.firstUpdateDone = false;
    }
}
