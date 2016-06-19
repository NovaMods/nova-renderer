package net.minecraft.entity.monster;

import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.base.Predicate;
import com.google.common.collect.Sets;
import java.util.Random;
import java.util.Set;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIAttackMelee;
import net.minecraft.entity.ai.EntityAIBase;
import net.minecraft.entity.ai.EntityAIHurtByTarget;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAINearestAttackableTarget;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.pathfinding.PathNodeType;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntityDamageSourceIndirect;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityEnderman extends EntityMob
{
    private static final UUID attackingSpeedBoostModifierUUID = UUID.fromString("020E0DFB-87AE-4653-9556-831010E291A0");
    private static final AttributeModifier attackingSpeedBoostModifier = (new AttributeModifier(attackingSpeedBoostModifierUUID, "Attacking speed boost", 0.15000000596046448D, 0)).setSaved(false);
    private static final Set<Block> carriableBlocks = Sets.<Block>newIdentityHashSet();
    private static final DataParameter<Optional<IBlockState>> CARRIED_BLOCK = EntityDataManager.<Optional<IBlockState>>createKey(EntityEnderman.class, DataSerializers.OPTIONAL_BLOCK_STATE);
    private static final DataParameter<Boolean> SCREAMING = EntityDataManager.<Boolean>createKey(EntityEnderman.class, DataSerializers.BOOLEAN);
    private int field_184720_bx = 0;
    private int field_184721_by = 0;

    public EntityEnderman(World worldIn)
    {
        super(worldIn);
        this.setSize(0.6F, 2.9F);
        this.stepHeight = 1.0F;
        this.func_184644_a(PathNodeType.WATER, -1.0F);
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(0, new EntityAISwimming(this));
        this.tasks.addTask(2, new EntityAIAttackMelee(this, 1.0D, false));
        this.tasks.addTask(7, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(8, new EntityAIWatchClosest(this, EntityPlayer.class, 8.0F));
        this.tasks.addTask(8, new EntityAILookIdle(this));
        this.tasks.addTask(10, new EntityEnderman.AIPlaceBlock(this));
        this.tasks.addTask(11, new EntityEnderman.AITakeBlock(this));
        this.targetTasks.addTask(1, new EntityEnderman.AIFindPlayer(this));
        this.targetTasks.addTask(2, new EntityAIHurtByTarget(this, false, new Class[0]));
        this.targetTasks.addTask(3, new EntityAINearestAttackableTarget(this, EntityEndermite.class, 10, true, false, new Predicate<EntityEndermite>()
        {
            public boolean apply(EntityEndermite p_apply_1_)
            {
                return p_apply_1_.isSpawnedByPlayer();
            }
        }));
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(40.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.30000001192092896D);
        this.getEntityAttribute(SharedMonsterAttributes.ATTACK_DAMAGE).setBaseValue(7.0D);
        this.getEntityAttribute(SharedMonsterAttributes.FOLLOW_RANGE).setBaseValue(64.0D);
    }

    /**
     * Sets the active target the Task system uses for tracking
     */
    public void setAttackTarget(EntityLivingBase entitylivingbaseIn)
    {
        super.setAttackTarget(entitylivingbaseIn);
        IAttributeInstance iattributeinstance = this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED);

        if (entitylivingbaseIn == null)
        {
            this.field_184721_by = 0;
            this.dataWatcher.set(SCREAMING, Boolean.valueOf(false));
            iattributeinstance.removeModifier(attackingSpeedBoostModifier);
        }
        else
        {
            this.field_184721_by = this.ticksExisted;
            this.dataWatcher.set(SCREAMING, Boolean.valueOf(true));

            if (!iattributeinstance.hasModifier(attackingSpeedBoostModifier))
            {
                iattributeinstance.applyModifier(attackingSpeedBoostModifier);
            }
        }
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(CARRIED_BLOCK, Optional.<IBlockState>absent());
        this.dataWatcher.register(SCREAMING, Boolean.valueOf(false));
    }

    public void func_184716_o()
    {
        if (this.ticksExisted >= this.field_184720_bx + 400)
        {
            this.field_184720_bx = this.ticksExisted;

            if (!this.isSilent())
            {
                this.worldObj.func_184134_a(this.posX, this.posY + (double)this.getEyeHeight(), this.posZ, SoundEvents.entity_endermen_stare, this.getSoundCategory(), 2.5F, 1.0F, false);
            }
        }
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        if (SCREAMING.equals(key) && this.isScreaming() && this.worldObj.isRemote)
        {
            this.func_184716_o();
        }

        super.notifyDataManagerChange(key);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        IBlockState iblockstate = this.getHeldBlockState();

        if (iblockstate != null)
        {
            tagCompound.setShort("carried", (short)Block.getIdFromBlock(iblockstate.getBlock()));
            tagCompound.setShort("carriedData", (short)iblockstate.getBlock().getMetaFromState(iblockstate));
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        IBlockState iblockstate;

        if (tagCompund.hasKey("carried", 8))
        {
            iblockstate = Block.getBlockFromName(tagCompund.getString("carried")).getStateFromMeta(tagCompund.getShort("carriedData") & 65535);
        }
        else
        {
            iblockstate = Block.getBlockById(tagCompund.getShort("carried")).getStateFromMeta(tagCompund.getShort("carriedData") & 65535);
        }

        if (iblockstate == null || iblockstate.getBlock() == null || iblockstate.getMaterial() == Material.air)
        {
            iblockstate = null;
        }

        this.setHeldBlockState(iblockstate);
    }

    /**
     * Checks to see if this enderman should be attacking this player
     */
    private boolean shouldAttackPlayer(EntityPlayer player)
    {
        ItemStack itemstack = player.inventory.armorInventory[3];

        if (itemstack != null && itemstack.getItem() == Item.getItemFromBlock(Blocks.pumpkin))
        {
            return false;
        }
        else
        {
            Vec3d vec3d = player.getLook(1.0F).normalize();
            Vec3d vec3d1 = new Vec3d(this.posX - player.posX, this.getEntityBoundingBox().minY + (double)(this.height / 2.0F) - (player.posY + (double)player.getEyeHeight()), this.posZ - player.posZ);
            double d0 = vec3d1.lengthVector();
            vec3d1 = vec3d1.normalize();
            double d1 = vec3d.dotProduct(vec3d1);
            return d1 > 1.0D - 0.025D / d0 ? player.canEntityBeSeen(this) : false;
        }
    }

    public float getEyeHeight()
    {
        return 2.55F;
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (this.worldObj.isRemote)
        {
            for (int i = 0; i < 2; ++i)
            {
                this.worldObj.spawnParticle(EnumParticleTypes.PORTAL, this.posX + (this.rand.nextDouble() - 0.5D) * (double)this.width, this.posY + this.rand.nextDouble() * (double)this.height - 0.25D, this.posZ + (this.rand.nextDouble() - 0.5D) * (double)this.width, (this.rand.nextDouble() - 0.5D) * 2.0D, -this.rand.nextDouble(), (this.rand.nextDouble() - 0.5D) * 2.0D, new int[0]);
            }
        }

        this.isJumping = false;
        super.onLivingUpdate();
    }

    protected void updateAITasks()
    {
        if (this.isWet())
        {
            this.attackEntityFrom(DamageSource.drown, 1.0F);
        }

        if (this.worldObj.isDaytime() && this.ticksExisted >= this.field_184721_by + 600)
        {
            float f = this.getBrightness(1.0F);

            if (f > 0.5F && this.worldObj.canSeeSky(new BlockPos(this)) && this.rand.nextFloat() * 30.0F < (f - 0.4F) * 2.0F)
            {
                this.setAttackTarget((EntityLivingBase)null);
                this.teleportRandomly();
            }
        }

        super.updateAITasks();
    }

    /**
     * Teleport the enderman to a random nearby position
     */
    protected boolean teleportRandomly()
    {
        double d0 = this.posX + (this.rand.nextDouble() - 0.5D) * 64.0D;
        double d1 = this.posY + (double)(this.rand.nextInt(64) - 32);
        double d2 = this.posZ + (this.rand.nextDouble() - 0.5D) * 64.0D;
        return this.teleportTo(d0, d1, d2);
    }

    /**
     * Teleport the enderman to another entity
     */
    protected boolean teleportToEntity(Entity p_70816_1_)
    {
        Vec3d vec3d = new Vec3d(this.posX - p_70816_1_.posX, this.getEntityBoundingBox().minY + (double)(this.height / 2.0F) - p_70816_1_.posY + (double)p_70816_1_.getEyeHeight(), this.posZ - p_70816_1_.posZ);
        vec3d = vec3d.normalize();
        double d0 = 16.0D;
        double d1 = this.posX + (this.rand.nextDouble() - 0.5D) * 8.0D - vec3d.xCoord * d0;
        double d2 = this.posY + (double)(this.rand.nextInt(16) - 8) - vec3d.yCoord * d0;
        double d3 = this.posZ + (this.rand.nextDouble() - 0.5D) * 8.0D - vec3d.zCoord * d0;
        return this.teleportTo(d1, d2, d3);
    }

    /**
     * Teleport the enderman
     */
    private boolean teleportTo(double x, double y, double z)
    {
        boolean flag = this.func_184595_k(x, y, z);

        if (flag)
        {
            this.worldObj.func_184148_a((EntityPlayer)null, this.prevPosX, this.prevPosY, this.prevPosZ, SoundEvents.entity_endermen_teleport, this.getSoundCategory(), 1.0F, 1.0F);
            this.playSound(SoundEvents.entity_endermen_teleport, 1.0F, 1.0F);
        }

        return flag;
    }

    protected SoundEvent getAmbientSound()
    {
        return this.isScreaming() ? SoundEvents.entity_endermen_scream : SoundEvents.entity_endermen_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_endermen_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_endermen_death;
    }

    /**
     * Drop the equipment for this entity.
     */
    protected void dropEquipment(boolean wasRecentlyHit, int lootingModifier)
    {
        super.dropEquipment(wasRecentlyHit, lootingModifier);
        IBlockState iblockstate = this.getHeldBlockState();

        if (iblockstate != null)
        {
            this.entityDropItem(new ItemStack(iblockstate.getBlock(), 1, iblockstate.getBlock().getMetaFromState(iblockstate)), 0.0F);
        }
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_ENDERMAN;
    }

    /**
     * Sets this enderman's held block state
     */
    public void setHeldBlockState(IBlockState state)
    {
        this.dataWatcher.set(CARRIED_BLOCK, Optional.fromNullable(state));
    }

    /**
     * Gets this enderman's held block state
     */
    public IBlockState getHeldBlockState()
    {
        return (IBlockState)((Optional)this.dataWatcher.get(CARRIED_BLOCK)).orNull();
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
        else if (source instanceof EntityDamageSourceIndirect)
        {
            for (int i = 0; i < 64; ++i)
            {
                if (this.teleportRandomly())
                {
                    return true;
                }
            }

            return false;
        }
        else
        {
            boolean flag = super.attackEntityFrom(source, amount);

            if (source.isUnblockable() && this.rand.nextInt(10) != 0)
            {
                this.teleportRandomly();
            }

            return flag;
        }
    }

    public boolean isScreaming()
    {
        return ((Boolean)this.dataWatcher.get(SCREAMING)).booleanValue();
    }

    static
    {
        carriableBlocks.add(Blocks.grass);
        carriableBlocks.add(Blocks.dirt);
        carriableBlocks.add(Blocks.sand);
        carriableBlocks.add(Blocks.gravel);
        carriableBlocks.add(Blocks.yellow_flower);
        carriableBlocks.add(Blocks.red_flower);
        carriableBlocks.add(Blocks.brown_mushroom);
        carriableBlocks.add(Blocks.red_mushroom);
        carriableBlocks.add(Blocks.tnt);
        carriableBlocks.add(Blocks.cactus);
        carriableBlocks.add(Blocks.clay);
        carriableBlocks.add(Blocks.pumpkin);
        carriableBlocks.add(Blocks.melon_block);
        carriableBlocks.add(Blocks.mycelium);
    }

    static class AIFindPlayer extends EntityAINearestAttackableTarget<EntityPlayer>
    {
        private final EntityEnderman enderman;
        private EntityPlayer player;
        private int field_179450_h;
        private int field_179451_i;

        public AIFindPlayer(EntityEnderman p_i45842_1_)
        {
            super(p_i45842_1_, EntityPlayer.class, false);
            this.enderman = p_i45842_1_;
        }

        public boolean shouldExecute()
        {
            double d0 = this.getTargetDistance();
            this.player = this.enderman.worldObj.func_184150_a(this.enderman.posX, this.enderman.posY, this.enderman.posZ, d0, d0, (Function<EntityPlayer, Double>)null, new Predicate<EntityPlayer>()
            {
                public boolean apply(EntityPlayer p_apply_1_)
                {
                    return p_apply_1_ != null && AIFindPlayer.this.enderman.shouldAttackPlayer(p_apply_1_);
                }
            });
            return this.player != null;
        }

        public void startExecuting()
        {
            this.field_179450_h = 5;
            this.field_179451_i = 0;
        }

        public void resetTask()
        {
            this.player = null;
            super.resetTask();
        }

        public boolean continueExecuting()
        {
            if (this.player != null)
            {
                if (!this.enderman.shouldAttackPlayer(this.player))
                {
                    return false;
                }
                else
                {
                    this.enderman.faceEntity(this.player, 10.0F, 10.0F);
                    return true;
                }
            }
            else
            {
                return this.targetEntity != null && ((EntityPlayer)this.targetEntity).isEntityAlive() ? true : super.continueExecuting();
            }
        }

        public void updateTask()
        {
            if (this.player != null)
            {
                if (--this.field_179450_h <= 0)
                {
                    this.targetEntity = this.player;
                    this.player = null;
                    super.startExecuting();
                }
            }
            else
            {
                if (this.targetEntity != null)
                {
                    if (this.enderman.shouldAttackPlayer((EntityPlayer)this.targetEntity))
                    {
                        if (((EntityPlayer)this.targetEntity).getDistanceSqToEntity(this.enderman) < 16.0D)
                        {
                            this.enderman.teleportRandomly();
                        }

                        this.field_179451_i = 0;
                    }
                    else if (((EntityPlayer)this.targetEntity).getDistanceSqToEntity(this.enderman) > 256.0D && this.field_179451_i++ >= 30 && this.enderman.teleportToEntity(this.targetEntity))
                    {
                        this.field_179451_i = 0;
                    }
                }

                super.updateTask();
            }
        }
    }

    static class AIPlaceBlock extends EntityAIBase
    {
        private final EntityEnderman enderman;

        public AIPlaceBlock(EntityEnderman p_i45843_1_)
        {
            this.enderman = p_i45843_1_;
        }

        public boolean shouldExecute()
        {
            return this.enderman.getHeldBlockState() == null ? false : (!this.enderman.worldObj.getGameRules().getBoolean("mobGriefing") ? false : this.enderman.getRNG().nextInt(2000) == 0);
        }

        public void updateTask()
        {
            Random random = this.enderman.getRNG();
            World world = this.enderman.worldObj;
            int i = MathHelper.floor_double(this.enderman.posX - 1.0D + random.nextDouble() * 2.0D);
            int j = MathHelper.floor_double(this.enderman.posY + random.nextDouble() * 2.0D);
            int k = MathHelper.floor_double(this.enderman.posZ - 1.0D + random.nextDouble() * 2.0D);
            BlockPos blockpos = new BlockPos(i, j, k);
            IBlockState iblockstate = world.getBlockState(blockpos);
            IBlockState iblockstate1 = world.getBlockState(blockpos.down());
            IBlockState iblockstate2 = this.enderman.getHeldBlockState();

            if (iblockstate2 != null && this.func_188518_a(world, blockpos, iblockstate2.getBlock(), iblockstate, iblockstate1))
            {
                world.setBlockState(blockpos, iblockstate2, 3);
                this.enderman.setHeldBlockState((IBlockState)null);
            }
        }

        private boolean func_188518_a(World p_188518_1_, BlockPos p_188518_2_, Block p_188518_3_, IBlockState p_188518_4_, IBlockState p_188518_5_)
        {
            return !p_188518_3_.canPlaceBlockAt(p_188518_1_, p_188518_2_) ? false : (p_188518_4_.getMaterial() != Material.air ? false : (p_188518_5_.getMaterial() == Material.air ? false : p_188518_5_.isFullCube()));
        }
    }

    static class AITakeBlock extends EntityAIBase
    {
        private final EntityEnderman enderman;

        public AITakeBlock(EntityEnderman p_i45841_1_)
        {
            this.enderman = p_i45841_1_;
        }

        public boolean shouldExecute()
        {
            return this.enderman.getHeldBlockState() != null ? false : (!this.enderman.worldObj.getGameRules().getBoolean("mobGriefing") ? false : this.enderman.getRNG().nextInt(20) == 0);
        }

        public void updateTask()
        {
            Random random = this.enderman.getRNG();
            World world = this.enderman.worldObj;
            int i = MathHelper.floor_double(this.enderman.posX - 2.0D + random.nextDouble() * 4.0D);
            int j = MathHelper.floor_double(this.enderman.posY + random.nextDouble() * 3.0D);
            int k = MathHelper.floor_double(this.enderman.posZ - 2.0D + random.nextDouble() * 4.0D);
            BlockPos blockpos = new BlockPos(i, j, k);
            IBlockState iblockstate = world.getBlockState(blockpos);
            Block block = iblockstate.getBlock();
            RayTraceResult raytraceresult = world.rayTraceBlocks(new Vec3d((double)((float)MathHelper.floor_double(this.enderman.posX) + 0.5F), (double)((float)j + 0.5F), (double)((float)MathHelper.floor_double(this.enderman.posZ) + 0.5F)), new Vec3d((double)((float)i + 0.5F), (double)((float)j + 0.5F), (double)((float)k + 0.5F)), false, true, false);
            boolean flag = raytraceresult != null && raytraceresult.getBlockPos().equals(blockpos);

            if (EntityEnderman.carriableBlocks.contains(block) && flag)
            {
                this.enderman.setHeldBlockState(iblockstate);
                world.setBlockToAir(blockpos);
            }
        }
    }
}
