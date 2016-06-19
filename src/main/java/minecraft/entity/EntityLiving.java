package net.minecraft.entity;

import com.google.common.collect.Maps;
import java.util.Arrays;
import java.util.Map;
import java.util.Random;
import java.util.UUID;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.ai.EntityAITasks;
import net.minecraft.entity.ai.EntityJumpHelper;
import net.minecraft.entity.ai.EntityLookHelper;
import net.minecraft.entity.ai.EntityMoveHelper;
import net.minecraft.entity.ai.EntitySenses;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.item.EntityBoat;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.monster.EntityGhast;
import net.minecraft.entity.monster.IMob;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.Item;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemBow;
import net.minecraft.item.ItemStack;
import net.minecraft.item.ItemSword;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagFloat;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.network.play.server.SPacketEntityAttach;
import net.minecraft.pathfinding.PathNavigate;
import net.minecraft.pathfinding.PathNavigateGround;
import net.minecraft.pathfinding.PathNodeType;
import net.minecraft.stats.AchievementList;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumHandSide;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;
import net.minecraft.world.storage.loot.LootContext;
import net.minecraft.world.storage.loot.LootTable;

public abstract class EntityLiving extends EntityLivingBase
{
    private static final DataParameter<Byte> AI_FLAGS = EntityDataManager.<Byte>createKey(EntityLiving.class, DataSerializers.BYTE);

    /** Number of ticks since this EntityLiving last produced its sound */
    public int livingSoundTime;

    /** The experience points the Entity gives. */
    protected int experienceValue;
    private EntityLookHelper lookHelper;
    protected EntityMoveHelper moveHelper;

    /** Entity jumping helper */
    protected EntityJumpHelper jumpHelper;
    private EntityBodyHelper bodyHelper;
    protected PathNavigate navigator;

    /** Passive tasks (wandering, look, idle, ...) */
    protected final EntityAITasks tasks;

    /** Fighting tasks (used by monsters, wolves, ocelots) */
    protected final EntityAITasks targetTasks;

    /** The active target the Task system uses for tracking */
    private EntityLivingBase attackTarget;
    private EntitySenses senses;
    private ItemStack[] inventoryHands = new ItemStack[2];

    /** Chances for equipment in hands dropping when this entity dies. */
    protected float[] inventoryHandsDropChances = new float[2];
    private ItemStack[] inventoryArmor = new ItemStack[4];

    /** Chances for armor dropping when this entity dies. */
    protected float[] inventoryArmorDropChances = new float[4];

    /** Whether this entity can pick up items from the ground. */
    private boolean canPickUpLoot;

    /** Whether this entity should NOT despawn. */
    private boolean persistenceRequired;
    private Map<PathNodeType, Float> field_184658_bz = Maps.newEnumMap(PathNodeType.class);
    private ResourceLocation deathLootTable;
    private long deathLootTableSeed;
    private boolean isLeashed;
    private Entity leashedToEntity;
    private NBTTagCompound leashNBTTag;

    public EntityLiving(World worldIn)
    {
        super(worldIn);
        this.tasks = new EntityAITasks(worldIn != null && worldIn.theProfiler != null ? worldIn.theProfiler : null);
        this.targetTasks = new EntityAITasks(worldIn != null && worldIn.theProfiler != null ? worldIn.theProfiler : null);
        this.lookHelper = new EntityLookHelper(this);
        this.moveHelper = new EntityMoveHelper(this);
        this.jumpHelper = new EntityJumpHelper(this);
        this.bodyHelper = this.createBodyHelper();
        this.navigator = this.getNewNavigator(worldIn);
        this.senses = new EntitySenses(this);

        for (int i = 0; i < this.inventoryArmorDropChances.length; ++i)
        {
            this.inventoryArmorDropChances[i] = 0.085F;
        }

        for (int j = 0; j < this.inventoryHandsDropChances.length; ++j)
        {
            this.inventoryHandsDropChances[j] = 0.085F;
        }

        if (worldIn != null && !worldIn.isRemote)
        {
            this.initEntityAI();
        }
    }

    protected void initEntityAI()
    {
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getAttributeMap().registerAttribute(SharedMonsterAttributes.FOLLOW_RANGE).setBaseValue(16.0D);
    }

    /**
     * Returns new PathNavigateGround instance
     */
    protected PathNavigate getNewNavigator(World worldIn)
    {
        return new PathNavigateGround(this, worldIn);
    }

    public float func_184643_a(PathNodeType p_184643_1_)
    {
        return this.field_184658_bz.containsKey(p_184643_1_) ? ((Float)this.field_184658_bz.get(p_184643_1_)).floatValue() : p_184643_1_.func_186289_a();
    }

    public void func_184644_a(PathNodeType p_184644_1_, float p_184644_2_)
    {
        this.field_184658_bz.put(p_184644_1_, Float.valueOf(p_184644_2_));
    }

    protected EntityBodyHelper createBodyHelper()
    {
        return new EntityBodyHelper(this);
    }

    public EntityLookHelper getLookHelper()
    {
        return this.lookHelper;
    }

    public EntityMoveHelper getMoveHelper()
    {
        return this.moveHelper;
    }

    public EntityJumpHelper getJumpHelper()
    {
        return this.jumpHelper;
    }

    public PathNavigate getNavigator()
    {
        return this.navigator;
    }

    /**
     * returns the EntitySenses Object for the EntityLiving
     */
    public EntitySenses getEntitySenses()
    {
        return this.senses;
    }

    /**
     * Gets the active target the Task system uses for tracking
     */
    public EntityLivingBase getAttackTarget()
    {
        return this.attackTarget;
    }

