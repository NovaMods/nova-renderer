package net.minecraft.entity.ai;

import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.effect.EntityLightningBolt;
import net.minecraft.entity.monster.EntitySkeleton;
import net.minecraft.entity.passive.EntityHorse;
import net.minecraft.entity.passive.HorseArmorType;
import net.minecraft.init.Items;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.DifficultyInstance;

public class EntityAISkeletonRiders extends EntityAIBase
{
    private final EntityHorse field_188516_a;

    public EntityAISkeletonRiders(EntityHorse p_i46797_1_)
    {
        this.field_188516_a = p_i46797_1_;
    }

    /**
     * Returns whether the EntityAIBase should begin execution.
     */
    public boolean shouldExecute()
    {
        return this.field_188516_a.worldObj.isAnyPlayerWithinRangeAt(this.field_188516_a.posX, this.field_188516_a.posY, this.field_188516_a.posZ, 10.0D);
    }

    /**
     * Updates the task
     */
    public void updateTask()
    {
        DifficultyInstance difficultyinstance = this.field_188516_a.worldObj.getDifficultyForLocation(new BlockPos(this.field_188516_a));
        this.field_188516_a.func_184784_x(false);
        this.field_188516_a.setType(HorseArmorType.SKELETON);
        this.field_188516_a.setHorseTamed(true);
        this.field_188516_a.setGrowingAge(0);
        this.field_188516_a.worldObj.addWeatherEffect(new EntityLightningBolt(this.field_188516_a.worldObj, this.field_188516_a.posX, this.field_188516_a.posY, this.field_188516_a.posZ, true));
        EntitySkeleton entityskeleton = this.func_188514_a(difficultyinstance, this.field_188516_a);
        entityskeleton.startRiding(this.field_188516_a);

        for (int i = 0; i < 3; ++i)
        {
            EntityHorse entityhorse = this.func_188515_a(difficultyinstance);
            EntitySkeleton entityskeleton1 = this.func_188514_a(difficultyinstance, entityhorse);
            entityskeleton1.startRiding(entityhorse);
            entityhorse.addVelocity(this.field_188516_a.getRNG().nextGaussian() * 0.5D, 0.0D, this.field_188516_a.getRNG().nextGaussian() * 0.5D);
        }
    }

    private EntityHorse func_188515_a(DifficultyInstance p_188515_1_)
    {
        EntityHorse entityhorse = new EntityHorse(this.field_188516_a.worldObj);
        entityhorse.onInitialSpawn(p_188515_1_, (IEntityLivingData)null);
        entityhorse.setPosition(this.field_188516_a.posX, this.field_188516_a.posY, this.field_188516_a.posZ);
        entityhorse.hurtResistantTime = 60;
        entityhorse.enablePersistence();
        entityhorse.setType(HorseArmorType.SKELETON);
        entityhorse.setHorseTamed(true);
        entityhorse.setGrowingAge(0);
        entityhorse.worldObj.spawnEntityInWorld(entityhorse);
        return entityhorse;
    }

    private EntitySkeleton func_188514_a(DifficultyInstance p_188514_1_, EntityHorse p_188514_2_)
    {
        EntitySkeleton entityskeleton = new EntitySkeleton(p_188514_2_.worldObj);
        entityskeleton.onInitialSpawn(p_188514_1_, (IEntityLivingData)null);
        entityskeleton.setPosition(p_188514_2_.posX, p_188514_2_.posY, p_188514_2_.posZ);
        entityskeleton.hurtResistantTime = 60;
        entityskeleton.enablePersistence();

        if (entityskeleton.getItemStackFromSlot(EntityEquipmentSlot.HEAD) == null)
        {
            entityskeleton.setItemStackToSlot(EntityEquipmentSlot.HEAD, new ItemStack(Items.iron_helmet));
        }

        EnchantmentHelper.addRandomEnchantment(entityskeleton.getRNG(), entityskeleton.getHeldItemMainhand(), (int)(5.0F + p_188514_1_.getClampedAdditionalDifficulty() * (float)entityskeleton.getRNG().nextInt(18)), false);
        EnchantmentHelper.addRandomEnchantment(entityskeleton.getRNG(), entityskeleton.getItemStackFromSlot(EntityEquipmentSlot.HEAD), (int)(5.0F + p_188514_1_.getClampedAdditionalDifficulty() * (float)entityskeleton.getRNG().nextInt(18)), false);
        entityskeleton.worldObj.spawnEntityInWorld(entityskeleton);
        return entityskeleton;
    }
}
