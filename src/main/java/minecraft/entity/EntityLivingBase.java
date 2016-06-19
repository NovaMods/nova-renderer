package net.minecraft.entity;

import com.google.common.base.Objects;
import com.google.common.collect.Maps;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.block.BlockLadder;
import net.minecraft.block.BlockTrapDoor;
import net.minecraft.block.SoundType;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.enchantment.EnchantmentFrostWalker;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.ai.attributes.AbstractAttributeMap;
import net.minecraft.entity.ai.attributes.AttributeMap;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttribute;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.item.EntityBoat;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.item.EntityXPOrb;
import net.minecraft.entity.passive.EntityHorse;
import net.minecraft.entity.passive.EntityWolf;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Blocks;
import net.minecraft.init.Enchantments;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.EnumAction;
import net.minecraft.item.Item;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemElytra;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.network.play.server.SPacketAnimation;
import net.minecraft.network.play.server.SPacketCollectItem;
import net.minecraft.network.play.server.SPacketEntityEquipment;
import net.minecraft.potion.Potion;
import net.minecraft.potion.PotionEffect;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.CombatRules;
import net.minecraft.util.CombatTracker;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntityDamageSource;
import net.minecraft.util.EntitySelectors;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumHandSide;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;

public abstract class EntityLivingBase extends Entity
{
    private static final UUID sprintingSpeedBoostModifierUUID = UUID.fromString("662A6B8D-DA3E-4C1C-8813-96EA6097278D");
    private static final AttributeModifier sprintingSpeedBoostModifier = (new AttributeModifier(sprintingSpeedBoostModifierUUID, "Sprinting speed boost", 0.30000001192092896D, 2)).setSaved(false);
    protected static final DataParameter<Byte> field_184621_as = EntityDataManager.<Byte>createKey(EntityLivingBase.class, DataSerializers.BYTE);
    private static final DataParameter<Float> field_184632_c = EntityDataManager.<Float>createKey(EntityLivingBase.class, DataSerializers.FLOAT);
    private static final DataParameter<Integer> field_184633_f = EntityDataManager.<Integer>createKey(EntityLivingBase.class, DataSerializers.VARINT);
    private static final DataParameter<Boolean> HIDE_PARTICLES = EntityDataManager.<Boolean>createKey(EntityLivingBase.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Integer> ARROW_COUNT_IN_ENTITY = EntityDataManager.<Integer>createKey(EntityLivingBase.class, DataSerializers.VARINT);
    private AbstractAttributeMap attributeMap;
    private final CombatTracker _combatTracker = new CombatTracker(this);
    private final Map<Potion, PotionEffect> activePotionsMap = Maps.<Potion, PotionEffect>newHashMap();
    private final ItemStack[] field_184630_bs = new ItemStack[2];

    /**
     * The array of item stacks that are used for armor in a living inventory.
     */
    private final ItemStack[] armorArray = new ItemStack[4];

    /** Whether an arm swing is currently in progress. */
    public boolean isSwingInProgress;
    public EnumHand field_184622_au;
    public int swingProgressInt;
    public int arrowHitTimer;

    /**
     * The amount of time remaining this entity should act 'hurt'. (Visual appearance of red tint)
     */
    public int hurtTime;

    /** What the hurt time was max set to last. */
    public int maxHurtTime;

    /** The yaw at which this entity was last attacked from. */
    public float attackedAtYaw;

    /**
     * The amount of time remaining this entity should act 'dead', i.e. have a corpse in the world.
     */
    public int deathTime;
    public float prevSwingProgress;
    public float swingProgress;
    protected int field_184617_aD;
    public float field_184618_aE;
    public float limbSwingAmount;
    public float field_184619_aG;
    public int maxHurtResistantTime = 20;
    public float prevCameraPitch;
    public float cameraPitch;
    public float randomUnused2;
    public float randomUnused1;
    public float renderYawOffset;
    public float prevRenderYawOffset;

    /** Entity head rotation yaw */
    public float rotationYawHead;

    /** Entity head rotation yaw at previous tick */
    public float prevRotationYawHead;

    /**
     * A factor used to determine how far this entity will move each tick if it is jumping or falling.
     */
    public float jumpMovementFactor = 0.02F;

    /** The most recent player that has attacked this entity */
    protected EntityPlayer attackingPlayer;

    /**
     * Set to 60 when hit by the player or the player's wolf, then decrements. Used to determine whether the entity
     * should drop items on death.
     */
    protected int recentlyHit;

    /**
     * This gets set on entity death, but never used. Looks like a duplicate of isDead
     */
    protected boolean dead;

    /** The age of this EntityLiving (used to determine when it dies) */
    protected int entityAge;
    protected float prevOnGroundSpeedFactor;
    protected float onGroundSpeedFactor;
    protected float movedDistance;
    protected float prevMovedDistance;
    protected float unused180;

    /** The score value of the Mob, the amount of points the mob is worth. */
    protected int scoreValue;

    /**
     * Damage taken in the last hit. Mobs are resistant to damage less than this for a short time after taking damage.
     */
    protected float lastDamage;

    /** used to check whether entity is jumping. */
    protected boolean isJumping;
    public float moveStrafing;
    public float moveForward;
    public float randomYawVelocity;

    /**
     * The number of updates over which the new position and rotation are to be applied to the entity.
     */
    protected int newPosRotationIncrements;
    protected double field_184623_bh;
    protected double field_184624_bi;
    protected double field_184625_bj;
    protected double field_184626_bk;

    /** The new X position to be applied to the entity. */
    protected double newPosX;

    /** Whether the DataWatcher needs to be updated with the active potions */
    private boolean potionsNeedUpdate = true;

    /** is only being set, has no uses as of MC 1.1 */
    private EntityLivingBase entityLivingToAttack;
    private int revengeTimer;
    private EntityLivingBase lastAttacker;

    /** Holds the value of ticksExisted when setLastAttacker was last called. */
    private int lastAttackerTime;

    /**
     * A factor used to determine how far this entity will move each tick if it is walking on land. Adjusted by speed,
     * and slipperiness of the current block.
     */
    private float landMovementFactor;

    /** Number of ticks since last jump */
    private int jumpTicks;
    private float absorptionAmount;
    protected ItemStack field_184627_bm;
    protected int field_184628_bn;
    protected int field_184629_bo;
    private BlockPos field_184620_bC;

    /**
     * Called by the /kill command.
     */
    public void onKillCommand()
    {
        this.attackEntityFrom(DamageSource.outOfWorld, Float.MAX_VALUE);
    }

    public EntityLivingBase(World worldIn)
    {
        super(worldIn);
        this.applyEntityAttributes();
        this.setHealth(this.getMaxHealth());
        this.preventEntitySpawning = true;
        this.randomUnused1 = (float)((Math.random() + 1.0D) * 0.009999999776482582D);
        this.setPosition(this.posX, this.posY, this.posZ);
        this.randomUnused2 = (float)Math.random() * 12398.0F;
        this.rotationYaw = (float)(Math.random() * (Math.PI * 2D));
        this.rotationYawHead = this.rotationYaw;
        this.stepHeight = 0.6F;
    }

    protected void entityInit()
    {
        this.dataWatcher.register(field_184621_as, Byte.valueOf((byte)0));
        this.dataWatcher.register(field_184633_f, Integer.valueOf(0));
        this.dataWatcher.register(HIDE_PARTICLES, Boolean.valueOf(false));
        this.dataWatcher.register(ARROW_COUNT_IN_ENTITY, Integer.valueOf(0));
        this.dataWatcher.register(field_184632_c, Float.valueOf(1.0F));
    }

    protected void applyEntityAttributes()
    {
        this.getAttributeMap().registerAttribute(SharedMonsterAttributes.MAX_HEALTH);
        this.getAttributeMap().registerAttribute(SharedMonsterAttributes.KNOCKBACK_RESISTANCE);
        this.getAttributeMap().registerAttribute(SharedMonsterAttributes.MOVEMENT_SPEED);
        this.getAttributeMap().registerAttribute(SharedMonsterAttributes.ARMOR);
    }

    protected void updateFallState(double y, boolean onGroundIn, IBlockState state, BlockPos pos)
    {
        if (!this.isInWater())
        {
            this.handleWaterMovement();
        }

        if (!this.worldObj.isRemote && this.fallDistance > 3.0F && onGroundIn)
        {
            float f = (float)MathHelper.ceiling_float_int(this.fallDistance - 3.0F);

            if (state.getMaterial() != Material.air)
            {
                double d0 = Math.min((double)(0.2F + f / 15.0F), 2.5D);
                int i = (int)(150.0D * d0);
                ((WorldServer)this.worldObj).spawnParticle(EnumParticleTypes.BLOCK_DUST, this.posX, this.posY, this.posZ, i, 0.0D, 0.0D, 0.0D, 0.15000000596046448D, new int[] {Block.getStateId(state)});
            }
        }

        super.updateFallState(y, onGroundIn, state, pos);
    }

    public boolean canBreatheUnderwater()
    {
        return false;
    }

    /**
     * Gets called every tick from main Entity class
     */
    public void onEntityUpdate()
    {
        this.prevSwingProgress = this.swingProgress;
        super.onEntityUpdate();
        this.worldObj.theProfiler.startSection("livingEntityBaseTick");
        boolean flag = this instanceof EntityPlayer;

        if (this.isEntityAlive())
        {
            if (this.isEntityInsideOpaqueBlock())
            {
                this.attackEntityFrom(DamageSource.inWall, 1.0F);
            }
            else if (flag && !this.worldObj.getWorldBorder().contains(this.getEntityBoundingBox()))
            {
                double d0 = this.worldObj.getWorldBorder().getClosestDistance(this) + this.worldObj.getWorldBorder().getDamageBuffer();

                if (d0 < 0.0D)
                {
                    this.attackEntityFrom(DamageSource.inWall, (float)Math.max(1, MathHelper.floor_double(-d0 * this.worldObj.getWorldBorder().getDamageAmount())));
                }
            }
        }

        if (this.isImmuneToFire() || this.worldObj.isRemote)
        {
            this.extinguish();
        }

        boolean flag1 = flag && ((EntityPlayer)this).capabilities.disableDamage;

        if (this.isEntityAlive())
        {
            if (!this.isInsideOfMaterial(Material.water))
            {
                this.setAir(300);
            }
            else
            {
                if (!this.canBreatheUnderwater() && !this.isPotionActive(MobEffects.waterBreathing) && !flag1)
                {
                    this.setAir(this.decreaseAirSupply(this.getAir()));

                    if (this.getAir() == -20)
                    {
                        this.setAir(0);

                        for (int i = 0; i < 8; ++i)
                        {
                            float f = this.rand.nextFloat() - this.rand.nextFloat();
                            float f1 = this.rand.nextFloat() - this.rand.nextFloat();
                            float f2 = this.rand.nextFloat() - this.rand.nextFloat();
                            this.worldObj.spawnParticle(EnumParticleTypes.WATER_BUBBLE, this.posX + (double)f, this.posY + (double)f1, this.posZ + (double)f2, this.motionX, this.motionY, this.motionZ, new int[0]);
                        }

                        this.attackEntityFrom(DamageSource.drown, 2.0F);
                    }
                }

                if (!this.worldObj.isRemote && this.isRiding() && this.getRidingEntity() instanceof EntityLivingBase)
                {
                    this.dismountRidingEntity();
                }
            }

            if (!this.worldObj.isRemote)
            {
                BlockPos blockpos = new BlockPos(this);

                if (!Objects.equal(this.field_184620_bC, blockpos))
                {
                    this.field_184620_bC = blockpos;
                    this.func_184594_b(blockpos);
                }
            }
        }

        if (this.isEntityAlive() && this.isWet())
        {
            this.extinguish();
        }

        this.prevCameraPitch = this.cameraPitch;

        if (this.hurtTime > 0)
        {
            --this.hurtTime;
        }

        if (this.hurtResistantTime > 0 && !(this instanceof EntityPlayerMP))
        {
            --this.hurtResistantTime;
        }

        if (this.getHealth() <= 0.0F)
        {
            this.onDeathUpdate();
        }

        if (this.recentlyHit > 0)
        {
            --this.recentlyHit;
        }
        else
        {
            this.attackingPlayer = null;
        }

        if (this.lastAttacker != null && !this.lastAttacker.isEntityAlive())
        {
            this.lastAttacker = null;
        }

        if (this.entityLivingToAttack != null)
        {
            if (!this.entityLivingToAttack.isEntityAlive())
            {
                this.setRevengeTarget((EntityLivingBase)null);
            }
            else if (this.ticksExisted - this.revengeTimer > 100)
            {
                this.setRevengeTarget((EntityLivingBase)null);
            }
        }

        this.updatePotionEffects();
        this.prevMovedDistance = this.movedDistance;
        this.prevRenderYawOffset = this.renderYawOffset;
        this.prevRotationYawHead = this.rotationYawHead;
        this.prevRotationYaw = this.rotationYaw;
        this.prevRotationPitch = this.rotationPitch;
        this.worldObj.theProfiler.endSection();
    }

    protected void func_184594_b(BlockPos p_184594_1_)
    {
        int i = EnchantmentHelper.func_185284_a(Enchantments.frostWalker, this);

        if (i > 0)
        {
            EnchantmentFrostWalker.func_185266_a(this, this.worldObj, p_184594_1_, i);
        }
    }

    /**
     * If Animal, checks if the age timer is negative
     */
    public boolean isChild()
    {
        return false;
    }

    /**
     * handles entity death timer, experience orb and particle creation
     */
    protected void onDeathUpdate()
    {
        ++this.deathTime;

        if (this.deathTime == 20)
        {
            if (!this.worldObj.isRemote && (this.isPlayer() || this.recentlyHit > 0 && this.canDropLoot() && this.worldObj.getGameRules().getBoolean("doMobLoot")))
            {
                int i = this.getExperiencePoints(this.attackingPlayer);

                while (i > 0)
                {
                    int j = EntityXPOrb.getXPSplit(i);
                    i -= j;
                    this.worldObj.spawnEntityInWorld(new EntityXPOrb(this.worldObj, this.posX, this.posY, this.posZ, j));
                }
            }

            this.setDead();

            for (int k = 0; k < 20; ++k)
            {
                double d2 = this.rand.nextGaussian() * 0.02D;
                double d0 = this.rand.nextGaussian() * 0.02D;
                double d1 = this.rand.nextGaussian() * 0.02D;
                this.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_NORMAL, this.posX + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, this.posY + (double)(this.rand.nextFloat() * this.height), this.posZ + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, d2, d0, d1, new int[0]);
            }
        }
    }

