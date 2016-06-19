package net.minecraft.entity.passive;

import net.minecraft.block.Block;
import net.minecraft.block.BlockCarrot;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityAgeable;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackMelee;
import net.minecraft.entity.ai.EntityAIAvoidEntity;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAIMate;
import net.minecraft.entity.ai.EntityAIMoveToBlock;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAIPanic;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAITempt;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.ai.EntityJumpHelper;
import net.minecraft.entity.ai.EntityMoveHelper;
import net.minecraft.entity.monster.EntityMob;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.pathfinding.PathEntity;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.World;
import net.minecraft.world.biome.BiomeGenBase;
import net.minecraft.world.biome.BiomeGenDesert;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityRabbit extends EntityAnimal
{
    private static final DataParameter<Integer> RABBIT_TYPE = EntityDataManager.<Integer>createKey(EntityRabbit.class, DataSerializers.VARINT);
    private int field_175540_bm = 0;
    private int field_175535_bn = 0;
    private boolean field_175537_bp = false;
    private int currentMoveTypeDuration = 0;
    private int carrotTicks = 0;

    public EntityRabbit(World worldIn)
    {
        super(worldIn);
        this.setSize(0.4F, 0.5F);
        this.jumpHelper = new EntityRabbit.RabbitJumpHelper(this);
        this.moveHelper = new EntityRabbit.RabbitMoveHelper(this);
        this.setMovementSpeed(0.0D);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAISwimming(this));
        this.tasks.addTask(1, new EntityRabbit.AIPanic(this, 2.2D));
        this.tasks.addTask(2, new EntityAIMate(this, 0.8D));
        this.tasks.addTask(3, new EntityAITempt(this, 1.0D, Items.carrot, false));
        this.tasks.addTask(3, new EntityAITempt(this, 1.0D, Items.golden_carrot, false));
        this.tasks.addTask(3, new EntityAITempt(this, 1.0D, Item.getItemFromBlock(Blocks.yellow_flower), false));
        this.tasks.addTask(4, new EntityRabbit.AIAvoidEntity(this, EntityPlayer.class, 8.0F, 2.2D, 2.2D));
        this.tasks.addTask(4, new EntityRabbit.AIAvoidEntity(this, EntityWolf.class, 10.0F, 2.2D, 2.2D));
        this.tasks.addTask(4, new EntityRabbit.AIAvoidEntity(this, EntityMob.class, 4.0F, 2.2D, 2.2D));
        this.tasks.addTask(5, new EntityRabbit.AIRaidFarm(this));
        this.tasks.addTask(6, new EntityAIWander(this, 0.6D));
        this.tasks.addTask(11, new EntityAIWatchClosest(this, EntityPlayer.class, 10.0F));
    }

    protected float getJumpUpwardsMotion()
    {
        if (!this.isCollidedHorizontally && (!this.moveHelper.isUpdating() || this.moveHelper.getY() <= this.posY + 0.5D))
        {
            PathEntity pathentity = this.navigator.getPath();

            if (pathentity != null && pathentity.getCurrentPathIndex() < pathentity.getCurrentPathLength())
            {
                Vec3d vec3d = pathentity.getPosition(this);

                if (vec3d.yCoord > this.posY)
                {
                    return 0.5F;
                }
            }

            return this.moveHelper.getSpeed() <= 0.6D ? 0.2F : 0.3F;
        }
        else
        {
            return 0.5F;
        }
    }

    /**
     * Causes this entity to do an upwards motion (jumping).
     */
    protected void jump()
    {
        super.jump();
        double d0 = this.moveHelper.getSpeed();

        if (d0 > 0.0D)
        {
            double d1 = this.motionX * this.motionX + this.motionZ * this.motionZ;

            if (d1 < 0.010000000000000002D)
            {
                this.moveFlying(0.0F, 1.0F, 0.1F);
            }
        }

        if (!this.worldObj.isRemote)
        {
            this.worldObj.setEntityState(this, (byte)1);
        }
    }

    public float func_175521_o(float p_175521_1_)
    {
        return this.field_175535_bn == 0 ? 0.0F : ((float)this.field_175540_bm + p_175521_1_) / (float)this.field_175535_bn;
    }

    public void setMovementSpeed(double newSpeed)
    {
        this.getNavigator().setSpeed(newSpeed);
        this.moveHelper.setMoveTo(this.moveHelper.getX(), this.moveHelper.getY(), this.moveHelper.getZ(), newSpeed);
    }

    public void setJumping(boolean jumping)
    {
        super.setJumping(jumping);

        if (jumping)
        {
            this.playSound(this.func_184771_da(), this.getSoundVolume(), ((this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F) * 0.8F);
        }
    }

    public void func_184770_cZ()
    {
        this.setJumping(true);
        this.field_175535_bn = 10;
        this.field_175540_bm = 0;
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(RABBIT_TYPE, Integer.valueOf(0));
    }

    public void updateAITasks()
    {
        if (this.currentMoveTypeDuration > 0)
        {
            --this.currentMoveTypeDuration;
        }

        if (this.carrotTicks > 0)
        {
            this.carrotTicks -= this.rand.nextInt(3);

            if (this.carrotTicks < 0)
            {
                this.carrotTicks = 0;
            }
        }

        if (this.onGround)
        {
            if (!this.field_175537_bp)
            {
                this.setJumping(false);
                this.func_175517_cu();
            }

            if (this.getRabbitType() == 99 && this.currentMoveTypeDuration == 0)
            {
                EntityLivingBase entitylivingbase = this.getAttackTarget();

                if (entitylivingbase != null && this.getDistanceSqToEntity(entitylivingbase) < 16.0D)
                {
                    this.calculateRotationYaw(entitylivingbase.posX, entitylivingbase.posZ);
                    this.moveHelper.setMoveTo(entitylivingbase.posX, entitylivingbase.posY, entitylivingbase.posZ, this.moveHelper.getSpeed());
                    this.func_184770_cZ();
                    this.field_175537_bp = true;
                }
            }

            EntityRabbit.RabbitJumpHelper entityrabbit$rabbitjumphelper = (EntityRabbit.RabbitJumpHelper)this.jumpHelper;

            if (!entityrabbit$rabbitjumphelper.getIsJumping())
            {
                if (this.moveHelper.isUpdating() && this.currentMoveTypeDuration == 0)
                {
                    PathEntity pathentity = this.navigator.getPath();
                    Vec3d vec3d = new Vec3d(this.moveHelper.getX(), this.moveHelper.getY(), this.moveHelper.getZ());

                    if (pathentity != null && pathentity.getCurrentPathIndex() < pathentity.getCurrentPathLength())
                    {
                        vec3d = pathentity.getPosition(this);
                    }

                    this.calculateRotationYaw(vec3d.xCoord, vec3d.zCoord);
                    this.func_184770_cZ();
                }
            }
            else if (!entityrabbit$rabbitjumphelper.func_180065_d())
            {
                this.func_175518_cr();
            }
        }

        this.field_175537_bp = this.onGround;
    }

    /**
     * Attempts to create sprinting particles if the entity is sprinting and not in water.
     */
    public void spawnRunningParticles()
    {
    }

    private void calculateRotationYaw(double x, double z)
    {
        this.rotationYaw = (float)(MathHelper.atan2(z - this.posZ, x - this.posX) * (180D / Math.PI)) - 90.0F;
    }

    private void func_175518_cr()
    {
        ((EntityRabbit.RabbitJumpHelper)this.jumpHelper).func_180066_a(true);
    }

    private void func_175520_cs()
    {
        ((EntityRabbit.RabbitJumpHelper)this.jumpHelper).func_180066_a(false);
    }

    private void updateMoveTypeDuration()
    {
        if (this.moveHelper.getSpeed() < 2.2D)
        {
            this.currentMoveTypeDuration = 10;
        }
        else
        {
            this.currentMoveTypeDuration = 1;
        }
    }

    private void func_175517_cu()
    {
        this.updateMoveTypeDuration();
        this.func_175520_cs();
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        super.onLivingUpdate();

        if (this.field_175540_bm != this.field_175535_bn)
        {
            ++this.field_175540_bm;
        }
        else if (this.field_175535_bn != 0)
        {
            this.field_175540_bm = 0;
            this.field_175535_bn = 0;
            this.setJumping(false);
        }
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(3.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.30000001192092896D);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setInteger("RabbitType", this.getRabbitType());
        tagCompound.setInteger("MoreCarrotTicks", this.carrotTicks);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.setRabbitType(tagCompund.getInteger("RabbitType"));
        this.carrotTicks = tagCompund.getInteger("MoreCarrotTicks");
    }

    protected SoundEvent func_184771_da()
    {
        return SoundEvents.entity_rabbit_jump;
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_rabbit_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_rabbit_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_rabbit_death;
    }

    public boolean attackEntityAsMob(Entity entityIn)
    {
        if (this.getRabbitType() == 99)
        {
            this.playSound(SoundEvents.entity_rabbit_attack, 1.0F, (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
            return entityIn.attackEntityFrom(DamageSource.causeMobDamage(this), 8.0F);
        }
        else
        {
            return entityIn.attackEntityFrom(DamageSource.causeMobDamage(this), 3.0F);
        }
    }

    public SoundCategory getSoundCategory()
    {
        return this.getRabbitType() == 99 ? SoundCategory.HOSTILE : SoundCategory.NEUTRAL;
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        return this.isEntityInvulnerable(source) ? false : super.attackEntityFrom(source, amount);
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_RABBIT;
    }

    private boolean isRabbitBreedingItem(Item itemIn)
    {
        return itemIn == Items.carrot || itemIn == Items.golden_carrot || itemIn == Item.getItemFromBlock(Blocks.yellow_flower);
    }

    public EntityRabbit createChild(EntityAgeable ageable)
    {
        EntityRabbit entityrabbit = new EntityRabbit(this.worldObj);
        int i = this.func_184772_dk();

        if (this.rand.nextInt(20) != 0)
        {
            if (ageable instanceof EntityRabbit && this.rand.nextBoolean())
            {
                i = ((EntityRabbit)ageable).getRabbitType();
            }
            else
            {
                i = this.getRabbitType();
            }
        }

        entityrabbit.setRabbitType(i);
        return entityrabbit;
    }

    /**
     * Checks if the parameter is an item which this animal can be fed to breed it (wheat, carrots or seeds depending on
     * the animal type)
     */
    public boolean isBreedingItem(ItemStack stack)
    {
        return stack != null && this.isRabbitBreedingItem(stack.getItem());
    }

    public int getRabbitType()
    {
        return ((Integer)this.dataWatcher.get(RABBIT_TYPE)).intValue();
    }

    public void setRabbitType(int rabbitTypeId)
    {
        if (rabbitTypeId == 99)
        {
            this.getEntityAttribute(SharedMonsterAttributes.ARMOR).setBaseValue(8.0D);
            this.tasks.addTask(4, new EntityRabbit.AIEvilAttack(this));
            this.targetTasks.addTask(1, new EntityAIHurtByTarget(this, false, new Class[0]));
            this.targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, EntityPlayer.class, true));
            this.targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, EntityWolf.class, true));

            if (!this.hasCustomName())
            {
                this.setCustomNameTag(I18n.translateToLocal("entity.KillerBunny.name"));
            }
        }

        this.dataWatcher.set(RABBIT_TYPE, Integer.valueOf(rabbitTypeId));
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        livingdata = super.onInitialSpawn(difficulty, livingdata);
        int i = this.func_184772_dk();
        boolean flag = false;

        if (livingdata instanceof EntityRabbit.RabbitTypeData)
        {
            i = ((EntityRabbit.RabbitTypeData)livingdata).typeData;
            flag = true;
        }
        else
        {
            livingdata = new EntityRabbit.RabbitTypeData(i);
        }

        this.setRabbitType(i);

        if (flag)
        {
            this.setGrowingAge(-24000);
        }

        return livingdata;
    }

    private int func_184772_dk()
    {
        BiomeGenBase biomegenbase = this.worldObj.getBiomeGenForCoords(new BlockPos(this));
        int i = this.rand.nextInt(100);
        return biomegenbase.isSnowyBiome() ? (i < 80 ? 1 : 3) : (biomegenbase instanceof BiomeGenDesert ? 4 : (i < 50 ? 0 : (i < 90 ? 5 : 2)));
    }

    /**
     * Returns true if {@link net.minecraft.entity.passive.EntityRabbit#carrotTicks carrotTicks} has reached zero
     */
    private boolean isCarrotEaten()
    {
        return this.carrotTicks == 0;
    }

    protected void createEatingParticles()
    {
        BlockCarrot blockcarrot = (BlockCarrot)Blocks.carrots;
        IBlockState iblockstate = blockcarrot.func_185528_e(blockcarrot.func_185526_g());
        this.worldObj.spawnParticle(EnumParticleTypes.BLOCK_DUST, this.posX + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, this.posY + 0.5D + (double)(this.rand.nextFloat() * this.height), this.posZ + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, 0.0D, 0.0D, 0.0D, new int[] {Block.getStateId(iblockstate)});
        this.carrotTicks = 40;
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 1)
        {
            this.createRunningParticles();
            this.field_175535_bn = 10;
            this.field_175540_bm = 0;
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        super.notifyDataManagerChange(key);
    }

    static class AIAvoidEntity<T extends Entity> extends EntityAIAvoidEntity<T>
    {
        private EntityRabbit entityInstance;

        public AIAvoidEntity(EntityRabbit rabbit, Class<T> p_i46403_2_, float p_i46403_3_, double p_i46403_4_, double p_i46403_6_)
        {
            super(rabbit, p_i46403_2_, p_i46403_3_, p_i46403_4_, p_i46403_6_);
            this.entityInstance = rabbit;
        }

        public boolean shouldExecute()
        {
            return this.entityInstance.getRabbitType() != 99 && super.shouldExecute();
        }
    }

    static class AIEvilAttack extends EntityAIAttackMelee
    {
        public AIEvilAttack(EntityRabbit rabbit)
        {
            super(rabbit, 1.4D, true);
        }

        protected double func_179512_a(EntityLivingBase attackTarget)
        {
            return (double)(4.0F + attackTarget.width);
        }
    }

    static class AIPanic extends EntityAIPanic
    {
        private EntityRabbit theEntity;

        public AIPanic(EntityRabbit rabbit, double speedIn)
        {
            super(rabbit, speedIn);
            this.theEntity = rabbit;
        }

        public void updateTask()
        {
            super.updateTask();
            this.theEntity.setMovementSpeed(this.speed);
        }
    }

    static class AIRaidFarm extends EntityAIMoveToBlock
    {
        private final EntityRabbit rabbit;
        private boolean field_179498_d;
        private boolean field_179499_e = false;

        public AIRaidFarm(EntityRabbit rabbitIn)
        {
            super(rabbitIn, 0.699999988079071D, 16);
            this.rabbit = rabbitIn;
        }

        public boolean shouldExecute()
        {
            if (this.runDelay <= 0)
            {
                if (!this.rabbit.worldObj.getGameRules().getBoolean("mobGriefing"))
                {
                    return false;
                }

                this.field_179499_e = false;
                this.field_179498_d = this.rabbit.isCarrotEaten();
                this.field_179498_d = true;
            }

            return super.shouldExecute();
        }

        public boolean continueExecuting()
        {
            return this.field_179499_e && super.continueExecuting();
        }

        public void startExecuting()
        {
            super.startExecuting();
        }

        public void resetTask()
        {
            super.resetTask();
        }

        public void updateTask()
        {
            super.updateTask();
            this.rabbit.getLookHelper().setLookPosition((double)this.destinationBlock.getX() + 0.5D, (double)(this.destinationBlock.getY() + 1), (double)this.destinationBlock.getZ() + 0.5D, 10.0F, (float)this.rabbit.getVerticalFaceSpeed());

            if (this.getIsAboveDestination())
            {
                World world = this.rabbit.worldObj;
                BlockPos blockpos = this.destinationBlock.up();
                IBlockState iblockstate = world.getBlockState(blockpos);
                Block block = iblockstate.getBlock();

                if (this.field_179499_e && block instanceof BlockCarrot)
                {
                    Integer integer = (Integer)iblockstate.getValue(BlockCarrot.AGE);

                    if (integer.intValue() == 0)
                    {
                        world.setBlockState(blockpos, Blocks.air.getDefaultState(), 2);
                        world.destroyBlock(blockpos, true);
                    }
                    else
                    {
                        world.setBlockState(blockpos, iblockstate.withProperty(BlockCarrot.AGE, Integer.valueOf(integer.intValue() - 1)), 2);
                        world.playAuxSFX(2001, blockpos, Block.getStateId(iblockstate));
                    }

                    this.rabbit.createEatingParticles();
                }

                this.field_179499_e = false;
                this.runDelay = 10;
            }
        }

        protected boolean shouldMoveTo(World worldIn, BlockPos pos)
        {
            Block block = worldIn.getBlockState(pos).getBlock();

            if (block == Blocks.farmland && this.field_179498_d && !this.field_179499_e)
            {
                pos = pos.up();
                IBlockState iblockstate = worldIn.getBlockState(pos);
                block = iblockstate.getBlock();

                if (block instanceof BlockCarrot && ((BlockCarrot)block).func_185525_y(iblockstate))
                {
                    this.field_179499_e = true;
                    return true;
                }
            }

            return false;
        }
    }

    public class RabbitJumpHelper extends EntityJumpHelper
    {
        private EntityRabbit theEntity;
        private boolean field_180068_d = false;

        public RabbitJumpHelper(EntityRabbit rabbit)
        {
            super(rabbit);
            this.theEntity = rabbit;
        }

        public boolean getIsJumping()
        {
            return this.isJumping;
        }

        public boolean func_180065_d()
        {
            return this.field_180068_d;
        }

        public void func_180066_a(boolean p_180066_1_)
        {
            this.field_180068_d = p_180066_1_;
        }

        public void doJump()
        {
            if (this.isJumping)
            {
                this.theEntity.func_184770_cZ();
                this.isJumping = false;
            }
        }
    }

    static class RabbitMoveHelper extends EntityMoveHelper
    {
        private EntityRabbit theEntity;
        private double field_188492_j;

        public RabbitMoveHelper(EntityRabbit rabbit)
        {
            super(rabbit);
            this.theEntity = rabbit;
        }

        public void onUpdateMoveHelper()
        {
            if (this.theEntity.onGround && !this.theEntity.isJumping && !((EntityRabbit.RabbitJumpHelper)this.theEntity.jumpHelper).getIsJumping())
            {
                this.theEntity.setMovementSpeed(0.0D);
            }
            else if (this.isUpdating())
            {
                this.theEntity.setMovementSpeed(this.field_188492_j);
            }

            super.onUpdateMoveHelper();
        }

        public void setMoveTo(double x, double y, double z, double speedIn)
        {
            if (this.theEntity.isInWater())
            {
                speedIn = 1.5D;
            }

            super.setMoveTo(x, y, z, speedIn);

            if (speedIn > 0.0D)
            {
                this.field_188492_j = speedIn;
            }
        }
    }

    public static class RabbitTypeData implements IEntityLivingData
    {
        public int typeData;

        public RabbitTypeData(int type)
        {
            this.typeData = type;
        }
    }
}
