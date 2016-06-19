package net.minecraft.entity.monster;

import com.google.common.base.Optional;
import com.google.common.base.Predicate;
import java.util.List;
import java.util.UUID;
import net.minecraft.block.BlockPistonBase;
import net.minecraft.block.BlockPistonExtension;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityBodyHelper;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIBase;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.entity.projectile.EntityShulkerBullet;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityShulker extends EntityGolem implements IMob
{
    private static final UUID COVERED_ARMOR_BONUS_ID = UUID.fromString("7E0292F2-9434-48D5-A29F-9583AF7DF27F");
    private static final AttributeModifier COVERED_ARMOR_BONUS_MODIFIER = (new AttributeModifier(COVERED_ARMOR_BONUS_ID, "Covered armor bonus", 20.0D, 0)).setSaved(false);
    protected static final DataParameter<EnumFacing> ATTACHED_FACE = EntityDataManager.<EnumFacing>createKey(EntityShulker.class, DataSerializers.FACING);
    protected static final DataParameter<Optional<BlockPos>> ATTACHED_BLOCK_POS = EntityDataManager.<Optional<BlockPos>>createKey(EntityShulker.class, DataSerializers.OPTIONAL_BLOCK_POS);
    protected static final DataParameter<Byte> PEEK_TICK = EntityDataManager.<Byte>createKey(EntityShulker.class, DataSerializers.BYTE);
    private float field_184705_bx;
    private float field_184706_by;
    private BlockPos field_184707_bz;
    private int field_184708_bA;

    public EntityShulker(World p_i46779_1_)
    {
        super(p_i46779_1_);
        this.setSize(1.0F, 1.0F);
        this.prevRenderYawOffset = 180.0F;
        this.renderYawOffset = 180.0F;
        this.isImmuneToFire = true;
        this.field_184707_bz = null;
        this.experienceValue = 5;
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        this.renderYawOffset = 180.0F;
        this.prevRenderYawOffset = 180.0F;
        this.rotationYaw = 180.0F;
        this.prevRotationYaw = 180.0F;
        this.rotationYawHead = 180.0F;
        this.prevRotationYawHead = 180.0F;
        return super.onInitialSpawn(difficulty, livingdata);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(1, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(4, new EntityShulker.AIAttack());
        this.tasks.addTask(7, new EntityShulker.AIPeek());
        this.tasks.addTask(8, new EntityAILookIdle(this));
        this.targetTasks.addTask(1, new EntityAIHurtByTarget(this, true, new Class[0]));
        this.targetTasks.addTask(2, new EntityShulker.AIAttackNearest(this));
        this.targetTasks.addTask(3, new EntityShulker.AIDefenseAttack(this));
    }

    /**
     * returns if this entity triggers Block.onEntityWalking on the blocks they walk on. used for spiders and wolves to
     * prevent them from trampling crops
     */
    protected boolean canTriggerWalking()
    {
        return false;
    }

    public SoundCategory getSoundCategory()
    {
        return SoundCategory.HOSTILE;
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_shulker_ambient;
    }

    /**
     * Plays living's sound at its position
     */
    public void playLivingSound()
    {
        if (!this.func_184686_df())
        {
            super.playLivingSound();
        }
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_shulker_death;
    }

    protected SoundEvent getHurtSound()
    {
        return this.func_184686_df() ? SoundEvents.entity_shulker_hurt_closed : SoundEvents.entity_shulker_hurt;
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(ATTACHED_FACE, EnumFacing.DOWN);
        this.dataWatcher.register(ATTACHED_BLOCK_POS, Optional.<BlockPos>absent());
        this.dataWatcher.register(PEEK_TICK, Byte.valueOf((byte)0));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(30.0D);
    }

    protected EntityBodyHelper createBodyHelper()
    {
        return new EntityShulker.BodyHelper(this);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.dataWatcher.set(ATTACHED_FACE, EnumFacing.getFront(tagCompund.getByte("AttachFace")));
        this.dataWatcher.set(PEEK_TICK, Byte.valueOf(tagCompund.getByte("Peek")));

        if (tagCompund.hasKey("APX"))
        {
            int i = tagCompund.getInteger("APX");
            int j = tagCompund.getInteger("APY");
            int k = tagCompund.getInteger("APZ");
            this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.of(new BlockPos(i, j, k)));
        }
        else
        {
            this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.<BlockPos>absent());
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setByte("AttachFace", (byte)((EnumFacing)this.dataWatcher.get(ATTACHED_FACE)).getIndex());
        tagCompound.setByte("Peek", ((Byte)this.dataWatcher.get(PEEK_TICK)).byteValue());
        BlockPos blockpos = this.func_184699_da();

        if (blockpos != null)
        {
            tagCompound.setInteger("APX", blockpos.getX());
            tagCompound.setInteger("APY", blockpos.getY());
            tagCompound.setInteger("APZ", blockpos.getZ());
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();
        BlockPos blockpos = (BlockPos)((Optional)this.dataWatcher.get(ATTACHED_BLOCK_POS)).orNull();

        if (blockpos == null && !this.worldObj.isRemote)
        {
            blockpos = new BlockPos(this);
            this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.of(blockpos));
        }

        if (this.isRiding())
        {
            blockpos = null;
            float f = this.getRidingEntity().rotationYaw;
            this.rotationYaw = f;
            this.renderYawOffset = f;
            this.prevRenderYawOffset = f;
            this.field_184708_bA = 0;
        }
        else if (!this.worldObj.isRemote)
        {
            IBlockState iblockstate = this.worldObj.getBlockState(blockpos);

            if (iblockstate.getBlock() != Blocks.air)
            {
                if (iblockstate.getBlock() == Blocks.piston_extension)
                {
                    EnumFacing enumfacing = (EnumFacing)iblockstate.getValue(BlockPistonBase.FACING);
                    blockpos = blockpos.offset(enumfacing);
                    this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.of(blockpos));
                }
                else if (iblockstate.getBlock() == Blocks.piston_head)
                {
                    EnumFacing enumfacing3 = (EnumFacing)iblockstate.getValue(BlockPistonExtension.FACING);
                    blockpos = blockpos.offset(enumfacing3);
                    this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.of(blockpos));
                }
                else
                {
                    this.func_184689_o();
                }
            }

            BlockPos blockpos1 = blockpos.offset(this.func_184696_cZ());

            if (!this.worldObj.isBlockNormalCube(blockpos1, false))
            {
                boolean flag = false;

                for (EnumFacing enumfacing1 : EnumFacing.values())
                {
                    blockpos1 = blockpos.offset(enumfacing1);

                    if (this.worldObj.isBlockNormalCube(blockpos1, false))
                    {
                        this.dataWatcher.set(ATTACHED_FACE, enumfacing1);
                        flag = true;
                        break;
                    }
                }

                if (!flag)
                {
                    this.func_184689_o();
                }
            }

            BlockPos blockpos2 = blockpos.offset(this.func_184696_cZ().getOpposite());

            if (this.worldObj.isBlockNormalCube(blockpos2, false))
            {
                this.func_184689_o();
            }
        }

        float f1 = (float)this.func_184684_db() * 0.01F;
        this.field_184705_bx = this.field_184706_by;

        if (this.field_184706_by > f1)
        {
            this.field_184706_by = MathHelper.clamp_float(this.field_184706_by - 0.05F, f1, 1.0F);
        }
        else if (this.field_184706_by < f1)
        {
            this.field_184706_by = MathHelper.clamp_float(this.field_184706_by + 0.05F, 0.0F, f1);
        }

        if (blockpos != null)
        {
            if (this.worldObj.isRemote)
            {
                if (this.field_184708_bA > 0 && this.field_184707_bz != null)
                {
                    --this.field_184708_bA;
                }
                else
                {
                    this.field_184707_bz = blockpos;
                }
            }

            this.lastTickPosX = this.prevPosX = this.posX = (double)blockpos.getX() + 0.5D;
            this.lastTickPosY = this.prevPosY = this.posY = (double)blockpos.getY();
            this.lastTickPosZ = this.prevPosZ = this.posZ = (double)blockpos.getZ() + 0.5D;
            double d3 = 0.5D - (double)MathHelper.sin((0.5F + this.field_184706_by) * (float)Math.PI) * 0.5D;
            double d4 = 0.5D - (double)MathHelper.sin((0.5F + this.field_184705_bx) * (float)Math.PI) * 0.5D;
            double d5 = d3 - d4;
            double d0 = 0.0D;
            double d1 = 0.0D;
            double d2 = 0.0D;
            EnumFacing enumfacing2 = this.func_184696_cZ();

            switch (enumfacing2)
            {
                case DOWN:
                default:
                    this.setEntityBoundingBox(new AxisAlignedBB(this.posX - 0.5D, this.posY, this.posZ - 0.5D, this.posX + 0.5D, this.posY + 1.0D + d3, this.posZ + 0.5D));
                    d1 = d5;
                    break;

                case UP:
                    this.setEntityBoundingBox(new AxisAlignedBB(this.posX - 0.5D, this.posY - d3, this.posZ - 0.5D, this.posX + 0.5D, this.posY + 1.0D, this.posZ + 0.5D));
                    d1 = -d5;
                    break;

                case NORTH:
                    this.setEntityBoundingBox(new AxisAlignedBB(this.posX - 0.5D, this.posY, this.posZ - 0.5D, this.posX + 0.5D, this.posY + 1.0D, this.posZ + 0.5D + d3));
                    d2 = d5;
                    break;

                case SOUTH:
                    this.setEntityBoundingBox(new AxisAlignedBB(this.posX - 0.5D, this.posY, this.posZ - 0.5D - d3, this.posX + 0.5D, this.posY + 1.0D, this.posZ + 0.5D));
                    d2 = -d5;
                    break;

                case WEST:
                    this.setEntityBoundingBox(new AxisAlignedBB(this.posX - 0.5D, this.posY, this.posZ - 0.5D, this.posX + 0.5D + d3, this.posY + 1.0D, this.posZ + 0.5D));
                    d0 = d5;
                    break;

                case EAST:
                    this.setEntityBoundingBox(new AxisAlignedBB(this.posX - 0.5D - d3, this.posY, this.posZ - 0.5D, this.posX + 0.5D, this.posY + 1.0D, this.posZ + 0.5D));
                    d0 = -d5;
            }

            if (d5 > 0.0D)
            {
                List<Entity> list = this.worldObj.getEntitiesWithinAABBExcludingEntity(this, this.getEntityBoundingBox());

                if (!list.isEmpty())
                {
                    for (Entity entity : list)
                    {
                        if (!(entity instanceof EntityShulker) && !entity.noClip)
                        {
                            entity.moveEntity(d0, d1, d2);
                        }
                    }
                }
            }
        }
    }

    /**
     * Sets the x,y,z of the entity from the given parameters. Also seems to set up a bounding box.
     */
    public void setPosition(double x, double y, double z)
    {
        super.setPosition(x, y, z);

        if (this.dataWatcher != null && this.ticksExisted != 0)
        {
            Optional<BlockPos> optional = (Optional)this.dataWatcher.get(ATTACHED_BLOCK_POS);
            Optional<BlockPos> optional1 = Optional.<BlockPos>of(new BlockPos(x, y, z));

            if (!optional1.equals(optional))
            {
                this.dataWatcher.set(ATTACHED_BLOCK_POS, optional1);
                this.dataWatcher.set(PEEK_TICK, Byte.valueOf((byte)0));
                this.isAirBorne = true;
            }
        }
    }

    protected boolean func_184689_o()
    {
        if (!this.isAIDisabled() && this.isEntityAlive())
        {
            BlockPos blockpos = new BlockPos(this);

            for (int i = 0; i < 5; ++i)
            {
                BlockPos blockpos1 = blockpos.add(8 - this.rand.nextInt(17), 8 - this.rand.nextInt(17), 8 - this.rand.nextInt(17));

                if (blockpos1.getY() > 0 && this.worldObj.isAirBlock(blockpos1) && this.worldObj.isInsideBorder(this.worldObj.getWorldBorder(), this) && this.worldObj.func_184144_a(this, new AxisAlignedBB(blockpos1)).isEmpty())
                {
                    boolean flag = false;

                    for (EnumFacing enumfacing : EnumFacing.values())
                    {
                        if (this.worldObj.isBlockNormalCube(blockpos1.offset(enumfacing), false))
                        {
                            this.dataWatcher.set(ATTACHED_FACE, enumfacing);
                            flag = true;
                            break;
                        }
                    }

                    if (flag)
                    {
                        this.playSound(SoundEvents.entity_shulker_teleport, 1.0F, 1.0F);
                        this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.of(blockpos1));
                        this.dataWatcher.set(PEEK_TICK, Byte.valueOf((byte)0));
                        this.setAttackTarget((EntityLivingBase)null);
                        return true;
                    }
                }
            }

            return false;
        }
        else
        {
            return true;
        }
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        super.onLivingUpdate();
        this.motionX = 0.0D;
        this.motionY = 0.0D;
        this.motionZ = 0.0D;
        this.prevRenderYawOffset = 180.0F;
        this.renderYawOffset = 180.0F;
        this.rotationYaw = 180.0F;
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        if (ATTACHED_BLOCK_POS.equals(key) && this.worldObj.isRemote && !this.isRiding())
        {
            BlockPos blockpos = this.func_184699_da();

            if (blockpos != null)
            {
                if (this.field_184707_bz == null)
                {
                    this.field_184707_bz = blockpos;
                }
                else
                {
                    this.field_184708_bA = 6;
                }

                this.lastTickPosX = this.prevPosX = this.posX = (double)blockpos.getX() + 0.5D;
                this.lastTickPosY = this.prevPosY = this.posY = (double)blockpos.getY();
                this.lastTickPosZ = this.prevPosZ = this.posZ = (double)blockpos.getZ() + 0.5D;
            }
        }

        super.notifyDataManagerChange(key);
    }

    public void setPositionAndRotation2(double x, double y, double z, float yaw, float pitch, int posRotationIncrements, boolean p_180426_10_)
    {
        this.newPosRotationIncrements = 0;
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        if (this.func_184686_df())
        {
            Entity entity = source.getSourceOfDamage();

            if (entity instanceof EntityArrow)
            {
                return false;
            }
        }

        if (super.attackEntityFrom(source, amount))
        {
            if ((double)this.getHealth() < (double)this.getMaxHealth() * 0.5D && this.rand.nextInt(4) == 0)
            {
                this.func_184689_o();
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    private boolean func_184686_df()
    {
        return this.func_184684_db() == 0;
    }

    /**
     * Returns the collision bounding box for this entity
     */
    public AxisAlignedBB getCollisionBoundingBox()
    {
        return this.isEntityAlive() ? this.getEntityBoundingBox() : null;
    }

    public EnumFacing func_184696_cZ()
    {
        return (EnumFacing)this.dataWatcher.get(ATTACHED_FACE);
    }

    public BlockPos func_184699_da()
    {
        return (BlockPos)((Optional)this.dataWatcher.get(ATTACHED_BLOCK_POS)).orNull();
    }

    public void func_184694_g(BlockPos p_184694_1_)
    {
        this.dataWatcher.set(ATTACHED_BLOCK_POS, Optional.fromNullable(p_184694_1_));
    }

    public int func_184684_db()
    {
        return ((Byte)this.dataWatcher.get(PEEK_TICK)).byteValue();
    }

    public void func_184691_a(int p_184691_1_)
    {
        if (!this.worldObj.isRemote)
        {
            this.getEntityAttribute(SharedMonsterAttributes.ARMOR).removeModifier(COVERED_ARMOR_BONUS_MODIFIER);

            if (p_184691_1_ == 0)
            {
                this.getEntityAttribute(SharedMonsterAttributes.ARMOR).applyModifier(COVERED_ARMOR_BONUS_MODIFIER);
                this.playSound(SoundEvents.entity_shulker_close, 1.0F, 1.0F);
            }
            else
            {
                this.playSound(SoundEvents.entity_shulker_open, 1.0F, 1.0F);
            }
        }

        this.dataWatcher.set(PEEK_TICK, Byte.valueOf((byte)p_184691_1_));
    }

    public float func_184688_a(float p_184688_1_)
    {
        return this.field_184705_bx + (this.field_184706_by - this.field_184705_bx) * p_184688_1_;
    }

    public int func_184693_dc()
    {
        return this.field_184708_bA;
    }

    public BlockPos func_184692_dd()
    {
        return this.field_184707_bz;
    }

    public float getEyeHeight()
    {
        return 0.5F;
    }

    /**
     * The speed it takes to move the entityliving's rotationPitch through the faceEntity method. This is only currently
     * use in wolves.
     */
    public int getVerticalFaceSpeed()
    {
        return 180;
    }

    public int func_184649_cE()
    {
        return 180;
    }

    /**
     * Applies a velocity to each of the entities pushing them away from each other. Args: entity
     */
    public void applyEntityCollision(Entity entityIn)
    {
    }

    public float getCollisionBorderSize()
    {
        return 0.0F;
    }

    public boolean func_184697_de()
    {
        return this.field_184707_bz != null && this.func_184699_da() != null;
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_SHULKER;
    }

    class AIAttack extends EntityAIBase
    {
        private int field_188520_b;

        public AIAttack()
        {
            this.setMutexBits(3);
        }

        public boolean shouldExecute()
        {
            EntityLivingBase entitylivingbase = EntityShulker.this.getAttackTarget();
            return entitylivingbase != null && entitylivingbase.isEntityAlive() ? EntityShulker.this.worldObj.getDifficulty() != EnumDifficulty.PEACEFUL : false;
        }

        public void startExecuting()
        {
            this.field_188520_b = 20;
            EntityShulker.this.func_184691_a(100);
        }

        public void resetTask()
        {
            EntityShulker.this.func_184691_a(0);
        }

        public void updateTask()
        {
            if (EntityShulker.this.worldObj.getDifficulty() != EnumDifficulty.PEACEFUL)
            {
                --this.field_188520_b;
                EntityLivingBase entitylivingbase = EntityShulker.this.getAttackTarget();
                EntityShulker.this.getLookHelper().setLookPositionWithEntity(entitylivingbase, 180.0F, 180.0F);
                double d0 = EntityShulker.this.getDistanceSqToEntity(entitylivingbase);

                if (d0 < 400.0D)
                {
                    if (this.field_188520_b <= 0)
                    {
                        this.field_188520_b = 20 + EntityShulker.this.rand.nextInt(10) * 20 / 2;
                        EntityShulkerBullet entityshulkerbullet = new EntityShulkerBullet(EntityShulker.this.worldObj, EntityShulker.this, entitylivingbase, EntityShulker.this.func_184696_cZ().getAxis());
                        EntityShulker.this.worldObj.spawnEntityInWorld(entityshulkerbullet);
                        EntityShulker.this.playSound(SoundEvents.entity_shulker_shoot, 2.0F, (EntityShulker.this.rand.nextFloat() - EntityShulker.this.rand.nextFloat()) * 0.2F + 1.0F);
                    }
                }
                else
                {
                    EntityShulker.this.setAttackTarget((EntityLivingBase)null);
                }

                super.updateTask();
            }
        }
    }

    class AIAttackNearest extends EntityAINearestAttackableTarget<EntityPlayer>
    {
        public AIAttackNearest(EntityShulker p_i47060_2_)
        {
            super(p_i47060_2_, EntityPlayer.class, true);
        }

        public boolean shouldExecute()
        {
            return EntityShulker.this.worldObj.getDifficulty() == EnumDifficulty.PEACEFUL ? false : super.shouldExecute();
        }

        protected AxisAlignedBB func_188511_a(double p_188511_1_)
        {
            EnumFacing enumfacing = ((EntityShulker)this.taskOwner).func_184696_cZ();
            return enumfacing.getAxis() == EnumFacing.Axis.X ? this.taskOwner.getEntityBoundingBox().expand(4.0D, p_188511_1_, p_188511_1_) : (enumfacing.getAxis() == EnumFacing.Axis.Z ? this.taskOwner.getEntityBoundingBox().expand(p_188511_1_, p_188511_1_, 4.0D) : this.taskOwner.getEntityBoundingBox().expand(p_188511_1_, 4.0D, p_188511_1_));
        }
    }

    static class AIDefenseAttack extends EntityAINearestAttackableTarget<EntityLivingBase>
    {
        public AIDefenseAttack(EntityShulker p_i47061_1_)
        {
            super(p_i47061_1_, EntityLivingBase.class, 10, true, false, new Predicate<EntityLivingBase>()
            {
                public boolean apply(EntityLivingBase p_apply_1_)
                {
                    return p_apply_1_ instanceof IMob;
                }
            });
        }

        public boolean shouldExecute()
        {
            return this.taskOwner.getTeam() == null ? false : super.shouldExecute();
        }

        protected AxisAlignedBB func_188511_a(double p_188511_1_)
        {
            EnumFacing enumfacing = ((EntityShulker)this.taskOwner).func_184696_cZ();
            return enumfacing.getAxis() == EnumFacing.Axis.X ? this.taskOwner.getEntityBoundingBox().expand(4.0D, p_188511_1_, p_188511_1_) : (enumfacing.getAxis() == EnumFacing.Axis.Z ? this.taskOwner.getEntityBoundingBox().expand(p_188511_1_, p_188511_1_, 4.0D) : this.taskOwner.getEntityBoundingBox().expand(p_188511_1_, 4.0D, p_188511_1_));
        }
    }

    class AIPeek extends EntityAIBase
    {
        private int field_188522_b;

        private AIPeek()
        {
        }

        public boolean shouldExecute()
        {
            return EntityShulker.this.getAttackTarget() == null && EntityShulker.this.rand.nextInt(40) == 0;
        }

        public boolean continueExecuting()
        {
            return EntityShulker.this.getAttackTarget() == null && this.field_188522_b > 0;
        }

        public void startExecuting()
        {
            this.field_188522_b = 20 * (1 + EntityShulker.this.rand.nextInt(3));
            EntityShulker.this.func_184691_a(30);
        }

        public void resetTask()
        {
            if (EntityShulker.this.getAttackTarget() == null)
            {
                EntityShulker.this.func_184691_a(0);
            }
        }

        public void updateTask()
        {
            --this.field_188522_b;
        }
    }

    class BodyHelper extends EntityBodyHelper
    {
        public BodyHelper(EntityLivingBase p_i47062_2_)
        {
            super(p_i47062_2_);
        }

        public void updateRenderAngles()
        {
        }
    }
}