    /**
     * Sets the active target the Task system uses for tracking
     */
    public void setAttackTarget(EntityLivingBase entitylivingbaseIn)
    {
        this.attackTarget = entitylivingbaseIn;
    }

    /**
     * Returns true if this entity can attack entities of the specified class.
     */
    public boolean canAttackClass(Class <? extends EntityLivingBase > cls)
    {
        return cls != EntityGhast.class;
    }

    /**
     * This function applies the benefits of growing back wool and faster growing up to the acting entity. (This
     * function is used in the AIEatGrass)
     */
    public void eatGrassBonus()
    {
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(AI_FLAGS, Byte.valueOf((byte)0));
    }

    /**
     * Get number of ticks, at least during which the living entity will be silent.
     */
    public int getTalkInterval()
    {
        return 80;
    }

    /**
     * Plays living's sound at its position
     */
    public void playLivingSound()
    {
        SoundEvent soundevent = this.getAmbientSound();

        if (soundevent != null)
        {
            this.playSound(soundevent, this.getSoundVolume(), this.getSoundPitch());
        }
    }

    /**
     * Gets called every tick from main Entity class
     */
    public void onEntityUpdate()
    {
        super.onEntityUpdate();
        this.worldObj.theProfiler.startSection("mobBaseTick");

        if (this.isEntityAlive() && this.rand.nextInt(1000) < this.livingSoundTime++)
        {
            this.applyEntityAI();
            this.playLivingSound();
        }

        this.worldObj.theProfiler.endSection();
    }

    protected void func_184581_c(DamageSource p_184581_1_)
    {
        this.applyEntityAI();
        super.func_184581_c(p_184581_1_);
    }

    private void applyEntityAI()
    {
        this.livingSoundTime = -this.getTalkInterval();
    }

    /**
     * Get the experience points the entity currently has.
     */
    protected int getExperiencePoints(EntityPlayer player)
    {
        if (this.experienceValue > 0)
        {
            int i = this.experienceValue;

            for (int j = 0; j < this.inventoryArmor.length; ++j)
            {
                if (this.inventoryArmor[j] != null && this.inventoryArmorDropChances[j] <= 1.0F)
                {
                    i += 1 + this.rand.nextInt(3);
                }
            }

            for (int k = 0; k < this.inventoryHands.length; ++k)
            {
                if (this.inventoryHands[k] != null && this.inventoryHandsDropChances[k] <= 1.0F)
                {
                    i += 1 + this.rand.nextInt(3);
                }
            }

            return i;
        }
        else
        {
            return this.experienceValue;
        }
    }

