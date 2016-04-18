package net.minecraft.entity.monster;

import net.minecraft.entity.Entity;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackMelee;
import net.minecraft.entity.ai.EntityAIAvoidEntity;
import net.minecraft.entity.ai.EntityAICreeperSwell;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.effect.EntityLightningBolt;
import net.minecraft.entity.passive.EntityOcelot;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityCreeper extends EntityMob
{
    private static final DataParameter<Integer> STATE = EntityDataManager.<Integer>createKey(EntityCreeper.class, DataSerializers.VARINT);
    private static final DataParameter<Boolean> POWERED = EntityDataManager.<Boolean>createKey(EntityCreeper.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Boolean> IGNITED = EntityDataManager.<Boolean>createKey(EntityCreeper.class, DataSerializers.BOOLEAN);

    /**
     * Time when this creeper was last in an active state (Messed up code here, probably causes creeper animation to go
     * weird)
     */
    private int lastActiveTime;

    /**
     * The amount of time since the creeper was close enough to the player to ignite
     */
    private int timeSinceIgnited;
    private int fuseTime = 30;

    /** Explosion radius for this creeper. */
    private int explosionRadius = 3;
    private int field_175494_bm = 0;

    public EntityCreeper(World worldIn)
    {
        super(worldIn);
        this.setSize(0.6F, 1.7F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAISwimming(this));
        this.tasks.addTask(2, new EntityAICreeperSwell(this));
        this.tasks.addTask(3, new EntityAIAvoidEntity(this, EntityOcelot.class, 6.0F, 1.0D, 1.2D));
        this.tasks.addTask(4, new EntityAIAttackMelee(this, 1.0D, false));
        this.tasks.addTask(5, new EntityAIWander(this, 0.8D));
        this.tasks.addTask(6, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(6, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAINearestAttackableTarget(this, EntityPlayer.class, true));
        this.targetTasks.addTask(2, new EntityAIHurtByTarget(this, false, new Class[0]));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.25D);
    }

    /**
     * The maximum height from where the entity is alowed to jump (used in pathfinder)
     */
    public int getMaxFallHeight()
    {
        return this.getAttackTarget() == null ? 3 : 3 + (int)(this.getHealth() - 1.0F);
    }

    public void fall(float distance, float damageMultiplier)
    {
        super.fall(distance, damageMultiplier);
        this.timeSinceIgnited = (int)((float)this.timeSinceIgnited + distance * 1.5F);

        if (this.timeSinceIgnited > this.fuseTime - 5)
        {
            this.timeSinceIgnited = this.fuseTime - 5;
        }
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(STATE, Integer.valueOf(-1));
        this.dataWatcher.register(POWERED, Boolean.valueOf(false));
        this.dataWatcher.register(IGNITED, Boolean.valueOf(false));
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);

        if (((Boolean)this.dataWatcher.get(POWERED)).booleanValue())
        {
            tagCompound.setBoolean("powered", true);
        }

        tagCompound.setShort("Fuse", (short)this.fuseTime);
        tagCompound.setByte("ExplosionRadius", (byte)this.explosionRadius);
        tagCompound.setBoolean("ignited", this.hasIgnited());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.dataWatcher.set(POWERED, Boolean.valueOf(tagCompund.getBoolean("powered")));

        if (tagCompund.hasKey("Fuse", 99))
        {
            this.fuseTime = tagCompund.getShort("Fuse");
        }

        if (tagCompund.hasKey("ExplosionRadius", 99))
        {
            this.explosionRadius = tagCompund.getByte("ExplosionRadius");
        }

        if (tagCompund.getBoolean("ignited"))
        {
            this.ignite();
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        if (this.isEntityAlive())
        {
            this.lastActiveTime = this.timeSinceIgnited;

            if (this.hasIgnited())
            {
                this.setCreeperState(1);
            }

            int i = this.getCreeperState();

            if (i > 0 && this.timeSinceIgnited == 0)
            {
                this.playSound(SoundEvents.entity_creeper_primed, 1.0F, 0.5F);
            }

            this.timeSinceIgnited += i;

            if (this.timeSinceIgnited < 0)
            {
                this.timeSinceIgnited = 0;
            }

            if (this.timeSinceIgnited >= this.fuseTime)
            {
                this.timeSinceIgnited = this.fuseTime;
                this.explode();
            }
        }

        super.onUpdate();
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_creeper_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_creeper_death;
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        super.onDeath(cause);

        if (this.worldObj.getGameRules().getBoolean("doMobLoot"))
        {
            if (cause.getEntity() instanceof EntitySkeleton)
            {
                int i = Item.getIdFromItem(Items.record_13);
                int j = Item.getIdFromItem(Items.record_wait);
                int k = i + this.rand.nextInt(j - i + 1);
                this.dropItem(Item.getItemById(k), 1);
            }
            else if (cause.getEntity() instanceof EntityCreeper && cause.getEntity() != this && ((EntityCreeper)cause.getEntity()).getPowered() && ((EntityCreeper)cause.getEntity()).isAIEnabled())
            {
                ((EntityCreeper)cause.getEntity()).func_175493_co();
                this.entityDropItem(new ItemStack(Items.skull, 1, 4), 0.0F);
            }
        }
    }

    public boolean attackEntityAsMob(Entity entityIn)
    {
        return true;
    }

    /**
     * Returns true if the creeper is powered by a lightning bolt.
     */
    public boolean getPowered()
    {
        return ((Boolean)this.dataWatcher.get(POWERED)).booleanValue();
    }

    /**
     * Params: (Float)Render tick. Returns the intensity of the creeper's flash when it is ignited.
     */
    public float getCreeperFlashIntensity(float p_70831_1_)
    {
        return ((float)this.lastActiveTime + (float)(this.timeSinceIgnited - this.lastActiveTime) * p_70831_1_) / (float)(this.fuseTime - 2);
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_CREEPER;
    }

    /**
     * Returns the current state of creeper, -1 is idle, 1 is 'in fuse'
     */
    public int getCreeperState()
    {
        return ((Integer)this.dataWatcher.get(STATE)).intValue();
    }

    /**
     * Sets the state of creeper, -1 to idle and 1 to be 'in fuse'
     */
    public void setCreeperState(int state)
    {
        this.dataWatcher.set(STATE, Integer.valueOf(state));
    }

    /**
     * Called when a lightning bolt hits the entity.
     */
    public void onStruckByLightning(EntityLightningBolt lightningBolt)
    {
        super.onStruckByLightning(lightningBolt);
        this.dataWatcher.set(POWERED, Boolean.valueOf(true));
    }

    protected boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (p_184645_3_ != null && p_184645_3_.getItem() == Items.flint_and_steel)
        {
            this.worldObj.func_184148_a(p_184645_1_, this.posX, this.posY, this.posZ, SoundEvents.item_flintandsteel_use, this.getSoundCategory(), 1.0F, this.rand.nextFloat() * 0.4F + 0.8F);
            p_184645_1_.swingArm(p_184645_2_);

            if (!this.worldObj.isRemote)
            {
                this.ignite();
                p_184645_3_.damageItem(1, p_184645_1_);
                return true;
            }
        }

        return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
    }

    /**
     * Creates an explosion as determined by this creeper's power and explosion radius.
     */
    private void explode()
    {
        if (!this.worldObj.isRemote)
        {
            boolean flag = this.worldObj.getGameRules().getBoolean("mobGriefing");
            float f = this.getPowered() ? 2.0F : 1.0F;
            this.dead = true;
            this.worldObj.createExplosion(this, this.posX, this.posY, this.posZ, (float)this.explosionRadius * f, flag);
            this.setDead();
        }
    }

    public boolean hasIgnited()
    {
        return ((Boolean)this.dataWatcher.get(IGNITED)).booleanValue();
    }

    public void ignite()
    {
        this.dataWatcher.set(IGNITED, Boolean.valueOf(true));
    }

    /**
     * Returns true if the newer Entity AI code should be run
     */
    public boolean isAIEnabled()
    {
        return this.field_175494_bm < 1 && this.worldObj.getGameRules().getBoolean("doMobLoot");
    }

    public void func_175493_co()
    {
        ++this.field_175494_bm;
    }
}
