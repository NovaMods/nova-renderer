package net.minecraft.entity.monster;

import java.util.Random;
import net.minecraft.block.Block;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.EnumCreatureAttribute;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackMelee;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILeapAtTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.MobEffects;
import net.minecraft.init.SoundEvents;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.pathfinding.PathNavigate;
import net.minecraft.pathfinding.PathNavigateClimber;
import net.minecraft.potion.Potion;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntitySpider extends EntityMob
{
    private static final DataParameter<Byte> field_184729_a = EntityDataManager.<Byte>createKey(EntitySpider.class, DataSerializers.BYTE);

    public EntitySpider(World worldIn)
    {
        super(worldIn);
        this.setSize(1.4F, 0.9F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAISwimming(this));
        this.tasks.addTask(3, new EntityAILeapAtTarget(this, 0.4F));
        this.tasks.addTask(4, new EntitySpider.AISpiderAttack(this));
        this.tasks.addTask(5, new EntityAIWander(this, 0.8D));
        this.tasks.addTask(6, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(6, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAIHurtByTarget(this, false, new Class[0]));
        this.targetTasks.addTask(2, new EntitySpider.AISpiderTarget(this, EntityPlayer.class));
        this.targetTasks.addTask(3, new EntitySpider.AISpiderTarget(this, EntityIronGolem.class));
    }

    /**
     * Returns the Y offset from the entity's position for any entity riding this one.
     */
    public double getMountedYOffset()
    {
        return (double)(this.height * 0.5F);
    }

    /**
     * Returns new PathNavigateGround instance
     */
    protected PathNavigate getNewNavigator(World worldIn)
    {
        return new PathNavigateClimber(this, worldIn);
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(field_184729_a, Byte.valueOf((byte)0));
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (!this.worldObj.isRemote)
        {
            this.setBesideClimbableBlock(this.isCollidedHorizontally);
        }
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(16.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.30000001192092896D);
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_spider_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_spider_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_spider_death;
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        this.playSound(SoundEvents.entity_spider_step, 0.15F, 1.0F);
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_SPIDER;
    }

    /**
     * returns true if this entity is by a ladder, false otherwise
     */
    public boolean isOnLadder()
    {
        return this.isBesideClimbableBlock();
    }

    /**
     * Sets the Entity inside a web block.
     */
    public void setInWeb()
    {
    }

    /**
     * Get this Entity's EnumCreatureAttribute
     */
    public EnumCreatureAttribute getCreatureAttribute()
    {
        return EnumCreatureAttribute.ARTHROPOD;
    }

    public boolean isPotionApplicable(PotionEffect potioneffectIn)
    {
        return potioneffectIn.func_188419_a() == MobEffects.poison ? false : super.isPotionApplicable(potioneffectIn);
    }

    /**
     * Returns true if the WatchableObject (Byte) is 0x01 otherwise returns false. The WatchableObject is updated using
     * setBesideClimableBlock.
     */
    public boolean isBesideClimbableBlock()
    {
        return (((Byte)this.dataWatcher.get(field_184729_a)).byteValue() & 1) != 0;
    }

    /**
     * Updates the WatchableObject (Byte) created in entityInit(), setting it to 0x01 if par1 is true or 0x00 if it is
     * false.
     */
    public void setBesideClimbableBlock(boolean p_70839_1_)
    {
        byte b0 = ((Byte)this.dataWatcher.get(field_184729_a)).byteValue();

        if (p_70839_1_)
        {
            b0 = (byte)(b0 | 1);
        }
        else
        {
            b0 = (byte)(b0 & -2);
        }

        this.dataWatcher.set(field_184729_a, Byte.valueOf(b0));
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        livingdata = super.onInitialSpawn(difficulty, livingdata);

        if (this.worldObj.rand.nextInt(100) == 0)
        {
            EntitySkeleton entityskeleton = new EntitySkeleton(this.worldObj);
            entityskeleton.setLocationAndAngles(this.posX, this.posY, this.posZ, this.rotationYaw, 0.0F);
            entityskeleton.onInitialSpawn(difficulty, (IEntityLivingData)null);
            this.worldObj.spawnEntityInWorld(entityskeleton);
            entityskeleton.startRiding(this);
        }

        if (livingdata == null)
        {
            livingdata = new EntitySpider.GroupData();

            if (this.worldObj.getDifficulty() == EnumDifficulty.HARD && this.worldObj.rand.nextFloat() < 0.1F * difficulty.getClampedAdditionalDifficulty())
            {
                ((EntitySpider.GroupData)livingdata).func_111104_a(this.worldObj.rand);
            }
        }

        if (livingdata instanceof EntitySpider.GroupData)
        {
            Potion potion = ((EntitySpider.GroupData)livingdata).field_188478_a;

            if (potion != null)
            {
                this.addPotionEffect(new PotionEffect(potion, Integer.MAX_VALUE));
            }
        }

        return livingdata;
    }

    public float getEyeHeight()
    {
        return 0.65F;
    }

    static class AISpiderAttack extends EntityAIAttackMelee
    {
        public AISpiderAttack(EntitySpider p_i46676_1_)
        {
            super(p_i46676_1_, 1.0D, true);
        }

        public boolean continueExecuting()
        {
            float f = this.attacker.getBrightness(1.0F);

            if (f >= 0.5F && this.attacker.getRNG().nextInt(100) == 0)
            {
                this.attacker.setAttackTarget((EntityLivingBase)null);
                return false;
            }
            else
            {
                return super.continueExecuting();
            }
        }

        protected double func_179512_a(EntityLivingBase attackTarget)
        {
            return (double)(4.0F + attackTarget.width);
        }
    }

    static class AISpiderTarget<T extends EntityLivingBase> extends EntityAINearestAttackableTarget<T>
    {
        public AISpiderTarget(EntitySpider spider, Class<T> classTarget)
        {
            super(spider, classTarget, true);
        }

        public boolean shouldExecute()
        {
            float f = this.taskOwner.getBrightness(1.0F);
            return f >= 0.5F ? false : super.shouldExecute();
        }
    }

    public static class GroupData implements IEntityLivingData
    {
        public Potion field_188478_a;

        public void func_111104_a(Random rand)
        {
            int i = rand.nextInt(5);

            if (i <= 1)
            {
                this.field_188478_a = MobEffects.moveSpeed;
            }
            else if (i <= 2)
            {
                this.field_188478_a = MobEffects.damageBoost;
            }
            else if (i <= 3)
            {
                this.field_188478_a = MobEffects.regeneration;
            }
            else if (i <= 4)
            {
                this.field_188478_a = MobEffects.invisibility;
            }
        }
    }
}