    /**
     * Spawns an explosion particle around the Entity's location
     */
    public void spawnExplosionParticle()
    {
        if (this.worldObj.isRemote)
        {
            for (int i = 0; i < 20; ++i)
            {
                double d0 = this.rand.nextGaussian() * 0.02D;
                double d1 = this.rand.nextGaussian() * 0.02D;
                double d2 = this.rand.nextGaussian() * 0.02D;
                double d3 = 10.0D;
                this.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_NORMAL, this.posX + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width - d0 * d3, this.posY + (double)(this.rand.nextFloat() * this.height) - d1 * d3, this.posZ + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width - d2 * d3, d0, d1, d2, new int[0]);
            }
        }
        else
        {
            this.worldObj.setEntityState(this, (byte)20);
        }
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 20)
        {
            this.spawnExplosionParticle();
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (!this.worldObj.isRemote)
        {
            this.updateLeashedState();

            if (this.ticksExisted % 5 == 0)
            {
                boolean flag = !(this.getControllingPassenger() instanceof EntityLiving);
                boolean flag1 = !(this.getRidingEntity() instanceof EntityBoat);
                this.tasks.func_188527_a(5, flag && flag1);
                this.tasks.func_188527_a(2, flag);
            }
        }
    }

    protected float updateDistance(float p_110146_1_, float p_110146_2_)
    {
        this.bodyHelper.updateRenderAngles();
        return p_110146_2_;
    }

    protected SoundEvent getAmbientSound()
    {
        return null;
    }

    protected Item getDropItem()
    {
        return null;
    }

    /**
     * Drop 0-2 items of this living's type
     */
    protected void dropFewItems(boolean wasRecentlyHit, int lootingModifier)
    {
        Item item = this.getDropItem();

        if (item != null)
        {
            int i = this.rand.nextInt(3);

            if (lootingModifier > 0)
            {
                i += this.rand.nextInt(lootingModifier + 1);
            }

            for (int j = 0; j < i; ++j)
            {
                this.dropItem(item, 1);
            }
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setBoolean("CanPickUpLoot", this.canPickUpLoot());
        tagCompound.setBoolean("PersistenceRequired", this.persistenceRequired);
        NBTTagList nbttaglist = new NBTTagList();

        for (int i = 0; i < this.inventoryArmor.length; ++i)
        {
            NBTTagCompound nbttagcompound = new NBTTagCompound();

            if (this.inventoryArmor[i] != null)
            {
                this.inventoryArmor[i].writeToNBT(nbttagcompound);
            }

            nbttaglist.appendTag(nbttagcompound);
        }

        tagCompound.setTag("ArmorItems", nbttaglist);
        NBTTagList nbttaglist1 = new NBTTagList();

        for (int k = 0; k < this.inventoryHands.length; ++k)
        {
            NBTTagCompound nbttagcompound1 = new NBTTagCompound();

            if (this.inventoryHands[k] != null)
            {
                this.inventoryHands[k].writeToNBT(nbttagcompound1);
            }

            nbttaglist1.appendTag(nbttagcompound1);
        }

        tagCompound.setTag("HandItems", nbttaglist1);
        NBTTagList nbttaglist2 = new NBTTagList();

        for (int l = 0; l < this.inventoryArmorDropChances.length; ++l)
        {
            nbttaglist2.appendTag(new NBTTagFloat(this.inventoryArmorDropChances[l]));
        }

        tagCompound.setTag("ArmorDropChances", nbttaglist2);
        NBTTagList nbttaglist3 = new NBTTagList();

        for (int j = 0; j < this.inventoryHandsDropChances.length; ++j)
        {
            nbttaglist3.appendTag(new NBTTagFloat(this.inventoryHandsDropChances[j]));
        }

        tagCompound.setTag("HandDropChances", nbttaglist3);
        tagCompound.setBoolean("Leashed", this.isLeashed);

        if (this.leashedToEntity != null)
        {
            NBTTagCompound nbttagcompound2 = new NBTTagCompound();

            if (this.leashedToEntity instanceof EntityLivingBase)
            {
                UUID uuid = this.leashedToEntity.getUniqueID();
                nbttagcompound2.setUniqueId("UUID", uuid);
            }
            else if (this.leashedToEntity instanceof EntityHanging)
            {
                BlockPos blockpos = ((EntityHanging)this.leashedToEntity).getHangingPosition();
                nbttagcompound2.setInteger("X", blockpos.getX());
                nbttagcompound2.setInteger("Y", blockpos.getY());
                nbttagcompound2.setInteger("Z", blockpos.getZ());
            }

            tagCompound.setTag("Leash", nbttagcompound2);
        }

        tagCompound.setBoolean("LeftHanded", this.isLeftHanded());

        if (this.deathLootTable != null)
        {
            tagCompound.setString("DeathLootTable", this.deathLootTable.toString());

            if (this.deathLootTableSeed != 0L)
            {
                tagCompound.setLong("DeathLootTableSeed", this.deathLootTableSeed);
            }
        }

        if (this.isAIDisabled())
        {
            tagCompound.setBoolean("NoAI", this.isAIDisabled());
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.hasKey("CanPickUpLoot", 1))
        {
            this.setCanPickUpLoot(tagCompund.getBoolean("CanPickUpLoot"));
        }

        this.persistenceRequired = tagCompund.getBoolean("PersistenceRequired");

        if (tagCompund.hasKey("ArmorItems", 9))
        {
            NBTTagList nbttaglist = tagCompund.getTagList("ArmorItems", 10);

            for (int i = 0; i < this.inventoryArmor.length; ++i)
            {
                this.inventoryArmor[i] = ItemStack.loadItemStackFromNBT(nbttaglist.getCompoundTagAt(i));
            }
        }

        if (tagCompund.hasKey("HandItems", 9))
        {
            NBTTagList nbttaglist1 = tagCompund.getTagList("HandItems", 10);

            for (int j = 0; j < this.inventoryHands.length; ++j)
            {
                this.inventoryHands[j] = ItemStack.loadItemStackFromNBT(nbttaglist1.getCompoundTagAt(j));
            }
        }

        if (tagCompund.hasKey("ArmorDropChances", 9))
        {
            NBTTagList nbttaglist2 = tagCompund.getTagList("ArmorDropChances", 5);

            for (int k = 0; k < nbttaglist2.tagCount(); ++k)
            {
                this.inventoryArmorDropChances[k] = nbttaglist2.getFloatAt(k);
            }
        }

        if (tagCompund.hasKey("HandDropChances", 9))
        {
            NBTTagList nbttaglist3 = tagCompund.getTagList("HandDropChances", 5);

            for (int l = 0; l < nbttaglist3.tagCount(); ++l)
            {
                this.inventoryHandsDropChances[l] = nbttaglist3.getFloatAt(l);
            }
        }

        this.isLeashed = tagCompund.getBoolean("Leashed");

        if (this.isLeashed && tagCompund.hasKey("Leash", 10))
        {
            this.leashNBTTag = tagCompund.getCompoundTag("Leash");
        }

        this.func_184641_n(tagCompund.getBoolean("LeftHanded"));

        if (tagCompund.hasKey("DeathLootTable", 8))
        {
            this.deathLootTable = new ResourceLocation(tagCompund.getString("DeathLootTable"));
            this.deathLootTableSeed = tagCompund.getLong("DeathLootTableSeed");
        }

        this.setNoAI(tagCompund.getBoolean("NoAI"));
    }

    protected ResourceLocation func_184647_J()
    {
        return null;
    }

    protected void func_184610_a(boolean p_184610_1_, int p_184610_2_, DamageSource source)
    {
        ResourceLocation resourcelocation = this.deathLootTable;

        if (resourcelocation == null)
        {
            resourcelocation = this.func_184647_J();
        }

        if (resourcelocation != null)
        {
            LootTable loottable = this.worldObj.getLootTableManager().func_186521_a(resourcelocation);
            this.deathLootTable = null;
            LootContext.Builder lootcontext$builder = (new LootContext.Builder((WorldServer)this.worldObj)).withLootedEntity(this).withDamageSource(source);

            if (p_184610_1_ && this.attackingPlayer != null)
            {
                lootcontext$builder = lootcontext$builder.withPlayer(this.attackingPlayer).withLuck(this.attackingPlayer.getLuck());
            }

            for (ItemStack itemstack : loottable.func_186462_a(this.deathLootTableSeed == 0L ? this.rand : new Random(this.deathLootTableSeed), lootcontext$builder.build()))
            {
                this.entityDropItem(itemstack, 0.0F);
            }

            this.dropEquipment(p_184610_1_, p_184610_2_);
        }
        else
        {
            super.func_184610_a(p_184610_1_, p_184610_2_, source);
        }
    }

    public void setMoveForward(float p_70657_1_)
    {
        this.moveForward = p_70657_1_;
    }

    public void func_184646_p(float p_184646_1_)
    {
        this.moveStrafing = p_184646_1_;
    }

    /**
     * set the movespeed used for the new AI system
     */
    public void setAIMoveSpeed(float speedIn)
    {
        super.setAIMoveSpeed(speedIn);
        this.setMoveForward(speedIn);
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        super.onLivingUpdate();
        this.worldObj.theProfiler.startSection("looting");

        if (!this.worldObj.isRemote && this.canPickUpLoot() && !this.dead && this.worldObj.getGameRules().getBoolean("mobGriefing"))
        {
            for (EntityItem entityitem : this.worldObj.getEntitiesWithinAABB(EntityItem.class, this.getEntityBoundingBox().expand(1.0D, 0.0D, 1.0D)))
            {
                if (!entityitem.isDead && entityitem.getEntityItem() != null && !entityitem.cannotPickup())
                {
                    this.updateEquipmentIfNeeded(entityitem);
                }
            }
        }

        this.worldObj.theProfiler.endSection();
    }

    /**
     * Tests if this entity should pickup a weapon or an armor. Entity drops current weapon or armor if the new one is
     * better.
     */
    protected void updateEquipmentIfNeeded(EntityItem itemEntity)
    {
        ItemStack itemstack = itemEntity.getEntityItem();
        EntityEquipmentSlot entityequipmentslot = func_184640_d(itemstack);
        boolean flag = true;
        ItemStack itemstack1 = this.getItemStackFromSlot(entityequipmentslot);

        if (itemstack1 != null)
        {
            if (entityequipmentslot.func_188453_a() == EntityEquipmentSlot.Type.HAND)
            {
                if (itemstack.getItem() instanceof ItemSword && !(itemstack1.getItem() instanceof ItemSword))
                {
                    flag = true;
                }
                else if (itemstack.getItem() instanceof ItemSword && itemstack1.getItem() instanceof ItemSword)
                {
                    ItemSword itemsword = (ItemSword)itemstack.getItem();
                    ItemSword itemsword1 = (ItemSword)itemstack1.getItem();

                    if (itemsword.getDamageVsEntity() == itemsword1.getDamageVsEntity())
                    {
                        flag = itemstack.getMetadata() > itemstack1.getMetadata() || itemstack.hasTagCompound() && !itemstack1.hasTagCompound();
                    }
                    else
                    {
                        flag = itemsword.getDamageVsEntity() > itemsword1.getDamageVsEntity();
                    }
                }
                else if (itemstack.getItem() instanceof ItemBow && itemstack1.getItem() instanceof ItemBow)
                {
                    flag = itemstack.hasTagCompound() && !itemstack1.hasTagCompound();
                }
                else
                {
                    flag = false;
                }
            }
            else if (itemstack.getItem() instanceof ItemArmor && !(itemstack1.getItem() instanceof ItemArmor))
            {
                flag = true;
            }
            else if (itemstack.getItem() instanceof ItemArmor && itemstack1.getItem() instanceof ItemArmor)
            {
                ItemArmor itemarmor = (ItemArmor)itemstack.getItem();
                ItemArmor itemarmor1 = (ItemArmor)itemstack1.getItem();

                if (itemarmor.damageReduceAmount == itemarmor1.damageReduceAmount)
                {
                    flag = itemstack.getMetadata() > itemstack1.getMetadata() || itemstack.hasTagCompound() && !itemstack1.hasTagCompound();
                }
                else
                {
                    flag = itemarmor.damageReduceAmount > itemarmor1.damageReduceAmount;
                }
            }
            else
            {
                flag = false;
            }
        }

        if (flag && this.func_175448_a(itemstack))
        {
            double d0;

            switch (entityequipmentslot.func_188453_a())
            {
                case HAND:
                    d0 = (double)this.inventoryHandsDropChances[entityequipmentslot.func_188454_b()];
                    break;

                case ARMOR:
                    d0 = (double)this.inventoryArmorDropChances[entityequipmentslot.func_188454_b()];
                    break;

                default:
                    d0 = 0.0D;
            }

            if (itemstack1 != null && (double)(this.rand.nextFloat() - 0.1F) < d0)
            {
                this.entityDropItem(itemstack1, 0.0F);
            }

            if (itemstack.getItem() == Items.diamond && itemEntity.getThrower() != null)
            {
                EntityPlayer entityplayer = this.worldObj.getPlayerEntityByName(itemEntity.getThrower());

                if (entityplayer != null)
                {
                    entityplayer.triggerAchievement(AchievementList.field_187996_x);
                }
            }

            this.setItemStackToSlot(entityequipmentslot, itemstack);

            switch (entityequipmentslot.func_188453_a())
            {
                case HAND:
                    this.inventoryHandsDropChances[entityequipmentslot.func_188454_b()] = 2.0F;
                    break;

                case ARMOR:
                    this.inventoryArmorDropChances[entityequipmentslot.func_188454_b()] = 2.0F;
            }

            this.persistenceRequired = true;
            this.onItemPickup(itemEntity, 1);
            itemEntity.setDead();
        }
    }

    protected boolean func_175448_a(ItemStack stack)
    {
        return true;
    }

    /**
     * Determines if an entity can be despawned, used on idle far away entities
     */
    protected boolean canDespawn()
    {
        return true;
    }

    /**
     * Makes the entity despawn if requirements are reached
     */
    protected void despawnEntity()
    {
        if (this.persistenceRequired)
        {
            this.entityAge = 0;
        }
        else
        {
            Entity entity = this.worldObj.getClosestPlayerToEntity(this, -1.0D);

            if (entity != null)
            {
                double d0 = entity.posX - this.posX;
                double d1 = entity.posY - this.posY;
                double d2 = entity.posZ - this.posZ;
                double d3 = d0 * d0 + d1 * d1 + d2 * d2;

                if (this.canDespawn() && d3 > 16384.0D)
                {
                    this.setDead();
                }

                if (this.entityAge > 600 && this.rand.nextInt(800) == 0 && d3 > 1024.0D && this.canDespawn())
                {
                    this.setDead();
                }
                else if (d3 < 1024.0D)
                {
                    this.entityAge = 0;
                }
            }
        }
    }

    protected final void updateEntityActionState()
    {
        ++this.entityAge;
        this.worldObj.theProfiler.startSection("checkDespawn");
        this.despawnEntity();
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("sensing");
        this.senses.clearSensingCache();
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("targetSelector");
        this.targetTasks.onUpdateTasks();
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("goalSelector");
        this.tasks.onUpdateTasks();
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("navigation");
        this.navigator.onUpdateNavigation();
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.startSection("mob tick");
        this.updateAITasks();
        this.worldObj.theProfiler.endSection();

        if (this.isRiding() && this.getRidingEntity() instanceof EntityLiving)
        {
            EntityLiving entityliving = (EntityLiving)this.getRidingEntity();
            entityliving.getNavigator().setPath(this.getNavigator().getPath(), 1.5D);
            entityliving.getMoveHelper().func_188487_a(this.getMoveHelper());
        }

        this.worldObj.theProfiler.startSection("controls");
        this.worldObj.theProfiler.startSection("move");
        this.moveHelper.onUpdateMoveHelper();
        this.worldObj.theProfiler.endStartSection("look");
        this.lookHelper.onUpdateLook();
        this.worldObj.theProfiler.endStartSection("jump");
        this.jumpHelper.doJump();
        this.worldObj.theProfiler.endSection();
        this.worldObj.theProfiler.endSection();
    }

    protected void updateAITasks()
    {
    }

    /**
     * The speed it takes to move the entityliving's rotationPitch through the faceEntity method. This is only currently
     * use in wolves.
     */
    public int getVerticalFaceSpeed()
    {
        return 40;
    }

    public int func_184649_cE()
    {
        return 10;
    }

    /**
     * Changes pitch and yaw so that the entity calling the function is facing the entity provided as an argument.
     */
    public void faceEntity(Entity entityIn, float p_70625_2_, float p_70625_3_)
    {
        double d0 = entityIn.posX - this.posX;
        double d2 = entityIn.posZ - this.posZ;
        double d1;

        if (entityIn instanceof EntityLivingBase)
        {
            EntityLivingBase entitylivingbase = (EntityLivingBase)entityIn;
            d1 = entitylivingbase.posY + (double)entitylivingbase.getEyeHeight() - (this.posY + (double)this.getEyeHeight());
        }
        else
        {
            d1 = (entityIn.getEntityBoundingBox().minY + entityIn.getEntityBoundingBox().maxY) / 2.0D - (this.posY + (double)this.getEyeHeight());
        }

        double d3 = (double)MathHelper.sqrt_double(d0 * d0 + d2 * d2);
        float f = (float)(MathHelper.atan2(d2, d0) * (180D / Math.PI)) - 90.0F;
        float f1 = (float)(-(MathHelper.atan2(d1, d3) * (180D / Math.PI)));
        this.rotationPitch = this.updateRotation(this.rotationPitch, f1, p_70625_3_);
        this.rotationYaw = this.updateRotation(this.rotationYaw, f, p_70625_2_);
    }

    /**
     * Arguments: current rotation, intended rotation, max increment.
     */
    private float updateRotation(float p_70663_1_, float p_70663_2_, float p_70663_3_)
    {
        float f = MathHelper.wrapAngleTo180_float(p_70663_2_ - p_70663_1_);

        if (f > p_70663_3_)
        {
            f = p_70663_3_;
        }

        if (f < -p_70663_3_)
        {
            f = -p_70663_3_;
        }

        return p_70663_1_ + f;
    }

    /**
     * Checks if the entity's current position is a valid location to spawn this entity.
     */
    public boolean getCanSpawnHere()
    {
        return true;
    }

    /**
     * Checks that the entity is not colliding with any blocks / liquids
     */
    public boolean isNotColliding()
    {
        return !this.worldObj.isAnyLiquid(this.getEntityBoundingBox()) && this.worldObj.func_184144_a(this, this.getEntityBoundingBox()).isEmpty() && this.worldObj.checkNoEntityCollision(this.getEntityBoundingBox(), this);
    }

    /**
     * Returns render size modifier
     */
    public float getRenderSizeModifier()
    {
        return 1.0F;
    }

    /**
     * Will return how many at most can spawn in a chunk at once.
     */
    public int getMaxSpawnedInChunk()
    {
        return 4;
    }

    /**
     * The maximum height from where the entity is alowed to jump (used in pathfinder)
     */
    public int getMaxFallHeight()
    {
        if (this.getAttackTarget() == null)
        {
            return 3;
        }
        else
        {
            int i = (int)(this.getHealth() - this.getMaxHealth() * 0.33F);
            i = i - (3 - this.worldObj.getDifficulty().getDifficultyId()) * 4;

            if (i < 0)
            {
                i = 0;
            }

            return i + 3;
        }
    }

    public Iterable<ItemStack> getHeldEquipment()
    {
        return Arrays.<ItemStack>asList(this.inventoryHands);
    }

    public Iterable<ItemStack> getArmorInventoryList()
    {
        return Arrays.<ItemStack>asList(this.inventoryArmor);
    }

    public ItemStack getItemStackFromSlot(EntityEquipmentSlot slotIn)
    {
        ItemStack itemstack = null;

        switch (slotIn.func_188453_a())
        {
            case HAND:
                itemstack = this.inventoryHands[slotIn.func_188454_b()];
                break;

            case ARMOR:
                itemstack = this.inventoryArmor[slotIn.func_188454_b()];
        }

        return itemstack;
    }

    public void setItemStackToSlot(EntityEquipmentSlot slotIn, ItemStack stack)
    {
        switch (slotIn.func_188453_a())
        {
            case HAND:
                this.inventoryHands[slotIn.func_188454_b()] = stack;
                break;

            case ARMOR:
                this.inventoryArmor[slotIn.func_188454_b()] = stack;
        }
    }

    /**
     * Drop the equipment for this entity.
     */
    protected void dropEquipment(boolean wasRecentlyHit, int lootingModifier)
    {
        for (EntityEquipmentSlot entityequipmentslot : EntityEquipmentSlot.values())
        {
            ItemStack itemstack = this.getItemStackFromSlot(entityequipmentslot);
            double d0;

            switch (entityequipmentslot.func_188453_a())
            {
                case HAND:
                    d0 = (double)this.inventoryHandsDropChances[entityequipmentslot.func_188454_b()];
                    break;

                case ARMOR:
                    d0 = (double)this.inventoryArmorDropChances[entityequipmentslot.func_188454_b()];
                    break;

                default:
                    d0 = 0.0D;
            }

            boolean flag = d0 > 1.0D;

            if (itemstack != null && (wasRecentlyHit || flag) && (double)(this.rand.nextFloat() - (float)lootingModifier * 0.01F) < d0)
            {
                if (!flag && itemstack.isItemStackDamageable())
                {
                    int i = Math.max(itemstack.getMaxDamage() - 25, 1);
                    int j = itemstack.getMaxDamage() - this.rand.nextInt(this.rand.nextInt(i) + 1);

                    if (j > i)
                    {
                        j = i;
                    }

                    if (j < 1)
                    {
                        j = 1;
                    }

                    itemstack.setItemDamage(j);
                }

                this.entityDropItem(itemstack, 0.0F);
            }
        }
    }

    /**
     * Gives armor or weapon for entity based on given DifficultyInstance
     */
    protected void setEquipmentBasedOnDifficulty(DifficultyInstance difficulty)
    {
        if (this.rand.nextFloat() < 0.15F * difficulty.getClampedAdditionalDifficulty())
        {
            int i = this.rand.nextInt(2);
            float f = this.worldObj.getDifficulty() == EnumDifficulty.HARD ? 0.1F : 0.25F;

            if (this.rand.nextFloat() < 0.095F)
            {
                ++i;
            }

            if (this.rand.nextFloat() < 0.095F)
            {
                ++i;
            }

            if (this.rand.nextFloat() < 0.095F)
            {
                ++i;
            }

            boolean flag = true;

            for (EntityEquipmentSlot entityequipmentslot : EntityEquipmentSlot.values())
            {
                if (entityequipmentslot.func_188453_a() == EntityEquipmentSlot.Type.ARMOR)
                {
                    ItemStack itemstack = this.getItemStackFromSlot(entityequipmentslot);

                    if (!flag && this.rand.nextFloat() < f)
                    {
                        break;
                    }

                    flag = false;

                    if (itemstack == null)
                    {
                        Item item = func_184636_a(entityequipmentslot, i);

                        if (item != null)
                        {
                            this.setItemStackToSlot(entityequipmentslot, new ItemStack(item));
                        }
                    }
                }
            }
        }
    }

    public static EntityEquipmentSlot func_184640_d(ItemStack p_184640_0_)
    {
        return p_184640_0_.getItem() != Item.getItemFromBlock(Blocks.pumpkin) && p_184640_0_.getItem() != Items.skull ? (p_184640_0_.getItem() == Items.elytra ? EntityEquipmentSlot.CHEST : (p_184640_0_.getItem() instanceof ItemArmor ? ((ItemArmor)p_184640_0_.getItem()).armorType : (p_184640_0_.getItem() == Items.elytra ? EntityEquipmentSlot.CHEST : EntityEquipmentSlot.MAINHAND))) : EntityEquipmentSlot.HEAD;
    }

    public static Item func_184636_a(EntityEquipmentSlot p_184636_0_, int p_184636_1_)
    {
        switch (p_184636_0_)
        {
            case HEAD:
                if (p_184636_1_ == 0)
                {
                    return Items.leather_helmet;
                }
                else if (p_184636_1_ == 1)
                {
                    return Items.golden_helmet;
                }
                else if (p_184636_1_ == 2)
                {
                    return Items.chainmail_helmet;
                }
                else if (p_184636_1_ == 3)
                {
                    return Items.iron_helmet;
                }
                else if (p_184636_1_ == 4)
                {
                    return Items.diamond_helmet;
                }

            case CHEST:
                if (p_184636_1_ == 0)
                {
                    return Items.leather_chestplate;
                }
                else if (p_184636_1_ == 1)
                {
                    return Items.golden_chestplate;
                }
                else if (p_184636_1_ == 2)
                {
                    return Items.chainmail_chestplate;
                }
                else if (p_184636_1_ == 3)
                {
                    return Items.iron_chestplate;
                }
                else if (p_184636_1_ == 4)
                {
                    return Items.diamond_chestplate;
                }

            case LEGS:
                if (p_184636_1_ == 0)
                {
                    return Items.leather_leggings;
                }
                else if (p_184636_1_ == 1)
                {
                    return Items.golden_leggings;
                }
                else if (p_184636_1_ == 2)
                {
                    return Items.chainmail_leggings;
                }
                else if (p_184636_1_ == 3)
                {
                    return Items.iron_leggings;
                }
                else if (p_184636_1_ == 4)
                {
                    return Items.diamond_leggings;
                }

            case FEET:
                if (p_184636_1_ == 0)
                {
                    return Items.leather_boots;
                }
                else if (p_184636_1_ == 1)
                {
                    return Items.golden_boots;
                }
                else if (p_184636_1_ == 2)
                {
                    return Items.chainmail_boots;
                }
                else if (p_184636_1_ == 3)
                {
                    return Items.iron_boots;
                }
                else if (p_184636_1_ == 4)
                {
                    return Items.diamond_boots;
                }

            default:
                return null;
        }
    }

    /**
     * Enchants Entity's current equipments based on given DifficultyInstance
     */
    protected void setEnchantmentBasedOnDifficulty(DifficultyInstance difficulty)
    {
        float f = difficulty.getClampedAdditionalDifficulty();

        if (this.getHeldItemMainhand() != null && this.rand.nextFloat() < 0.25F * f)
        {
            EnchantmentHelper.addRandomEnchantment(this.rand, this.getHeldItemMainhand(), (int)(5.0F + f * (float)this.rand.nextInt(18)), false);
        }

        for (EntityEquipmentSlot entityequipmentslot : EntityEquipmentSlot.values())
        {
            if (entityequipmentslot.func_188453_a() == EntityEquipmentSlot.Type.ARMOR)
            {
                ItemStack itemstack = this.getItemStackFromSlot(entityequipmentslot);

                if (itemstack != null && this.rand.nextFloat() < 0.5F * f)
                {
                    EnchantmentHelper.addRandomEnchantment(this.rand, itemstack, (int)(5.0F + f * (float)this.rand.nextInt(18)), false);
                }
            }
        }
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        this.getEntityAttribute(SharedMonsterAttributes.FOLLOW_RANGE).applyModifier(new AttributeModifier("Random spawn bonus", this.rand.nextGaussian() * 0.05D, 1));

        if (this.rand.nextFloat() < 0.05F)
        {
            this.func_184641_n(true);
        }
        else
        {
            this.func_184641_n(false);
        }

        return livingdata;
    }

    /**
     * returns true if all the conditions for steering the entity are met. For pigs, this is true if it is being ridden
     * by a player and the player is holding a carrot-on-a-stick
     */
    public boolean canBeSteered()
    {
        return false;
    }

    /**
     * Enable the Entity persistence
     */
    public void enablePersistence()
    {
        this.persistenceRequired = true;
    }

    public void func_184642_a(EntityEquipmentSlot slotIn, float chance)
    {
        switch (slotIn.func_188453_a())
        {
            case HAND:
                this.inventoryHandsDropChances[slotIn.func_188454_b()] = chance;
                break;

            case ARMOR:
                this.inventoryArmorDropChances[slotIn.func_188454_b()] = chance;
        }
    }

    public boolean canPickUpLoot()
    {
        return this.canPickUpLoot;
    }

    public void setCanPickUpLoot(boolean canPickup)
    {
        this.canPickUpLoot = canPickup;
    }

    public boolean isNoDespawnRequired()
    {
        return this.persistenceRequired;
    }

    public final boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        if (this.getLeashed() && this.getLeashedToEntity() == p_184230_1_)
        {
            this.clearLeashed(true, !p_184230_1_.capabilities.isCreativeMode);
            return true;
        }
        else if (p_184230_2_ != null && p_184230_2_.getItem() == Items.lead && this.func_184652_a(p_184230_1_))
        {
            this.setLeashedToEntity(p_184230_1_, true);
            --p_184230_2_.stackSize;
            return true;
        }
        else
        {
            return this.func_184645_a(p_184230_1_, p_184230_3_, p_184230_2_) ? true : super.func_184230_a(p_184230_1_, p_184230_2_, p_184230_3_);
        }
    }

    protected boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        return false;
    }

    /**
     * Applies logic related to leashes, for example dragging the entity or breaking the leash.
     */
    protected void updateLeashedState()
    {
        if (this.leashNBTTag != null)
        {
            this.recreateLeash();
        }

        if (this.isLeashed)
        {
            if (!this.isEntityAlive())
            {
                this.clearLeashed(true, true);
            }

            if (this.leashedToEntity == null || this.leashedToEntity.isDead)
            {
                this.clearLeashed(true, true);
            }
        }
    }

    /**
     * Removes the leash from this entity
     */
    public void clearLeashed(boolean sendPacket, boolean dropLead)
    {
        if (this.isLeashed)
        {
            this.isLeashed = false;
            this.leashedToEntity = null;

            if (!this.worldObj.isRemote && dropLead)
            {
                this.dropItem(Items.lead, 1);
            }

            if (!this.worldObj.isRemote && sendPacket && this.worldObj instanceof WorldServer)
            {
                ((WorldServer)this.worldObj).getEntityTracker().sendToAllTrackingEntity(this, new SPacketEntityAttach(this, (Entity)null));
            }
        }
    }

    public boolean func_184652_a(EntityPlayer p_184652_1_)
    {
        return !this.getLeashed() && !(this instanceof IMob);
    }

    public boolean getLeashed()
    {
        return this.isLeashed;
    }

    public Entity getLeashedToEntity()
    {
        return this.leashedToEntity;
    }

    /**
     * Sets the entity to be leashed to.
     */
    public void setLeashedToEntity(Entity entityIn, boolean sendAttachNotification)
    {
        this.isLeashed = true;
        this.leashedToEntity = entityIn;

        if (!this.worldObj.isRemote && sendAttachNotification && this.worldObj instanceof WorldServer)
        {
            ((WorldServer)this.worldObj).getEntityTracker().sendToAllTrackingEntity(this, new SPacketEntityAttach(this, this.leashedToEntity));
        }

        if (this.isRiding())
        {
            this.dismountRidingEntity();
        }
    }

    public boolean startRiding(Entity entityIn, boolean force)
    {
        boolean flag = super.startRiding(entityIn, force);

        if (flag && this.getLeashed())
        {
            this.clearLeashed(true, true);
        }

        return flag;
    }

    private void recreateLeash()
    {
        if (this.isLeashed && this.leashNBTTag != null)
        {
            if (this.leashNBTTag.hasUniqueId("UUID"))
            {
                UUID uuid = this.leashNBTTag.getUniqueId("UUID");

                for (EntityLivingBase entitylivingbase : this.worldObj.getEntitiesWithinAABB(EntityLivingBase.class, this.getEntityBoundingBox().func_186662_g(10.0D)))
                {
                    if (entitylivingbase.getUniqueID().equals(uuid))
                    {
                        this.leashedToEntity = entitylivingbase;
                        break;
                    }
                }
            }
            else if (this.leashNBTTag.hasKey("X", 99) && this.leashNBTTag.hasKey("Y", 99) && this.leashNBTTag.hasKey("Z", 99))
            {
                BlockPos blockpos = new BlockPos(this.leashNBTTag.getInteger("X"), this.leashNBTTag.getInteger("Y"), this.leashNBTTag.getInteger("Z"));
                EntityLeashKnot entityleashknot = EntityLeashKnot.getKnotForPosition(this.worldObj, blockpos);

                if (entityleashknot == null)
                {
                    entityleashknot = EntityLeashKnot.createKnot(this.worldObj, blockpos);
                }

                this.leashedToEntity = entityleashknot;
            }
            else
            {
                this.clearLeashed(false, true);
            }
        }

        this.leashNBTTag = null;
    }

    public boolean replaceItemInInventory(int inventorySlot, ItemStack itemStackIn)
    {
        EntityEquipmentSlot entityequipmentslot;

        if (inventorySlot == 98)
        {
            entityequipmentslot = EntityEquipmentSlot.MAINHAND;
        }
        else if (inventorySlot == 99)
        {
            entityequipmentslot = EntityEquipmentSlot.OFFHAND;
        }
        else if (inventorySlot == 100 + EntityEquipmentSlot.HEAD.func_188454_b())
        {
            entityequipmentslot = EntityEquipmentSlot.HEAD;
        }
        else if (inventorySlot == 100 + EntityEquipmentSlot.CHEST.func_188454_b())
        {
            entityequipmentslot = EntityEquipmentSlot.CHEST;
        }
        else if (inventorySlot == 100 + EntityEquipmentSlot.LEGS.func_188454_b())
        {
            entityequipmentslot = EntityEquipmentSlot.LEGS;
        }
        else
        {
            if (inventorySlot != 100 + EntityEquipmentSlot.FEET.func_188454_b())
            {
                return false;
            }

            entityequipmentslot = EntityEquipmentSlot.FEET;
        }

        if (itemStackIn != null && !func_184648_b(entityequipmentslot, itemStackIn) && entityequipmentslot != EntityEquipmentSlot.HEAD)
        {
            return false;
        }
        else
        {
            this.setItemStackToSlot(entityequipmentslot, itemStackIn);
            return true;
        }
    }

    public static boolean func_184648_b(EntityEquipmentSlot p_184648_0_, ItemStack p_184648_1_)
    {
        EntityEquipmentSlot entityequipmentslot = func_184640_d(p_184648_1_);
        return entityequipmentslot == p_184648_0_ || entityequipmentslot == EntityEquipmentSlot.MAINHAND && p_184648_0_ == EntityEquipmentSlot.OFFHAND;
    }

    /**
     * Returns whether the entity is in a server world
     */
    public boolean isServerWorld()
    {
        return super.isServerWorld() && !this.isAIDisabled();
    }

    /**
     * Set whether this Entity's AI is disabled
     */
    public void setNoAI(boolean disable)
    {
        byte b0 = ((Byte)this.dataWatcher.get(AI_FLAGS)).byteValue();
        this.dataWatcher.set(AI_FLAGS, Byte.valueOf(disable ? (byte)(b0 | 1) : (byte)(b0 & -2)));
    }

    public void func_184641_n(boolean p_184641_1_)
    {
        byte b0 = ((Byte)this.dataWatcher.get(AI_FLAGS)).byteValue();
        this.dataWatcher.set(AI_FLAGS, Byte.valueOf(p_184641_1_ ? (byte)(b0 | 2) : (byte)(b0 & -3)));
    }

    /**
     * Get whether this Entity's AI is disabled
     */
    public boolean isAIDisabled()
    {
        return (((Byte)this.dataWatcher.get(AI_FLAGS)).byteValue() & 1) != 0;
    }

    public boolean isLeftHanded()
    {
        return (((Byte)this.dataWatcher.get(AI_FLAGS)).byteValue() & 2) != 0;
    }

    public EnumHandSide getPrimaryHand()
    {
        return this.isLeftHanded() ? EnumHandSide.LEFT : EnumHandSide.RIGHT;
    }

    public static enum SpawnPlacementType
    {
        ON_GROUND,
        IN_AIR,
        IN_WATER;
    }
}
