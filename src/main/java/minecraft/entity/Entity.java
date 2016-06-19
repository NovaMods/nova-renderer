package net.minecraft.entity;

import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Callable;
import net.minecraft.block.Block;
import net.minecraft.block.BlockFence;
import net.minecraft.block.BlockFenceGate;
import net.minecraft.block.BlockLiquid;
import net.minecraft.block.BlockWall;
import net.minecraft.block.SoundType;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.block.state.pattern.BlockPattern;
import net.minecraft.command.CommandResultStats;
import net.minecraft.command.ICommandSender;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.enchantment.EnchantmentProtection;
import net.minecraft.entity.effect.EntityLightningBolt;
import net.minecraft.entity.item.EntityBoat;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagDouble;
import net.minecraft.nbt.NBTTagFloat;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.nbt.NBTTagString;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.scoreboard.ScorePlayerTeam;
import net.minecraft.scoreboard.Team;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.Mirror;
import net.minecraft.util.ReportedException;
import net.minecraft.util.Rotation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.event.HoverEvent;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.Explosion;
import net.minecraft.world.Teleporter;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public abstract class Entity implements ICommandSender
{
    private static final Logger LOGGER = LogManager.getLogger();
    private static final AxisAlignedBB ZERO_AABB = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.0D, 0.0D, 0.0D);
    private static double renderDistanceWeight = 1.0D;
    private static int nextEntityID;
    private int entityId;

    /**
     * Blocks entities from spawning when they do their AABB check to make sure the spot is clear of entities that can
     * prevent spawning.
     */
    public boolean preventEntitySpawning;
    private final List<Entity> riddenByEntities;
    protected int field_184245_j;
    private Entity ridingEntity;
    public boolean forceSpawn;

    /** Reference to the World object. */
    public World worldObj;
    public double prevPosX;
    public double prevPosY;
    public double prevPosZ;

    /** Entity position X */
    public double posX;

    /** Entity position Y */
    public double posY;

    /** Entity position Z */
    public double posZ;

    /** Entity motion X */
    public double motionX;

    /** Entity motion Y */
    public double motionY;

    /** Entity motion Z */
    public double motionZ;

    /** Entity rotation Yaw */
    public float rotationYaw;

    /** Entity rotation Pitch */
    public float rotationPitch;
    public float prevRotationYaw;
    public float prevRotationPitch;

    /** Axis aligned bounding box. */
    private AxisAlignedBB boundingBox;
    public boolean onGround;

    /**
     * True if after a move this entity has collided with something on X- or Z-axis
     */
    public boolean isCollidedHorizontally;

    /**
     * True if after a move this entity has collided with something on Y-axis
     */
    public boolean isCollidedVertically;

    /**
     * True if after a move this entity has collided with something either vertically or horizontally
     */
    public boolean isCollided;
    public boolean velocityChanged;
    protected boolean isInWeb;
    private boolean isOutsideBorder;

    /**
     * gets set by setEntityDead, so this must be the flag whether an Entity is dead (inactive may be better term)
     */
    public boolean isDead;

    /** How wide this entity is considered to be */
    public float width;

    /** How high this entity is considered to be */
    public float height;

    /** The previous ticks distance walked multiplied by 0.6 */
    public float prevDistanceWalkedModified;

    /** The distance walked multiplied by 0.6 */
    public float distanceWalkedModified;
    public float distanceWalkedOnStepModified;
    public float fallDistance;

    /**
     * The distance that has to be exceeded in order to triger a new step sound and an onEntityWalking event on a block
     */
    private int nextStepDistance;

    /**
     * The entity's X coordinate at the previous tick, used to calculate position during rendering routines
     */
    public double lastTickPosX;

    /**
     * The entity's Y coordinate at the previous tick, used to calculate position during rendering routines
     */
    public double lastTickPosY;

    /**
     * The entity's Z coordinate at the previous tick, used to calculate position during rendering routines
     */
    public double lastTickPosZ;

    /**
     * How high this entity can step up when running into a block to try to get over it (currently make note the entity
     * will always step up this amount and not just the amount needed)
     */
    public float stepHeight;

    /**
     * Whether this entity won't clip with collision or not (make note it won't disable gravity)
     */
    public boolean noClip;

    /**
     * Reduces the velocity applied by entity collisions by the specified percent.
     */
    public float entityCollisionReduction;
    protected Random rand;

    /** How many ticks has this entity had ran since being alive */
    public int ticksExisted;

    /**
     * The amount of ticks you have to stand inside of fire before be set on fire
     */
    public int fireResistance;
    private int fire;

    /**
     * Whether this entity is currently inside of water (if it handles water movement that is)
     */
    protected boolean inWater;

    /**
     * Remaining time an entity will be "immune" to further damage after being hurt.
     */
    public int hurtResistantTime;
    protected boolean firstUpdate;
    protected boolean isImmuneToFire;
    protected EntityDataManager dataWatcher;
    private static final DataParameter<Byte> FLAGS = EntityDataManager.<Byte>createKey(Entity.class, DataSerializers.BYTE);
    private static final DataParameter<Integer> AIR = EntityDataManager.<Integer>createKey(Entity.class, DataSerializers.VARINT);
    private static final DataParameter<String> CUSTOM_NAME = EntityDataManager.<String>createKey(Entity.class, DataSerializers.STRING);
    private static final DataParameter<Boolean> CUSTOM_NAME_VISIBLE = EntityDataManager.<Boolean>createKey(Entity.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Boolean> SILENT = EntityDataManager.<Boolean>createKey(Entity.class, DataSerializers.BOOLEAN);

    /** Has this entity been added to the chunk its within */
    public boolean addedToChunk;
    public int chunkCoordX;
    public int chunkCoordY;
    public int chunkCoordZ;
    public long serverPosX;
    public long serverPosY;
    public long serverPosZ;

    /**
     * Render entity even if it is outside the camera frustum. Only true in EntityFish for now. Used in RenderGlobal:
     * render if ignoreFrustumCheck or in frustum.
     */
    public boolean ignoreFrustumCheck;
    public boolean isAirBorne;
    public int timeUntilPortal;

    /** Whether the entity is inside a Portal */
    protected boolean inPortal;
    protected int portalCounter;

    /** Which dimension the player is in (-1 = the Nether, 0 = normal world) */
    public int dimension;

    /** The position of the last portal the entity was in */
    protected BlockPos lastPortalPos;

    /**
     * A horizontal vector related to the position of the last portal the entity was in
     */
    protected Vec3d lastPortalVec;

    /**
     * A direction related to the position of the last portal the entity was in
     */
    protected EnumFacing teleportDirection;
    private boolean invulnerable;
    protected UUID entityUniqueID;

    /** The command result statistics for this Entity. */
    private final CommandResultStats cmdResultStats;
    private final List<ItemStack> field_184235_aE;
    protected boolean glowing;
    private final Set<String> tags;
    private boolean field_184237_aG;

    public Entity(World worldIn)
    {
        this.entityId = nextEntityID++;
        this.riddenByEntities = Lists.<Entity>newArrayList();
        this.boundingBox = ZERO_AABB;
        this.width = 0.6F;
        this.height = 1.8F;
        this.nextStepDistance = 1;
        this.rand = new Random();
        this.fireResistance = 1;
        this.firstUpdate = true;
        this.entityUniqueID = MathHelper.getRandomUuid(this.rand);
        this.cmdResultStats = new CommandResultStats();
        this.field_184235_aE = Lists.<ItemStack>newArrayList();
        this.tags = Sets.<String>newHashSet();
        this.worldObj = worldIn;
        this.setPosition(0.0D, 0.0D, 0.0D);

        if (worldIn != null)
        {
            this.dimension = worldIn.provider.getDimensionType().getId();
        }

        this.dataWatcher = new EntityDataManager(this);
        this.dataWatcher.register(FLAGS, Byte.valueOf((byte)0));
        this.dataWatcher.register(AIR, Integer.valueOf(300));
        this.dataWatcher.register(CUSTOM_NAME_VISIBLE, Boolean.valueOf(false));
        this.dataWatcher.register(CUSTOM_NAME, "");
        this.dataWatcher.register(SILENT, Boolean.valueOf(false));
        this.entityInit();
    }

    public int getEntityId()
    {
        return this.entityId;
    }

    public void setEntityId(int id)
    {
        this.entityId = id;
    }

    public Set<String> getTags()
    {
        return this.tags;
    }

    public boolean addTag(String tag)
    {
        if (this.tags.size() >= 1024)
        {
            return false;
        }
        else
        {
            this.tags.add(tag);
            return true;
        }
    }

    public boolean removeTag(String tag)
    {
        return this.tags.remove(tag);
    }

    /**
     * Called by the /kill command.
     */
    public void onKillCommand()
    {
        this.setDead();
    }

    protected abstract void entityInit();

    public EntityDataManager getDataManager()
    {
        return this.dataWatcher;
    }

    public boolean equals(Object p_equals_1_)
    {
        return p_equals_1_ instanceof Entity ? ((Entity)p_equals_1_).entityId == this.entityId : false;
    }

    public int hashCode()
    {
        return this.entityId;
    }

    /**
     * Keeps moving the entity up so it isn't colliding with blocks and other requirements for this entity to be spawned
     * (only actually used on players though its also on Entity)
     */
    protected void preparePlayerToSpawn()
    {
        if (this.worldObj != null)
        {
            while (this.posY > 0.0D && this.posY < 256.0D)
            {
                this.setPosition(this.posX, this.posY, this.posZ);

                if (this.worldObj.func_184144_a(this, this.getEntityBoundingBox()).isEmpty())
                {
                    break;
                }

                ++this.posY;
            }

            this.motionX = this.motionY = this.motionZ = 0.0D;
            this.rotationPitch = 0.0F;
        }
    }

    /**
     * Will get destroyed next tick.
     */
    public void setDead()
    {
        this.isDead = true;
    }

    public void func_184174_b(boolean p_184174_1_)
    {
    }

    /**
     * Sets the width and height of the entity. Args: width, height
     */
    protected void setSize(float width, float height)
    {
        if (width != this.width || height != this.height)
        {
            float f = this.width;
            this.width = width;
            this.height = height;
            AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
            this.setEntityBoundingBox(new AxisAlignedBB(axisalignedbb.minX, axisalignedbb.minY, axisalignedbb.minZ, axisalignedbb.minX + (double)this.width, axisalignedbb.minY + (double)this.height, axisalignedbb.minZ + (double)this.width));

            if (this.width > f && !this.firstUpdate && !this.worldObj.isRemote)
            {
                this.moveEntity((double)(f - this.width), 0.0D, (double)(f - this.width));
            }
        }
    }

    /**
     * Sets the rotation of the entity. Args: yaw, pitch (both in degrees)
     */
    protected void setRotation(float yaw, float pitch)
    {
        this.rotationYaw = yaw % 360.0F;
        this.rotationPitch = pitch % 360.0F;
    }

    /**
     * Sets the x,y,z of the entity from the given parameters. Also seems to set up a bounding box.
     */
    public void setPosition(double x, double y, double z)
    {
        this.posX = x;
        this.posY = y;
        this.posZ = z;
        float f = this.width / 2.0F;
        float f1 = this.height;
        this.setEntityBoundingBox(new AxisAlignedBB(x - (double)f, y, z - (double)f, x + (double)f, y + (double)f1, z + (double)f));
    }

    /**
     * Adds 15% to the entity's yaw and subtracts 15% from the pitch. Clamps pitch from -90 to 90. Both arguments in
     * degrees.
     */
    public void setAngles(float yaw, float pitch)
    {
        float f = this.rotationPitch;
        float f1 = this.rotationYaw;
        this.rotationYaw = (float)((double)this.rotationYaw + (double)yaw * 0.15D);
        this.rotationPitch = (float)((double)this.rotationPitch - (double)pitch * 0.15D);
        this.rotationPitch = MathHelper.clamp_float(this.rotationPitch, -90.0F, 90.0F);
        this.prevRotationPitch += this.rotationPitch - f;
        this.prevRotationYaw += this.rotationYaw - f1;

        if (this.ridingEntity != null)
        {
            this.ridingEntity.func_184190_l(this);
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        if (!this.worldObj.isRemote)
        {
            this.setFlag(6, this.isGlowing());
        }

        this.onEntityUpdate();
    }

    /**
     * Gets called every tick from main Entity class
     */
    public void onEntityUpdate()
    {
        this.worldObj.theProfiler.startSection("entityBaseTick");

        if (this.isRiding() && this.getRidingEntity().isDead)
        {
            this.dismountRidingEntity();
        }

        if (this.field_184245_j > 0)
        {
            --this.field_184245_j;
        }

        this.prevDistanceWalkedModified = this.distanceWalkedModified;
        this.prevPosX = this.posX;
        this.prevPosY = this.posY;
        this.prevPosZ = this.posZ;
        this.prevRotationPitch = this.rotationPitch;
        this.prevRotationYaw = this.rotationYaw;

        if (!this.worldObj.isRemote && this.worldObj instanceof WorldServer)
        {
            this.worldObj.theProfiler.startSection("portal");

            if (this.inPortal)
            {
                MinecraftServer minecraftserver = this.worldObj.getMinecraftServer();

                if (minecraftserver.getAllowNether())
                {
                    if (!this.isRiding())
                    {
                        int i = this.getMaxInPortalTime();

                        if (this.portalCounter++ >= i)
                        {
                            this.portalCounter = i;
                            this.timeUntilPortal = this.getPortalCooldown();
                            int j;

                            if (this.worldObj.provider.getDimensionType().getId() == -1)
                            {
                                j = 0;
                            }
                            else
                            {
                                j = -1;
                            }

                            this.changeDimension(j);
                        }
                    }

                    this.inPortal = false;
                }
            }
            else
            {
                if (this.portalCounter > 0)
                {
                    this.portalCounter -= 4;
                }

                if (this.portalCounter < 0)
                {
                    this.portalCounter = 0;
                }
            }

            this.func_184173_H();
            this.worldObj.theProfiler.endSection();
        }

        this.spawnRunningParticles();
        this.handleWaterMovement();

        if (this.worldObj.isRemote)
        {
            this.fire = 0;
        }
        else if (this.fire > 0)
        {
            if (this.isImmuneToFire)
            {
                this.fire -= 4;

                if (this.fire < 0)
                {
                    this.fire = 0;
                }
            }
            else
            {
                if (this.fire % 20 == 0)
                {
                    this.attackEntityFrom(DamageSource.onFire, 1.0F);
                }

                --this.fire;
            }
        }

        if (this.isInLava())
        {
            this.setOnFireFromLava();
            this.fallDistance *= 0.5F;
        }

        if (this.posY < -64.0D)
        {
            this.kill();
        }

        if (!this.worldObj.isRemote)
        {
            this.setFlag(0, this.fire > 0);
        }

        this.firstUpdate = false;
        this.worldObj.theProfiler.endSection();
    }

    protected void func_184173_H()
    {
        if (this.timeUntilPortal > 0)
        {
            --this.timeUntilPortal;
        }
    }

    /**
     * Return the amount of time this entity should stay in a portal before being transported.
     */
    public int getMaxInPortalTime()
    {
        return 1;
    }

    /**
     * Called whenever the entity is walking inside of lava.
     */
    protected void setOnFireFromLava()
    {
        if (!this.isImmuneToFire)
        {
            this.attackEntityFrom(DamageSource.lava, 4.0F);
            this.setFire(15);
        }
    }

    /**
     * Sets entity to burn for x amount of seconds, cannot lower amount of existing fire.
     */
    public void setFire(int seconds)
    {
        int i = seconds * 20;

        if (this instanceof EntityLivingBase)
        {
            i = EnchantmentProtection.getFireTimeForEntity((EntityLivingBase)this, i);
        }

        if (this.fire < i)
        {
            this.fire = i;
        }
    }

    /**
     * Removes fire from entity.
     */
    public void extinguish()
    {
        this.fire = 0;
    }

    /**
     * sets the dead flag. Used when you fall off the bottom of the world.
     */
    protected void kill()
    {
        this.setDead();
    }

    /**
     * Checks if the offset position from the entity's current position is inside of liquid. Args: x, y, z
     */
    public boolean isOffsetPositionInLiquid(double x, double y, double z)
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox().offset(x, y, z);
        return this.isLiquidPresentInAABB(axisalignedbb);
    }

    /**
     * Determines if a liquid is present within the specified AxisAlignedBB.
     */
    private boolean isLiquidPresentInAABB(AxisAlignedBB bb)
    {
        return this.worldObj.func_184144_a(this, bb).isEmpty() && !this.worldObj.isAnyLiquid(bb);
    }

    /**
     * Tries to moves the entity by the passed in displacement. Args: x, y, z
     */
    public void moveEntity(double x, double y, double z)
    {
        if (this.noClip)
        {
            this.setEntityBoundingBox(this.getEntityBoundingBox().offset(x, y, z));
            this.resetPositionToBB();
        }
        else
        {
            this.worldObj.theProfiler.startSection("move");
            double d0 = this.posX;
            double d1 = this.posY;
            double d2 = this.posZ;

            if (this.isInWeb)
            {
                this.isInWeb = false;
                x *= 0.25D;
                y *= 0.05000000074505806D;
                z *= 0.25D;
                this.motionX = 0.0D;
                this.motionY = 0.0D;
                this.motionZ = 0.0D;
            }

            double d3 = x;
            double d4 = y;
            double d5 = z;
            boolean flag = this.onGround && this.isSneaking() && this instanceof EntityPlayer;

            if (flag)
            {
                double d6;

                for (d6 = 0.05D; x != 0.0D && this.worldObj.func_184144_a(this, this.getEntityBoundingBox().offset(x, -1.0D, 0.0D)).isEmpty(); d3 = x)
                {
                    if (x < d6 && x >= -d6)
                    {
                        x = 0.0D;
                    }
                    else if (x > 0.0D)
                    {
                        x -= d6;
                    }
                    else
                    {
                        x += d6;
                    }
                }

                for (; z != 0.0D && this.worldObj.func_184144_a(this, this.getEntityBoundingBox().offset(0.0D, -1.0D, z)).isEmpty(); d5 = z)
                {
                    if (z < d6 && z >= -d6)
                    {
                        z = 0.0D;
                    }
                    else if (z > 0.0D)
                    {
                        z -= d6;
                    }
                    else
                    {
                        z += d6;
                    }
                }

                for (; x != 0.0D && z != 0.0D && this.worldObj.func_184144_a(this, this.getEntityBoundingBox().offset(x, -1.0D, z)).isEmpty(); d5 = z)
                {
                    if (x < d6 && x >= -d6)
                    {
                        x = 0.0D;
                    }
                    else if (x > 0.0D)
                    {
                        x -= d6;
                    }
                    else
                    {
                        x += d6;
                    }

                    d3 = x;

                    if (z < d6 && z >= -d6)
                    {
                        z = 0.0D;
                    }
                    else if (z > 0.0D)
                    {
                        z -= d6;
                    }
                    else
                    {
                        z += d6;
                    }
                }
            }

            List<AxisAlignedBB> list1 = this.worldObj.func_184144_a(this, this.getEntityBoundingBox().addCoord(x, y, z));
            AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
            int i = 0;

            for (int j = list1.size(); i < j; ++i)
            {
                y = ((AxisAlignedBB)list1.get(i)).calculateYOffset(this.getEntityBoundingBox(), y);
            }

            this.setEntityBoundingBox(this.getEntityBoundingBox().offset(0.0D, y, 0.0D));
            boolean i_ = this.onGround || d4 != y && d4 < 0.0D;
            int j4 = 0;

            for (int k = list1.size(); j4 < k; ++j4)
            {
                x = ((AxisAlignedBB)list1.get(j4)).calculateXOffset(this.getEntityBoundingBox(), x);
            }

            this.setEntityBoundingBox(this.getEntityBoundingBox().offset(x, 0.0D, 0.0D));
            j4 = 0;

            for (int k4 = list1.size(); j4 < k4; ++j4)
            {
                z = ((AxisAlignedBB)list1.get(j4)).calculateZOffset(this.getEntityBoundingBox(), z);
            }

            this.setEntityBoundingBox(this.getEntityBoundingBox().offset(0.0D, 0.0D, z));

            if (this.stepHeight > 0.0F && i_ && (d3 != x || d5 != z))
            {
                double d11 = x;
                double d7 = y;
                double d8 = z;
                AxisAlignedBB axisalignedbb1 = this.getEntityBoundingBox();
                this.setEntityBoundingBox(axisalignedbb);
                y = (double)this.stepHeight;
                List<AxisAlignedBB> list = this.worldObj.func_184144_a(this, this.getEntityBoundingBox().addCoord(d3, y, d5));
                AxisAlignedBB axisalignedbb2 = this.getEntityBoundingBox();
                AxisAlignedBB axisalignedbb3 = axisalignedbb2.addCoord(d3, 0.0D, d5);
                double d9 = y;
                int l = 0;

                for (int i1 = list.size(); l < i1; ++l)
                {
                    d9 = ((AxisAlignedBB)list.get(l)).calculateYOffset(axisalignedbb3, d9);
                }

                axisalignedbb2 = axisalignedbb2.offset(0.0D, d9, 0.0D);
                double d15 = d3;
                int j1 = 0;

                for (int k1 = list.size(); j1 < k1; ++j1)
                {
                    d15 = ((AxisAlignedBB)list.get(j1)).calculateXOffset(axisalignedbb2, d15);
                }

                axisalignedbb2 = axisalignedbb2.offset(d15, 0.0D, 0.0D);
                double d16 = d5;
                int l1 = 0;

                for (int i2 = list.size(); l1 < i2; ++l1)
                {
                    d16 = ((AxisAlignedBB)list.get(l1)).calculateZOffset(axisalignedbb2, d16);
                }

                axisalignedbb2 = axisalignedbb2.offset(0.0D, 0.0D, d16);
                AxisAlignedBB axisalignedbb4 = this.getEntityBoundingBox();
                double d17 = y;
                int j2 = 0;

                for (int k2 = list.size(); j2 < k2; ++j2)
                {
                    d17 = ((AxisAlignedBB)list.get(j2)).calculateYOffset(axisalignedbb4, d17);
                }

                axisalignedbb4 = axisalignedbb4.offset(0.0D, d17, 0.0D);
                double d18 = d3;
                int l2 = 0;

                for (int i3 = list.size(); l2 < i3; ++l2)
                {
                    d18 = ((AxisAlignedBB)list.get(l2)).calculateXOffset(axisalignedbb4, d18);
                }

                axisalignedbb4 = axisalignedbb4.offset(d18, 0.0D, 0.0D);
                double d19 = d5;
                int j3 = 0;

                for (int k3 = list.size(); j3 < k3; ++j3)
                {
                    d19 = ((AxisAlignedBB)list.get(j3)).calculateZOffset(axisalignedbb4, d19);
                }

                axisalignedbb4 = axisalignedbb4.offset(0.0D, 0.0D, d19);
                double d20 = d15 * d15 + d16 * d16;
                double d10 = d18 * d18 + d19 * d19;

                if (d20 > d10)
                {
                    x = d15;
                    z = d16;
                    y = -d9;
                    this.setEntityBoundingBox(axisalignedbb2);
                }
                else
                {
                    x = d18;
                    z = d19;
                    y = -d17;
                    this.setEntityBoundingBox(axisalignedbb4);
                }

                int l3 = 0;

                for (int i4 = list.size(); l3 < i4; ++l3)
                {
                    y = ((AxisAlignedBB)list.get(l3)).calculateYOffset(this.getEntityBoundingBox(), y);
                }

                this.setEntityBoundingBox(this.getEntityBoundingBox().offset(0.0D, y, 0.0D));

                if (d11 * d11 + d8 * d8 >= x * x + z * z)
                {
                    x = d11;
                    y = d7;
                    z = d8;
                    this.setEntityBoundingBox(axisalignedbb1);
                }
            }

            this.worldObj.theProfiler.endSection();
            this.worldObj.theProfiler.startSection("rest");
            this.resetPositionToBB();
            this.isCollidedHorizontally = d3 != x || d5 != z;
            this.isCollidedVertically = d4 != y;
            this.onGround = this.isCollidedVertically && d4 < 0.0D;
            this.isCollided = this.isCollidedHorizontally || this.isCollidedVertically;
            j4 = MathHelper.floor_double(this.posX);
            int l4 = MathHelper.floor_double(this.posY - 0.20000000298023224D);
            int i5 = MathHelper.floor_double(this.posZ);
            BlockPos blockpos = new BlockPos(j4, l4, i5);
            IBlockState iblockstate = this.worldObj.getBlockState(blockpos);

            if (iblockstate.getMaterial() == Material.air)
            {
                BlockPos blockpos1 = blockpos.down();
                IBlockState iblockstate1 = this.worldObj.getBlockState(blockpos1);
                Block block1 = iblockstate1.getBlock();

                if (block1 instanceof BlockFence || block1 instanceof BlockWall || block1 instanceof BlockFenceGate)
                {
                    iblockstate = iblockstate1;
                    blockpos = blockpos1;
                }
            }

            this.updateFallState(y, this.onGround, iblockstate, blockpos);

            if (d3 != x)
            {
                this.motionX = 0.0D;
            }

            if (d5 != z)
            {
                this.motionZ = 0.0D;
            }

            Block block = iblockstate.getBlock();

            if (d4 != y)
            {
                block.onLanded(this.worldObj, this);
            }

            if (this.canTriggerWalking() && !flag && !this.isRiding())
            {
                double d12 = this.posX - d0;
                double d13 = this.posY - d1;
                double d14 = this.posZ - d2;

                if (block != Blocks.ladder)
                {
                    d13 = 0.0D;
                }

                if (block != null && this.onGround)
                {
                    block.onEntityCollidedWithBlock(this.worldObj, blockpos, this);
                }

                this.distanceWalkedModified = (float)((double)this.distanceWalkedModified + (double)MathHelper.sqrt_double(d12 * d12 + d14 * d14) * 0.6D);
                this.distanceWalkedOnStepModified = (float)((double)this.distanceWalkedOnStepModified + (double)MathHelper.sqrt_double(d12 * d12 + d13 * d13 + d14 * d14) * 0.6D);

                if (this.distanceWalkedOnStepModified > (float)this.nextStepDistance && iblockstate.getMaterial() != Material.air)
                {
                    this.nextStepDistance = (int)this.distanceWalkedOnStepModified + 1;

                    if (this.isInWater())
                    {
                        float f = MathHelper.sqrt_double(this.motionX * this.motionX * 0.20000000298023224D + this.motionY * this.motionY + this.motionZ * this.motionZ * 0.20000000298023224D) * 0.35F;

                        if (f > 1.0F)
                        {
                            f = 1.0F;
                        }

                        this.playSound(this.getSwimSound(), f, 1.0F + (this.rand.nextFloat() - this.rand.nextFloat()) * 0.4F);
                    }

                    this.playStepSound(blockpos, block);
                }
            }

            try
            {
                this.doBlockCollisions();
            }
            catch (Throwable throwable)
            {
                CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Checking entity block collision");
                CrashReportCategory crashreportcategory = crashreport.makeCategory("Entity being checked for collision");
                this.addEntityCrashInfo(crashreportcategory);
                throw new ReportedException(crashreport);
            }

            boolean flag1 = this.isWet();

            if (this.worldObj.isFlammableWithin(this.getEntityBoundingBox().func_186664_h(0.001D)))
            {
                this.dealFireDamage(1);

                if (!flag1)
                {
                    ++this.fire;

                    if (this.fire == 0)
                    {
                        this.setFire(8);
                    }
                }
            }
            else if (this.fire <= 0)
            {
                this.fire = -this.fireResistance;
            }

            if (flag1 && this.fire > 0)
            {
                this.playSound(SoundEvents.entity_generic_extinguish_fire, 0.7F, 1.6F + (this.rand.nextFloat() - this.rand.nextFloat()) * 0.4F);
                this.fire = -this.fireResistance;
            }

            this.worldObj.theProfiler.endSection();
        }
    }

    /**
     * Resets the entity's position to the center (planar) and bottom (vertical) points of its bounding box.
     */
    public void resetPositionToBB()
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
        this.posX = (axisalignedbb.minX + axisalignedbb.maxX) / 2.0D;
        this.posY = axisalignedbb.minY;
        this.posZ = (axisalignedbb.minZ + axisalignedbb.maxZ) / 2.0D;
    }

    protected SoundEvent getSwimSound()
    {
        return SoundEvents.entity_generic_swim;
    }

    protected SoundEvent getSplashSound()
    {
        return SoundEvents.entity_generic_splash;
    }

    protected void doBlockCollisions()
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain(axisalignedbb.minX + 0.001D, axisalignedbb.minY + 0.001D, axisalignedbb.minZ + 0.001D);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos1 = BlockPos.PooledMutableBlockPos.retain(axisalignedbb.maxX - 0.001D, axisalignedbb.maxY - 0.001D, axisalignedbb.maxZ - 0.001D);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos2 = BlockPos.PooledMutableBlockPos.retain();

        if (this.worldObj.isAreaLoaded(blockpos$pooledmutableblockpos, blockpos$pooledmutableblockpos1))
        {
            for (int i = blockpos$pooledmutableblockpos.getX(); i <= blockpos$pooledmutableblockpos1.getX(); ++i)
            {
                for (int j = blockpos$pooledmutableblockpos.getY(); j <= blockpos$pooledmutableblockpos1.getY(); ++j)
                {
                    for (int k = blockpos$pooledmutableblockpos.getZ(); k <= blockpos$pooledmutableblockpos1.getZ(); ++k)
                    {
                        blockpos$pooledmutableblockpos2.set(i, j, k);
                        IBlockState iblockstate = this.worldObj.getBlockState(blockpos$pooledmutableblockpos2);

                        try
                        {
                            iblockstate.getBlock().onEntityCollidedWithBlock(this.worldObj, blockpos$pooledmutableblockpos2, iblockstate, this);
                        }
                        catch (Throwable throwable)
                        {
                            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Colliding entity with block");
                            CrashReportCategory crashreportcategory = crashreport.makeCategory("Block being collided with");
                            CrashReportCategory.addBlockInfo(crashreportcategory, blockpos$pooledmutableblockpos2, iblockstate);
                            throw new ReportedException(crashreport);
                        }
                    }
                }
            }
        }

        blockpos$pooledmutableblockpos.release();
        blockpos$pooledmutableblockpos1.release();
        blockpos$pooledmutableblockpos2.release();
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        SoundType soundtype = blockIn.getStepSound();

        if (this.worldObj.getBlockState(pos.up()).getBlock() == Blocks.snow_layer)
        {
            soundtype = Blocks.snow_layer.getStepSound();
            this.playSound(soundtype.func_185844_d(), soundtype.func_185843_a() * 0.15F, soundtype.func_185847_b());
        }
        else if (!blockIn.getDefaultState().getMaterial().isLiquid())
        {
            this.playSound(soundtype.func_185844_d(), soundtype.func_185843_a() * 0.15F, soundtype.func_185847_b());
        }
    }

    public void playSound(SoundEvent soundIn, float volume, float pitch)
    {
        if (!this.isSilent())
        {
            this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, soundIn, this.getSoundCategory(), volume, pitch);
        }
    }

    /**
     * @return True if this entity will not play sounds
     */
    public boolean isSilent()
    {
        return ((Boolean)this.dataWatcher.get(SILENT)).booleanValue();
    }

    /**
     * When set to true the entity will not play sounds.
     */
    public void setSilent(boolean isSilent)
    {
        this.dataWatcher.set(SILENT, Boolean.valueOf(isSilent));
    }

    /**
     * returns if this entity triggers Block.onEntityWalking on the blocks they walk on. used for spiders and wolves to
     * prevent them from trampling crops
     */
    protected boolean canTriggerWalking()
    {
        return true;
    }

    protected void updateFallState(double y, boolean onGroundIn, IBlockState state, BlockPos pos)
    {
        if (onGroundIn)
        {
            if (this.fallDistance > 0.0F)
            {
                state.getBlock().onFallenUpon(this.worldObj, pos, this, this.fallDistance);
            }

            this.fallDistance = 0.0F;
        }
        else if (y < 0.0D)
        {
            this.fallDistance = (float)((double)this.fallDistance - y);
        }
    }

    /**
     * Returns the collision bounding box for this entity
     */
    public AxisAlignedBB getCollisionBoundingBox()
    {
        return null;
    }

    /**
     * Will deal the specified amount of damage to the entity if the entity isn't immune to fire damage. Args:
     * amountDamage
     */
    protected void dealFireDamage(int amount)
    {
        if (!this.isImmuneToFire)
        {
            this.attackEntityFrom(DamageSource.inFire, (float)amount);
        }
    }

    public final boolean isImmuneToFire()
    {
        return this.isImmuneToFire;
    }

    public void fall(float distance, float damageMultiplier)
    {
        if (this.isBeingRidden())
        {
            for (Entity entity : this.getPassengers())
            {
                entity.fall(distance, damageMultiplier);
            }
        }
    }

    /**
     * Checks if this entity is either in water or on an open air block in rain (used in wolves).
     */
    public boolean isWet()
    {
        if (this.inWater)
        {
            return true;
        }
        else
        {
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain(this.posX, this.posY, this.posZ);

            if (!this.worldObj.isRainingAt(blockpos$pooledmutableblockpos) && !this.worldObj.isRainingAt(blockpos$pooledmutableblockpos.set(this.posX, this.posY + (double)this.height, this.posZ)))
            {
                blockpos$pooledmutableblockpos.release();
                return false;
            }
            else
            {
                blockpos$pooledmutableblockpos.release();
                return true;
            }
        }
    }

    /**
     * Checks if this entity is inside water (if inWater field is true as a result of handleWaterMovement() returning
     * true)
     */
    public boolean isInWater()
    {
        return this.inWater;
    }

    /**
     * Returns if this entity is in water and will end up adding the waters velocity to the entity
     */
    public boolean handleWaterMovement()
    {
        if (this.getRidingEntity() instanceof EntityBoat)
        {
            this.inWater = false;
        }
        else if (this.worldObj.handleMaterialAcceleration(this.getEntityBoundingBox().expand(0.0D, -0.4000000059604645D, 0.0D).func_186664_h(0.001D), Material.water, this))
        {
            if (!this.inWater && !this.firstUpdate)
            {
                this.resetHeight();
            }

            this.fallDistance = 0.0F;
            this.inWater = true;
            this.fire = 0;
        }
        else
        {
            this.inWater = false;
        }

        return this.inWater;
    }

    /**
     * sets the players height back to normal after doing things like sleeping and dieing
     */
    protected void resetHeight()
    {
        float f = MathHelper.sqrt_double(this.motionX * this.motionX * 0.20000000298023224D + this.motionY * this.motionY + this.motionZ * this.motionZ * 0.20000000298023224D) * 0.2F;

        if (f > 1.0F)
        {
            f = 1.0F;
        }

        this.playSound(this.getSplashSound(), f, 1.0F + (this.rand.nextFloat() - this.rand.nextFloat()) * 0.4F);
        float f1 = (float)MathHelper.floor_double(this.getEntityBoundingBox().minY);

        for (int i = 0; (float)i < 1.0F + this.width * 20.0F; ++i)
        {
            float f2 = (this.rand.nextFloat() * 2.0F - 1.0F) * this.width;
            float f3 = (this.rand.nextFloat() * 2.0F - 1.0F) * this.width;
            this.worldObj.spawnParticle(EnumParticleTypes.WATER_BUBBLE, this.posX + (double)f2, (double)(f1 + 1.0F), this.posZ + (double)f3, this.motionX, this.motionY - (double)(this.rand.nextFloat() * 0.2F), this.motionZ, new int[0]);
        }

        for (int j = 0; (float)j < 1.0F + this.width * 20.0F; ++j)
        {
            float f4 = (this.rand.nextFloat() * 2.0F - 1.0F) * this.width;
            float f5 = (this.rand.nextFloat() * 2.0F - 1.0F) * this.width;
            this.worldObj.spawnParticle(EnumParticleTypes.WATER_SPLASH, this.posX + (double)f4, (double)(f1 + 1.0F), this.posZ + (double)f5, this.motionX, this.motionY, this.motionZ, new int[0]);
        }
    }

    /**
     * Attempts to create sprinting particles if the entity is sprinting and not in water.
     */
    public void spawnRunningParticles()
    {
        if (this.isSprinting() && !this.isInWater())
        {
            this.createRunningParticles();
        }
    }

    protected void createRunningParticles()
    {
        int i = MathHelper.floor_double(this.posX);
        int j = MathHelper.floor_double(this.posY - 0.20000000298023224D);
        int k = MathHelper.floor_double(this.posZ);
        BlockPos blockpos = new BlockPos(i, j, k);
        IBlockState iblockstate = this.worldObj.getBlockState(blockpos);

        if (iblockstate.getRenderType() != EnumBlockRenderType.INVISIBLE)
        {
            this.worldObj.spawnParticle(EnumParticleTypes.BLOCK_CRACK, this.posX + ((double)this.rand.nextFloat() - 0.5D) * (double)this.width, this.getEntityBoundingBox().minY + 0.1D, this.posZ + ((double)this.rand.nextFloat() - 0.5D) * (double)this.width, -this.motionX * 4.0D, 1.5D, -this.motionZ * 4.0D, new int[] {Block.getStateId(iblockstate)});
        }
    }

    /**
     * Checks if the current block the entity is within of the specified material type
     */
    public boolean isInsideOfMaterial(Material materialIn)
    {
        if (this.getRidingEntity() instanceof EntityBoat)
        {
            return false;
        }
        else
        {
            double d0 = this.posY + (double)this.getEyeHeight();
            BlockPos blockpos = new BlockPos(this.posX, d0, this.posZ);
            IBlockState iblockstate = this.worldObj.getBlockState(blockpos);

            if (iblockstate.getMaterial() == materialIn)
            {
                float f = BlockLiquid.getLiquidHeightPercent(iblockstate.getBlock().getMetaFromState(iblockstate)) - 0.11111111F;
                float f1 = (float)(blockpos.getY() + 1) - f;
                boolean flag = d0 < (double)f1;
                return !flag && this instanceof EntityPlayer ? false : flag;
            }
            else
            {
                return false;
            }
        }
    }

    public boolean isInLava()
    {
        return this.worldObj.isMaterialInBB(this.getEntityBoundingBox().expand(-0.10000000149011612D, -0.4000000059604645D, -0.10000000149011612D), Material.lava);
    }

    /**
     * Used in both water and by flying objects
     */
    public void moveFlying(float strafe, float forward, float friction)
    {
        float f = strafe * strafe + forward * forward;

        if (f >= 1.0E-4F)
        {
            f = MathHelper.sqrt_float(f);

            if (f < 1.0F)
            {
                f = 1.0F;
            }

            f = friction / f;
            strafe = strafe * f;
            forward = forward * f;
            float f1 = MathHelper.sin(this.rotationYaw * 0.017453292F);
            float f2 = MathHelper.cos(this.rotationYaw * 0.017453292F);
            this.motionX += (double)(strafe * f2 - forward * f1);
            this.motionZ += (double)(forward * f2 + strafe * f1);
        }
    }

    public int getBrightnessForRender(float partialTicks)
    {
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos(MathHelper.floor_double(this.posX), 0, MathHelper.floor_double(this.posZ));

        if (this.worldObj.isBlockLoaded(blockpos$mutableblockpos))
        {
            blockpos$mutableblockpos.setY(MathHelper.floor_double(this.posY + (double)this.getEyeHeight()));
            return this.worldObj.getCombinedLight(blockpos$mutableblockpos, 0);
        }
        else
        {
            return 0;
        }
    }

    /**
     * Gets how bright this entity is.
     */
    public float getBrightness(float partialTicks)
    {
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos(MathHelper.floor_double(this.posX), 0, MathHelper.floor_double(this.posZ));

        if (this.worldObj.isBlockLoaded(blockpos$mutableblockpos))
        {
            blockpos$mutableblockpos.setY(MathHelper.floor_double(this.posY + (double)this.getEyeHeight()));
            return this.worldObj.getLightBrightness(blockpos$mutableblockpos);
        }
        else
        {
            return 0.0F;
        }
    }

    /**
     * Sets the reference to the World object.
     */
    public void setWorld(World worldIn)
    {
        this.worldObj = worldIn;
    }

    /**
     * Sets the entity's position and rotation.
     */
    public void setPositionAndRotation(double x, double y, double z, float yaw, float pitch)
    {
        this.prevPosX = this.posX = MathHelper.clamp_double(x, -3.0E7D, 3.0E7D);
        this.prevPosY = this.posY = y;
        this.prevPosZ = this.posZ = MathHelper.clamp_double(z, -3.0E7D, 3.0E7D);
        pitch = MathHelper.clamp_float(pitch, -90.0F, 90.0F);
        this.prevRotationYaw = this.rotationYaw = yaw;
        this.prevRotationPitch = this.rotationPitch = pitch;
        double d0 = (double)(this.prevRotationYaw - yaw);

        if (d0 < -180.0D)
        {
            this.prevRotationYaw += 360.0F;
        }

        if (d0 >= 180.0D)
        {
            this.prevRotationYaw -= 360.0F;
        }

        this.setPosition(this.posX, this.posY, this.posZ);
        this.setRotation(yaw, pitch);
    }

    public void moveToBlockPosAndAngles(BlockPos pos, float rotationYawIn, float rotationPitchIn)
    {
        this.setLocationAndAngles((double)pos.getX() + 0.5D, (double)pos.getY(), (double)pos.getZ() + 0.5D, rotationYawIn, rotationPitchIn);
    }

    /**
     * Sets the location and Yaw/Pitch of an entity in the world
     */
    public void setLocationAndAngles(double x, double y, double z, float yaw, float pitch)
    {
        this.lastTickPosX = this.prevPosX = this.posX = x;
        this.lastTickPosY = this.prevPosY = this.posY = y;
        this.lastTickPosZ = this.prevPosZ = this.posZ = z;
        this.rotationYaw = yaw;
        this.rotationPitch = pitch;
        this.setPosition(this.posX, this.posY, this.posZ);
    }

    /**
     * Returns the distance to the entity. Args: entity
     */
    public float getDistanceToEntity(Entity entityIn)
    {
        float f = (float)(this.posX - entityIn.posX);
        float f1 = (float)(this.posY - entityIn.posY);
        float f2 = (float)(this.posZ - entityIn.posZ);
        return MathHelper.sqrt_float(f * f + f1 * f1 + f2 * f2);
    }

    /**
     * Gets the squared distance to the position. Args: x, y, z
     */
    public double getDistanceSq(double x, double y, double z)
    {
        double d0 = this.posX - x;
        double d1 = this.posY - y;
        double d2 = this.posZ - z;
        return d0 * d0 + d1 * d1 + d2 * d2;
    }

    public double getDistanceSq(BlockPos pos)
    {
        return pos.distanceSq(this.posX, this.posY, this.posZ);
    }

    public double getDistanceSqToCenter(BlockPos pos)
    {
        return pos.distanceSqToCenter(this.posX, this.posY, this.posZ);
    }

    /**
     * Gets the distance to the position. Args: x, y, z
     */
    public double getDistance(double x, double y, double z)
    {
        double d0 = this.posX - x;
        double d1 = this.posY - y;
        double d2 = this.posZ - z;
        return (double)MathHelper.sqrt_double(d0 * d0 + d1 * d1 + d2 * d2);
    }

    /**
     * Returns the squared distance to the entity. Args: entity
     */
    public double getDistanceSqToEntity(Entity entityIn)
    {
        double d0 = this.posX - entityIn.posX;
        double d1 = this.posY - entityIn.posY;
        double d2 = this.posZ - entityIn.posZ;
        return d0 * d0 + d1 * d1 + d2 * d2;
    }

    /**
     * Called by a player entity when they collide with an entity
     */
    public void onCollideWithPlayer(EntityPlayer entityIn)
    {
    }

    /**
     * Applies a velocity to each of the entities pushing them away from each other. Args: entity
     */
    public void applyEntityCollision(Entity entityIn)
    {
        if (!this.isRidingSameEntity(entityIn))
        {
            if (!entityIn.noClip && !this.noClip)
            {
                double d0 = entityIn.posX - this.posX;
                double d1 = entityIn.posZ - this.posZ;
                double d2 = MathHelper.abs_max(d0, d1);

                if (d2 >= 0.009999999776482582D)
                {
                    d2 = (double)MathHelper.sqrt_double(d2);
                    d0 = d0 / d2;
                    d1 = d1 / d2;
                    double d3 = 1.0D / d2;

                    if (d3 > 1.0D)
                    {
                        d3 = 1.0D;
                    }

                    d0 = d0 * d3;
                    d1 = d1 * d3;
                    d0 = d0 * 0.05000000074505806D;
                    d1 = d1 * 0.05000000074505806D;
                    d0 = d0 * (double)(1.0F - this.entityCollisionReduction);
                    d1 = d1 * (double)(1.0F - this.entityCollisionReduction);

                    if (!this.isBeingRidden())
                    {
                        this.addVelocity(-d0, 0.0D, -d1);
                    }

                    if (!entityIn.isBeingRidden())
                    {
                        entityIn.addVelocity(d0, 0.0D, d1);
                    }
                }
            }
        }
    }

    /**
     * Adds to the current velocity of the entity. Args: x, y, z
     */
    public void addVelocity(double x, double y, double z)
    {
        this.motionX += x;
        this.motionY += y;
        this.motionZ += z;
        this.isAirBorne = true;
    }

    /**
     * Sets that this entity has been attacked.
     */
    protected void setBeenAttacked()
    {
        this.velocityChanged = true;
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
            this.setBeenAttacked();
            return false;
        }
    }

    /**
     * interpolated look vector
     */
    public Vec3d getLook(float partialTicks)
    {
        if (partialTicks == 1.0F)
        {
            return this.getVectorForRotation(this.rotationPitch, this.rotationYaw);
        }
        else
        {
            float f = this.prevRotationPitch + (this.rotationPitch - this.prevRotationPitch) * partialTicks;
            float f1 = this.prevRotationYaw + (this.rotationYaw - this.prevRotationYaw) * partialTicks;
            return this.getVectorForRotation(f, f1);
        }
    }

    /**
     * Creates a Vec3 using the pitch and yaw of the entities rotation.
     */
    protected final Vec3d getVectorForRotation(float pitch, float yaw)
    {
        float f = MathHelper.cos(-yaw * 0.017453292F - (float)Math.PI);
        float f1 = MathHelper.sin(-yaw * 0.017453292F - (float)Math.PI);
        float f2 = -MathHelper.cos(-pitch * 0.017453292F);
        float f3 = MathHelper.sin(-pitch * 0.017453292F);
        return new Vec3d((double)(f1 * f2), (double)f3, (double)(f * f2));
    }

    public Vec3d getPositionEyes(float partialTicks)
    {
        if (partialTicks == 1.0F)
        {
            return new Vec3d(this.posX, this.posY + (double)this.getEyeHeight(), this.posZ);
        }
        else
        {
            double d0 = this.prevPosX + (this.posX - this.prevPosX) * (double)partialTicks;
            double d1 = this.prevPosY + (this.posY - this.prevPosY) * (double)partialTicks + (double)this.getEyeHeight();
            double d2 = this.prevPosZ + (this.posZ - this.prevPosZ) * (double)partialTicks;
            return new Vec3d(d0, d1, d2);
        }
    }

    public RayTraceResult rayTrace(double blockReachDistance, float partialTicks)
    {
        Vec3d vec3d = this.getPositionEyes(partialTicks);
        Vec3d vec3d1 = this.getLook(partialTicks);
        Vec3d vec3d2 = vec3d.addVector(vec3d1.xCoord * blockReachDistance, vec3d1.yCoord * blockReachDistance, vec3d1.zCoord * blockReachDistance);
        return this.worldObj.rayTraceBlocks(vec3d, vec3d2, false, false, true);
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return false;
    }

    /**
     * Returns true if this entity should push and be pushed by other entities when colliding.
     */
    public boolean canBePushed()
    {
        return false;
    }

    /**
     * Adds a value to the player score. Currently not actually used and the entity passed in does nothing. Args:
     * entity, scoreToAdd
     */
    public void addToPlayerScore(Entity entityIn, int amount)
    {
    }

    public boolean isInRangeToRender3d(double x, double y, double z)
    {
        double d0 = this.posX - x;
        double d1 = this.posY - y;
        double d2 = this.posZ - z;
        double d3 = d0 * d0 + d1 * d1 + d2 * d2;
        return this.isInRangeToRenderDist(d3);
    }

    /**
     * Checks if the entity is in range to render by using the past in distance and comparing it to its average edge
     * length * 64 * renderDistanceWeight Args: distance
     */
    public boolean isInRangeToRenderDist(double distance)
    {
        double d0 = this.getEntityBoundingBox().getAverageEdgeLength();

        if (Double.isNaN(d0))
        {
            d0 = 1.0D;
        }

        d0 = d0 * 64.0D * renderDistanceWeight;
        return distance < d0 * d0;
    }

    public boolean func_184198_c(NBTTagCompound p_184198_1_)
    {
        String s = this.getEntityString();

        if (!this.isDead && s != null)
        {
            p_184198_1_.setString("id", s);
            this.writeToNBT(p_184198_1_);
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Either write this entity to the NBT tag given and return true, or return false without doing anything. If this
     * returns false the entity is not saved on disk. Ridden entities return false here as they are saved with their
     * rider.
     */
    public boolean writeToNBTOptional(NBTTagCompound tagCompund)
    {
        String s = this.getEntityString();

        if (!this.isDead && s != null && !this.isRiding())
        {
            tagCompund.setString("id", s);
            this.writeToNBT(tagCompund);
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Save the entity to NBT (calls an abstract helper method to write extra data)
     */
    public void writeToNBT(NBTTagCompound tagCompund)
    {
        try
        {
            tagCompund.setTag("Pos", this.newDoubleNBTList(new double[] {this.posX, this.posY, this.posZ}));
            tagCompund.setTag("Motion", this.newDoubleNBTList(new double[] {this.motionX, this.motionY, this.motionZ}));
            tagCompund.setTag("Rotation", this.newFloatNBTList(new float[] {this.rotationYaw, this.rotationPitch}));
            tagCompund.setFloat("FallDistance", this.fallDistance);
            tagCompund.setShort("Fire", (short)this.fire);
            tagCompund.setShort("Air", (short)this.getAir());
            tagCompund.setBoolean("OnGround", this.onGround);
            tagCompund.setInteger("Dimension", this.dimension);
            tagCompund.setBoolean("Invulnerable", this.invulnerable);
            tagCompund.setInteger("PortalCooldown", this.timeUntilPortal);
            tagCompund.setUniqueId("UUID", this.getUniqueID());

            if (this.getCustomNameTag() != null && !this.getCustomNameTag().isEmpty())
            {
                tagCompund.setString("CustomName", this.getCustomNameTag());
            }

            if (this.getAlwaysRenderNameTag())
            {
                tagCompund.setBoolean("CustomNameVisible", this.getAlwaysRenderNameTag());
            }

            this.cmdResultStats.writeStatsToNBT(tagCompund);

            if (this.isSilent())
            {
                tagCompund.setBoolean("Silent", this.isSilent());
            }

            if (this.glowing)
            {
                tagCompund.setBoolean("Glowing", this.glowing);
            }

            if (this.tags.size() > 0)
            {
                NBTTagList nbttaglist = new NBTTagList();

                for (String s : this.tags)
                {
                    nbttaglist.appendTag(new NBTTagString(s));
                }

                tagCompund.setTag("Tags", nbttaglist);
            }

            this.writeEntityToNBT(tagCompund);

            if (this.isBeingRidden())
            {
                NBTTagList nbttaglist1 = new NBTTagList();

                for (Entity entity : this.getPassengers())
                {
                    NBTTagCompound nbttagcompound = new NBTTagCompound();

                    if (entity.func_184198_c(nbttagcompound))
                    {
                        nbttaglist1.appendTag(nbttagcompound);
                    }
                }

                if (!nbttaglist1.hasNoTags())
                {
                    tagCompund.setTag("Passengers", nbttaglist1);
                }
            }
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Saving entity NBT");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Entity being saved");
            this.addEntityCrashInfo(crashreportcategory);
            throw new ReportedException(crashreport);
        }
    }

    /**
     * Reads the entity from NBT (calls an abstract helper method to read specialized data)
     */
    public void readFromNBT(NBTTagCompound tagCompund)
    {
        try
        {
            NBTTagList nbttaglist = tagCompund.getTagList("Pos", 6);
            NBTTagList nbttaglist2 = tagCompund.getTagList("Motion", 6);
            NBTTagList nbttaglist3 = tagCompund.getTagList("Rotation", 5);
            this.motionX = nbttaglist2.getDoubleAt(0);
            this.motionY = nbttaglist2.getDoubleAt(1);
            this.motionZ = nbttaglist2.getDoubleAt(2);

            if (Math.abs(this.motionX) > 10.0D)
            {
                this.motionX = 0.0D;
            }

            if (Math.abs(this.motionY) > 10.0D)
            {
                this.motionY = 0.0D;
            }

            if (Math.abs(this.motionZ) > 10.0D)
            {
                this.motionZ = 0.0D;
            }

            this.prevPosX = this.lastTickPosX = this.posX = nbttaglist.getDoubleAt(0);
            this.prevPosY = this.lastTickPosY = this.posY = nbttaglist.getDoubleAt(1);
            this.prevPosZ = this.lastTickPosZ = this.posZ = nbttaglist.getDoubleAt(2);
            this.prevRotationYaw = this.rotationYaw = nbttaglist3.getFloatAt(0);
            this.prevRotationPitch = this.rotationPitch = nbttaglist3.getFloatAt(1);
            this.setRotationYawHead(this.rotationYaw);
            this.setRenderYawOffset(this.rotationYaw);
            this.fallDistance = tagCompund.getFloat("FallDistance");
            this.fire = tagCompund.getShort("Fire");
            this.setAir(tagCompund.getShort("Air"));
            this.onGround = tagCompund.getBoolean("OnGround");

            if (tagCompund.hasKey("Dimension"))
            {
                this.dimension = tagCompund.getInteger("Dimension");
            }

            this.invulnerable = tagCompund.getBoolean("Invulnerable");
            this.timeUntilPortal = tagCompund.getInteger("PortalCooldown");

            if (tagCompund.hasUniqueId("UUID"))
            {
                this.entityUniqueID = tagCompund.getUniqueId("UUID");
            }

            this.setPosition(this.posX, this.posY, this.posZ);
            this.setRotation(this.rotationYaw, this.rotationPitch);

            if (tagCompund.hasKey("CustomName", 8))
            {
                this.setCustomNameTag(tagCompund.getString("CustomName"));
            }

            this.setAlwaysRenderNameTag(tagCompund.getBoolean("CustomNameVisible"));
            this.cmdResultStats.readStatsFromNBT(tagCompund);
            this.setSilent(tagCompund.getBoolean("Silent"));
            this.setGlowing(tagCompund.getBoolean("Glowing"));

            if (tagCompund.hasKey("Tags", 9))
            {
                this.tags.clear();
                NBTTagList nbttaglist1 = tagCompund.getTagList("Tags", 8);
                int i = Math.min(nbttaglist1.tagCount(), 1024);

                for (int j = 0; j < i; ++j)
                {
                    this.tags.add(nbttaglist1.getStringTagAt(j));
                }
            }

            this.readEntityFromNBT(tagCompund);

            if (this.shouldSetPosAfterLoading())
            {
                this.setPosition(this.posX, this.posY, this.posZ);
            }
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Loading entity NBT");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Entity being loaded");
            this.addEntityCrashInfo(crashreportcategory);
            throw new ReportedException(crashreport);
        }
    }

    protected boolean shouldSetPosAfterLoading()
    {
        return true;
    }

    /**
     * Returns the string that identifies this Entity's class
     */
    protected final String getEntityString()
    {
        return EntityList.getEntityString(this);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected abstract void readEntityFromNBT(NBTTagCompound tagCompund);

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected abstract void writeEntityToNBT(NBTTagCompound tagCompound);

    public void onChunkLoad()
    {
    }

    /**
     * creates a NBT list from the array of doubles passed to this function
     */
    protected NBTTagList newDoubleNBTList(double... numbers)
    {
        NBTTagList nbttaglist = new NBTTagList();

        for (double d0 : numbers)
        {
            nbttaglist.appendTag(new NBTTagDouble(d0));
        }

        return nbttaglist;
    }

    /**
     * Returns a new NBTTagList filled with the specified floats
     */
    protected NBTTagList newFloatNBTList(float... numbers)
    {
        NBTTagList nbttaglist = new NBTTagList();

        for (float f : numbers)
        {
            nbttaglist.appendTag(new NBTTagFloat(f));
        }

        return nbttaglist;
    }

    public EntityItem dropItem(Item itemIn, int size)
    {
        return this.dropItemWithOffset(itemIn, size, 0.0F);
    }

    public EntityItem dropItemWithOffset(Item itemIn, int size, float offsetY)
    {
        return this.entityDropItem(new ItemStack(itemIn, size, 0), offsetY);
    }

    /**
     * Drops an item at the position of the entity.
     */
    public EntityItem entityDropItem(ItemStack itemStackIn, float offsetY)
    {
        if (itemStackIn.stackSize != 0 && itemStackIn.getItem() != null)
        {
            EntityItem entityitem = new EntityItem(this.worldObj, this.posX, this.posY + (double)offsetY, this.posZ, itemStackIn);
            entityitem.setDefaultPickupDelay();
            this.worldObj.spawnEntityInWorld(entityitem);
            return entityitem;
        }
        else
        {
            return null;
        }
    }

    /**
     * Checks whether target entity is alive.
     */
    public boolean isEntityAlive()
    {
        return !this.isDead;
    }

    /**
     * Checks if this entity is inside of an opaque block
     */
    public boolean isEntityInsideOpaqueBlock()
    {
        if (this.noClip)
        {
            return false;
        }
        else
        {
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

            for (int i = 0; i < 8; ++i)
            {
                int j = MathHelper.floor_double(this.posY + (double)(((float)((i >> 0) % 2) - 0.5F) * 0.1F) + (double)this.getEyeHeight());
                int k = MathHelper.floor_double(this.posX + (double)(((float)((i >> 1) % 2) - 0.5F) * this.width * 0.8F));
                int l = MathHelper.floor_double(this.posZ + (double)(((float)((i >> 2) % 2) - 0.5F) * this.width * 0.8F));

                if (blockpos$pooledmutableblockpos.getX() != k || blockpos$pooledmutableblockpos.getY() != j || blockpos$pooledmutableblockpos.getZ() != l)
                {
                    blockpos$pooledmutableblockpos.set(k, j, l);

                    if (this.worldObj.getBlockState(blockpos$pooledmutableblockpos).getBlock().isVisuallyOpaque())
                    {
                        blockpos$pooledmutableblockpos.release();
                        return true;
                    }
                }
            }

            blockpos$pooledmutableblockpos.release();
            return false;
        }
    }

    public boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        return false;
    }

    /**
     * Returns a boundingBox used to collide the entity with other entities and blocks. This enables the entity to be
     * pushable on contact, like boats or minecarts.
     */
    public AxisAlignedBB getCollisionBox(Entity entityIn)
    {
        return null;
    }

    /**
     * Handles updating while being ridden by an entity
     */
    public void updateRidden()
    {
        Entity entity = this.getRidingEntity();

        if (this.isRiding() && entity.isDead)
        {
            this.dismountRidingEntity();
        }
        else
        {
            this.motionX = 0.0D;
            this.motionY = 0.0D;
            this.motionZ = 0.0D;
            this.onUpdate();

            if (this.isRiding())
            {
                entity.updatePassenger(this);
            }
        }
    }

    public void updatePassenger(Entity passenger)
    {
        if (this.isPassenger(passenger))
        {
            passenger.setPosition(this.posX, this.posY + this.getMountedYOffset() + passenger.getYOffset(), this.posZ);
        }
    }

    public void func_184190_l(Entity p_184190_1_)
    {
    }

    /**
     * Returns the Y Offset of this entity.
     */
    public double getYOffset()
    {
        return 0.0D;
    }

    /**
     * Returns the Y offset from the entity's position for any entity riding this one.
     */
    public double getMountedYOffset()
    {
        return (double)this.height * 0.75D;
    }

    public boolean startRiding(Entity entityIn)
    {
        return this.startRiding(entityIn, false);
    }

    public boolean startRiding(Entity entityIn, boolean force)
    {
        if (force || this.canBeRidden(entityIn) && entityIn.canFitPassenger(this))
        {
            if (this.isRiding())
            {
                this.dismountRidingEntity();
            }

            this.ridingEntity = entityIn;
            this.ridingEntity.addPassenger(this);
            return true;
        }
        else
        {
            return false;
        }
    }

    protected boolean canBeRidden(Entity entityIn)
    {
        return this.field_184245_j <= 0;
    }

    public void removePassengers()
    {
        for (int i = this.riddenByEntities.size() - 1; i >= 0; --i)
        {
            ((Entity)this.riddenByEntities.get(i)).dismountRidingEntity();
        }
    }

    public void dismountRidingEntity()
    {
        if (this.ridingEntity != null)
        {
            Entity entity = this.ridingEntity;
            this.ridingEntity = null;
            entity.removePassenger(this);
        }
    }

    protected void addPassenger(Entity passenger)
    {
        if (passenger.getRidingEntity() != this)
        {
            throw new IllegalStateException("Use x.startRiding(y), not y.addPassenger(x)");
        }
        else
        {
            if (!this.worldObj.isRemote && passenger instanceof EntityPlayer && !(this.getControllingPassenger() instanceof EntityPlayer))
            {
                this.riddenByEntities.add(0, passenger);
            }
            else
            {
                this.riddenByEntities.add(passenger);
            }
        }
    }

    protected void removePassenger(Entity passenger)
    {
        if (passenger.getRidingEntity() == this)
        {
            throw new IllegalStateException("Use x.stopRiding(y), not y.removePassenger(x)");
        }
        else
        {
            this.riddenByEntities.remove(passenger);
            passenger.field_184245_j = 60;
        }
    }

    protected boolean canFitPassenger(Entity passenger)
    {
        return this.getPassengers().size() < 1;
    }

    public void setPositionAndRotation2(double x, double y, double z, float yaw, float pitch, int posRotationIncrements, boolean p_180426_10_)
    {
        this.setPosition(x, y, z);
        this.setRotation(yaw, pitch);
    }

    public float getCollisionBorderSize()
    {
        return 0.0F;
    }

    /**
     * returns a (normalized) vector of where this entity is looking
     */
    public Vec3d getLookVec()
    {
        return null;
    }

    /**
     * Marks the entity as being inside a portal, activating teleportation logic in onEntityUpdate() in the following
     * tick(s).
     */
    public void setPortal(BlockPos pos)
    {
        if (this.timeUntilPortal > 0)
        {
            this.timeUntilPortal = this.getPortalCooldown();
        }
        else
        {
            if (!this.worldObj.isRemote && !pos.equals(this.lastPortalPos))
            {
                this.lastPortalPos = new BlockPos(pos);
                BlockPattern.PatternHelper blockpattern$patternhelper = Blocks.portal.func_181089_f(this.worldObj, this.lastPortalPos);
                double d0 = blockpattern$patternhelper.getFinger().getAxis() == EnumFacing.Axis.X ? (double)blockpattern$patternhelper.getPos().getZ() : (double)blockpattern$patternhelper.getPos().getX();
                double d1 = blockpattern$patternhelper.getFinger().getAxis() == EnumFacing.Axis.X ? this.posZ : this.posX;
                d1 = Math.abs(MathHelper.func_181160_c(d1 - (double)(blockpattern$patternhelper.getFinger().rotateY().getAxisDirection() == EnumFacing.AxisDirection.NEGATIVE ? 1 : 0), d0, d0 - (double)blockpattern$patternhelper.func_181118_d()));
                double d2 = MathHelper.func_181160_c(this.posY - 1.0D, (double)blockpattern$patternhelper.getPos().getY(), (double)(blockpattern$patternhelper.getPos().getY() - blockpattern$patternhelper.func_181119_e()));
                this.lastPortalVec = new Vec3d(d1, d2, 0.0D);
                this.teleportDirection = blockpattern$patternhelper.getFinger();
            }

            this.inPortal = true;
        }
    }

    /**
     * Return the amount of cooldown before this entity can use a portal again.
     */
    public int getPortalCooldown()
    {
        return 300;
    }

    /**
     * Sets the velocity to the args. Args: x, y, z
     */
    public void setVelocity(double x, double y, double z)
    {
        this.motionX = x;
        this.motionY = y;
        this.motionZ = z;
    }

    public void handleStatusUpdate(byte id)
    {
    }

    /**
     * Setups the entity to do the hurt animation. Only used by packets in multiplayer.
     */
    public void performHurtAnimation()
    {
    }

    public Iterable<ItemStack> getHeldEquipment()
    {
        return this.field_184235_aE;
    }

    public Iterable<ItemStack> getArmorInventoryList()
    {
        return this.field_184235_aE;
    }

    public Iterable<ItemStack> func_184209_aF()
    {
        return Iterables.<ItemStack>concat(this.getHeldEquipment(), this.getArmorInventoryList());
    }

    public void setItemStackToSlot(EntityEquipmentSlot slotIn, ItemStack stack)
    {
    }

    /**
     * Returns true if the entity is on fire. Used by render to add the fire effect on rendering.
     */
    public boolean isBurning()
    {
        boolean flag = this.worldObj != null && this.worldObj.isRemote;
        return !this.isImmuneToFire && (this.fire > 0 || flag && this.getFlag(0));
    }

    public boolean isRiding()
    {
        return this.getRidingEntity() != null;
    }

    /**
     * If at least 1 entity is riding this one
     */
    public boolean isBeingRidden()
    {
        return !this.getPassengers().isEmpty();
    }

    /**
     * Returns if this entity is sneaking.
     */
    public boolean isSneaking()
    {
        return this.getFlag(1);
    }

    /**
     * Sets the sneaking flag.
     */
    public void setSneaking(boolean sneaking)
    {
        this.setFlag(1, sneaking);
    }

    /**
     * Get if the Entity is sprinting.
     */
    public boolean isSprinting()
    {
        return this.getFlag(3);
    }

    /**
     * Set sprinting switch for Entity.
     */
    public void setSprinting(boolean sprinting)
    {
        this.setFlag(3, sprinting);
    }

    public boolean isGlowing()
    {
        return this.glowing || this.worldObj.isRemote && this.getFlag(6);
    }

    public void setGlowing(boolean p_184195_1_)
    {
        this.glowing = p_184195_1_;

        if (!this.worldObj.isRemote)
        {
            this.setFlag(6, this.glowing);
        }
    }

    public boolean isInvisible()
    {
        return this.getFlag(5);
    }

    /**
     * Only used by renderer in EntityLivingBase subclasses.
     * Determines if an entity is visible or not to a specfic player, if the entity is normally invisible.
     * For EntityLivingBase subclasses, returning false when invisible will render the entity semitransparent.
     */
    public boolean isInvisibleToPlayer(EntityPlayer player)
    {
        if (player.isSpectator())
        {
            return false;
        }
        else
        {
            Team team = this.getTeam();
            return team != null && player != null && player.getTeam() == team && team.getSeeFriendlyInvisiblesEnabled() ? false : this.isInvisible();
        }
    }

    public Team getTeam()
    {
        return this.worldObj.getScoreboard().getPlayersTeam(this.getUniqueID().toString());
    }

    public boolean func_184191_r(Entity p_184191_1_)
    {
        return this.func_184194_a(p_184191_1_.getTeam());
    }

    public boolean func_184194_a(Team p_184194_1_)
    {
        return this.getTeam() != null ? this.getTeam().isSameTeam(p_184194_1_) : false;
    }

    public void setInvisible(boolean invisible)
    {
        this.setFlag(5, invisible);
    }

    /**
     * Returns true if the flag is active for the entity. Known flags: 0) is burning; 1) is sneaking; 2) is riding
     * something; 3) is sprinting; 4) is eating
     */
    protected boolean getFlag(int flag)
    {
        return (((Byte)this.dataWatcher.get(FLAGS)).byteValue() & 1 << flag) != 0;
    }

    /**
     * Enable or disable a entity flag, see getEntityFlag to read the know flags.
     */
    protected void setFlag(int flag, boolean set)
    {
        byte b0 = ((Byte)this.dataWatcher.get(FLAGS)).byteValue();

        if (set)
        {
            this.dataWatcher.set(FLAGS, Byte.valueOf((byte)(b0 | 1 << flag)));
        }
        else
        {
            this.dataWatcher.set(FLAGS, Byte.valueOf((byte)(b0 & ~(1 << flag))));
        }
    }

    public int getAir()
    {
        return ((Integer)this.dataWatcher.get(AIR)).intValue();
    }

    public void setAir(int air)
    {
        this.dataWatcher.set(AIR, Integer.valueOf(air));
    }

    /**
     * Called when a lightning bolt hits the entity.
     */
    public void onStruckByLightning(EntityLightningBolt lightningBolt)
    {
        this.attackEntityFrom(DamageSource.lightningBolt, 5.0F);
        ++this.fire;

        if (this.fire == 0)
        {
            this.setFire(8);
        }
    }

    /**
     * This method gets called when the entity kills another one.
     */
    public void onKillEntity(EntityLivingBase entityLivingIn)
    {
    }

    protected boolean pushOutOfBlocks(double x, double y, double z)
    {
        BlockPos blockpos = new BlockPos(x, y, z);
        double d0 = x - (double)blockpos.getX();
        double d1 = y - (double)blockpos.getY();
        double d2 = z - (double)blockpos.getZ();
        List<AxisAlignedBB> list = this.worldObj.getCollisionBoxes(this.getEntityBoundingBox());

        if (list.isEmpty())
        {
            return false;
        }
        else
        {
            EnumFacing enumfacing = EnumFacing.UP;
            double d3 = Double.MAX_VALUE;

            if (!this.worldObj.isBlockFullCube(blockpos.west()) && d0 < d3)
            {
                d3 = d0;
                enumfacing = EnumFacing.WEST;
            }

            if (!this.worldObj.isBlockFullCube(blockpos.east()) && 1.0D - d0 < d3)
            {
                d3 = 1.0D - d0;
                enumfacing = EnumFacing.EAST;
            }

            if (!this.worldObj.isBlockFullCube(blockpos.north()) && d2 < d3)
            {
                d3 = d2;
                enumfacing = EnumFacing.NORTH;
            }

            if (!this.worldObj.isBlockFullCube(blockpos.south()) && 1.0D - d2 < d3)
            {
                d3 = 1.0D - d2;
                enumfacing = EnumFacing.SOUTH;
            }

            if (!this.worldObj.isBlockFullCube(blockpos.up()) && 1.0D - d1 < d3)
            {
                d3 = 1.0D - d1;
                enumfacing = EnumFacing.UP;
            }

            float f = this.rand.nextFloat() * 0.2F + 0.1F;
            float f1 = (float)enumfacing.getAxisDirection().getOffset();

            if (enumfacing.getAxis() == EnumFacing.Axis.X)
            {
                this.motionX += (double)(f1 * f);
            }
            else if (enumfacing.getAxis() == EnumFacing.Axis.Y)
            {
                this.motionY += (double)(f1 * f);
            }
            else if (enumfacing.getAxis() == EnumFacing.Axis.Z)
            {
                this.motionZ += (double)(f1 * f);
            }

            return true;
        }
    }

    /**
     * Sets the Entity inside a web block.
     */
    public void setInWeb()
    {
        this.isInWeb = true;
        this.fallDistance = 0.0F;
    }

    /**
     * Get the name of this object. For players this returns their username
     */
    public String getName()
    {
        if (this.hasCustomName())
        {
            return this.getCustomNameTag();
        }
        else
        {
            String s = EntityList.getEntityString(this);

            if (s == null)
            {
                s = "generic";
            }

            return I18n.translateToLocal("entity." + s + ".name");
        }
    }

    /**
     * Return the Entity parts making up this Entity (currently only for dragons)
     */
    public Entity[] getParts()
    {
        return null;
    }

    /**
     * Returns true if Entity argument is equal to this Entity
     */
    public boolean isEntityEqual(Entity entityIn)
    {
        return this == entityIn;
    }

    public float getRotationYawHead()
    {
        return 0.0F;
    }

    /**
     * Sets the head's yaw rotation of the entity.
     */
    public void setRotationYawHead(float rotation)
    {
    }

    /**
     * Set the render yaw offset
     */
    public void setRenderYawOffset(float offset)
    {
    }

    /**
     * If returns false, the item will not inflict any damage against entities.
     */
    public boolean canAttackWithItem()
    {
        return true;
    }

    /**
     * Called when a player attacks an entity. If this returns true the attack will not happen.
     */
    public boolean hitByEntity(Entity entityIn)
    {
        return false;
    }

    public String toString()
    {
        return String.format("%s[\'%s\'/%d, l=\'%s\', x=%.2f, y=%.2f, z=%.2f]", new Object[] {this.getClass().getSimpleName(), this.getName(), Integer.valueOf(this.entityId), this.worldObj == null ? "~NULL~" : this.worldObj.getWorldInfo().getWorldName(), Double.valueOf(this.posX), Double.valueOf(this.posY), Double.valueOf(this.posZ)});
    }

    public boolean isEntityInvulnerable(DamageSource source)
    {
        return this.invulnerable && source != DamageSource.outOfWorld && !source.isCreativePlayer();
    }

    public void func_184224_h(boolean p_184224_1_)
    {
        this.invulnerable = p_184224_1_;
    }

    /**
     * Sets this entity's location and angles to the location and angles of the passed in entity.
     */
    public void copyLocationAndAnglesFrom(Entity entityIn)
    {
        this.setLocationAndAngles(entityIn.posX, entityIn.posY, entityIn.posZ, entityIn.rotationYaw, entityIn.rotationPitch);
    }

    /**
     * Prepares this entity in new dimension by copying NBT data from entity in old dimension
     */
    private void copyDataFromOld(Entity entityIn)
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        entityIn.writeToNBT(nbttagcompound);
        nbttagcompound.removeTag("Dimension");
        this.readFromNBT(nbttagcompound);
        this.timeUntilPortal = entityIn.timeUntilPortal;
        this.lastPortalPos = entityIn.lastPortalPos;
        this.lastPortalVec = entityIn.lastPortalVec;
        this.teleportDirection = entityIn.teleportDirection;
    }

    public Entity changeDimension(int dimensionIn)
    {
        if (!this.worldObj.isRemote && !this.isDead)
        {
            this.worldObj.theProfiler.startSection("changeDimension");
            MinecraftServer minecraftserver = this.getServer();
            int i = this.dimension;
            WorldServer worldserver = minecraftserver.worldServerForDimension(i);
            WorldServer worldserver1 = minecraftserver.worldServerForDimension(dimensionIn);
            this.dimension = dimensionIn;

            if (i == 1 && dimensionIn == 1)
            {
                worldserver1 = minecraftserver.worldServerForDimension(0);
                this.dimension = 0;
            }

            this.worldObj.removeEntity(this);
            this.isDead = false;
            this.worldObj.theProfiler.startSection("reposition");
            BlockPos blockpos;

            if (dimensionIn == 1)
            {
                blockpos = worldserver1.getSpawnCoordinate();
            }
            else
            {
                double d0 = this.posX;
                double d1 = this.posZ;
                double d2 = 8.0D;

                if (dimensionIn == -1)
                {
                    d0 = MathHelper.clamp_double(d0 / d2, worldserver1.getWorldBorder().minX() + 16.0D, worldserver1.getWorldBorder().maxX() - 16.0D);
                    d1 = MathHelper.clamp_double(d1 / d2, worldserver1.getWorldBorder().minZ() + 16.0D, worldserver1.getWorldBorder().maxZ() - 16.0D);
                }
                else if (dimensionIn == 0)
                {
                    d0 = MathHelper.clamp_double(d0 * d2, worldserver1.getWorldBorder().minX() + 16.0D, worldserver1.getWorldBorder().maxX() - 16.0D);
                    d1 = MathHelper.clamp_double(d1 * d2, worldserver1.getWorldBorder().minZ() + 16.0D, worldserver1.getWorldBorder().maxZ() - 16.0D);
                }

                d0 = (double)MathHelper.clamp_int((int)d0, -29999872, 29999872);
                d1 = (double)MathHelper.clamp_int((int)d1, -29999872, 29999872);
                float f = this.rotationYaw;
                this.setLocationAndAngles(d0, this.posY, d1, 90.0F, 0.0F);
                Teleporter teleporter = worldserver1.getDefaultTeleporter();
                teleporter.placeInExistingPortal(this, f);
                blockpos = new BlockPos(this);
            }

            worldserver.updateEntityWithOptionalForce(this, false);
            this.worldObj.theProfiler.endStartSection("reloading");
            Entity entity = EntityList.createEntityByName(EntityList.getEntityString(this), worldserver1);

            if (entity != null)
            {
                entity.copyDataFromOld(this);

                if (i == 1 && dimensionIn == 1)
                {
                    BlockPos blockpos1 = worldserver1.getTopSolidOrLiquidBlock(worldserver1.getSpawnPoint());
                    entity.moveToBlockPosAndAngles(blockpos1, entity.rotationYaw, entity.rotationPitch);
                }
                else
                {
                    entity.moveToBlockPosAndAngles(blockpos, entity.rotationYaw, entity.rotationPitch);
                }

                boolean flag = entity.forceSpawn;
                entity.forceSpawn = true;
                worldserver1.spawnEntityInWorld(entity);
                entity.forceSpawn = flag;
                worldserver1.updateEntityWithOptionalForce(entity, false);
            }

            this.isDead = true;
            this.worldObj.theProfiler.endSection();
            worldserver.resetUpdateEntityTick();
            worldserver1.resetUpdateEntityTick();
            this.worldObj.theProfiler.endSection();
            return entity;
        }
        else
        {
            return null;
        }
    }

    public boolean func_184222_aU()
    {
        return true;
    }

    /**
     * Explosion resistance of a block relative to this entity
     */
    public float getExplosionResistance(Explosion explosionIn, World worldIn, BlockPos pos, IBlockState blockStateIn)
    {
        return blockStateIn.getBlock().getExplosionResistance(this);
    }

    public boolean verifyExplosion(Explosion explosionIn, World worldIn, BlockPos pos, IBlockState blockStateIn, float p_174816_5_)
    {
        return true;
    }

    /**
     * The maximum height from where the entity is alowed to jump (used in pathfinder)
     */
    public int getMaxFallHeight()
    {
        return 3;
    }

    public Vec3d getLastPortalVec()
    {
        return this.lastPortalVec;
    }

    public EnumFacing getTeleportDirection()
    {
        return this.teleportDirection;
    }

    /**
     * Return whether this entity should NOT trigger a pressure plate or a tripwire.
     */
    public boolean doesEntityNotTriggerPressurePlate()
    {
        return false;
    }

    public void addEntityCrashInfo(CrashReportCategory category)
    {
        category.addCrashSectionCallable("Entity Type", new Callable<String>()
        {
            public String call() throws Exception
            {
                return EntityList.getEntityString(Entity.this) + " (" + Entity.this.getClass().getCanonicalName() + ")";
            }
        });
        category.addCrashSection("Entity ID", Integer.valueOf(this.entityId));
        category.addCrashSectionCallable("Entity Name", new Callable<String>()
        {
            public String call() throws Exception
            {
                return Entity.this.getName();
            }
        });
        category.addCrashSection("Entity\'s Exact location", String.format("%.2f, %.2f, %.2f", new Object[] {Double.valueOf(this.posX), Double.valueOf(this.posY), Double.valueOf(this.posZ)}));
        category.addCrashSection("Entity\'s Block location", CrashReportCategory.func_184876_a(MathHelper.floor_double(this.posX), MathHelper.floor_double(this.posY), MathHelper.floor_double(this.posZ)));
        category.addCrashSection("Entity\'s Momentum", String.format("%.2f, %.2f, %.2f", new Object[] {Double.valueOf(this.motionX), Double.valueOf(this.motionY), Double.valueOf(this.motionZ)}));
        category.addCrashSectionCallable("Entity\'s Passengers", new Callable<String>()
        {
            public String call() throws Exception
            {
                return Entity.this.getPassengers().toString();
            }
        });
        category.addCrashSectionCallable("Entity\'s Vehicle", new Callable<String>()
        {
            public String call() throws Exception
            {
                return Entity.this.getRidingEntity().toString();
            }
        });
    }

    /**
     * Return whether this entity should be rendered as on fire.
     */
    public boolean canRenderOnFire()
    {
        return this.isBurning();
    }

    public void setUniqueId(UUID uniqueIdIn)
    {
        this.entityUniqueID = uniqueIdIn;
    }

    public UUID getUniqueID()
    {
        return this.entityUniqueID;
    }

    public boolean isPushedByWater()
    {
        return true;
    }

    public static double func_184183_bd()
    {
        return renderDistanceWeight;
    }

    public static void func_184227_b(double p_184227_0_)
    {
        renderDistanceWeight = p_184227_0_;
    }

    /**
     * Get the formatted ChatComponent that will be used for the sender's username in chat
     */
    public ITextComponent getDisplayName()
    {
        TextComponentString textcomponentstring = new TextComponentString(ScorePlayerTeam.formatPlayerName(this.getTeam(), this.getName()));
        textcomponentstring.getChatStyle().setChatHoverEvent(this.getHoverEvent());
        textcomponentstring.getChatStyle().setInsertion(this.getUniqueID().toString());
        return textcomponentstring;
    }

    /**
     * Sets the custom name tag for this entity
     */
    public void setCustomNameTag(String name)
    {
        this.dataWatcher.set(CUSTOM_NAME, name);
    }

    public String getCustomNameTag()
    {
        return (String)this.dataWatcher.get(CUSTOM_NAME);
    }

    /**
     * Returns true if this thing is named
     */
    public boolean hasCustomName()
    {
        return !((String)this.dataWatcher.get(CUSTOM_NAME)).isEmpty();
    }

    public void setAlwaysRenderNameTag(boolean alwaysRenderNameTag)
    {
        this.dataWatcher.set(CUSTOM_NAME_VISIBLE, Boolean.valueOf(alwaysRenderNameTag));
    }

    public boolean getAlwaysRenderNameTag()
    {
        return ((Boolean)this.dataWatcher.get(CUSTOM_NAME_VISIBLE)).booleanValue();
    }

    /**
     * Sets the position of the entity and updates the 'last' variables
     */
    public void setPositionAndUpdate(double x, double y, double z)
    {
        this.field_184237_aG = true;
        this.setLocationAndAngles(x, y, z, this.rotationYaw, this.rotationPitch);
        this.worldObj.updateEntityWithOptionalForce(this, false);
    }

    public boolean getAlwaysRenderNameTagForRender()
    {
        return this.getAlwaysRenderNameTag();
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
    }

    public EnumFacing getHorizontalFacing()
    {
        return EnumFacing.getHorizontal(MathHelper.floor_double((double)(this.rotationYaw * 4.0F / 360.0F) + 0.5D) & 3);
    }

    public EnumFacing func_184172_bi()
    {
        return this.getHorizontalFacing();
    }

    protected HoverEvent getHoverEvent()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        String s = EntityList.getEntityString(this);
        nbttagcompound.setString("id", this.getUniqueID().toString());

        if (s != null)
        {
            nbttagcompound.setString("type", s);
        }

        nbttagcompound.setString("name", this.getName());
        return new HoverEvent(HoverEvent.Action.SHOW_ENTITY, new TextComponentString(nbttagcompound.toString()));
    }

    public boolean isSpectatedByPlayer(EntityPlayerMP player)
    {
        return true;
    }

    public AxisAlignedBB getEntityBoundingBox()
    {
        return this.boundingBox;
    }

    public AxisAlignedBB func_184177_bl()
    {
        return this.getEntityBoundingBox();
    }

    public void setEntityBoundingBox(AxisAlignedBB bb)
    {
        this.boundingBox = bb;
    }

    public float getEyeHeight()
    {
        return this.height * 0.85F;
    }

    public boolean isOutsideBorder()
    {
        return this.isOutsideBorder;
    }

    public void setOutsideBorder(boolean outsideBorder)
    {
        this.isOutsideBorder = outsideBorder;
    }

    public boolean replaceItemInInventory(int inventorySlot, ItemStack itemStackIn)
    {
        return false;
    }

    /**
     * Send a chat message to the CommandSender
     */
    public void addChatMessage(ITextComponent component)
    {
    }

    /**
     * Returns {@code true} if the CommandSender is allowed to execute the command, {@code false} if not
     */
    public boolean canCommandSenderUseCommand(int permLevel, String commandName)
    {
        return true;
    }

    /**
     * Get the position in the world. <b>{@code null} is not allowed!</b> If you are not an entity in the world, return
     * the coordinates 0, 0, 0
     */
    public BlockPos getPosition()
    {
        return new BlockPos(this.posX, this.posY + 0.5D, this.posZ);
    }

    /**
     * Get the position vector. <b>{@code null} is not allowed!</b> If you are not an entity in the world, return 0.0D,
     * 0.0D, 0.0D
     */
    public Vec3d getPositionVector()
    {
        return new Vec3d(this.posX, this.posY, this.posZ);
    }

    /**
     * Get the world, if available. <b>{@code null} is not allowed!</b> If you are not an entity in the world, return
     * the overworld
     */
    public World getEntityWorld()
    {
        return this.worldObj;
    }

    /**
     * Returns the entity associated with the command sender. MAY BE NULL!
     */
    public Entity getCommandSenderEntity()
    {
        return this;
    }

    /**
     * Returns true if the command sender should be sent feedback about executed commands
     */
    public boolean sendCommandFeedback()
    {
        return false;
    }

    public void setCommandStat(CommandResultStats.Type type, int amount)
    {
        if (this.worldObj != null && !this.worldObj.isRemote)
        {
            this.cmdResultStats.func_184932_a(this.worldObj.getMinecraftServer(), this, type, amount);
        }
    }

    /**
     * Get the Minecraft server instance
     */
    public MinecraftServer getServer()
    {
        return this.worldObj.getMinecraftServer();
    }

    public CommandResultStats getCommandStats()
    {
        return this.cmdResultStats;
    }

    /**
     * Set the CommandResultStats from the entity
     */
    public void setCommandStats(Entity entityIn)
    {
        this.cmdResultStats.addAllStats(entityIn.getCommandStats());
    }

    public EnumActionResult func_184199_a(EntityPlayer player, Vec3d vec, ItemStack stack, EnumHand hand)
    {
        return EnumActionResult.PASS;
    }

    public boolean isImmuneToExplosions()
    {
        return false;
    }

    protected void applyEnchantments(EntityLivingBase entityLivingBaseIn, Entity entityIn)
    {
        if (entityIn instanceof EntityLivingBase)
        {
            EnchantmentHelper.applyThornEnchantments((EntityLivingBase)entityIn, entityLivingBaseIn);
        }

        EnchantmentHelper.applyArthropodEnchantments(entityLivingBaseIn, entityIn);
    }

    public void func_184178_b(EntityPlayerMP p_184178_1_)
    {
    }

    public void func_184203_c(EntityPlayerMP p_184203_1_)
    {
    }

    public float func_184229_a(Rotation p_184229_1_)
    {
        float f = MathHelper.wrapAngleTo180_float(this.rotationYaw);

        switch (p_184229_1_)
        {
            case CLOCKWISE_180:
                return f + 180.0F;

            case COUNTERCLOCKWISE_90:
                return f + 270.0F;

            case CLOCKWISE_90:
                return f + 90.0F;

            default:
                return f;
        }
    }

    public float func_184217_a(Mirror p_184217_1_)
    {
        float f = MathHelper.wrapAngleTo180_float(this.rotationYaw);

        switch (p_184217_1_)
        {
            case LEFT_RIGHT:
                return -f;

            case FRONT_BACK:
                return 180.0F - f;

            default:
                return f;
        }
    }

    public boolean func_184213_bq()
    {
        return false;
    }

    public boolean func_184189_br()
    {
        boolean flag = this.field_184237_aG;
        this.field_184237_aG = false;
        return flag;
    }

    /**
     * For vehicles, the first passenger is generally considered the controller and "drives" the vehicle. For example,
     * Pigs, Horses, and Boats are generally "steered" by the controlling passenger.
     */
    public Entity getControllingPassenger()
    {
        return null;
    }

    public List<Entity> getPassengers()
    {
        return (List<Entity>)(this.riddenByEntities.isEmpty() ? Collections.emptyList() : Lists.newArrayList(this.riddenByEntities));
    }

    public boolean isPassenger(Entity entityIn)
    {
        for (Entity entity : this.getPassengers())
        {
            if (entity.equals(entityIn))
            {
                return true;
            }
        }

        return false;
    }

    public Collection<Entity> func_184182_bu()
    {
        Set<Entity> set = Sets.<Entity>newHashSet();
        this.func_184175_a(Entity.class, set);
        return set;
    }

    public <T extends Entity> Collection<T> func_184180_b(Class<T> p_184180_1_)
    {
        Set<T> set = Sets.<T>newHashSet();
        this.func_184175_a(p_184180_1_, set);
        return set;
    }

    private <T extends Entity> void func_184175_a(Class<T> p_184175_1_, Set<T> p_184175_2_)
    {
        for (Entity entity : this.getPassengers())
        {
            if (p_184175_1_.isAssignableFrom(entity.getClass()))
            {
                p_184175_2_.add((T)entity);
            }

            entity.func_184175_a(p_184175_1_, p_184175_2_);
        }
    }

    public Entity getLowestRidingEntity()
    {
        Entity entity;

        for (entity = this; entity.isRiding(); entity = entity.getRidingEntity())
        {
            ;
        }

        return entity;
    }

    public boolean isRidingSameEntity(Entity p_184223_1_)
    {
        return this.getLowestRidingEntity() == p_184223_1_.getLowestRidingEntity();
    }

    public boolean isRidingOrBeingRiddenBy(Entity p_184215_1_)
    {
        for (Entity entity : this.getPassengers())
        {
            if (entity.equals(p_184215_1_))
            {
                return true;
            }

            if (entity.isRidingOrBeingRiddenBy(p_184215_1_))
            {
                return true;
            }
        }

        return false;
    }

    public boolean func_184186_bw()
    {
        Entity entity = this.getControllingPassenger();
        return entity instanceof EntityPlayer ? ((EntityPlayer)entity).isUser() : !this.worldObj.isRemote;
    }

    /**
     * Get entity this is riding
     */
    public Entity getRidingEntity()
    {
        return this.ridingEntity;
    }

    public EnumPushReaction getPushReaction()
    {
        return EnumPushReaction.NORMAL;
    }

    public SoundCategory getSoundCategory()
    {
        return SoundCategory.NEUTRAL;
    }
}
