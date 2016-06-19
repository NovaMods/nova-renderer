package net.minecraft.entity.monster;

import net.minecraft.block.material.Material;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IRangedAttackMob;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackRanged;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntitySnowball;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemStack;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntitySnowman extends EntityGolem implements IRangedAttackMob
{
    private static final DataParameter<Byte> PUMPKIN_EQUIPPED = EntityDataManager.<Byte>createKey(EntitySnowman.class, DataSerializers.BYTE);

    public EntitySnowman(World worldIn)
    {
        super(worldIn);
        this.setSize(0.7F, 1.9F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAIAttackRanged(this, 1.25D, 20, 10.0F));
        this.tasks.addTask(2, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(3, new EntityAIWatchClosest(this, EntityPlayer.class, 6.0F));
        this.tasks.addTask(4, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAINearestAttackableTarget(this, EntityLiving.class, 10, true, false, IMob.mobSelector));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(4.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.20000000298023224D);
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(PUMPKIN_EQUIPPED, Byte.valueOf((byte)0));
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        super.onLivingUpdate();

        if (!this.worldObj.isRemote)
        {
            int i = MathHelper.floor_double(this.posX);
            int j = MathHelper.floor_double(this.posY);
            int k = MathHelper.floor_double(this.posZ);

            if (this.isWet())
            {
                this.attackEntityFrom(DamageSource.drown, 1.0F);
            }

            if (this.worldObj.getBiomeGenForCoords(new BlockPos(i, 0, k)).getFloatTemperature(new BlockPos(i, j, k)) > 1.0F)
            {
                this.attackEntityFrom(DamageSource.onFire, 1.0F);
            }

            if (!this.worldObj.getGameRules().getBoolean("mobGriefing"))
            {
                return;
            }

            for (int l = 0; l < 4; ++l)
            {
                i = MathHelper.floor_double(this.posX + (double)((float)(l % 2 * 2 - 1) * 0.25F));
                j = MathHelper.floor_double(this.posY);
                k = MathHelper.floor_double(this.posZ + (double)((float)(l / 2 % 2 * 2 - 1) * 0.25F));
                BlockPos blockpos = new BlockPos(i, j, k);

                if (this.worldObj.getBlockState(blockpos).getMaterial() == Material.air && this.worldObj.getBiomeGenForCoords(new BlockPos(i, 0, k)).getFloatTemperature(blockpos) < 0.8F && Blocks.snow_layer.canPlaceBlockAt(this.worldObj, blockpos))
                {
                    this.worldObj.setBlockState(blockpos, Blocks.snow_layer.getDefaultState());
                }
            }
        }
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_SNOWMAN;
    }

    /**
     * Attack the specified entity using a ranged attack.
     */
    public void attackEntityWithRangedAttack(EntityLivingBase target, float p_82196_2_)
    {
        EntitySnowball entitysnowball = new EntitySnowball(this.worldObj, this);
        double d0 = target.posY + (double)target.getEyeHeight() - 1.100000023841858D;
        double d1 = target.posX - this.posX;
        double d2 = d0 - entitysnowball.posY;
        double d3 = target.posZ - this.posZ;
        float f = MathHelper.sqrt_double(d1 * d1 + d3 * d3) * 0.2F;
        entitysnowball.setThrowableHeading(d1, d2 + (double)f, d3, 1.6F, 12.0F);
        this.playSound(SoundEvents.entity_snowman_shoot, 1.0F, 1.0F / (this.getRNG().nextFloat() * 0.4F + 0.8F));
        this.worldObj.spawnEntityInWorld(entitysnowball);
    }

    public float getEyeHeight()
    {
        return 1.7F;
    }

    protected boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (p_184645_3_ != null && p_184645_3_.getItem() == Items.shears && !this.func_184748_o() && !this.worldObj.isRemote)
        {
            this.func_184747_a(true);
            p_184645_3_.damageItem(1, p_184645_1_);
        }

        return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
    }

    public boolean func_184748_o()
    {
        return (((Byte)this.dataWatcher.get(PUMPKIN_EQUIPPED)).byteValue() & 16) != 0;
    }

    public void func_184747_a(boolean p_184747_1_)
    {
        byte b0 = ((Byte)this.dataWatcher.get(PUMPKIN_EQUIPPED)).byteValue();

        if (p_184747_1_)
        {
            this.dataWatcher.set(PUMPKIN_EQUIPPED, Byte.valueOf((byte)(b0 | 16)));
        }
        else
        {
            this.dataWatcher.set(PUMPKIN_EQUIPPED, Byte.valueOf((byte)(b0 & -17)));
        }
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_snowman_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_snowman_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_snowman_death;
    }
}
