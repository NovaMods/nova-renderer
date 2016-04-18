package net.minecraft.entity.passive;

import com.google.common.collect.Sets;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityAgeable;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIFollowParent;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAIMate;
import net.minecraft.entity.ai.EntityAIPanic;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAITempt;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.effect.EntityLightningBolt;
import net.minecraft.entity.monster.EntityPigZombie;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.stats.AchievementList;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityPig extends EntityAnimal
{
    private static final DataParameter<Boolean> SADDLED = EntityDataManager.<Boolean>createKey(EntityPig.class, DataSerializers.BOOLEAN);
    private static final Set<Item> field_184764_bw = Sets.newHashSet(new Item[] {Items.carrot, Items.potato, Items.beetroot});
    private boolean field_184765_bx;
    private int field_184766_bz;
    private int field_184767_bA;

    public EntityPig(World worldIn)
    {
        super(worldIn);
        this.setSize(0.9F, 0.9F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(0, new EntityAISwimming(this));
        this.tasks.addTask(1, new EntityAIPanic(this, 1.25D));
        this.tasks.addTask(3, new EntityAIMate(this, 1.0D));
        this.tasks.addTask(4, new EntityAITempt(this, 1.2D, Items.carrot_on_a_stick, false));
        this.tasks.addTask(4, new EntityAITempt(this, 1.2D, false, field_184764_bw));
        this.tasks.addTask(5, new EntityAIFollowParent(this, 1.1D));
        this.tasks.addTask(6, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(7, new EntityAIWatchClosest(this, EntityPlayer.class, 6.0F));
        this.tasks.addTask(8, new EntityAILookIdle(this));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(10.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.25D);
    }

    /**
     * For vehicles, the first passenger is generally considered the controller and "drives" the vehicle. For example,
     * Pigs, Horses, and Boats are generally "steered" by the controlling passenger.
     */
    public Entity getControllingPassenger()
    {
        return this.getPassengers().isEmpty() ? null : (Entity)this.getPassengers().get(0);
    }

    /**
     * returns true if all the conditions for steering the entity are met. For pigs, this is true if it is being ridden
     * by a player and the player is holding a carrot-on-a-stick
     */
    public boolean canBeSteered()
    {
        Entity entity = this.getControllingPassenger();

        if (!(entity instanceof EntityPlayer))
        {
            return false;
        }
        else
        {
            EntityPlayer entityplayer = (EntityPlayer)entity;
            ItemStack itemstack = entityplayer.getHeldItemMainhand();

            if (itemstack != null && itemstack.getItem() == Items.carrot_on_a_stick)
            {
                return true;
            }
            else
            {
                itemstack = entityplayer.getHeldItemOffhand();
                return itemstack != null && itemstack.getItem() == Items.carrot_on_a_stick;
            }
        }
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(SADDLED, Boolean.valueOf(false));
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setBoolean("Saddle", this.getSaddled());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.setSaddled(tagCompund.getBoolean("Saddle"));
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_pig_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_pig_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_pig_death;
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        this.playSound(SoundEvents.entity_pig_step, 0.15F, 1.0F);
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (!super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_))
        {
            if (this.getSaddled() && !this.worldObj.isRemote && !this.isBeingRidden())
            {
                p_184645_1_.startRiding(this);
                return true;
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
     * Drop the equipment for this entity.
     */
    protected void dropEquipment(boolean wasRecentlyHit, int lootingModifier)
    {
        super.dropEquipment(wasRecentlyHit, lootingModifier);

        if (this.getSaddled())
        {
            this.dropItem(Items.saddle, 1);
        }
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_PIG;
    }

    /**
     * Returns true if the pig is saddled.
     */
    public boolean getSaddled()
    {
        return ((Boolean)this.dataWatcher.get(SADDLED)).booleanValue();
    }

    /**
     * Set or remove the saddle of the pig.
     */
    public void setSaddled(boolean saddled)
    {
        if (saddled)
        {
            this.dataWatcher.set(SADDLED, Boolean.valueOf(true));
        }
        else
        {
            this.dataWatcher.set(SADDLED, Boolean.valueOf(false));
        }
    }

    /**
     * Called when a lightning bolt hits the entity.
     */
    public void onStruckByLightning(EntityLightningBolt lightningBolt)
    {
        if (!this.worldObj.isRemote && !this.isDead)
        {
            EntityPigZombie entitypigzombie = new EntityPigZombie(this.worldObj);
            entitypigzombie.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, new ItemStack(Items.golden_sword));
            entitypigzombie.setLocationAndAngles(this.posX, this.posY, this.posZ, this.rotationYaw, this.rotationPitch);
            entitypigzombie.setNoAI(this.isAIDisabled());

            if (this.hasCustomName())
            {
                entitypigzombie.setCustomNameTag(this.getCustomNameTag());
                entitypigzombie.setAlwaysRenderNameTag(this.getAlwaysRenderNameTag());
            }

            this.worldObj.spawnEntityInWorld(entitypigzombie);
            this.setDead();
        }
    }

    public void fall(float distance, float damageMultiplier)
    {
        super.fall(distance, damageMultiplier);

        if (distance > 5.0F)
        {
            for (EntityPlayer entityplayer : this.func_184180_b(EntityPlayer.class))
            {
                entityplayer.triggerAchievement(AchievementList.field_187993_u);
            }
        }
    }

    /**
     * Moves the entity based on the specified heading.  Args: strafe, forward
     */
    public void moveEntityWithHeading(float strafe, float forward)
    {
        Entity entity = this.getPassengers().isEmpty() ? null : (Entity)this.getPassengers().get(0);

        if (this.isBeingRidden() && this.canBeSteered())
        {
            this.prevRotationYaw = this.rotationYaw = entity.rotationYaw;
            this.rotationPitch = entity.rotationPitch * 0.5F;
            this.setRotation(this.rotationYaw, this.rotationPitch);
            this.rotationYawHead = this.renderYawOffset = this.rotationYaw;
            this.stepHeight = 1.0F;
            this.jumpMovementFactor = this.getAIMoveSpeed() * 0.1F;

            if (this.func_184186_bw())
            {
                float f = (float)this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).getAttributeValue() * 0.225F;

                if (this.field_184765_bx)
                {
                    if (this.field_184766_bz++ > this.field_184767_bA)
                    {
                        this.field_184765_bx = false;
                    }

                    f += f * 1.15F * MathHelper.sin((float)this.field_184766_bz / (float)this.field_184767_bA * (float)Math.PI);
                }

                this.setAIMoveSpeed(f);
                super.moveEntityWithHeading(0.0F, 1.0F);
            }
            else
            {
                this.motionX = 0.0D;
                this.motionY = 0.0D;
                this.motionZ = 0.0D;
            }

            this.field_184618_aE = this.limbSwingAmount;
            double d1 = this.posX - this.prevPosX;
            double d0 = this.posZ - this.prevPosZ;
            float f1 = MathHelper.sqrt_double(d1 * d1 + d0 * d0) * 4.0F;

            if (f1 > 1.0F)
            {
                f1 = 1.0F;
            }

            this.limbSwingAmount += (f1 - this.limbSwingAmount) * 0.4F;
            this.field_184619_aG += this.limbSwingAmount;
        }
        else
        {
            this.stepHeight = 0.5F;
            this.jumpMovementFactor = 0.02F;
            super.moveEntityWithHeading(strafe, forward);
        }
    }

    public boolean func_184762_da()
    {
        if (this.field_184765_bx)
        {
            return false;
        }
        else
        {
            this.field_184765_bx = true;
            this.field_184766_bz = 0;
            this.field_184767_bA = this.getRNG().nextInt(841) + 140;
            return true;
        }
    }

    public EntityPig createChild(EntityAgeable ageable)
    {
        return new EntityPig(this.worldObj);
    }

    /**
     * Checks if the parameter is an item which this animal can be fed to breed it (wheat, carrots or seeds depending on
     * the animal type)
     */
    public boolean isBreedingItem(ItemStack stack)
    {
        return stack != null && field_184764_bw.contains(stack.getItem());
    }
}
