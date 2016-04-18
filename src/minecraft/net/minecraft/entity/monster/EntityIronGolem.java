package net.minecraft.entity.monster;

import com.google.common.base.Predicate;
import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackMelee;
import net.minecraft.entity.ai.EntityAIDefendVillage;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILookAtVillager;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAIMoveThroughVillage;
import net.minecraft.entity.ai.EntityAIMoveTowardsRestriction;
import net.minecraft.entity.ai.EntityAIMoveTowardsTarget;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.village.Village;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityIronGolem extends EntityGolem
{
    protected static final DataParameter<Byte> PLAYER_CREATED = EntityDataManager.<Byte>createKey(EntityIronGolem.class, DataSerializers.BYTE);

    /** deincrements, and a distance-to-home check is done at 0 */
    private int homeCheckTimer;
    Village villageObj;
    private int attackTimer;
    private int holdRoseTick;

    public EntityIronGolem(World worldIn)
    {
        super(worldIn);
        this.setSize(1.4F, 2.7F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAIAttackMelee(this, 1.0D, true));
        this.tasks.addTask(2, new EntityAIMoveTowardsTarget(this, 0.9D, 32.0F));
        this.tasks.addTask(3, new EntityAIMoveThroughVillage(this, 0.6D, true));
        this.tasks.addTask(4, new EntityAIMoveTowardsRestriction(this, 1.0D));
        this.tasks.addTask(5, new EntityAILookAtVillager(this));
        this.tasks.addTask(6, new EntityAIWander(this, 0.6D));
        this.tasks.addTask(7, new EntityAIWatchClosest(this, EntityPlayer.class, 6.0F));
        this.tasks.addTask(8, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAIDefendVillage(this));
        this.targetTasks.addTask(2, new EntityAIHurtByTarget(this, false, new Class[0]));
        this.targetTasks.addTask(3, new EntityAINearestAttackableTarget(this, EntityLiving.class, 10, false, true, new Predicate<EntityLiving>()
        {
            public boolean apply(EntityLiving p_apply_1_)
            {
                return p_apply_1_ != null && IMob.VISIBLE_MOB_SELECTOR.apply(p_apply_1_) && !(p_apply_1_ instanceof EntityCreeper);
            }
        }));
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(PLAYER_CREATED, Byte.valueOf((byte)0));
    }

    protected void updateAITasks()
    {
        if (--this.homeCheckTimer <= 0)
        {
            this.homeCheckTimer = 70 + this.rand.nextInt(50);
            this.villageObj = this.worldObj.getVillageCollection().getNearestVillage(new BlockPos(this), 32);

            if (this.villageObj == null)
            {
                this.detachHome();
            }
            else
            {
                BlockPos blockpos = this.villageObj.getCenter();
                this.setHomePosAndDistance(blockpos, (int)((float)this.villageObj.getVillageRadius() * 0.6F));
            }
        }

        super.updateAITasks();
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(100.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.25D);
        this.getEntityAttribute(SharedMonsterAttributes.KNOCKBACK_RESISTANCE).setBaseValue(1.0D);
    }

    /**
     * Decrements the entity's air supply when underwater
     */
    protected int decreaseAirSupply(int p_70682_1_)
    {
        return p_70682_1_;
    }

    protected void collideWithEntity(Entity entityIn)
    {
        if (entityIn instanceof IMob && !(entityIn instanceof EntityCreeper) && this.getRNG().nextInt(20) == 0)
        {
            this.setAttackTarget((EntityLivingBase)entityIn);
        }

        super.collideWithEntity(entityIn);
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        super.onLivingUpdate();

        if (this.attackTimer > 0)
        {
            --this.attackTimer;
        }

        if (this.holdRoseTick > 0)
        {
            --this.holdRoseTick;
        }

        if (this.motionX * this.motionX + this.motionZ * this.motionZ > 2.500000277905201E-7D && this.rand.nextInt(5) == 0)
        {
            int i = MathHelper.floor_double(this.posX);
            int j = MathHelper.floor_double(this.posY - 0.20000000298023224D);
            int k = MathHelper.floor_double(this.posZ);
            IBlockState iblockstate = this.worldObj.getBlockState(new BlockPos(i, j, k));

            if (iblockstate.getMaterial() != Material.air)
            {
                this.worldObj.spawnParticle(EnumParticleTypes.BLOCK_CRACK, this.posX + ((double)this.rand.nextFloat() - 0.5D) * (double)this.width, this.getEntityBoundingBox().minY + 0.1D, this.posZ + ((double)this.rand.nextFloat() - 0.5D) * (double)this.width, 4.0D * ((double)this.rand.nextFloat() - 0.5D), 0.5D, ((double)this.rand.nextFloat() - 0.5D) * 4.0D, new int[] {Block.getStateId(iblockstate)});
            }
        }
    }

    /**
     * Returns true if this entity can attack entities of the specified class.
     */
    public boolean canAttackClass(Class <? extends EntityLivingBase > cls)
    {
        return this.isPlayerCreated() && EntityPlayer.class.isAssignableFrom(cls) ? false : (cls == EntityCreeper.class ? false : super.canAttackClass(cls));
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setBoolean("PlayerCreated", this.isPlayerCreated());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.setPlayerCreated(tagCompund.getBoolean("PlayerCreated"));
    }

    public boolean attackEntityAsMob(Entity entityIn)
    {
        this.attackTimer = 10;
        this.worldObj.setEntityState(this, (byte)4);
        boolean flag = entityIn.attackEntityFrom(DamageSource.causeMobDamage(this), (float)(7 + this.rand.nextInt(15)));

        if (flag)
        {
            entityIn.motionY += 0.4000000059604645D;
            this.applyEnchantments(this, entityIn);
        }

        this.playSound(SoundEvents.entity_irongolem_attack, 1.0F, 1.0F);
        return flag;
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 4)
        {
            this.attackTimer = 10;
            this.playSound(SoundEvents.entity_irongolem_attack, 1.0F, 1.0F);
        }
        else if (id == 11)
        {
            this.holdRoseTick = 400;
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    public Village getVillage()
    {
        return this.villageObj;
    }

    public int getAttackTimer()
    {
        return this.attackTimer;
    }

    public void setHoldingRose(boolean p_70851_1_)
    {
        this.holdRoseTick = p_70851_1_ ? 400 : 0;
        this.worldObj.setEntityState(this, (byte)11);
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_irongolem_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_irongolem_death;
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        this.playSound(SoundEvents.entity_irongolem_step, 1.0F, 1.0F);
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_IRON_GOLEM;
    }

    public int getHoldRoseTick()
    {
        return this.holdRoseTick;
    }

    public boolean isPlayerCreated()
    {
        return (((Byte)this.dataWatcher.get(PLAYER_CREATED)).byteValue() & 1) != 0;
    }

    public void setPlayerCreated(boolean p_70849_1_)
    {
        byte b0 = ((Byte)this.dataWatcher.get(PLAYER_CREATED)).byteValue();

        if (p_70849_1_)
        {
            this.dataWatcher.set(PLAYER_CREATED, Byte.valueOf((byte)(b0 | 1)));
        }
        else
        {
            this.dataWatcher.set(PLAYER_CREATED, Byte.valueOf((byte)(b0 & -2)));
        }
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        if (!this.isPlayerCreated() && this.attackingPlayer != null && this.villageObj != null)
        {
            this.villageObj.setReputationForPlayer(this.attackingPlayer.getName(), -5);
        }

        super.onDeath(cause);
    }
}
