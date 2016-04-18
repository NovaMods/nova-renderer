package net.minecraft.entity.passive;

import com.google.common.base.Predicate;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityAgeable;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackMelee;
import net.minecraft.entity.ai.EntityAIBeg;
import net.minecraft.entity.ai.EntityAIFollowOwner;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILeapAtTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAIMate;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAIOwnerHurtByTarget;
import net.minecraft.entity.ai.EntityAIOwnerHurtTarget;
import net.minecraft.entity.ai.EntityAISit;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAITargetNonTamed;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.monster.EntityCreeper;
import net.minecraft.entity.monster.EntityGhast;
import net.minecraft.entity.monster.EntitySkeleton;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.EnumDyeColor;
import net.minecraft.item.ItemFood;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityWolf extends EntityTameable
{
    private static final DataParameter<Float> field_184759_bz = EntityDataManager.<Float>createKey(EntityWolf.class, DataSerializers.FLOAT);
    private static final DataParameter<Boolean> BEGGING = EntityDataManager.<Boolean>createKey(EntityWolf.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Integer> COLLAR_COLOR = EntityDataManager.<Integer>createKey(EntityWolf.class, DataSerializers.VARINT);

    /** Float used to smooth the rotation of the wolf head */
    private float headRotationCourse;
    private float headRotationCourseOld;

    /** true is the wolf is wet else false */
    private boolean isWet;

    /** True if the wolf is shaking else False */
    private boolean isShaking;

    /**
     * This time increases while wolf is shaking and emitting water particles.
     */
    private float timeWolfIsShaking;
    private float prevTimeWolfIsShaking;

    public EntityWolf(World worldIn)
    {
        super(worldIn);
        this.setSize(0.6F, 0.85F);
        this.setTamed(false);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAISwimming(this));
        this.tasks.addTask(2, this.aiSit = new EntityAISit(this));
        this.tasks.addTask(3, new EntityAILeapAtTarget(this, 0.4F));
        this.tasks.addTask(4, new EntityAIAttackMelee(this, 1.0D, true));
        this.tasks.addTask(5, new EntityAIFollowOwner(this, 1.0D, 10.0F, 2.0F));
        this.tasks.addTask(6, new EntityAIMate(this, 1.0D));
        this.tasks.addTask(7, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(8, new EntityAIBeg(this, 8.0F));
        this.tasks.addTask(9, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(9, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAIOwnerHurtByTarget(this));
        this.targetTasks.addTask(2, new EntityAIOwnerHurtTarget(this));
        this.targetTasks.addTask(3, new EntityAIHurtByTarget(this, true, new Class[0]));
        this.targetTasks.addTask(4, new EntityAITargetNonTamed(this, EntityAnimal.class, false, new Predicate<Entity>()
        {
            public boolean apply(Entity p_apply_1_)
            {
                return p_apply_1_ instanceof EntitySheep || p_apply_1_ instanceof EntityRabbit;
            }
        }));
        this.targetTasks.addTask(5, new EntityAINearestAttackableTarget(this, EntitySkeleton.class, false));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.30000001192092896D);

        if (this.isTamed())
        {
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(20.0D);
        }
        else
        {
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(8.0D);
        }

        this.getAttributeMap().registerAttribute(SharedMonsterAttributes.ATTACK_DAMAGE).setBaseValue(2.0D);
    }

    /**
     * Sets the active target the Task system uses for tracking
     */
    public void setAttackTarget(EntityLivingBase entitylivingbaseIn)
    {
        super.setAttackTarget(entitylivingbaseIn);

        if (entitylivingbaseIn == null)
        {
            this.setAngry(false);
        }
        else if (!this.isTamed())
        {
            this.setAngry(true);
        }
    }

    protected void updateAITasks()
    {
        this.dataWatcher.set(field_184759_bz, Float.valueOf(this.getHealth()));
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(field_184759_bz, Float.valueOf(this.getHealth()));
        this.dataWatcher.register(BEGGING, Boolean.valueOf(false));
        this.dataWatcher.register(COLLAR_COLOR, Integer.valueOf(EnumDyeColor.RED.getDyeDamage()));
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        this.playSound(SoundEvents.entity_wolf_step, 0.15F, 1.0F);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setBoolean("Angry", this.isAngry());
        tagCompound.setByte("CollarColor", (byte)this.getCollarColor().getDyeDamage());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.setAngry(tagCompund.getBoolean("Angry"));

        if (tagCompund.hasKey("CollarColor", 99))
        {
            this.setCollarColor(EnumDyeColor.byDyeDamage(tagCompund.getByte("CollarColor")));
        }
    }

    protected SoundEvent getAmbientSound()
    {
        return this.isAngry() ? SoundEvents.entity_wolf_growl : (this.rand.nextInt(3) == 0 ? (this.isTamed() && ((Float)this.dataWatcher.get(field_184759_bz)).floatValue() < 10.0F ? SoundEvents.entity_wolf_whine : SoundEvents.entity_wolf_pant) : SoundEvents.entity_wolf_ambient);
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_wolf_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_wolf_death;
    }

    /**
     * Returns the volume for the sounds this mob makes.
     */
    protected float getSoundVolume()
    {
        return 0.4F;
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_WOLF;
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        super.onLivingUpdate();

        if (!this.worldObj.isRemote && this.isWet && !this.isShaking && !this.hasPath() && this.onGround)
        {
            this.isShaking = true;
            this.timeWolfIsShaking = 0.0F;
            this.prevTimeWolfIsShaking = 0.0F;
            this.worldObj.setEntityState(this, (byte)8);
        }

        if (!this.worldObj.isRemote && this.getAttackTarget() == null && this.isAngry())
        {
            this.setAngry(false);
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();
        this.headRotationCourseOld = this.headRotationCourse;

        if (this.isBegging())
        {
            this.headRotationCourse += (1.0F - this.headRotationCourse) * 0.4F;
        }
        else
        {
            this.headRotationCourse += (0.0F - this.headRotationCourse) * 0.4F;
        }

        if (this.isWet())
        {
            this.isWet = true;
            this.isShaking = false;
            this.timeWolfIsShaking = 0.0F;
            this.prevTimeWolfIsShaking = 0.0F;
        }
        else if ((this.isWet || this.isShaking) && this.isShaking)
        {
            if (this.timeWolfIsShaking == 0.0F)
            {
                this.playSound(SoundEvents.entity_wolf_shake, this.getSoundVolume(), (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
            }

            this.prevTimeWolfIsShaking = this.timeWolfIsShaking;
            this.timeWolfIsShaking += 0.05F;

            if (this.prevTimeWolfIsShaking >= 2.0F)
            {
                this.isWet = false;
                this.isShaking = false;
                this.prevTimeWolfIsShaking = 0.0F;
                this.timeWolfIsShaking = 0.0F;
            }

            if (this.timeWolfIsShaking > 0.4F)
            {
                float f = (float)this.getEntityBoundingBox().minY;
                int i = (int)(MathHelper.sin((this.timeWolfIsShaking - 0.4F) * (float)Math.PI) * 7.0F);

                for (int j = 0; j < i; ++j)
                {
                    float f1 = (this.rand.nextFloat() * 2.0F - 1.0F) * this.width * 0.5F;
                    float f2 = (this.rand.nextFloat() * 2.0F - 1.0F) * this.width * 0.5F;
                    this.worldObj.spawnParticle(EnumParticleTypes.WATER_SPLASH, this.posX + (double)f1, (double)(f + 0.8F), this.posZ + (double)f2, this.motionX, this.motionY, this.motionZ, new int[0]);
                }
            }
        }
    }

    /**
     * True if the wolf is wet
     */
    public boolean isWolfWet()
    {
        return this.isWet;
    }

    /**
     * Used when calculating the amount of shading to apply while the wolf is wet.
     */
    public float getShadingWhileWet(float p_70915_1_)
    {
        return 0.75F + (this.prevTimeWolfIsShaking + (this.timeWolfIsShaking - this.prevTimeWolfIsShaking) * p_70915_1_) / 2.0F * 0.25F;
    }

    public float getShakeAngle(float p_70923_1_, float p_70923_2_)
    {
        float f = (this.prevTimeWolfIsShaking + (this.timeWolfIsShaking - this.prevTimeWolfIsShaking) * p_70923_1_ + p_70923_2_) / 1.8F;

        if (f < 0.0F)
        {
            f = 0.0F;
        }
        else if (f > 1.0F)
        {
            f = 1.0F;
        }

        return MathHelper.sin(f * (float)Math.PI) * MathHelper.sin(f * (float)Math.PI * 11.0F) * 0.15F * (float)Math.PI;
    }

    public float getInterestedAngle(float p_70917_1_)
    {
        return (this.headRotationCourseOld + (this.headRotationCourse - this.headRotationCourseOld) * p_70917_1_) * 0.15F * (float)Math.PI;
    }

    public float getEyeHeight()
    {
        return this.height * 0.8F;
    }

    /**
     * The speed it takes to move the entityliving's rotationPitch through the faceEntity method. This is only currently
     * use in wolves.
     */
    public int getVerticalFaceSpeed()
    {
        return this.isSitting() ? 20 : super.getVerticalFaceSpeed();
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
            Entity entity = source.getEntity();

            if (this.aiSit != null)
            {
                this.aiSit.setSitting(false);
            }

            if (entity != null && !(entity instanceof EntityPlayer) && !(entity instanceof EntityArrow))
            {
                amount = (amount + 1.0F) / 2.0F;
            }

            return super.attackEntityFrom(source, amount);
        }
    }

    public boolean attackEntityAsMob(Entity entityIn)
    {
        boolean flag = entityIn.attackEntityFrom(DamageSource.causeMobDamage(this), (float)((int)this.getEntityAttribute(SharedMonsterAttributes.ATTACK_DAMAGE).getAttributeValue()));

        if (flag)
        {
            this.applyEnchantments(this, entityIn);
        }

        return flag;
    }

    public void setTamed(boolean tamed)
    {
        super.setTamed(tamed);

        if (tamed)
        {
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(20.0D);
        }
        else
        {
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(8.0D);
        }

        this.getEntityAttribute(SharedMonsterAttributes.ATTACK_DAMAGE).setBaseValue(4.0D);
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (this.isTamed())
        {
            if (p_184645_3_ != null)
            {
                if (p_184645_3_.getItem() instanceof ItemFood)
                {
                    ItemFood itemfood = (ItemFood)p_184645_3_.getItem();

                    if (itemfood.isWolfsFavoriteMeat() && ((Float)this.dataWatcher.get(field_184759_bz)).floatValue() < 20.0F)
                    {
                        if (!p_184645_1_.capabilities.isCreativeMode)
                        {
                            --p_184645_3_.stackSize;
                        }

                        this.heal((float)itemfood.getHealAmount(p_184645_3_));
                        return true;
                    }
                }
                else if (p_184645_3_.getItem() == Items.dye)
                {
                    EnumDyeColor enumdyecolor = EnumDyeColor.byDyeDamage(p_184645_3_.getMetadata());

                    if (enumdyecolor != this.getCollarColor())
                    {
                        this.setCollarColor(enumdyecolor);

                        if (!p_184645_1_.capabilities.isCreativeMode)
                        {
                            --p_184645_3_.stackSize;
                        }

                        return true;
                    }
                }
            }

            if (this.isOwner(p_184645_1_) && !this.worldObj.isRemote && !this.isBreedingItem(p_184645_3_))
            {
                this.aiSit.setSitting(!this.isSitting());
                this.isJumping = false;
                this.navigator.clearPathEntity();
                this.setAttackTarget((EntityLivingBase)null);
            }
        }
        else if (p_184645_3_ != null && p_184645_3_.getItem() == Items.bone && !this.isAngry())
        {
            if (!p_184645_1_.capabilities.isCreativeMode)
            {
                --p_184645_3_.stackSize;
            }

            if (!this.worldObj.isRemote)
            {
                if (this.rand.nextInt(3) == 0)
                {
                    this.setTamed(true);
                    this.navigator.clearPathEntity();
                    this.setAttackTarget((EntityLivingBase)null);
                    this.aiSit.setSitting(true);
                    this.setHealth(20.0F);
                    this.setOwnerId(p_184645_1_.getUniqueID());
                    this.playTameEffect(true);
                    this.worldObj.setEntityState(this, (byte)7);
                }
                else
                {
                    this.playTameEffect(false);
                    this.worldObj.setEntityState(this, (byte)6);
                }
            }

            return true;
        }

        return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 8)
        {
            this.isShaking = true;
            this.timeWolfIsShaking = 0.0F;
            this.prevTimeWolfIsShaking = 0.0F;
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    public float getTailRotation()
    {
        return this.isAngry() ? 1.5393804F : (this.isTamed() ? (0.55F - (20.0F - ((Float)this.dataWatcher.get(field_184759_bz)).floatValue()) * 0.02F) * (float)Math.PI : ((float)Math.PI / 5F));
    }

    /**
     * Checks if the parameter is an item which this animal can be fed to breed it (wheat, carrots or seeds depending on
     * the animal type)
     */
    public boolean isBreedingItem(ItemStack stack)
    {
        return stack == null ? false : (!(stack.getItem() instanceof ItemFood) ? false : ((ItemFood)stack.getItem()).isWolfsFavoriteMeat());
    }

    /**
     * Will return how many at most can spawn in a chunk at once.
     */
    public int getMaxSpawnedInChunk()
    {
        return 8;
    }

    /**
     * Determines whether this wolf is angry or not.
     */
    public boolean isAngry()
    {
        return (((Byte)this.dataWatcher.get(TAMED)).byteValue() & 2) != 0;
    }

    /**
     * Sets whether this wolf is angry or not.
     */
    public void setAngry(boolean angry)
    {
        byte b0 = ((Byte)this.dataWatcher.get(TAMED)).byteValue();

        if (angry)
        {
            this.dataWatcher.set(TAMED, Byte.valueOf((byte)(b0 | 2)));
        }
        else
        {
            this.dataWatcher.set(TAMED, Byte.valueOf((byte)(b0 & -3)));
        }
    }

    public EnumDyeColor getCollarColor()
    {
        return EnumDyeColor.byDyeDamage(((Integer)this.dataWatcher.get(COLLAR_COLOR)).intValue() & 15);
    }

    public void setCollarColor(EnumDyeColor collarcolor)
    {
        this.dataWatcher.set(COLLAR_COLOR, Integer.valueOf(collarcolor.getDyeDamage()));
    }

    public EntityWolf createChild(EntityAgeable ageable)
    {
        EntityWolf entitywolf = new EntityWolf(this.worldObj);
        UUID uuid = this.getOwnerId();

        if (uuid != null)
        {
            entitywolf.setOwnerId(uuid);
            entitywolf.setTamed(true);
        }

        return entitywolf;
    }

    public void setBegging(boolean beg)
    {
        this.dataWatcher.set(BEGGING, Boolean.valueOf(beg));
    }

    /**
     * Returns true if the mob is currently able to mate with the specified mob.
     */
    public boolean canMateWith(EntityAnimal otherAnimal)
    {
        if (otherAnimal == this)
        {
            return false;
        }
        else if (!this.isTamed())
        {
            return false;
        }
        else if (!(otherAnimal instanceof EntityWolf))
        {
            return false;
        }
        else
        {
            EntityWolf entitywolf = (EntityWolf)otherAnimal;
            return !entitywolf.isTamed() ? false : (entitywolf.isSitting() ? false : this.isInLove() && entitywolf.isInLove());
        }
    }

    public boolean isBegging()
    {
        return ((Boolean)this.dataWatcher.get(BEGGING)).booleanValue();
    }

    /**
     * Determines if an entity can be despawned, used on idle far away entities
     */
    protected boolean canDespawn()
    {
        return !this.isTamed() && this.ticksExisted > 2400;
    }

    public boolean shouldAttackEntity(EntityLivingBase p_142018_1_, EntityLivingBase p_142018_2_)
    {
        if (!(p_142018_1_ instanceof EntityCreeper) && !(p_142018_1_ instanceof EntityGhast))
        {
            if (p_142018_1_ instanceof EntityWolf)
            {
                EntityWolf entitywolf = (EntityWolf)p_142018_1_;

                if (entitywolf.isTamed() && entitywolf.getOwner() == p_142018_2_)
                {
                    return false;
                }
            }

            return p_142018_1_ instanceof EntityPlayer && p_142018_2_ instanceof EntityPlayer && !((EntityPlayer)p_142018_2_).canAttackPlayer((EntityPlayer)p_142018_1_) ? false : !(p_142018_1_ instanceof EntityHorse) || !((EntityHorse)p_142018_1_).isTame();
        }
        else
        {
            return false;
        }
    }

    public boolean func_184652_a(EntityPlayer p_184652_1_)
    {
        return !this.isAngry() && super.func_184652_a(p_184652_1_);
    }
}
