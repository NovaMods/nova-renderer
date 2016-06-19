package net.minecraft.entity.monster;

import java.util.List;
import java.util.UUID;
import net.minecraft.block.material.Material;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IRangedAttackMob;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackRanged;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityPotion;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.init.PotionTypes;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.potion.PotionEffect;
import net.minecraft.potion.PotionType;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityWitch extends EntityMob implements IRangedAttackMob
{
    private static final UUID MODIFIER_UUID = UUID.fromString("5CD17E52-A79A-43D3-A529-90FDE04B181E");
    private static final AttributeModifier MODIFIER = (new AttributeModifier(MODIFIER_UUID, "Drinking speed penalty", -0.25D, 0)).setSaved(false);
    private static final DataParameter<Boolean> IS_AGGRESSIVE = EntityDataManager.<Boolean>createKey(EntityWitch.class, DataSerializers.BOOLEAN);

    /**
     * Timer used as interval for a witch's attack, decremented every tick if aggressive and when reaches zero the witch
     * will throw a potion at the target entity.
     */
    private int witchAttackTimer;

    public EntityWitch(World worldIn)
    {
        super(worldIn);
        this.setSize(0.6F, 1.95F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAISwimming(this));
        this.tasks.addTask(2, new EntityAIAttackRanged(this, 1.0D, 60, 10.0F));
        this.tasks.addTask(2, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(3, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(3, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAIHurtByTarget(this, false, new Class[0]));
        this.targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, EntityPlayer.class, true));
    }

    protected void entityInit()
    {
        super.entityInit();
        this.getDataManager().register(IS_AGGRESSIVE, Boolean.valueOf(false));
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_witch_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_witch_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_witch_death;
    }

    /**
     * Set whether this witch is aggressive at an entity.
     */
    public void setAggressive(boolean aggressive)
    {
        this.getDataManager().set(IS_AGGRESSIVE, Boolean.valueOf(aggressive));
    }

    public boolean func_184730_o()
    {
        return ((Boolean)this.getDataManager().get(IS_AGGRESSIVE)).booleanValue();
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(26.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.25D);
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (!this.worldObj.isRemote)
        {
            if (this.func_184730_o())
            {
                if (this.witchAttackTimer-- <= 0)
                {
                    this.setAggressive(false);
                    ItemStack itemstack = this.getHeldItemMainhand();
                    this.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, (ItemStack)null);

                    if (itemstack != null && itemstack.getItem() == Items.potionitem)
                    {
                        List<PotionEffect> list = PotionUtils.getEffectsFromStack(itemstack);

                        if (list != null)
                        {
                            for (PotionEffect potioneffect : list)
                            {
                                this.addPotionEffect(new PotionEffect(potioneffect));
                            }
                        }
                    }

                    this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).removeModifier(MODIFIER);
                }
            }
            else
            {
                PotionType potiontype = null;

                if (this.rand.nextFloat() < 0.15F && this.isInsideOfMaterial(Material.water) && !this.isPotionActive(MobEffects.waterBreathing))
                {
                    potiontype = PotionTypes.water_breathing;
                }
                else if (this.rand.nextFloat() < 0.15F && this.isBurning() && !this.isPotionActive(MobEffects.fireResistance))
                {
                    potiontype = PotionTypes.fire_resistance;
                }
                else if (this.rand.nextFloat() < 0.05F && this.getHealth() < this.getMaxHealth())
                {
                    potiontype = PotionTypes.healing;
                }
                else if (this.rand.nextFloat() < 0.5F && this.getAttackTarget() != null && !this.isPotionActive(MobEffects.moveSpeed) && this.getAttackTarget().getDistanceSqToEntity(this) > 121.0D)
                {
                    potiontype = PotionTypes.swiftness;
                }

                if (potiontype != null)
                {
                    this.setItemStackToSlot(EntityEquipmentSlot.MAINHAND, PotionUtils.func_185188_a(new ItemStack(Items.potionitem), potiontype));
                    this.witchAttackTimer = this.getHeldItemMainhand().getMaxItemUseDuration();
                    this.setAggressive(true);
                    this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, SoundEvents.entity_witch_drink, this.getSoundCategory(), 1.0F, 0.8F + this.rand.nextFloat() * 0.4F);
                    IAttributeInstance iattributeinstance = this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED);
                    iattributeinstance.removeModifier(MODIFIER);
                    iattributeinstance.applyModifier(MODIFIER);
                }
            }

            if (this.rand.nextFloat() < 7.5E-4F)
            {
                this.worldObj.setEntityState(this, (byte)15);
            }
        }

        super.onLivingUpdate();
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 15)
        {
            for (int i = 0; i < this.rand.nextInt(35) + 10; ++i)
            {
                this.worldObj.spawnParticle(EnumParticleTypes.SPELL_WITCH, this.posX + this.rand.nextGaussian() * 0.12999999523162842D, this.getEntityBoundingBox().maxY + 0.5D + this.rand.nextGaussian() * 0.12999999523162842D, this.posZ + this.rand.nextGaussian() * 0.12999999523162842D, 0.0D, 0.0D, 0.0D, new int[0]);
            }
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    /**
     * Reduces damage, depending on potions
     */
    protected float applyPotionDamageCalculations(DamageSource source, float damage)
    {
        damage = super.applyPotionDamageCalculations(source, damage);

        if (source.getEntity() == this)
        {
            damage = 0.0F;
        }

        if (source.isMagicDamage())
        {
            damage = (float)((double)damage * 0.15D);
        }

        return damage;
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_WITCH;
    }

    /**
     * Attack the specified entity using a ranged attack.
     */
    public void attackEntityWithRangedAttack(EntityLivingBase target, float p_82196_2_)
    {
        if (!this.func_184730_o())
        {
            double d0 = target.posY + (double)target.getEyeHeight() - 1.100000023841858D;
            double d1 = target.posX + target.motionX - this.posX;
            double d2 = d0 - this.posY;
            double d3 = target.posZ + target.motionZ - this.posZ;
            float f = MathHelper.sqrt_double(d1 * d1 + d3 * d3);
            PotionType potiontype = PotionTypes.harming;

            if (f >= 8.0F && !target.isPotionActive(MobEffects.moveSlowdown))
            {
                potiontype = PotionTypes.slowness;
            }
            else if (target.getHealth() >= 8.0F && !target.isPotionActive(MobEffects.poison))
            {
                potiontype = PotionTypes.poison;
            }
            else if (f <= 3.0F && !target.isPotionActive(MobEffects.weakness) && this.rand.nextFloat() < 0.25F)
            {
                potiontype = PotionTypes.weakness;
            }

            EntityPotion entitypotion = new EntityPotion(this.worldObj, this, PotionUtils.func_185188_a(new ItemStack(Items.splash_potion), potiontype));
            entitypotion.rotationPitch -= -20.0F;
            entitypotion.setThrowableHeading(d1, d2 + (double)(f * 0.2F), d3, 0.75F, 8.0F);
            this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, SoundEvents.entity_witch_throw, this.getSoundCategory(), 1.0F, 0.8F + this.rand.nextFloat() * 0.4F);
            this.worldObj.spawnEntityInWorld(entitypotion);
        }
    }

    public float getEyeHeight()
    {
        return 1.62F;
    }
}
