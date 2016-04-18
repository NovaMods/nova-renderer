package net.minecraft.entity.monster;

import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityMagmaCube extends EntitySlime
{
    public EntityMagmaCube(World worldIn)
    {
        super(worldIn);
        this.isImmuneToFire = true;
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.20000000298023224D);
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

    protected void setSlimeSize(int size)
    {
        super.setSlimeSize(size);
        this.getEntityAttribute(SharedMonsterAttributes.ARMOR).setBaseValue((double)(size * 3));
    }

    public int getBrightnessForRender(float partialTicks)
    {
        return 15728880;
    }

    /**
     * Gets how bright this entity is.
     */
    public float getBrightness(float partialTicks)
    {
        return 1.0F;
    }

    protected EnumParticleTypes getParticleType()
    {
        return EnumParticleTypes.FLAME;
    }

    protected EntitySlime createInstance()
    {
        return new EntityMagmaCube(this.worldObj);
    }

    protected ResourceLocation func_184647_J()
    {
        return !this.func_189101_db() ? LootTableList.ENTITIES_MAGMA_CUBE : LootTableList.EMPTY;
    }

    /**
     * Returns true if the entity is on fire. Used by render to add the fire effect on rendering.
     */
    public boolean isBurning()
    {
        return false;
    }

    /**
     * Gets the amount of time the slime needs to wait between jumps.
     */
    protected int getJumpDelay()
    {
        return super.getJumpDelay() * 4;
    }

    protected void alterSquishAmount()
    {
        this.squishAmount *= 0.9F;
    }

    /**
     * Causes this entity to do an upwards motion (jumping).
     */
    protected void jump()
    {
        this.motionY = (double)(0.42F + (float)this.getSlimeSize() * 0.1F);
        this.isAirBorne = true;
    }

    protected void handleJumpLava()
    {
        this.motionY = (double)(0.22F + (float)this.getSlimeSize() * 0.05F);
        this.isAirBorne = true;
    }

    public void fall(float distance, float damageMultiplier)
    {
    }

    /**
     * Indicates weather the slime is able to damage the player (based upon the slime's size)
     */
    protected boolean canDamagePlayer()
    {
        return true;
    }

    /**
     * Gets the amount of damage dealt to the player when "attacked" by the slime.
     */
    protected int getAttackStrength()
    {
        return super.getAttackStrength() + 2;
    }

    protected SoundEvent getHurtSound()
    {
        return this.func_189101_db() ? SoundEvents.entity_small_magmacube_hurt : SoundEvents.entity_magmacube_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return this.func_189101_db() ? SoundEvents.entity_small_magmacube_death : SoundEvents.entity_magmacube_death;
    }

    protected SoundEvent func_184709_cY()
    {
        return this.func_189101_db() ? SoundEvents.entity_small_magmacube_squish : SoundEvents.entity_magmacube_squish;
    }

    protected SoundEvent func_184710_cZ()
    {
        return SoundEvents.entity_magmacube_jump;
    }
}
