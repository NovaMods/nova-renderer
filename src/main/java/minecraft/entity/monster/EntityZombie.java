package net.minecraft.entity.monster;

import java.util.Calendar;
import java.util.List;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.EnumCreatureAttribute;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIBreakDoor;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAIMoveThroughVillage;
import net.minecraft.entity.ai.EntityAIMoveTowardsRestriction;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.ai.EntityAIZombieAttack;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttribute;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.ai.attributes.RangedAttribute;
import net.minecraft.entity.item.EntityBoat;
import net.minecraft.entity.passive.EntityChicken;
import net.minecraft.entity.passive.EntityVillager;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.pathfinding.PathNavigateGround;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntitySelectors;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityZombie extends EntityMob
{
    /**
     * The attribute which determines the chance that this mob will spawn reinforcements
     */
    protected static final IAttribute reinforcementChance = (new RangedAttribute((IAttribute)null, "zombie.spawnReinforcements", 0.0D, 0.0D, 1.0D)).setDescription("Spawn Reinforcements Chance");
    private static final UUID babySpeedBoostUUID = UUID.fromString("B9766B59-9566-4402-BC1F-2EE2A276D836");
    private static final AttributeModifier babySpeedBoostModifier = new AttributeModifier(babySpeedBoostUUID, "Baby speed boost", 0.5D, 1);
    private static final DataParameter<Boolean> IS_CHILD = EntityDataManager.<Boolean>createKey(EntityZombie.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Integer> VILLAGER_TYPE = EntityDataManager.<Integer>createKey(EntityZombie.class, DataSerializers.VARINT);
    private static final DataParameter<Boolean> CONVERTING = EntityDataManager.<Boolean>createKey(EntityZombie.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Boolean> ARMS_RAISED = EntityDataManager.<Boolean>createKey(EntityZombie.class, DataSerializers.BOOLEAN);
    private final EntityAIBreakDoor breakDoor = new EntityAIBreakDoor(this);

    /**
     * Ticker used to determine the time remaining for this zombie to convert into a villager when cured.
     */
    private int conversionTime;
    private boolean isBreakDoorsTaskSet = false;

    /** The width of the entity */
    private float zombieWidth = -1.0F;

    /** The height of the the entity. */
    private float zombieHeight;

    public EntityZombie(World worldIn)
    {
        super(worldIn);
        this.setSize(0.6F, 1.95F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(0, new EntityAISwimming(this));
        this.tasks.addTask(2, new EntityAIZombieAttack(this, 1.0D, false));
        this.tasks.addTask(5, new EntityAIMoveTowardsRestriction(this, 1.0D));
        this.tasks.addTask(7, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(8, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(8, new EntityAILookIdle(this));
        this.applyEntityAI();
    }

    protected void applyEntityAI()
    {
        this.tasks.addTask(6, new EntityAIMoveThroughVillage(this, 1.0D, false));
        this.targetTasks.addTask(1, new EntityAIHurtByTarget(this, true, new Class[] {EntityPigZombie.class}));
        this.targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, EntityPlayer.class, true));
        this.targetTasks.addTask(3, new EntityAINearestAttackableTarget(this, EntityVillager.class, false));
        this.targetTasks.addTask(3, new EntityAINearestAttackableTarget(this, EntityIronGolem.class, true));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.FOLLOW_RANGE).setBaseValue(35.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.23000000417232513D);
        this.getEntityAttribute(SharedMonsterAttributes.ATTACK_DAMAGE).setBaseValue(3.0D);
        this.getEntityAttribute(SharedMonsterAttributes.ARMOR).setBaseValue(2.0D);
        this.getAttributeMap().registerAttribute(reinforcementChance).setBaseValue(this.rand.nextDouble() * 0.10000000149011612D);
    }

    protected void entityInit()
    {
        super.entityInit();
        this.getDataManager().register(IS_CHILD, Boolean.valueOf(false));
        this.getDataManager().register(VILLAGER_TYPE, Integer.valueOf(0));
        this.getDataManager().register(CONVERTING, Boolean.valueOf(false));
        this.getDataManager().register(ARMS_RAISED, Boolean.valueOf(false));
    }

    public void setArmsRaised(boolean p_184733_1_)
    {
        this.getDataManager().set(ARMS_RAISED, Boolean.valueOf(p_184733_1_));
    }

    public boolean isArmsRaised()
    {
        return ((Boolean)this.getDataManager().get(ARMS_RAISED)).booleanValue();
    }

    public boolean isBreakDoorsTaskSet()
    {
        return this.isBreakDoorsTaskSet;
    }

    /**
     * Sets or removes EntityAIBreakDoor task
     */
    public void setBreakDoorsAItask(boolean par1)
    {
        if (this.isBreakDoorsTaskSet != par1)
        {
            this.isBreakDoorsTaskSet = par1;
            ((PathNavigateGround)this.getNavigator()).setBreakDoors(par1);

            if (par1)
            {
                this.tasks.addTask(1, this.breakDoor);
            }
            else
            {
                this.tasks.removeTask(this.breakDoor);
            }
        }
    }

    /**
     * If Animal, checks if the age timer is negative
     */
    public boolean isChild()
    {
        return ((Boolean)this.getDataManager().get(IS_CHILD)).booleanValue();
    }

    /**
     * Get the experience points the entity currently has.
     */
    protected int getExperiencePoints(EntityPlayer player)
    {
        if (this.isChild())
        {
            this.experienceValue = (int)((float)this.experienceValue * 2.5F);
        }

        return super.getExperiencePoints(player);
    }

    /**
     * Set whether this zombie is a child.
     */
    public void setChild(boolean childZombie)
    {
        this.getDataManager().set(IS_CHILD, Boolean.valueOf(childZombie));

        if (this.worldObj != null && !this.worldObj.isRemote)
        {
            IAttributeInstance iattributeinstance = this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED);
            iattributeinstance.removeModifier(babySpeedBoostModifier);

            if (childZombie)
            {
                iattributeinstance.applyModifier(babySpeedBoostModifier);
            }
        }

        this.setChildSize(childZombie);
    }

    /**
     * Return whether this zombie is a villager.
     */
    public boolean isVillager()
    {
        return ((Integer)this.getDataManager().get(VILLAGER_TYPE)).intValue() > 0;
    }

    public int getVillagerType()
    {
        return ((Integer)this.getDataManager().get(VILLAGER_TYPE)).intValue() - 1;
    }

    public void setVillagerType(int p_184735_1_)
    {
        this.getDataManager().set(VILLAGER_TYPE, Integer.valueOf(p_184735_1_ + 1));
    }

    public void setToNotVillager()
    {
        this.getDataManager().set(VILLAGER_TYPE, Integer.valueOf(0));
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        if (IS_CHILD.equals(key))
        {
            this.setChildSize(this.isChild());
        }

        super.notifyDataManagerChange(key);
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (this.worldObj.isDaytime() && !this.worldObj.isRemote && !this.isChild())
        {
            float f = this.getBrightness(1.0F);
            BlockPos blockpos = this.getRidingEntity() instanceof EntityBoat ? (new BlockPos(this.posX, (double)Math.round(this.posY), this.posZ)).up() : new BlockPos(this.posX, (double)Math.round(this.posY), this.posZ);

            if (f > 0.5F && this.rand.nextFloat() * 30.0F < (f - 0.4F) * 2.0F && this.worldObj.canSeeSky(blockpos))
            {
                boolean flag = true;
                ItemStack itemstack = this.getItemStackFromSlot(EntityEquipmentSlot.HEAD);

                if (itemstack != null)
                {
                    if (itemstack.isItemStackDamageable())
                    {
                        itemstack.setItemDamage(itemstack.getItemDamage() + this.rand.nextInt(2));

                        if (itemstack.getItemDamage() >= itemstack.getMaxDamage())
                        {
                            this.renderBrokenItemStack(itemstack);
                            this.setItemStackToSlot(EntityEquipmentSlot.HEAD, (ItemStack)null);
                        }
                    }

                    flag = false;
                }

                if (flag)
                {
                    this.setFire(8);
                }
            }
        }

        super.onLivingUpdate();
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        if (super.attackEntityFrom(source, amount))
        {
            EntityLivingBase entitylivingbase = this.getAttackTarget();

            if (entitylivingbase == null && source.getEntity() instanceof EntityLivingBase)
            {
                entitylivingbase = (EntityLivingBase)source.getEntity();
            }

            if (entitylivingbase != null && this.worldObj.getDifficulty() == EnumDifficulty.HARD && (double)this.rand.nextFloat() < this.getEntityAttribute(reinforcementChance).getAttributeValue() && this.worldObj.getGameRules().getBoolean("doMobSpawning"))
            {
                int i = MathHelper.floor_double(this.posX);
                int j = MathHelper.floor_double(this.posY);
                int k = MathHelper.floor_double(this.posZ);
                EntityZombie entityzombie = new EntityZombie(this.worldObj);

                for (int l = 0; l < 50; ++l)
                {
                    int i1 = i + MathHelper.getRandomIntegerInRange(this.rand, 7, 40) * MathHelper.getRandomIntegerInRange(this.rand, -1, 1);
                    int j1 = j + MathHelper.getRandomIntegerInRange(this.rand, 7, 40) * MathHelper.getRandomIntegerInRange(this.rand, -1, 1);
                    int k1 = k + MathHelper.getRandomIntegerInRange(this.rand, 7, 40) * MathHelper.getRandomIntegerInRange(this.rand, -1, 1);

                    if (this.worldObj.getBlockState(new BlockPos(i1, j1 - 1, k1)).func_185896_q() && this.worldObj.getLightFromNeighbors(new BlockPos(i1, j1, k1)) < 10)
                    {
                        entityzombie.setPosition((double)i1, (double)j1, (double)k1);

                        if (!this.worldObj.isAnyPlayerWithinRangeAt((double)i1, (double)j1, (double)k1, 7.0D) && this.worldObj.checkNoEntityCollision(entityzombie.getEntityBoundingBox(), entityzombie) && this.worldObj.func_184144_a(entityzombie, entityzombie.getEntityBoundingBox()).isEmpty() && !this.worldObj.isAnyLiquid(entityzombie.getEntityBoundingBox()))
                        {
                            this.worldObj.spawnEntityInWorld(entityzombie);
                            entityzombie.setAttackTarget(entitylivingbase);
                            entityzombie.onInitialSpawn(this.worldObj.getDifficultyForLocation(new BlockPos(entityzombie)), (IEntityLivingData)null);
                            this.getEntityAttribute(reinforcementChance).applyModifier(new AttributeModifier("Zombie reinforcement caller charge", -0.05000000074505806D, 0));
                            entityzombie.getEntityAttribute(reinforcementChance).applyModifier(new AttributeModifier("Zombie reinforcement callee charge", -0.05000000074505806D, 0));
                            break;
                        }
                    }
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        if (!this.worldObj.isRemote && this.isConverting())
        {
            int i = this.getConversionTimeBoost();
            this.conversionTime -= i;

            if (this.conversionTime <= 0)
            {
                this.convertToVillager();
            }
        }

        super.onUpdate();
    }

    public boolean attackEntityAsMob(Entity entityIn)
    {
        boolean flag = super.attackEntityAsMob(entityIn);

        if (flag)
        {
            int i = this.worldObj.getDifficulty().getDifficultyId();

            if (this.getHeldItemMainhand() == null && this.isBurning() && this.rand.nextFloat() < (float)i * 0.3F)
            {
                entityIn.setFire(2 * i);
            }
        }

        return flag;
    }

    protected SoundEvent getAmbientSound()
    {
        return this.isVillager() ? SoundEvents.entity_zombie_villager_ambient : SoundEvents.entity_zombie_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return this.isVillager() ? SoundEvents.entity_zombie_villager_hurt : SoundEvents.entity_zombie_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return this.isVillager() ? SoundEvents.entity_zombie_villager_death : SoundEvents.entity_zombie_death;
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        this.playSound(this.isVillager() ? SoundEvents.entity_zombie_villager_step : SoundEvents.entity_zombie_step, 0.15F, 1.0F);
    }

    /**
     * Get this Entity's EnumCreatureAttribute
     */
    public EnumCreatureAttribute getCreatureAttribute()
    {
        return EnumCreatureAttribute.UNDEAD;
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_ZOMBIE;
    }

    /**
     * Gives armor or weapon for entity based on given DifficultyInstance
     */
    protected void setEquipmentBasedOnDifficulty(DifficultyInstance difficulty)
    {
        super.setEquipmentBasedOnDifficulty(difficulty);

        if (this.rand.nextFloat() < (this.worldObj.getDifficulty() == EnumDifficulty.HARD ? 0.05F : 0.01F))
        {
            int i = this.rand.nextInt(3);

            if (i == 0)
            {
                this.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, new ItemStack(Items.iron_sword));
            }
            else
            {
                this.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, new ItemStack(Items.iron_shovel));
            }
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);

        if (this.isChild())
        {
            tagCompound.setBoolean("IsBaby", true);
        }

        if (this.isVillager())
        {
            tagCompound.setBoolean("IsVillager", true);
            tagCompound.setInteger("VillagerProfession", this.getVillagerType());
        }

        tagCompound.setInteger("ConversionTime", this.isConverting() ? this.conversionTime : -1);
        tagCompound.setBoolean("CanBreakDoors", this.isBreakDoorsTaskSet());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.getBoolean("IsBaby"))
        {
            this.setChild(true);
        }

        if (tagCompund.getBoolean("IsVillager"))
        {
            if (tagCompund.hasKey("VillagerProfession", 99))
            {
                this.setVillagerType(tagCompund.getInteger("VillagerProfession"));
            }
            else
            {
                this.setVillagerType(this.worldObj.rand.nextInt(5));
            }
        }

        if (tagCompund.hasKey("ConversionTime", 99) && tagCompund.getInteger("ConversionTime") > -1)
        {
            this.startConversion(tagCompund.getInteger("ConversionTime"));
        }

        this.setBreakDoorsAItask(tagCompund.getBoolean("CanBreakDoors"));
    }

    /**
     * This method gets called when the entity kills another one.
     */
    public void onKillEntity(EntityLivingBase entityLivingIn)
    {
        super.onKillEntity(entityLivingIn);

        if ((this.worldObj.getDifficulty() == EnumDifficulty.NORMAL || this.worldObj.getDifficulty() == EnumDifficulty.HARD) && entityLivingIn instanceof EntityVillager)
        {
            if (this.worldObj.getDifficulty() != EnumDifficulty.HARD && this.rand.nextBoolean())
            {
                return;
            }

            EntityVillager entityvillager = (EntityVillager)entityLivingIn;
            EntityZombie entityzombie = new EntityZombie(this.worldObj);
            entityzombie.copyLocationAndAnglesFrom(entityLivingIn);
            this.worldObj.removeEntity(entityLivingIn);
            entityzombie.onInitialSpawn(this.worldObj.getDifficultyForLocation(new BlockPos(entityzombie)), new EntityZombie.GroupData(false, true));
            entityzombie.setVillagerType(entityvillager.getProfession());
            entityzombie.setChild(entityLivingIn.isChild());
            entityzombie.setNoAI(entityvillager.isAIDisabled());

            if (entityvillager.hasCustomName())
            {
                entityzombie.setCustomNameTag(entityvillager.getCustomNameTag());
                entityzombie.setAlwaysRenderNameTag(entityvillager.getAlwaysRenderNameTag());
            }

            this.worldObj.spawnEntityInWorld(entityzombie);
            this.worldObj.playAuxSFXAtEntity((EntityPlayer)null, 1026, new BlockPos((int)this.posX, (int)this.posY, (int)this.posZ), 0);
        }
    }

    public float getEyeHeight()
    {
        float f = 1.74F;

        if (this.isChild())
        {
            f = (float)((double)f - 0.81D);
        }

        return f;
    }

    protected boolean func_175448_a(ItemStack stack)
    {
        return stack.getItem() == Items.egg && this.isChild() && this.isRiding() ? false : super.func_175448_a(stack);
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        livingdata = super.onInitialSpawn(difficulty, livingdata);
        float f = difficulty.getClampedAdditionalDifficulty();
        this.setCanPickUpLoot(this.rand.nextFloat() < 0.55F * f);

        if (livingdata == null)
        {
            livingdata = new EntityZombie.GroupData(this.worldObj.rand.nextFloat() < 0.05F, this.worldObj.rand.nextFloat() < 0.05F);
        }

        if (livingdata instanceof EntityZombie.GroupData)
        {
            EntityZombie.GroupData entityzombie$groupdata = (EntityZombie.GroupData)livingdata;

            if (entityzombie$groupdata.isVillager)
            {
                this.setVillagerType(this.rand.nextInt(5));
            }

            if (entityzombie$groupdata.isChild)
            {
                this.setChild(true);

                if ((double)this.worldObj.rand.nextFloat() < 0.05D)
                {
                    List<EntityChicken> list = this.worldObj.<EntityChicken>getEntitiesWithinAABB(EntityChicken.class, this.getEntityBoundingBox().expand(5.0D, 3.0D, 5.0D), EntitySelectors.IS_STANDALONE);

                    if (!list.isEmpty())
                    {
                        EntityChicken entitychicken = (EntityChicken)list.get(0);
                        entitychicken.setChickenJockey(true);
                        this.startRiding(entitychicken);
                    }
                }
                else if ((double)this.worldObj.rand.nextFloat() < 0.05D)
                {
                    EntityChicken entitychicken1 = new EntityChicken(this.worldObj);
                    entitychicken1.setLocationAndAngles(this.posX, this.posY, this.posZ, this.rotationYaw, 0.0F);
                    entitychicken1.onInitialSpawn(difficulty, (IEntityLivingData)null);
                    entitychicken1.setChickenJockey(true);
                    this.worldObj.spawnEntityInWorld(entitychicken1);
                    this.startRiding(entitychicken1);
                }
            }
        }

        this.setBreakDoorsAItask(this.rand.nextFloat() < f * 0.1F);
        this.setEquipmentBasedOnDifficulty(difficulty);
        this.setEnchantmentBasedOnDifficulty(difficulty);

        if (this.getItemStackFromSlot(EntityEquipmentSlot.HEAD) == null)
        {
            Calendar calendar = this.worldObj.getCurrentDate();

            if (calendar.get(2) + 1 == 10 && calendar.get(5) == 31 && this.rand.nextFloat() < 0.25F)
            {
                this.setItemStackToSlot(EntityEquipmentSlot.HEAD, new ItemStack(this.rand.nextFloat() < 0.1F ? Blocks.lit_pumpkin : Blocks.pumpkin));
                this.inventoryArmorDropChances[EntityEquipmentSlot.HEAD.func_188454_b()] = 0.0F;
            }
        }

        this.getEntityAttribute(SharedMonsterAttributes.KNOCKBACK_RESISTANCE).applyModifier(new AttributeModifier("Random spawn bonus", this.rand.nextDouble() * 0.05000000074505806D, 0));
        double d0 = this.rand.nextDouble() * 1.5D * (double)f;

        if (d0 > 1.0D)
        {
            this.getEntityAttribute(SharedMonsterAttributes.FOLLOW_RANGE).applyModifier(new AttributeModifier("Random zombie-spawn bonus", d0, 2));
        }

        if (this.rand.nextFloat() < f * 0.05F)
        {
            this.getEntityAttribute(reinforcementChance).applyModifier(new AttributeModifier("Leader zombie bonus", this.rand.nextDouble() * 0.25D + 0.5D, 0));
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).applyModifier(new AttributeModifier("Leader zombie bonus", this.rand.nextDouble() * 3.0D + 1.0D, 2));
            this.setBreakDoorsAItask(true);
        }

        return livingdata;
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (p_184645_3_ != null && p_184645_3_.getItem() == Items.golden_apple && p_184645_3_.getMetadata() == 0 && this.isVillager() && this.isPotionActive(MobEffects.weakness))
        {
            if (!p_184645_1_.capabilities.isCreativeMode)
            {
                --p_184645_3_.stackSize;
            }

            if (!this.worldObj.isRemote)
            {
                this.startConversion(this.rand.nextInt(2401) + 3600);
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Starts converting this zombie into a villager. The zombie converts into a villager after the specified time in
     * ticks.
     */
    protected void startConversion(int ticks)
    {
        this.conversionTime = ticks;
        this.getDataManager().set(CONVERTING, Boolean.valueOf(true));
        this.func_184589_d(MobEffects.weakness);
        this.addPotionEffect(new PotionEffect(MobEffects.damageBoost, ticks, Math.min(this.worldObj.getDifficulty().getDifficultyId() - 1, 0)));
        this.worldObj.setEntityState(this, (byte)16);
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 16)
        {
            if (!this.isSilent())
            {
                this.worldObj.func_184134_a(this.posX + 0.5D, this.posY + 0.5D, this.posZ + 0.5D, SoundEvents.entity_zombie_villager_cure, this.getSoundCategory(), 1.0F + this.rand.nextFloat(), this.rand.nextFloat() * 0.7F + 0.3F, false);
            }
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    /**
     * Determines if an entity can be despawned, used on idle far away entities
     */
    protected boolean canDespawn()
    {
        return !this.isConverting();
    }

    /**
     * Returns whether this zombie is in the process of converting to a villager
     */
    public boolean isConverting()
    {
        return ((Boolean)this.getDataManager().get(CONVERTING)).booleanValue();
    }

    /**
     * Convert this zombie into a villager.
     */
    protected void convertToVillager()
    {
        EntityVillager entityvillager = new EntityVillager(this.worldObj);
        entityvillager.copyLocationAndAnglesFrom(this);
        entityvillager.onInitialSpawn(this.worldObj.getDifficultyForLocation(new BlockPos(entityvillager)), (IEntityLivingData)null);
        entityvillager.setLookingForHome();

        if (this.isChild())
        {
            entityvillager.setGrowingAge(-24000);
        }

        this.worldObj.removeEntity(this);
        entityvillager.setNoAI(this.isAIDisabled());
        entityvillager.setProfession(this.getVillagerType());

        if (this.hasCustomName())
        {
            entityvillager.setCustomNameTag(this.getCustomNameTag());
            entityvillager.setAlwaysRenderNameTag(this.getAlwaysRenderNameTag());
        }

        this.worldObj.spawnEntityInWorld(entityvillager);
        entityvillager.addPotionEffect(new PotionEffect(MobEffects.confusion, 200, 0));
        this.worldObj.playAuxSFXAtEntity((EntityPlayer)null, 1027, new BlockPos((int)this.posX, (int)this.posY, (int)this.posZ), 0);
    }

    /**
     * Return the amount of time decremented from conversionTime every tick.
     */
    protected int getConversionTimeBoost()
    {
        int i = 1;

        if (this.rand.nextFloat() < 0.01F)
        {
            int j = 0;
            BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos();

            for (int k = (int)this.posX - 4; k < (int)this.posX + 4 && j < 14; ++k)
            {
                for (int l = (int)this.posY - 4; l < (int)this.posY + 4 && j < 14; ++l)
                {
                    for (int i1 = (int)this.posZ - 4; i1 < (int)this.posZ + 4 && j < 14; ++i1)
                    {
                        Block block = this.worldObj.getBlockState(blockpos$mutableblockpos.set(k, l, i1)).getBlock();

                        if (block == Blocks.iron_bars || block == Blocks.bed)
                        {
                            if (this.rand.nextFloat() < 0.3F)
                            {
                                ++i;
                            }

                            ++j;
                        }
                    }
                }
            }
        }

        return i;
    }

    /**
     * sets the size of the entity to be half of its current size if true.
     */
    public void setChildSize(boolean isChild)
    {
        this.multiplySize(isChild ? 0.5F : 1.0F);
    }

    /**
     * Sets the width and height of the entity. Args: width, height
     */
    protected final void setSize(float width, float height)
    {
        boolean flag = this.zombieWidth > 0.0F && this.zombieHeight > 0.0F;
        this.zombieWidth = width;
        this.zombieHeight = height;

        if (!flag)
        {
            this.multiplySize(1.0F);
        }
    }

    /**
     * Multiplies the height and width by the provided float.
     */
    protected final void multiplySize(float size)
    {
        super.setSize(this.zombieWidth * size, this.zombieHeight * size);
    }

    /**
     * Returns the Y Offset of this entity.
     */
    public double getYOffset()
    {
        return this.isChild() ? 0.0D : -0.35D;
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        super.onDeath(cause);

        if (cause.getEntity() instanceof EntityCreeper && !(this instanceof EntityPigZombie) && ((EntityCreeper)cause.getEntity()).getPowered() && ((EntityCreeper)cause.getEntity()).isAIEnabled())
        {
            ((EntityCreeper)cause.getEntity()).func_175493_co();
            this.entityDropItem(new ItemStack(Items.skull, 1, 2), 0.0F);
        }
    }

    class GroupData implements IEntityLivingData
    {
        public boolean isChild;
        public boolean isVillager;

        private GroupData(boolean isBaby, boolean isVillagerZombie)
        {
            this.isChild = false;
            this.isVillager = false;
            this.isChild = isBaby;
            this.isVillager = isVillagerZombie;
        }
    }
}