    /**
     * Entity won't drop items or experience points if this returns false
     */
    protected boolean canDropLoot()
    {
        return !this.isChild();
    }

    /**
     * Decrements the entity's air supply when underwater
     */
    protected int decreaseAirSupply(int p_70682_1_)
    {
        int i = EnchantmentHelper.func_185292_c(this);
        return i > 0 && this.rand.nextInt(i + 1) > 0 ? p_70682_1_ : p_70682_1_ - 1;
    }

    /**
     * Get the experience points the entity currently has.
     */
    protected int getExperiencePoints(EntityPlayer player)
    {
        return 0;
    }

    /**
     * Only use is to identify if class is an instance of player for experience dropping
     */
    protected boolean isPlayer()
    {
        return false;
    }

    public Random getRNG()
    {
        return this.rand;
    }

    public EntityLivingBase getAITarget()
    {
        return this.entityLivingToAttack;
    }

    public int getRevengeTimer()
    {
        return this.revengeTimer;
    }

    public void setRevengeTarget(EntityLivingBase livingBase)
    {
        this.entityLivingToAttack = livingBase;
        this.revengeTimer = this.ticksExisted;
    }

    public EntityLivingBase getLastAttacker()
    {
        return this.lastAttacker;
    }

    public int getLastAttackerTime()
    {
        return this.lastAttackerTime;
    }

    public void setLastAttacker(Entity entityIn)
    {
        if (entityIn instanceof EntityLivingBase)
        {
            this.lastAttacker = (EntityLivingBase)entityIn;
        }
        else
        {
            this.lastAttacker = null;
        }

        this.lastAttackerTime = this.ticksExisted;
    }

    public int getAge()
    {
        return this.entityAge;
    }

