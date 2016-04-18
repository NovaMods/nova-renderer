package net.minecraft.entity.monster;

import java.util.UUID;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityCreature;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityPigZombie extends EntityZombie
{
    private static final UUID ATTACK_SPEED_BOOST_MODIFIER_UUID = UUID.fromString("49455A49-7EC5-45BA-B886-3B90B23A1718");
    private static final AttributeModifier ATTACK_SPEED_BOOST_MODIFIER = (new AttributeModifier(ATTACK_SPEED_BOOST_MODIFIER_UUID, "Attacking speed boost", 0.05D, 0)).setSaved(false);

    /** Above zero if this PigZombie is Angry. */
    private int angerLevel;

    /** A random delay until this PigZombie next makes a sound. */
    private int randomSoundDelay;
    private UUID angerTargetUUID;

    public EntityPigZombie(World worldIn)
    {
        super(worldIn);
        this.isImmuneToFire = true;
    }

    public void setRevengeTarget(EntityLivingBase livingBase)
    {
        super.setRevengeTarget(livingBase);

        if (livingBase != null)
        {
            this.angerTargetUUID = livingBase.getUniqueID();
        }
    }

    protected void applyEntityAI()
    {
        this.targetTasks.addTask(1, new EntityPigZombie.AIHurtByAggressor(this));
        this.targetTasks.addTask(2, new EntityPigZombie.AITargetAggressor(this));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(reinforcementChance).setBaseValue(0.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.23000000417232513D);
        this.getEntityAttribute(SharedMonsterAttributes.ATTACK_DAMAGE).setBaseValue(5.0D);
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();
    }

    protected void updateAITasks()
    {
        IAttributeInstance iattributeinstance = this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED);

        if (this.isAngry())
        {
            if (!this.isChild() && !iattributeinstance.hasModifier(ATTACK_SPEED_BOOST_MODIFIER))
            {
                iattributeinstance.applyModifier(ATTACK_SPEED_BOOST_MODIFIER);
            }

            --this.angerLevel;
        }
        else if (iattributeinstance.hasModifier(ATTACK_SPEED_BOOST_MODIFIER))
        {
            iattributeinstance.removeModifier(ATTACK_SPEED_BOOST_MODIFIER);
        }

        if (this.randomSoundDelay > 0 && --this.randomSoundDelay == 0)
        {
            this.playSound(SoundEvents.entity_zombie_pig_angry, this.getSoundVolume() * 2.0F, ((this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F) * 1.8F);
        }

        if (this.angerLevel > 0 && this.angerTargetUUID != null && this.getAITarget() == null)
        {
            EntityPlayer entityplayer = this.worldObj.getPlayerEntityByUUID(this.angerTargetUUID);
            this.setRevengeTarget(entityplayer);
            this.attackingPlayer = entityplayer;
            this.recentlyHit = this.getRevengeTimer();
        }

        super.updateAITasks();
    }

    /**
     * Checks if the entity's current position is a valid location to spawn this entity.
     */
    public boolean getCanSpawnHere()
    {
        return this.worldObj.getDifficulty() != EnumDifficulty.PEACEFUL;
    }

    /**
     * Checks that the entity is not colliding with any blocks / liquids
     */
    public boolean isNotColliding()
    {
        return this.worldObj.checkNoEntityCollision(this.getEntityBoundingBox(), this) && this.worldObj.func_184144_a(this, this.getEntityBoundingBox()).isEmpty() && !this.worldObj.isAnyLiquid(this.getEntityBoundingBox());
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setShort("Anger", (short)this.angerLevel);

        if (this.angerTargetUUID != null)
        {
            tagCompound.setString("HurtBy", this.angerTargetUUID.toString());
        }
        else
        {
            tagCompound.setString("HurtBy", "");
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.angerLevel = tagCompund.getShort("Anger");
        String s = tagCompund.getString("HurtBy");

        if (!s.isEmpty())
        {
            this.angerTargetUUID = UUID.fromString(s);
            EntityPlayer entityplayer = this.worldObj.getPlayerEntityByUUID(this.angerTargetUUID);
            this.setRevengeTarget(entityplayer);

            if (entityplayer != null)
            {
                this.attackingPlayer = entityplayer;
                this.recentlyHit = this.getRevengeTimer();
            }
        }
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

            if (entity instanceof EntityPlayer)
            {
                this.becomeAngryAt(entity);
            }

            return super.attackEntityFrom(source, amount);
        }
    }

    /**
     * Causes this PigZombie to become angry at the supplied Entity (which will be a player).
     */
    private void becomeAngryAt(Entity p_70835_1_)
    {
        this.angerLevel = 400 + this.rand.nextInt(400);
        this.randomSoundDelay = this.rand.nextInt(40);

        if (p_70835_1_ instanceof EntityLivingBase)
        {
            this.setRevengeTarget((EntityLivingBase)p_70835_1_);
        }
    }

    public boolean isAngry()
    {
        return this.angerLevel > 0;
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_zombie_pig_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_zombie_pig_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_zombie_pig_death;
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_ZOMBIE_PIGMAN;
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        return false;
    }

    /**
     * Gives armor or weapon for entity based on given DifficultyInstance
     */
    protected void setEquipmentBasedOnDifficulty(DifficultyInstance difficulty)
    {
        this.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, new ItemStack(Items.golden_sword));
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        super.onInitialSpawn(difficulty, livingdata);
        this.setToNotVillager();
        return livingdata;
    }

    static class AIHurtByAggressor extends EntityAIHurtByTarget
    {
        public AIHurtByAggressor(EntityPigZombie p_i45828_1_)
        {
            super(p_i45828_1_, true, new Class[0]);
        }

        protected void setEntityAttackTarget(EntityCreature creatureIn, EntityLivingBase entityLivingBaseIn)
        {
            super.setEntityAttackTarget(creatureIn, entityLivingBaseIn);

            if (creatureIn instanceof EntityPigZombie)
            {
                ((EntityPigZombie)creatureIn).becomeAngryAt(entityLivingBaseIn);
            }
        }
    }

    static class AITargetAggressor extends EntityAINearestAttackableTarget<EntityPlayer>
    {
        public AITargetAggressor(EntityPigZombie p_i45829_1_)
        {
            super(p_i45829_1_, EntityPlayer.class, true);
        }

        public boolean shouldExecute()
        {
            return ((EntityPigZombie)this.taskOwner).isAngry() && super.shouldExecute();
        }
    }
}