    protected void func_184606_a_(ItemStack p_184606_1_)
    {
        if (p_184606_1_ != null)
        {
            SoundEvent soundevent = SoundEvents.item_armor_equip_generic;
            Item item = p_184606_1_.getItem();

            if (item instanceof ItemArmor)
            {
                soundevent = ((ItemArmor)item).getArmorMaterial().func_185017_b();
            }
            else if (item == Items.elytra)
            {
                soundevent = SoundEvents.item_armor_equip_leather;
            }

            this.playSound(soundevent, 1.0F, 1.0F);
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        tagCompound.setFloat("Health", this.getHealth());
        tagCompound.setShort("HurtTime", (short)this.hurtTime);
        tagCompound.setInteger("HurtByTimestamp", this.revengeTimer);
        tagCompound.setShort("DeathTime", (short)this.deathTime);
        tagCompound.setFloat("AbsorptionAmount", this.getAbsorptionAmount());

        for (EntityEquipmentSlot entityequipmentslot : EntityEquipmentSlot.values())
        {
            ItemStack itemstack = this.getItemStackFromSlot(entityequipmentslot);

            if (itemstack != null)
            {
                this.getAttributeMap().removeAttributeModifiers(itemstack.getAttributeModifiers(entityequipmentslot));
            }
        }

        tagCompound.setTag("Attributes", SharedMonsterAttributes.writeBaseAttributeMapToNBT(this.getAttributeMap()));

        for (EntityEquipmentSlot entityequipmentslot1 : EntityEquipmentSlot.values())
        {
            ItemStack itemstack1 = this.getItemStackFromSlot(entityequipmentslot1);

            if (itemstack1 != null)
            {
                this.getAttributeMap().applyAttributeModifiers(itemstack1.getAttributeModifiers(entityequipmentslot1));
            }
        }

        if (!this.activePotionsMap.isEmpty())
        {
            NBTTagList nbttaglist = new NBTTagList();

            for (PotionEffect potioneffect : this.activePotionsMap.values())
            {
                nbttaglist.appendTag(potioneffect.writeCustomPotionEffectToNBT(new NBTTagCompound()));
            }

            tagCompound.setTag("ActiveEffects", nbttaglist);
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        this.setAbsorptionAmount(tagCompund.getFloat("AbsorptionAmount"));

        if (tagCompund.hasKey("Attributes", 9) && this.worldObj != null && !this.worldObj.isRemote)
        {
            SharedMonsterAttributes.setAttributeModifiers(this.getAttributeMap(), tagCompund.getTagList("Attributes", 10));
        }

        if (tagCompund.hasKey("ActiveEffects", 9))
        {
            NBTTagList nbttaglist = tagCompund.getTagList("ActiveEffects", 10);

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                NBTTagCompound nbttagcompound = nbttaglist.getCompoundTagAt(i);
                PotionEffect potioneffect = PotionEffect.readCustomPotionEffectFromNBT(nbttagcompound);

                if (potioneffect != null)
                {
                    this.activePotionsMap.put(potioneffect.func_188419_a(), potioneffect);
                }
            }
        }

        if (tagCompund.hasKey("Health", 99))
        {
            this.setHealth(tagCompund.getFloat("Health"));
        }

        this.hurtTime = tagCompund.getShort("HurtTime");
        this.deathTime = tagCompund.getShort("DeathTime");
        this.revengeTimer = tagCompund.getInteger("HurtByTimestamp");

        if (tagCompund.hasKey("Team", 8))
        {
            String s = tagCompund.getString("Team");
            this.worldObj.getScoreboard().addPlayerToTeam(this.getUniqueID().toString(), s);
        }
    }

    protected void updatePotionEffects()
    {
        Iterator<Potion> iterator = this.activePotionsMap.keySet().iterator();

        while (iterator.hasNext())
        {
            Potion potion = (Potion)iterator.next();
            PotionEffect potioneffect = (PotionEffect)this.activePotionsMap.get(potion);

            if (!potioneffect.onUpdate(this))
            {
                if (!this.worldObj.isRemote)
                {
                    iterator.remove();
                    this.onFinishedPotionEffect(potioneffect);
                }
            }
            else if (potioneffect.getDuration() % 600 == 0)
            {
                this.onChangedPotionEffect(potioneffect, false);
            }
        }

        if (this.potionsNeedUpdate)
        {
            if (!this.worldObj.isRemote)
            {
                this.updatePotionMetadata();
            }

            this.potionsNeedUpdate = false;
        }

        int i = ((Integer)this.dataWatcher.get(field_184633_f)).intValue();
        boolean flag1 = ((Boolean)this.dataWatcher.get(HIDE_PARTICLES)).booleanValue();

        if (i > 0)
        {
            boolean flag = false;

            if (!this.isInvisible())
            {
                flag = this.rand.nextBoolean();
            }
            else
            {
                flag = this.rand.nextInt(15) == 0;
            }

            if (flag1)
            {
                flag &= this.rand.nextInt(5) == 0;
            }

            if (flag && i > 0)
            {
                double d0 = (double)(i >> 16 & 255) / 255.0D;
                double d1 = (double)(i >> 8 & 255) / 255.0D;
                double d2 = (double)(i >> 0 & 255) / 255.0D;
                this.worldObj.spawnParticle(flag1 ? EnumParticleTypes.SPELL_MOB_AMBIENT : EnumParticleTypes.SPELL_MOB, this.posX + (this.rand.nextDouble() - 0.5D) * (double)this.width, this.posY + this.rand.nextDouble() * (double)this.height, this.posZ + (this.rand.nextDouble() - 0.5D) * (double)this.width, d0, d1, d2, new int[0]);
            }
        }
    }

    /**
     * Clears potion metadata values if the entity has no potion effects. Otherwise, updates potion effect color,
     * ambience, and invisibility metadata values
     */
    protected void updatePotionMetadata()
    {
        if (this.activePotionsMap.isEmpty())
        {
            this.resetPotionEffectMetadata();
            this.setInvisible(false);
        }
        else
        {
            Collection<PotionEffect> collection = this.activePotionsMap.values();
            this.dataWatcher.set(HIDE_PARTICLES, Boolean.valueOf(func_184593_a(collection)));
            this.dataWatcher.set(field_184633_f, Integer.valueOf(PotionUtils.func_185181_a(collection)));
            this.setInvisible(this.isPotionActive(MobEffects.invisibility));
        }
    }

    public static boolean func_184593_a(Collection<PotionEffect> p_184593_0_)
    {
        for (PotionEffect potioneffect : p_184593_0_)
        {
            if (!potioneffect.getIsAmbient())
            {
                return false;
            }
        }

        return true;
    }

    /**
     * Resets the potion effect color and ambience metadata values
     */
    protected void resetPotionEffectMetadata()
    {
        this.dataWatcher.set(HIDE_PARTICLES, Boolean.valueOf(false));
        this.dataWatcher.set(field_184633_f, Integer.valueOf(0));
    }

    public void clearActivePotions()
    {
        if (!this.worldObj.isRemote)
        {
            Iterator<PotionEffect> iterator = this.activePotionsMap.values().iterator();

            while (iterator.hasNext())
            {
                this.onFinishedPotionEffect((PotionEffect)iterator.next());
                iterator.remove();
            }
        }
    }

    public Collection<PotionEffect> getActivePotionEffects()
    {
        return this.activePotionsMap.values();
    }

    public boolean isPotionActive(Potion potionIn)
    {
        return this.activePotionsMap.containsKey(potionIn);
    }

    /**
     * returns the PotionEffect for the supplied Potion if it is active, null otherwise.
     */
    public PotionEffect getActivePotionEffect(Potion potionIn)
    {
        return (PotionEffect)this.activePotionsMap.get(potionIn);
    }

    /**
     * adds a PotionEffect to the entity
     */
    public void addPotionEffect(PotionEffect potioneffectIn)
    {
        if (this.isPotionApplicable(potioneffectIn))
        {
            PotionEffect potioneffect = (PotionEffect)this.activePotionsMap.get(potioneffectIn.func_188419_a());

            if (potioneffect == null)
            {
                this.activePotionsMap.put(potioneffectIn.func_188419_a(), potioneffectIn);
                this.onNewPotionEffect(potioneffectIn);
            }
            else
            {
                potioneffect.combine(potioneffectIn);
                this.onChangedPotionEffect(potioneffect, true);
            }
        }
    }

    public boolean isPotionApplicable(PotionEffect potioneffectIn)
    {
        if (this.getCreatureAttribute() == EnumCreatureAttribute.UNDEAD)
        {
            Potion potion = potioneffectIn.func_188419_a();

            if (potion == MobEffects.regeneration || potion == MobEffects.poison)
            {
                return false;
            }
        }

        return true;
    }

    /**
     * Returns true if this entity is undead.
     */
    public boolean isEntityUndead()
    {
        return this.getCreatureAttribute() == EnumCreatureAttribute.UNDEAD;
    }

    public PotionEffect func_184596_c(Potion p_184596_1_)
    {
        return (PotionEffect)this.activePotionsMap.remove(p_184596_1_);
    }

    public void func_184589_d(Potion p_184589_1_)
    {
        PotionEffect potioneffect = this.func_184596_c(p_184589_1_);

        if (potioneffect != null)
        {
            this.onFinishedPotionEffect(potioneffect);
        }
    }

    protected void onNewPotionEffect(PotionEffect id)
    {
        this.potionsNeedUpdate = true;

        if (!this.worldObj.isRemote)
        {
            id.func_188419_a().applyAttributesModifiersToEntity(this, this.getAttributeMap(), id.getAmplifier());
        }
    }

    protected void onChangedPotionEffect(PotionEffect id, boolean p_70695_2_)
    {
        this.potionsNeedUpdate = true;

        if (p_70695_2_ && !this.worldObj.isRemote)
        {
            Potion potion = id.func_188419_a();
            potion.removeAttributesModifiersFromEntity(this, this.getAttributeMap(), id.getAmplifier());
            potion.applyAttributesModifiersToEntity(this, this.getAttributeMap(), id.getAmplifier());
        }
    }

    protected void onFinishedPotionEffect(PotionEffect effect)
    {
        this.potionsNeedUpdate = true;

        if (!this.worldObj.isRemote)
        {
            effect.func_188419_a().removeAttributesModifiersFromEntity(this, this.getAttributeMap(), effect.getAmplifier());
        }
    }

    /**
     * Heal living entity (param: amount of half-hearts)
     */
    public void heal(float healAmount)
    {
        float f = this.getHealth();

        if (f > 0.0F)
        {
            this.setHealth(f + healAmount);
        }
    }

    public final float getHealth()
    {
        return ((Float)this.dataWatcher.get(field_184632_c)).floatValue();
    }

    public void setHealth(float health)
    {
        this.dataWatcher.set(field_184632_c, Float.valueOf(MathHelper.clamp_float(health, 0.0F, this.getMaxHealth())));
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
        else if (this.worldObj.isRemote)
        {
            return false;
        }
        else
        {
            this.entityAge = 0;

            if (this.getHealth() <= 0.0F)
            {
                return false;
            }
            else if (source.isFireDamage() && this.isPotionActive(MobEffects.fireResistance))
            {
                return false;
            }
            else
            {
                if ((source == DamageSource.anvil || source == DamageSource.fallingBlock) && this.getItemStackFromSlot(EntityEquipmentSlot.HEAD) != null)
                {
                    this.getItemStackFromSlot(EntityEquipmentSlot.HEAD).damageItem((int)(amount * 4.0F + this.rand.nextFloat() * amount * 2.0F), this);
                    amount *= 0.75F;
                }

                boolean flag = false;

                if (amount > 0.0F && this.func_184583_d(source))
                {
                    this.func_184590_k(amount);

                    if (source.isProjectile())
                    {
                        amount = 0.0F;
                    }
                    else
                    {
                        amount *= 0.33F;

                        if (source.getSourceOfDamage() instanceof EntityLivingBase)
                        {
                            ((EntityLivingBase)source.getSourceOfDamage()).knockBack(this, 0.5F, this.posX - source.getSourceOfDamage().posX, this.posZ - source.getSourceOfDamage().posZ);
                        }
                    }

                    flag = true;
                }

                this.limbSwingAmount = 1.5F;
                boolean flag1 = true;

                if ((float)this.hurtResistantTime > (float)this.maxHurtResistantTime / 2.0F)
                {
                    if (amount <= this.lastDamage)
                    {
                        return false;
                    }

                    this.damageEntity(source, amount - this.lastDamage);
                    this.lastDamage = amount;
                    flag1 = false;
                }
                else
                {
                    this.lastDamage = amount;
                    this.hurtResistantTime = this.maxHurtResistantTime;
                    this.damageEntity(source, amount);
                    this.hurtTime = this.maxHurtTime = 10;
                }

                this.attackedAtYaw = 0.0F;
                Entity entity = source.getEntity();

                if (entity != null)
                {
                    if (entity instanceof EntityLivingBase)
                    {
                        this.setRevengeTarget((EntityLivingBase)entity);
                    }

                    if (entity instanceof EntityPlayer)
                    {
                        this.recentlyHit = 100;
                        this.attackingPlayer = (EntityPlayer)entity;
                    }
                    else if (entity instanceof EntityWolf)
                    {
                        EntityWolf entitywolf = (EntityWolf)entity;

                        if (entitywolf.isTamed())
                        {
                            this.recentlyHit = 100;
                            this.attackingPlayer = null;
                        }
                    }
                }

                if (flag1)
                {
                    if (flag)
                    {
                        this.worldObj.setEntityState(this, (byte)29);
                    }
                    else if (source instanceof EntityDamageSource && ((EntityDamageSource)source).getIsThornsDamage())
                    {
                        this.worldObj.setEntityState(this, (byte)33);
                    }
                    else
                    {
                        this.worldObj.setEntityState(this, (byte)2);
                    }

                    if (source != DamageSource.drown && (!flag || amount > 0.0F))
                    {
                        this.setBeenAttacked();
                    }

                    if (entity != null)
                    {
                        double d1 = entity.posX - this.posX;
                        double d0;

                        for (d0 = entity.posZ - this.posZ; d1 * d1 + d0 * d0 < 1.0E-4D; d0 = (Math.random() - Math.random()) * 0.01D)
                        {
                            d1 = (Math.random() - Math.random()) * 0.01D;
                        }

                        this.attackedAtYaw = (float)(MathHelper.atan2(d0, d1) * (180D / Math.PI) - (double)this.rotationYaw);
                        this.knockBack(entity, 0.4F, d1, d0);
                    }
                    else
                    {
                        this.attackedAtYaw = (float)((int)(Math.random() * 2.0D) * 180);
                    }
                }

                if (this.getHealth() <= 0.0F)
                {
                    SoundEvent soundevent = this.getDeathSound();

                    if (flag1 && soundevent != null)
                    {
                        this.playSound(soundevent, this.getSoundVolume(), this.getSoundPitch());
                    }

                    this.onDeath(source);
                }
                else if (flag1)
                {
                    this.func_184581_c(source);
                }

                return !flag || amount > 0.0F;
            }
        }
    }

    protected void func_184581_c(DamageSource p_184581_1_)
    {
        SoundEvent soundevent = this.getHurtSound();

        if (soundevent != null)
        {
            this.playSound(soundevent, this.getSoundVolume(), this.getSoundPitch());
        }
    }

    private boolean func_184583_d(DamageSource p_184583_1_)
    {
        if (!p_184583_1_.isUnblockable() && this.func_184585_cz())
        {
            Vec3d vec3d = p_184583_1_.func_188404_v();

            if (vec3d != null)
            {
                Vec3d vec3d1 = this.getLook(1.0F);
                Vec3d vec3d2 = vec3d.subtractReverse(new Vec3d(this.posX, this.posY, this.posZ)).normalize();
                vec3d2 = new Vec3d(vec3d2.xCoord, 0.0D, vec3d2.zCoord);

                if (vec3d2.dotProduct(vec3d1) < 0.0D)
                {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * Renders broken item particles using the given ItemStack
     */
    public void renderBrokenItemStack(ItemStack stack)
    {
        this.playSound(SoundEvents.entity_item_break, 0.8F, 0.8F + this.worldObj.rand.nextFloat() * 0.4F);

        for (int i = 0; i < 5; ++i)
        {
            Vec3d vec3d = new Vec3d(((double)this.rand.nextFloat() - 0.5D) * 0.1D, Math.random() * 0.1D + 0.1D, 0.0D);
            vec3d = vec3d.rotatePitch(-this.rotationPitch * 0.017453292F);
            vec3d = vec3d.rotateYaw(-this.rotationYaw * 0.017453292F);
            double d0 = (double)(-this.rand.nextFloat()) * 0.6D - 0.3D;
            Vec3d vec3d1 = new Vec3d(((double)this.rand.nextFloat() - 0.5D) * 0.3D, d0, 0.6D);
            vec3d1 = vec3d1.rotatePitch(-this.rotationPitch * 0.017453292F);
            vec3d1 = vec3d1.rotateYaw(-this.rotationYaw * 0.017453292F);
            vec3d1 = vec3d1.addVector(this.posX, this.posY + (double)this.getEyeHeight(), this.posZ);
            this.worldObj.spawnParticle(EnumParticleTypes.ITEM_CRACK, vec3d1.xCoord, vec3d1.yCoord, vec3d1.zCoord, vec3d.xCoord, vec3d.yCoord + 0.05D, vec3d.zCoord, new int[] {Item.getIdFromItem(stack.getItem())});
        }
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        if (!this.dead)
        {
            Entity entity = cause.getEntity();
            EntityLivingBase entitylivingbase = this.getAttackingEntity();

            if (this.scoreValue >= 0 && entitylivingbase != null)
            {
                entitylivingbase.addToPlayerScore(this, this.scoreValue);
            }

            if (entity != null)
            {
                entity.onKillEntity(this);
            }

            this.dead = true;
            this.getCombatTracker().reset();

            if (!this.worldObj.isRemote)
            {
                int i = 0;

                if (entity instanceof EntityPlayer)
                {
                    i = EnchantmentHelper.func_185283_h((EntityLivingBase)entity);
                }

                if (this.canDropLoot() && this.worldObj.getGameRules().getBoolean("doMobLoot"))
                {
                    boolean flag = this.recentlyHit > 0;
                    this.func_184610_a(flag, i, cause);
                }
            }

            this.worldObj.setEntityState(this, (byte)3);
        }
    }

    protected void func_184610_a(boolean p_184610_1_, int p_184610_2_, DamageSource source)
    {
        this.dropFewItems(p_184610_1_, p_184610_2_);
        this.dropEquipment(p_184610_1_, p_184610_2_);
    }

    /**
     * Drop the equipment for this entity.
     */
    protected void dropEquipment(boolean wasRecentlyHit, int lootingModifier)
    {
    }

    /**
     * knocks back this entity
     */
    public void knockBack(Entity entityIn, float p_70653_2_, double p_70653_3_, double p_70653_5_)
    {
        if (this.rand.nextDouble() >= this.getEntityAttribute(SharedMonsterAttributes.KNOCKBACK_RESISTANCE).getAttributeValue())
        {
            this.isAirBorne = true;
            float f = MathHelper.sqrt_double(p_70653_3_ * p_70653_3_ + p_70653_5_ * p_70653_5_);
            this.motionX /= 2.0D;
            this.motionZ /= 2.0D;
            this.motionX -= p_70653_3_ / (double)f * (double)p_70653_2_;
            this.motionZ -= p_70653_5_ / (double)f * (double)p_70653_2_;

            if (this.onGround)
            {
                this.motionY /= 2.0D;
                this.motionY += (double)p_70653_2_;

                if (this.motionY > 0.4000000059604645D)
                {
                    this.motionY = 0.4000000059604645D;
                }
            }
        }
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_generic_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_generic_death;
    }

    protected SoundEvent getFallSound(int heightIn)
    {
        return heightIn > 4 ? SoundEvents.entity_generic_big_fall : SoundEvents.entity_generic_small_fall;
    }

    /**
     * Drop 0-2 items of this living's type
     */
    protected void dropFewItems(boolean wasRecentlyHit, int lootingModifier)
    {
    }

    /**
     * returns true if this entity is by a ladder, false otherwise
     */
    public boolean isOnLadder()
    {
        int i = MathHelper.floor_double(this.posX);
        int j = MathHelper.floor_double(this.getEntityBoundingBox().minY);
        int k = MathHelper.floor_double(this.posZ);

        if (this instanceof EntityPlayer && ((EntityPlayer)this).isSpectator())
        {
            return false;
        }
        else
        {
            BlockPos blockpos = new BlockPos(i, j, k);
            IBlockState iblockstate = this.worldObj.getBlockState(blockpos);
            Block block = iblockstate.getBlock();
            return block != Blocks.ladder && block != Blocks.vine ? block instanceof BlockTrapDoor && this.func_184604_a(blockpos, iblockstate) : true;
        }
    }

    private boolean func_184604_a(BlockPos p_184604_1_, IBlockState p_184604_2_)
    {
        if (((Boolean)p_184604_2_.getValue(BlockTrapDoor.OPEN)).booleanValue())
        {
            IBlockState iblockstate = this.worldObj.getBlockState(p_184604_1_.down());

            if (iblockstate.getBlock() == Blocks.ladder && iblockstate.getValue(BlockLadder.FACING) == p_184604_2_.getValue(BlockTrapDoor.FACING))
            {
                return true;
            }
        }

        return false;
    }

    /**
     * Checks whether target entity is alive.
     */
    public boolean isEntityAlive()
    {
        return !this.isDead && this.getHealth() > 0.0F;
    }

    public void fall(float distance, float damageMultiplier)
    {
        super.fall(distance, damageMultiplier);
        PotionEffect potioneffect = this.getActivePotionEffect(MobEffects.jump);
        float f = potioneffect == null ? 0.0F : (float)(potioneffect.getAmplifier() + 1);
        int i = MathHelper.ceiling_float_int((distance - 3.0F - f) * damageMultiplier);

        if (i > 0)
        {
            this.playSound(this.getFallSound(i), 1.0F, 1.0F);
            this.attackEntityFrom(DamageSource.fall, (float)i);
            int j = MathHelper.floor_double(this.posX);
            int k = MathHelper.floor_double(this.posY - 0.20000000298023224D);
            int l = MathHelper.floor_double(this.posZ);
            IBlockState iblockstate = this.worldObj.getBlockState(new BlockPos(j, k, l));

            if (iblockstate.getMaterial() != Material.air)
            {
                SoundType soundtype = iblockstate.getBlock().getStepSound();
                this.playSound(soundtype.func_185842_g(), soundtype.func_185843_a() * 0.5F, soundtype.func_185847_b() * 0.75F);
            }
        }
    }

    /**
     * Setups the entity to do the hurt animation. Only used by packets in multiplayer.
     */
    public void performHurtAnimation()
    {
        this.hurtTime = this.maxHurtTime = 10;
        this.attackedAtYaw = 0.0F;
    }

    /**
     * Returns the current armor value as determined by a call to InventoryPlayer.getTotalArmorValue
     */
    public int getTotalArmorValue()
    {
        IAttributeInstance iattributeinstance = this.getEntityAttribute(SharedMonsterAttributes.ARMOR);
        return MathHelper.floor_double(iattributeinstance.getAttributeValue());
    }

    protected void damageArmor(float p_70675_1_)
    {
    }

    protected void func_184590_k(float p_184590_1_)
    {
    }

    /**
     * Reduces damage, depending on armor
     */
    protected float applyArmorCalculations(DamageSource source, float damage)
    {
        if (!source.isUnblockable())
        {
            this.damageArmor(damage);
            damage = CombatRules.func_188402_a(damage, (float)this.getTotalArmorValue());
        }

        return damage;
    }

    /**
     * Reduces damage, depending on potions
     */
    protected float applyPotionDamageCalculations(DamageSource source, float damage)
    {
        if (source.isDamageAbsolute())
        {
            return damage;
        }
        else
        {
            if (this.isPotionActive(MobEffects.resistance) && source != DamageSource.outOfWorld)
            {
                int i = (this.getActivePotionEffect(MobEffects.resistance).getAmplifier() + 1) * 5;
                int j = 25 - i;
                float f = damage * (float)j;
                damage = f / 25.0F;
            }

            if (damage <= 0.0F)
            {
                return 0.0F;
            }
            else
            {
                int k = EnchantmentHelper.getEnchantmentModifierDamage(this.getArmorInventoryList(), source);

                if (k > 0)
                {
                    damage = CombatRules.func_188401_b(damage, (float)k);
                }

                return damage;
            }
        }
    }

    /**
     * Deals damage to the entity. If its a EntityPlayer then will take damage from the armor first and then health
     * second with the reduced value. Args: damageAmount
     */
    protected void damageEntity(DamageSource damageSrc, float damageAmount)
    {
        if (!this.isEntityInvulnerable(damageSrc))
        {
            damageAmount = this.applyArmorCalculations(damageSrc, damageAmount);
            damageAmount = this.applyPotionDamageCalculations(damageSrc, damageAmount);
            float f = damageAmount;
            damageAmount = Math.max(damageAmount - this.getAbsorptionAmount(), 0.0F);
            this.setAbsorptionAmount(this.getAbsorptionAmount() - (f - damageAmount));

            if (damageAmount != 0.0F)
            {
                float f1 = this.getHealth();
                this.setHealth(f1 - damageAmount);
                this.getCombatTracker().trackDamage(damageSrc, f1, damageAmount);
                this.setAbsorptionAmount(this.getAbsorptionAmount() - damageAmount);
            }
        }
    }

    /**
     * 1.8.9
     */
    public CombatTracker getCombatTracker()
    {
        return this._combatTracker;
    }

    public EntityLivingBase getAttackingEntity()
    {
        return (EntityLivingBase)(this._combatTracker.func_94550_c() != null ? this._combatTracker.func_94550_c() : (this.attackingPlayer != null ? this.attackingPlayer : (this.entityLivingToAttack != null ? this.entityLivingToAttack : null)));
    }

    public final float getMaxHealth()
    {
        return (float)this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).getAttributeValue();
    }

    /**
     * counts the amount of arrows stuck in the entity. getting hit by arrows increases this, used in rendering
     */
    public final int getArrowCountInEntity()
    {
        return ((Integer)this.dataWatcher.get(ARROW_COUNT_IN_ENTITY)).intValue();
    }

    /**
     * sets the amount of arrows stuck in the entity. used for rendering those
     */
    public final void setArrowCountInEntity(int count)
    {
        this.dataWatcher.set(ARROW_COUNT_IN_ENTITY, Integer.valueOf(count));
    }

    /**
     * Returns an integer indicating the end point of the swing animation, used by {@link #swingProgress} to provide a
     * progress indicator. Takes dig speed enchantments into account.
     */
    private int getArmSwingAnimationEnd()
    {
        return this.isPotionActive(MobEffects.digSpeed) ? 6 - (1 + this.getActivePotionEffect(MobEffects.digSpeed).getAmplifier()) : (this.isPotionActive(MobEffects.digSlowdown) ? 6 + (1 + this.getActivePotionEffect(MobEffects.digSlowdown).getAmplifier()) * 2 : 6);
    }

    public void swingArm(EnumHand hand)
    {
        if (!this.isSwingInProgress || this.swingProgressInt >= this.getArmSwingAnimationEnd() / 2 || this.swingProgressInt < 0)
        {
            this.swingProgressInt = -1;
            this.isSwingInProgress = true;
            this.field_184622_au = hand;

            if (this.worldObj instanceof WorldServer)
            {
                ((WorldServer)this.worldObj).getEntityTracker().sendToAllTrackingEntity(this, new SPacketAnimation(this, hand == EnumHand.MAIN_HAND ? 0 : 3));
            }
        }
    }

    public void handleStatusUpdate(byte id)
    {
        boolean flag = id == 33;

        if (id != 2 && !flag)
        {
            if (id == 3)
            {
                SoundEvent soundevent1 = this.getDeathSound();

                if (soundevent1 != null)
                {
                    this.playSound(soundevent1, this.getSoundVolume(), (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
                }

                this.setHealth(0.0F);
                this.onDeath(DamageSource.generic);
            }
            else if (id == 30)
            {
                this.playSound(SoundEvents.item_shield_break, 0.8F, 0.8F + this.worldObj.rand.nextFloat() * 0.4F);
            }
            else if (id == 29)
            {
                this.playSound(SoundEvents.item_shield_block, 1.0F, 0.8F + this.worldObj.rand.nextFloat() * 0.4F);
            }
            else
            {
                super.handleStatusUpdate(id);
            }
        }
        else
        {
            this.limbSwingAmount = 1.5F;
            this.hurtResistantTime = this.maxHurtResistantTime;
            this.hurtTime = this.maxHurtTime = 10;
            this.attackedAtYaw = 0.0F;

            if (flag)
            {
                this.playSound(SoundEvents.enchant_thorns_hit, this.getSoundVolume(), (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
            }

            SoundEvent soundevent = this.getHurtSound();

            if (soundevent != null)
            {
                this.playSound(soundevent, this.getSoundVolume(), (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
            }

            this.attackEntityFrom(DamageSource.generic, 0.0F);
        }
    }

    /**
     * sets the dead flag. Used when you fall off the bottom of the world.
     */
    protected void kill()
    {
        this.attackEntityFrom(DamageSource.outOfWorld, 4.0F);
    }

    /**
     * Updates the arm swing progress counters and animation progress
     */
    protected void updateArmSwingProgress()
    {
        int i = this.getArmSwingAnimationEnd();

        if (this.isSwingInProgress)
        {
            ++this.swingProgressInt;

            if (this.swingProgressInt >= i)
            {
                this.swingProgressInt = 0;
                this.isSwingInProgress = false;
            }
        }
        else
        {
            this.swingProgressInt = 0;
        }

        this.swingProgress = (float)this.swingProgressInt / (float)i;
    }

    public IAttributeInstance getEntityAttribute(IAttribute attribute)
    {
        return this.getAttributeMap().getAttributeInstance(attribute);
    }

    public AbstractAttributeMap getAttributeMap()
    {
        if (this.attributeMap == null)
        {
            this.attributeMap = new AttributeMap();
        }

        return this.attributeMap;
    }

    /**
     * Get this Entity's EnumCreatureAttribute
     */
    public EnumCreatureAttribute getCreatureAttribute()
    {
        return EnumCreatureAttribute.UNDEFINED;
    }

    public ItemStack getHeldItemMainhand()
    {
        return this.getItemStackFromSlot(EntityEquipmentSlot.MAINHAND);
    }

    public ItemStack getHeldItemOffhand()
    {
        return this.getItemStackFromSlot(EntityEquipmentSlot.OFFHAND);
    }

    public ItemStack getHeldItem(EnumHand hand)
    {
        if (hand == EnumHand.MAIN_HAND)
        {
            return this.getItemStackFromSlot(EntityEquipmentSlot.MAINHAND);
        }
        else if (hand == EnumHand.OFF_HAND)
        {
            return this.getItemStackFromSlot(EntityEquipmentSlot.OFFHAND);
        }
        else
        {
            throw new IllegalArgumentException("Invalid hand " + hand);
        }
    }

    public void setHeldItem(EnumHand hand, ItemStack stack)
    {
        if (hand == EnumHand.MAIN_HAND)
        {
            this.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, stack);
        }
        else
        {
            if (hand != EnumHand.OFF_HAND)
            {
                throw new IllegalArgumentException("Invalid hand " + hand);
            }

            this.setItemStackToSlot(EntityEquipmentSlot.OFFHAND, stack);
        }
    }

    public abstract Iterable<ItemStack> getArmorInventoryList();

    public abstract ItemStack getItemStackFromSlot(EntityEquipmentSlot slotIn);

    public abstract void setItemStackToSlot(EntityEquipmentSlot slotIn, ItemStack stack);

    /**
     * Set sprinting switch for Entity.
     */
    public void setSprinting(boolean sprinting)
    {
        super.setSprinting(sprinting);
        IAttributeInstance iattributeinstance = this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED);

        if (iattributeinstance.getModifier(sprintingSpeedBoostModifierUUID) != null)
        {
            iattributeinstance.removeModifier(sprintingSpeedBoostModifier);
        }

        if (sprinting)
        {
            iattributeinstance.applyModifier(sprintingSpeedBoostModifier);
        }
    }

    /**
     * Returns the volume for the sounds this mob makes.
     */
    protected float getSoundVolume()
    {
        return 1.0F;
    }

    /**
     * Gets the pitch of living sounds in living entities.
     */
    protected float getSoundPitch()
    {
        return this.isChild() ? (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.5F : (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F;
    }

    /**
     * Dead and sleeping entities cannot move
     */
    protected boolean isMovementBlocked()
    {
        return this.getHealth() <= 0.0F;
    }

    /**
     * Moves the entity to a position out of the way of its mount.
     */
    public void dismountEntity(Entity entityIn)
    {
        if (!(entityIn instanceof EntityBoat) && !(entityIn instanceof EntityHorse))
        {
            double d1 = entityIn.posX;
            double d13 = entityIn.getEntityBoundingBox().minY + (double)entityIn.height;
            double d14 = entityIn.posZ;
            EnumFacing enumfacing1 = entityIn.func_184172_bi();
            EnumFacing enumfacing = enumfacing1.rotateY();
            int[][] aint1 = new int[][] {{0, 1}, {0, -1}, { -1, 1}, { -1, -1}, {1, 1}, {1, -1}, { -1, 0}, {1, 0}, {0, 1}};
            double d5 = Math.floor(this.posX) + 0.5D;
            double d6 = Math.floor(this.posZ) + 0.5D;
            double d7 = this.getEntityBoundingBox().maxX - this.getEntityBoundingBox().minX;
            double d8 = this.getEntityBoundingBox().maxZ - this.getEntityBoundingBox().minZ;
            AxisAlignedBB axisalignedbb = new AxisAlignedBB(d5 - d7 / 2.0D, this.getEntityBoundingBox().minY, d6 - d8 / 2.0D, d5 + d7 / 2.0D, this.getEntityBoundingBox().maxY, d6 + d8 / 2.0D);

            for (int[] aint : aint1)
            {
                double d9 = (double)(enumfacing1.getFrontOffsetX() * aint[0] + enumfacing.getFrontOffsetX() * aint[1]);
                double d10 = (double)(enumfacing1.getFrontOffsetZ() * aint[0] + enumfacing.getFrontOffsetZ() * aint[1]);
                double d11 = d5 + d9;
                double d12 = d6 + d10;
                AxisAlignedBB axisalignedbb1 = axisalignedbb.offset(d9, 1.0D, d10);

                if (!this.worldObj.func_184143_b(axisalignedbb1))
                {
                    if (this.worldObj.getBlockState(new BlockPos(d11, this.posY, d12)).func_185896_q())
                    {
                        this.setPositionAndUpdate(d11, this.posY + 1.0D, d12);
                        return;
                    }

                    BlockPos blockpos = new BlockPos(d11, this.posY - 1.0D, d12);

                    if (this.worldObj.getBlockState(blockpos).func_185896_q() || this.worldObj.getBlockState(blockpos).getMaterial() == Material.water)
                    {
                        d1 = d11;
                        d13 = this.posY + 1.0D;
                        d14 = d12;
                    }
                }
                else if (!this.worldObj.func_184143_b(axisalignedbb1.offset(0.0D, 1.0D, 0.0D)) && this.worldObj.getBlockState(new BlockPos(d11, this.posY + 1.0D, d12)).func_185896_q())
                {
                    d1 = d11;
                    d13 = this.posY + 2.0D;
                    d14 = d12;
                }
            }

            this.setPositionAndUpdate(d1, d13, d14);
        }
        else
        {
            double d0 = (double)(this.width / 2.0F + entityIn.width / 2.0F) + 0.4D;
            float f;

            if (entityIn instanceof EntityBoat)
            {
                f = 0.0F;
            }
            else
            {
                f = ((float)Math.PI / 2F) * (float)(this.getPrimaryHand() == EnumHandSide.RIGHT ? -1 : 1);
            }

            float f1 = -MathHelper.sin(-this.rotationYaw * 0.017453292F - (float)Math.PI + f);
            float f2 = -MathHelper.cos(-this.rotationYaw * 0.017453292F - (float)Math.PI + f);
            double d2 = Math.abs(f1) > Math.abs(f2) ? d0 / (double)Math.abs(f1) : d0 / (double)Math.abs(f2);
            double d3 = this.posX + (double)f1 * d2;
            double d4 = this.posZ + (double)f2 * d2;
            this.setPosition(d3, entityIn.posY + (double)entityIn.height + 0.001D, d4);

            if (this.worldObj.func_184143_b(this.getEntityBoundingBox()))
            {
                this.setPosition(d3, entityIn.posY + (double)entityIn.height + 1.001D, d4);

                if (this.worldObj.func_184143_b(this.getEntityBoundingBox()))
                {
                    this.setPosition(entityIn.posX, entityIn.posY + (double)this.height + 0.001D, entityIn.posZ);
                }
            }
        }
    }

    public boolean getAlwaysRenderNameTagForRender()
    {
        return this.getAlwaysRenderNameTag();
    }

    protected float getJumpUpwardsMotion()
    {
        return 0.42F;
    }

    /**
     * Causes this entity to do an upwards motion (jumping).
     */
    protected void jump()
    {
        this.motionY = (double)this.getJumpUpwardsMotion();

        if (this.isPotionActive(MobEffects.jump))
        {
            this.motionY += (double)((float)(this.getActivePotionEffect(MobEffects.jump).getAmplifier() + 1) * 0.1F);
        }

        if (this.isSprinting())
        {
            float f = this.rotationYaw * 0.017453292F;
            this.motionX -= (double)(MathHelper.sin(f) * 0.2F);
            this.motionZ += (double)(MathHelper.cos(f) * 0.2F);
        }

        this.isAirBorne = true;
    }

    /**
     * main AI tick function, replaces updateEntityActionState
     */
    protected void updateAITick()
    {
        this.motionY += 0.03999999910593033D;
    }

    protected void handleJumpLava()
    {
        this.motionY += 0.03999999910593033D;
    }

    /**
     * Moves the entity based on the specified heading.  Args: strafe, forward
     */
    public void moveEntityWithHeading(float strafe, float forward)
    {
        if (this.isServerWorld() || this.func_184186_bw())
        {
            if (!this.isInWater() || this instanceof EntityPlayer && ((EntityPlayer)this).capabilities.isFlying)
            {
                if (!this.isInLava() || this instanceof EntityPlayer && ((EntityPlayer)this).capabilities.isFlying)
                {
                    if (this.func_184613_cA())
                    {
                        if (this.motionY > -0.5D)
                        {
                            this.fallDistance = 1.0F;
                        }

                        Vec3d vec3d = this.getLookVec();
                        float f = this.rotationPitch * 0.017453292F;
                        double d6 = Math.sqrt(vec3d.xCoord * vec3d.xCoord + vec3d.zCoord * vec3d.zCoord);
                        double d8 = Math.sqrt(this.motionX * this.motionX + this.motionZ * this.motionZ);
                        double d1 = vec3d.lengthVector();
                        float f4 = MathHelper.cos(f);
                        f4 = (float)((double)f4 * (double)f4 * Math.min(1.0D, d1 / 0.4D));
                        this.motionY += -0.08D + (double)f4 * 0.06D;

                        if (this.motionY < 0.0D && d6 > 0.0D)
                        {
                            double d2 = this.motionY * -0.1D * (double)f4;
                            this.motionY += d2;
                            this.motionX += vec3d.xCoord * d2 / d6;
                            this.motionZ += vec3d.zCoord * d2 / d6;
                        }

                        if (f < 0.0F)
                        {
                            double d9 = d8 * (double)(-MathHelper.sin(f)) * 0.04D;
                            this.motionY += d9 * 3.2D;
                            this.motionX -= vec3d.xCoord * d9 / d6;
                            this.motionZ -= vec3d.zCoord * d9 / d6;
                        }

                        if (d6 > 0.0D)
                        {
                            this.motionX += (vec3d.xCoord / d6 * d8 - this.motionX) * 0.1D;
                            this.motionZ += (vec3d.zCoord / d6 * d8 - this.motionZ) * 0.1D;
                        }

                        this.motionX *= 0.9900000095367432D;
                        this.motionY *= 0.9800000190734863D;
                        this.motionZ *= 0.9900000095367432D;
                        this.moveEntity(this.motionX, this.motionY, this.motionZ);

                        if (this.isCollidedHorizontally && !this.worldObj.isRemote)
                        {
                            double d10 = Math.sqrt(this.motionX * this.motionX + this.motionZ * this.motionZ);
                            double d3 = d8 - d10;
                            float f5 = (float)(d3 * 10.0D - 3.0D);

                            if (f5 > 0.0F)
                            {
                                this.playSound(this.getFallSound((int)f5), 1.0F, 1.0F);
                                this.attackEntityFrom(DamageSource.flyIntoWall, f5);
                            }
                        }

                        if (this.onGround && !this.worldObj.isRemote)
                        {
                            this.setFlag(7, false);
                        }
                    }
                    else
                    {
                        float f6 = 0.91F;
                        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain(this.posX, this.getEntityBoundingBox().minY - 1.0D, this.posZ);

                        if (this.onGround)
                        {
                            f6 = this.worldObj.getBlockState(blockpos$pooledmutableblockpos).getBlock().slipperiness * 0.91F;
                        }

                        float f7 = 0.16277136F / (f6 * f6 * f6);
                        float f8;

                        if (this.onGround)
                        {
                            f8 = this.getAIMoveSpeed() * f7;
                        }
                        else
                        {
                            f8 = this.jumpMovementFactor;
                        }

                        this.moveFlying(strafe, forward, f8);
                        f6 = 0.91F;

                        if (this.onGround)
                        {
                            f6 = this.worldObj.getBlockState(blockpos$pooledmutableblockpos.set(this.posX, this.getEntityBoundingBox().minY - 1.0D, this.posZ)).getBlock().slipperiness * 0.91F;
                        }

                        if (this.isOnLadder())
                        {
                            float f9 = 0.15F;
                            this.motionX = MathHelper.clamp_double(this.motionX, (double)(-f9), (double)f9);
                            this.motionZ = MathHelper.clamp_double(this.motionZ, (double)(-f9), (double)f9);
                            this.fallDistance = 0.0F;

                            if (this.motionY < -0.15D)
                            {
                                this.motionY = -0.15D;
                            }

                            boolean flag = this.isSneaking() && this instanceof EntityPlayer;

                            if (flag && this.motionY < 0.0D)
                            {
                                this.motionY = 0.0D;
                            }
                        }

                        this.moveEntity(this.motionX, this.motionY, this.motionZ);

                        if (this.isCollidedHorizontally && this.isOnLadder())
                        {
                            this.motionY = 0.2D;
                        }

                        if (this.isPotionActive(MobEffects.levitation))
                        {
                            this.motionY += (0.05D * (double)(this.getActivePotionEffect(MobEffects.levitation).getAmplifier() + 1) - this.motionY) * 0.2D;
                        }
                        else
                        {
                            blockpos$pooledmutableblockpos.set(this.posX, 0.0D, this.posZ);

                            if (this.worldObj.isRemote && (!this.worldObj.isBlockLoaded(blockpos$pooledmutableblockpos) || !this.worldObj.getChunkFromBlockCoords(blockpos$pooledmutableblockpos).isLoaded()))
                            {
                                if (this.posY > 0.0D)
                                {
                                    this.motionY = -0.1D;
                                }
                                else
                                {
                                    this.motionY = 0.0D;
                                }
                            }
                            else
                            {
                                this.motionY -= 0.08D;
                            }
                        }

                        this.motionY *= 0.9800000190734863D;
                        this.motionX *= (double)f6;
                        this.motionZ *= (double)f6;
                        blockpos$pooledmutableblockpos.release();
                    }
                }
                else
                {
                    double d4 = this.posY;
                    this.moveFlying(strafe, forward, 0.02F);
                    this.moveEntity(this.motionX, this.motionY, this.motionZ);
                    this.motionX *= 0.5D;
                    this.motionY *= 0.5D;
                    this.motionZ *= 0.5D;
                    this.motionY -= 0.02D;

                    if (this.isCollidedHorizontally && this.isOffsetPositionInLiquid(this.motionX, this.motionY + 0.6000000238418579D - this.posY + d4, this.motionZ))
                    {
                        this.motionY = 0.30000001192092896D;
                    }
                }
            }
            else
            {
                double d0 = this.posY;
                float f1 = 0.8F;
                float f2 = 0.02F;
                float f3 = (float)EnchantmentHelper.func_185294_d(this);

                if (f3 > 3.0F)
                {
                    f3 = 3.0F;
                }

                if (!this.onGround)
                {
                    f3 *= 0.5F;
                }

                if (f3 > 0.0F)
                {
                    f1 += (0.54600006F - f1) * f3 / 3.0F;
                    f2 += (this.getAIMoveSpeed() - f2) * f3 / 3.0F;
                }

                this.moveFlying(strafe, forward, f2);
                this.moveEntity(this.motionX, this.motionY, this.motionZ);
                this.motionX *= (double)f1;
                this.motionY *= 0.800000011920929D;
                this.motionZ *= (double)f1;
                this.motionY -= 0.02D;

                if (this.isCollidedHorizontally && this.isOffsetPositionInLiquid(this.motionX, this.motionY + 0.6000000238418579D - this.posY + d0, this.motionZ))
                {
                    this.motionY = 0.30000001192092896D;
                }
            }
        }

        this.field_184618_aE = this.limbSwingAmount;
        double d5 = this.posX - this.prevPosX;
        double d7 = this.posZ - this.prevPosZ;
        float f10 = MathHelper.sqrt_double(d5 * d5 + d7 * d7) * 4.0F;

        if (f10 > 1.0F)
        {
            f10 = 1.0F;
        }

        this.limbSwingAmount += (f10 - this.limbSwingAmount) * 0.4F;
        this.field_184619_aG += this.limbSwingAmount;
    }

    /**
     * the movespeed used for the new AI system
     */
    public float getAIMoveSpeed()
    {
        return this.landMovementFactor;
    }

    /**
     * set the movespeed used for the new AI system
     */
    public void setAIMoveSpeed(float speedIn)
    {
        this.landMovementFactor = speedIn;
    }

    public boolean attackEntityAsMob(Entity entityIn)
    {
        this.setLastAttacker(entityIn);
        return false;
    }

    /**
     * Returns whether player is sleeping or not
     */
    public boolean isPlayerSleeping()
    {
        return false;
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();
        this.func_184608_ct();

        if (!this.worldObj.isRemote)
        {
            int i = this.getArrowCountInEntity();

            if (i > 0)
            {
                if (this.arrowHitTimer <= 0)
                {
                    this.arrowHitTimer = 20 * (30 - i);
                }

                --this.arrowHitTimer;

                if (this.arrowHitTimer <= 0)
                {
                    this.setArrowCountInEntity(i - 1);
                }
            }

            for (EntityEquipmentSlot entityequipmentslot : EntityEquipmentSlot.values())
            {
                ItemStack itemstack;

                switch (entityequipmentslot.func_188453_a())
                {
                    case HAND:
                        itemstack = this.field_184630_bs[entityequipmentslot.func_188454_b()];
                        break;

                    case ARMOR:
                        itemstack = this.armorArray[entityequipmentslot.func_188454_b()];
                        break;

                    default:
                        continue;
                }

                ItemStack itemstack1 = this.getItemStackFromSlot(entityequipmentslot);

                if (!ItemStack.areItemStacksEqual(itemstack1, itemstack))
                {
                    ((WorldServer)this.worldObj).getEntityTracker().sendToAllTrackingEntity(this, new SPacketEntityEquipment(this.getEntityId(), entityequipmentslot, itemstack1));

                    if (itemstack != null)
                    {
                        this.getAttributeMap().removeAttributeModifiers(itemstack.getAttributeModifiers(entityequipmentslot));
                    }

                    if (itemstack1 != null)
                    {
                        this.getAttributeMap().applyAttributeModifiers(itemstack1.getAttributeModifiers(entityequipmentslot));
                    }

                    switch (entityequipmentslot.func_188453_a())
                    {
                        case HAND:
                            this.field_184630_bs[entityequipmentslot.func_188454_b()] = itemstack1 == null ? null : itemstack1.copy();
                            break;

                        case ARMOR:
                            this.armorArray[entityequipmentslot.func_188454_b()] = itemstack1 == null ? null : itemstack1.copy();
                    }
                }
            }

            if (this.ticksExisted % 20 == 0)
            {
                this.getCombatTracker().reset();
            }

            if (!this.glowing)
            {
                boolean flag = this.isPotionActive(MobEffects.glowing);

                if (this.getFlag(6) != flag)
                {
                    this.setFlag(6, flag);
                }
            }
        }

        this.onLivingUpdate();
        double d0 = this.posX - this.prevPosX;
        double d1 = this.posZ - this.prevPosZ;
        float f1 = (float)(d0 * d0 + d1 * d1);
        float f2 = this.renderYawOffset;
        float f3 = 0.0F;
        this.prevOnGroundSpeedFactor = this.onGroundSpeedFactor;
        float f = 0.0F;

        if (f1 > 0.0025000002F)
        {
            f = 1.0F;
            f3 = (float)Math.sqrt((double)f1) * 3.0F;
            f2 = (float)MathHelper.atan2(d1, d0) * (180F / (float)Math.PI) - 90.0F;
        }

        if (this.swingProgress > 0.0F)
        {
            f2 = this.rotationYaw;
        }

        if (!this.onGround)
        {
            f = 0.0F;
        }

        this.onGroundSpeedFactor += (f - this.onGroundSpeedFactor) * 0.3F;
        this.worldObj.theProfiler.startSection("headTurn");
        f3 = this.updateDistance(f2, f3);
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("rangeChecks");

        while (this.rotationYaw - this.prevRotationYaw < -180.0F)
        {
            this.prevRotationYaw -= 360.0F;
        }

        while (this.rotationYaw - this.prevRotationYaw >= 180.0F)
        {
            this.prevRotationYaw += 360.0F;
        }

        while (this.renderYawOffset - this.prevRenderYawOffset < -180.0F)
        {
            this.prevRenderYawOffset -= 360.0F;
        }

        while (this.renderYawOffset - this.prevRenderYawOffset >= 180.0F)
        {
            this.prevRenderYawOffset += 360.0F;
        }

        while (this.rotationPitch - this.prevRotationPitch < -180.0F)
        {
            this.prevRotationPitch -= 360.0F;
        }

        while (this.rotationPitch - this.prevRotationPitch >= 180.0F)
        {
            this.prevRotationPitch += 360.0F;
        }

        while (this.rotationYawHead - this.prevRotationYawHead < -180.0F)
        {
            this.prevRotationYawHead -= 360.0F;
        }

        while (this.rotationYawHead - this.prevRotationYawHead >= 180.0F)
        {
            this.prevRotationYawHead += 360.0F;
        }

        this.worldObj.theProfiler.endSection();
        this.movedDistance += f3;

        if (this.func_184613_cA())
        {
            ++this.field_184629_bo;
        }
        else
        {
            this.field_184629_bo = 0;
        }
    }

    protected float updateDistance(float p_110146_1_, float p_110146_2_)
    {
        float f = MathHelper.wrapAngleTo180_float(p_110146_1_ - this.renderYawOffset);
        this.renderYawOffset += f * 0.3F;
        float f1 = MathHelper.wrapAngleTo180_float(this.rotationYaw - this.renderYawOffset);
        boolean flag = f1 < -90.0F || f1 >= 90.0F;

        if (f1 < -75.0F)
        {
            f1 = -75.0F;
        }

        if (f1 >= 75.0F)
        {
            f1 = 75.0F;
        }

        this.renderYawOffset = this.rotationYaw - f1;

        if (f1 * f1 > 2500.0F)
        {
            this.renderYawOffset += f1 * 0.2F;
        }

        if (flag)
        {
            p_110146_2_ *= -1.0F;
        }

        return p_110146_2_;
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (this.jumpTicks > 0)
        {
            --this.jumpTicks;
        }

        if (this.newPosRotationIncrements > 0 && !this.func_184186_bw())
        {
            double d0 = this.posX + (this.field_184623_bh - this.posX) / (double)this.newPosRotationIncrements;
            double d1 = this.posY + (this.field_184624_bi - this.posY) / (double)this.newPosRotationIncrements;
            double d2 = this.posZ + (this.field_184625_bj - this.posZ) / (double)this.newPosRotationIncrements;
            double d3 = MathHelper.wrapAngleTo180_double(this.field_184626_bk - (double)this.rotationYaw);
            this.rotationYaw = (float)((double)this.rotationYaw + d3 / (double)this.newPosRotationIncrements);
            this.rotationPitch = (float)((double)this.rotationPitch + (this.newPosX - (double)this.rotationPitch) / (double)this.newPosRotationIncrements);
            --this.newPosRotationIncrements;
            this.setPosition(d0, d1, d2);
            this.setRotation(this.rotationYaw, this.rotationPitch);
        }
        else if (!this.isServerWorld())
        {
            this.motionX *= 0.98D;
            this.motionY *= 0.98D;
            this.motionZ *= 0.98D;
        }

        if (Math.abs(this.motionX) < 0.003D)
        {
            this.motionX = 0.0D;
        }

        if (Math.abs(this.motionY) < 0.003D)
        {
            this.motionY = 0.0D;
        }

        if (Math.abs(this.motionZ) < 0.003D)
        {
            this.motionZ = 0.0D;
        }

        this.worldObj.theProfiler.startSection("ai");

        if (this.isMovementBlocked())
        {
            this.isJumping = false;
            this.moveStrafing = 0.0F;
            this.moveForward = 0.0F;
            this.randomYawVelocity = 0.0F;
        }
        else if (this.isServerWorld())
        {
            this.worldObj.theProfiler.startSection("newAi");
            this.updateEntityActionState();
            this.worldObj.theProfiler.endSection();
        }

        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("jump");

        if (this.isJumping)
        {
            if (this.isInWater())
            {
                this.updateAITick();
            }
            else if (this.isInLava())
            {
                this.handleJumpLava();
            }
            else if (this.onGround && this.jumpTicks == 0)
            {
                this.jump();
                this.jumpTicks = 10;
            }
        }
        else
        {
            this.jumpTicks = 0;
        }

        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("travel");
        this.moveStrafing *= 0.98F;
        this.moveForward *= 0.98F;
        this.randomYawVelocity *= 0.9F;
        this.func_184616_r();
        this.moveEntityWithHeading(this.moveStrafing, this.moveForward);
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("push");
        this.collideWithNearbyEntities();
        this.worldObj.theProfiler.endSection();
    }

    private void func_184616_r()
    {
        boolean flag = this.getFlag(7);

        if (flag && !this.onGround && !this.isRiding())
        {
            ItemStack itemstack = this.getItemStackFromSlot(EntityEquipmentSlot.CHEST);

            if (itemstack != null && itemstack.getItem() == Items.elytra && ItemElytra.isBroken(itemstack))
            {
                flag = true;

                if (!this.worldObj.isRemote && (this.field_184629_bo + 1) % 20 == 0)
                {
                    itemstack.damageItem(1, this);
                }
            }
            else
            {
                flag = false;
            }
        }
        else
        {
            flag = false;
        }

        if (!this.worldObj.isRemote)
        {
            this.setFlag(7, flag);
        }
    }

    protected void updateEntityActionState()
    {
    }

    protected void collideWithNearbyEntities()
    {
        List<Entity> list = this.worldObj.getEntitiesInAABBexcluding(this, this.getEntityBoundingBox(), EntitySelectors.<Entity>func_188442_a(this));

        if (!list.isEmpty())
        {
            for (int i = 0; i < list.size(); ++i)
            {
                Entity entity = (Entity)list.get(i);
                this.collideWithEntity(entity);
            }
        }
    }

    protected void collideWithEntity(Entity entityIn)
    {
        entityIn.applyEntityCollision(this);
    }

    public void dismountRidingEntity()
    {
        Entity entity = this.getRidingEntity();
        super.dismountRidingEntity();

        if (entity != null && entity != this.getRidingEntity() && !this.worldObj.isRemote)
        {
            this.dismountEntity(entity);
        }
    }

    /**
     * Handles updating while being ridden by an entity
     */
    public void updateRidden()
    {
        super.updateRidden();
        this.prevOnGroundSpeedFactor = this.onGroundSpeedFactor;
        this.onGroundSpeedFactor = 0.0F;
        this.fallDistance = 0.0F;
    }

    public void setPositionAndRotation2(double x, double y, double z, float yaw, float pitch, int posRotationIncrements, boolean p_180426_10_)
    {
        this.field_184623_bh = x;
        this.field_184624_bi = y;
        this.field_184625_bj = z;
        this.field_184626_bk = (double)yaw;
        this.newPosX = (double)pitch;
        this.newPosRotationIncrements = posRotationIncrements;
    }

    public void setJumping(boolean jumping)
    {
        this.isJumping = jumping;
    }

    /**
     * Called whenever an item is picked up from walking over it. Args: pickedUpEntity, stackSize
     */
    public void onItemPickup(Entity entityIn, int quantity)
    {
        if (!entityIn.isDead && !this.worldObj.isRemote)
        {
            EntityTracker entitytracker = ((WorldServer)this.worldObj).getEntityTracker();

            if (entityIn instanceof EntityItem)
            {
                entitytracker.sendToAllTrackingEntity(entityIn, new SPacketCollectItem(entityIn.getEntityId(), this.getEntityId()));
            }

            if (entityIn instanceof EntityArrow)
            {
                entitytracker.sendToAllTrackingEntity(entityIn, new SPacketCollectItem(entityIn.getEntityId(), this.getEntityId()));
            }

            if (entityIn instanceof EntityXPOrb)
            {
                entitytracker.sendToAllTrackingEntity(entityIn, new SPacketCollectItem(entityIn.getEntityId(), this.getEntityId()));
            }
        }
    }

    /**
     * returns true if the entity provided in the argument can be seen. (Raytrace)
     */
    public boolean canEntityBeSeen(Entity entityIn)
    {
        return this.worldObj.rayTraceBlocks(new Vec3d(this.posX, this.posY + (double)this.getEyeHeight(), this.posZ), new Vec3d(entityIn.posX, entityIn.posY + (double)entityIn.getEyeHeight(), entityIn.posZ), false, true, false) == null;
    }

    /**
     * returns a (normalized) vector of where this entity is looking
     */
    public Vec3d getLookVec()
    {
        return this.getLook(1.0F);
    }

    /**
     * interpolated look vector
     */
    public Vec3d getLook(float partialTicks)
    {
        if (partialTicks == 1.0F)
        {
            return this.getVectorForRotation(this.rotationPitch, this.rotationYawHead);
        }
        else
        {
            float f = this.prevRotationPitch + (this.rotationPitch - this.prevRotationPitch) * partialTicks;
            float f1 = this.prevRotationYawHead + (this.rotationYawHead - this.prevRotationYawHead) * partialTicks;
            return this.getVectorForRotation(f, f1);
        }
    }

    /**
     * Returns where in the swing animation the living entity is (from 0 to 1).  Args: partialTickTime
     */
    public float getSwingProgress(float partialTickTime)
    {
        float f = this.swingProgress - this.prevSwingProgress;

        if (f < 0.0F)
        {
            ++f;
        }

        return this.prevSwingProgress + f * partialTickTime;
    }

    /**
     * Returns whether the entity is in a server world
     */
    public boolean isServerWorld()
    {
        return !this.worldObj.isRemote;
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return !this.isDead;
    }

    /**
     * Returns true if this entity should push and be pushed by other entities when colliding.
     */
    public boolean canBePushed()
    {
        return !this.isDead;
    }

    /**
     * Sets that this entity has been attacked.
     */
    protected void setBeenAttacked()
    {
        this.velocityChanged = this.rand.nextDouble() >= this.getEntityAttribute(SharedMonsterAttributes.KNOCKBACK_RESISTANCE).getAttributeValue();
    }

    public float getRotationYawHead()
    {
        return this.rotationYawHead;
    }

    /**
     * Sets the head's yaw rotation of the entity.
     */
    public void setRotationYawHead(float rotation)
    {
        this.rotationYawHead = rotation;
    }

    /**
     * Set the render yaw offset
     */
    public void setRenderYawOffset(float offset)
    {
        this.renderYawOffset = offset;
    }

    public float getAbsorptionAmount()
    {
        return this.absorptionAmount;
    }

    public void setAbsorptionAmount(float amount)
    {
        if (amount < 0.0F)
        {
            amount = 0.0F;
        }

        this.absorptionAmount = amount;
    }

    /**
     * Sends an ENTER_COMBAT packet to the client
     */
    public void sendEnterCombat()
    {
    }

    /**
     * Sends an END_COMBAT packet to the client
     */
    public void sendEndCombat()
    {
    }

    protected void markPotionsDirty()
    {
        this.potionsNeedUpdate = true;
    }

    public abstract EnumHandSide getPrimaryHand();

    public boolean func_184587_cr()
    {
        return (((Byte)this.dataWatcher.get(field_184621_as)).byteValue() & 1) > 0;
    }

    public EnumHand func_184600_cs()
    {
        return (((Byte)this.dataWatcher.get(field_184621_as)).byteValue() & 2) > 0 ? EnumHand.OFF_HAND : EnumHand.MAIN_HAND;
    }

    protected void func_184608_ct()
    {
        if (this.func_184587_cr())
        {
            ItemStack itemstack = this.getHeldItem(this.func_184600_cs());

            if (itemstack == this.field_184627_bm)
            {
                if (this.getItemInUseCount() <= 25 && this.getItemInUseCount() % 4 == 0)
                {
                    this.func_184584_a(this.field_184627_bm, 5);
                }

                if (--this.field_184628_bn == 0 && !this.worldObj.isRemote)
                {
                    this.onItemUseFinish();
                }
            }
            else
            {
                this.func_184602_cy();
            }
        }
    }

    public void func_184598_c(EnumHand p_184598_1_)
    {
        ItemStack itemstack = this.getHeldItem(p_184598_1_);

        if (itemstack != null && !this.func_184587_cr())
        {
            this.field_184627_bm = itemstack;
            this.field_184628_bn = itemstack.getMaxItemUseDuration();

            if (!this.worldObj.isRemote)
            {
                int i = 1;

                if (p_184598_1_ == EnumHand.OFF_HAND)
                {
                    i |= 2;
                }

                this.dataWatcher.set(field_184621_as, Byte.valueOf((byte)i));
            }
        }
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        super.notifyDataManagerChange(key);

        if (field_184621_as.equals(key) && this.worldObj.isRemote)
        {
            if (this.func_184587_cr() && this.field_184627_bm == null)
            {
                this.field_184627_bm = this.getHeldItem(this.func_184600_cs());

                if (this.field_184627_bm != null)
                {
                    this.field_184628_bn = this.field_184627_bm.getMaxItemUseDuration();
                }
            }
            else if (!this.func_184587_cr() && this.field_184627_bm != null)
            {
                this.field_184627_bm = null;
                this.field_184628_bn = 0;
            }
        }
    }

    protected void func_184584_a(ItemStack p_184584_1_, int p_184584_2_)
    {
        if (p_184584_1_ != null && this.func_184587_cr())
        {
            if (p_184584_1_.getItemUseAction() == EnumAction.DRINK)
            {
                this.playSound(SoundEvents.entity_generic_drink, 0.5F, this.worldObj.rand.nextFloat() * 0.1F + 0.9F);
            }

            if (p_184584_1_.getItemUseAction() == EnumAction.EAT)
            {
                for (int i = 0; i < p_184584_2_; ++i)
                {
                    Vec3d vec3d = new Vec3d(((double)this.rand.nextFloat() - 0.5D) * 0.1D, Math.random() * 0.1D + 0.1D, 0.0D);
                    vec3d = vec3d.rotatePitch(-this.rotationPitch * 0.017453292F);
                    vec3d = vec3d.rotateYaw(-this.rotationYaw * 0.017453292F);
                    double d0 = (double)(-this.rand.nextFloat()) * 0.6D - 0.3D;
                    Vec3d vec3d1 = new Vec3d(((double)this.rand.nextFloat() - 0.5D) * 0.3D, d0, 0.6D);
                    vec3d1 = vec3d1.rotatePitch(-this.rotationPitch * 0.017453292F);
                    vec3d1 = vec3d1.rotateYaw(-this.rotationYaw * 0.017453292F);
                    vec3d1 = vec3d1.addVector(this.posX, this.posY + (double)this.getEyeHeight(), this.posZ);

                    if (p_184584_1_.getHasSubtypes())
                    {
                        this.worldObj.spawnParticle(EnumParticleTypes.ITEM_CRACK, vec3d1.xCoord, vec3d1.yCoord, vec3d1.zCoord, vec3d.xCoord, vec3d.yCoord + 0.05D, vec3d.zCoord, new int[] {Item.getIdFromItem(p_184584_1_.getItem()), p_184584_1_.getMetadata()});
                    }
                    else
                    {
                        this.worldObj.spawnParticle(EnumParticleTypes.ITEM_CRACK, vec3d1.xCoord, vec3d1.yCoord, vec3d1.zCoord, vec3d.xCoord, vec3d.yCoord + 0.05D, vec3d.zCoord, new int[] {Item.getIdFromItem(p_184584_1_.getItem())});
                    }
                }

                this.playSound(SoundEvents.entity_generic_eat, 0.5F + 0.5F * (float)this.rand.nextInt(2), (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
            }
        }
    }

    /**
     * Used for when item use count runs out, ie: eating completed
     */
    protected void onItemUseFinish()
    {
        if (this.field_184627_bm != null && this.func_184587_cr())
        {
            this.func_184584_a(this.field_184627_bm, 16);
            ItemStack itemstack = this.field_184627_bm.onItemUseFinish(this.worldObj, this);

            if (itemstack != null && itemstack.stackSize == 0)
            {
                itemstack = null;
            }

            this.setHeldItem(this.func_184600_cs(), itemstack);
            this.func_184602_cy();
        }
    }

    public ItemStack func_184607_cu()
    {
        return this.field_184627_bm;
    }

    public int getItemInUseCount()
    {
        return this.field_184628_bn;
    }

    public int func_184612_cw()
    {
        return this.func_184587_cr() ? this.field_184627_bm.getMaxItemUseDuration() - this.getItemInUseCount() : 0;
    }

    public void func_184597_cx()
    {
        if (this.field_184627_bm != null)
        {
            this.field_184627_bm.onPlayerStoppedUsing(this.worldObj, this, this.getItemInUseCount());
        }

        this.func_184602_cy();
    }

    public void func_184602_cy()
    {
        if (!this.worldObj.isRemote)
        {
            this.dataWatcher.set(field_184621_as, Byte.valueOf((byte)0));
        }

        this.field_184627_bm = null;
        this.field_184628_bn = 0;
    }

    public boolean func_184585_cz()
    {
        if (this.func_184587_cr() && this.field_184627_bm != null)
        {
            Item item = this.field_184627_bm.getItem();
            return item.getItemUseAction(this.field_184627_bm) != EnumAction.BLOCK ? false : item.getMaxItemUseDuration(this.field_184627_bm) - this.field_184628_bn >= 5;
        }
        else
        {
            return false;
        }
    }

    public boolean func_184613_cA()
    {
        return this.getFlag(7);
    }

    public int func_184599_cB()
    {
        return this.field_184629_bo;
    }

    public boolean func_184595_k(double p_184595_1_, double p_184595_3_, double p_184595_5_)
    {
        double d0 = this.posX;
        double d1 = this.posY;
        double d2 = this.posZ;
        this.posX = p_184595_1_;
        this.posY = p_184595_3_;
        this.posZ = p_184595_5_;
        boolean flag = false;
        BlockPos blockpos = new BlockPos(this);
        World world = this.worldObj;
        Random random = this.getRNG();

        if (world.isBlockLoaded(blockpos))
        {
            boolean flag1 = false;

            while (!flag1 && blockpos.getY() > 0)
            {
                BlockPos blockpos1 = blockpos.down();
                IBlockState iblockstate = world.getBlockState(blockpos1);

                if (iblockstate.getMaterial().blocksMovement())
                {
                    flag1 = true;
                }
                else
                {
                    --this.posY;
                    blockpos = blockpos1;
                }
            }

            if (flag1)
            {
                this.setPositionAndUpdate(this.posX, this.posY, this.posZ);

                if (world.func_184144_a(this, this.getEntityBoundingBox()).isEmpty() && !world.isAnyLiquid(this.getEntityBoundingBox()))
                {
                    flag = true;
                }
            }
        }

        if (!flag)
        {
            this.setPositionAndUpdate(d0, d1, d2);
            return false;
        }
        else
        {
            int i = 128;

            for (int j = 0; j < i; ++j)
            {
                double d6 = (double)j / ((double)i - 1.0D);
                float f = (random.nextFloat() - 0.5F) * 0.2F;
                float f1 = (random.nextFloat() - 0.5F) * 0.2F;
                float f2 = (random.nextFloat() - 0.5F) * 0.2F;
                double d3 = d0 + (this.posX - d0) * d6 + (random.nextDouble() - 0.5D) * (double)this.width * 2.0D;
                double d4 = d1 + (this.posY - d1) * d6 + random.nextDouble() * (double)this.height;
                double d5 = d2 + (this.posZ - d2) * d6 + (random.nextDouble() - 0.5D) * (double)this.width * 2.0D;
                world.spawnParticle(EnumParticleTypes.PORTAL, d3, d4, d5, (double)f, (double)f1, (double)f2, new int[0]);
            }

            if (this instanceof EntityCreature)
            {
                ((EntityCreature)this).getNavigator().clearPathEntity();
            }

            return true;
        }
    }

    public boolean func_184603_cC()
    {
        return true;
    }
}
