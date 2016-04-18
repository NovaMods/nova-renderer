package net.minecraft.entity.boss;

import java.util.List;
import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IEntityMultiPart;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.boss.dragon.phase.IPhase;
import net.minecraft.entity.boss.dragon.phase.PhaseList;
import net.minecraft.entity.boss.dragon.phase.PhaseManager;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.entity.item.EntityXPOrb;
import net.minecraft.entity.monster.IMob;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.pathfinding.Path;
import net.minecraft.pathfinding.PathEntity;
import net.minecraft.pathfinding.PathPoint;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntityDamageSource;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;
import net.minecraft.world.WorldProviderEnd;
import net.minecraft.world.end.DragonFightManager;
import net.minecraft.world.gen.feature.WorldGenEndPodium;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class EntityDragon extends EntityLiving implements IEntityMultiPart, IMob
{
    private static final Logger LOGGER = LogManager.getLogger();
    public static final DataParameter<Integer> PHASE = EntityDataManager.<Integer>createKey(EntityDragon.class, DataSerializers.VARINT);

    /**
     * Ring buffer array for the last 64 Y-positions and yaw rotations. Used to calculate offsets for the animations.
     */
    public double[][] ringBuffer = new double[64][3];

    /**
     * Index into the ring buffer. Incremented once per tick and restarts at 0 once it reaches the end of the buffer.
     */
    public int ringBufferIndex = -1;

    /** An array containing all body parts of this dragon */
    public EntityDragonPart[] dragonPartArray;

    /** The head bounding box of a dragon */
    public EntityDragonPart dragonPartHead;
    public EntityDragonPart dragonPartNeck;

    /** The body bounding box of a dragon */
    public EntityDragonPart dragonPartBody;
    public EntityDragonPart dragonPartTail1;
    public EntityDragonPart dragonPartTail2;
    public EntityDragonPart dragonPartTail3;
    public EntityDragonPart dragonPartWing1;
    public EntityDragonPart dragonPartWing2;

    /** Animation time at previous tick. */
    public float prevAnimTime;

    /**
     * Animation time, used to control the speed of the animation cycles (wings flapping, jaw opening, etc.)
     */
    public float animTime;

    /**
     * Activated if the dragon is flying though obsidian, white stone or bedrock. Slows movement and animation speed.
     */
    public boolean slowed;
    public int deathTicks;

    /** The current endercrystal that is healing this dragon */
    public EntityEnderCrystal healingEnderCrystal;
    private final DragonFightManager fightManager;
    private final PhaseManager phaseManager;
    private int field_184678_bK = 200;
    private int field_184679_bL;
    private final PathPoint[] field_184680_bM = new PathPoint[24];
    private final int[] field_184681_bN = new int[24];
    private final Path field_184682_bO = new Path();

    public EntityDragon(World worldIn)
    {
        super(worldIn);
        this.dragonPartArray = new EntityDragonPart[] {this.dragonPartHead = new EntityDragonPart(this, "head", 6.0F, 6.0F), this.dragonPartNeck = new EntityDragonPart(this, "neck", 6.0F, 6.0F), this.dragonPartBody = new EntityDragonPart(this, "body", 8.0F, 8.0F), this.dragonPartTail1 = new EntityDragonPart(this, "tail", 4.0F, 4.0F), this.dragonPartTail2 = new EntityDragonPart(this, "tail", 4.0F, 4.0F), this.dragonPartTail3 = new EntityDragonPart(this, "tail", 4.0F, 4.0F), this.dragonPartWing1 = new EntityDragonPart(this, "wing", 4.0F, 4.0F), this.dragonPartWing2 = new EntityDragonPart(this, "wing", 4.0F, 4.0F)};
        this.setHealth(this.getMaxHealth());
        this.setSize(16.0F, 8.0F);
        this.noClip = true;
        this.isImmuneToFire = true;
        this.field_184678_bK = 100;
        this.ignoreFrustumCheck = true;

        if (!worldIn.isRemote && worldIn.provider instanceof WorldProviderEnd)
        {
            this.fightManager = ((WorldProviderEnd)worldIn.provider).getDragonFightManager();
        }
        else
        {
            this.fightManager = null;
        }

        this.phaseManager = new PhaseManager(this);
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(200.0D);
    }

    protected void entityInit()
    {
        super.entityInit();
        this.getDataManager().register(PHASE, Integer.valueOf(PhaseList.HOVER.getId()));
    }

    /**
     * Returns a double[3] array with movement offsets, used to calculate trailing tail/neck positions. [0] = yaw
     * offset, [1] = y offset, [2] = unused, always 0. Parameters: buffer index offset, partial ticks.
     */
    public double[] getMovementOffsets(int p_70974_1_, float p_70974_2_)
    {
        if (this.getHealth() <= 0.0F)
        {
            p_70974_2_ = 0.0F;
        }

        p_70974_2_ = 1.0F - p_70974_2_;
        int i = this.ringBufferIndex - p_70974_1_ & 63;
        int j = this.ringBufferIndex - p_70974_1_ - 1 & 63;
        double[] adouble = new double[3];
        double d0 = this.ringBuffer[i][0];
        double d1 = MathHelper.wrapAngleTo180_double(this.ringBuffer[j][0] - d0);
        adouble[0] = d0 + d1 * (double)p_70974_2_;
        d0 = this.ringBuffer[i][1];
        d1 = this.ringBuffer[j][1] - d0;
        adouble[1] = d0 + d1 * (double)p_70974_2_;
        adouble[2] = this.ringBuffer[i][2] + (this.ringBuffer[j][2] - this.ringBuffer[i][2]) * (double)p_70974_2_;
        return adouble;
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (this.worldObj.isRemote)
        {
            this.setHealth(this.getHealth());

            if (!this.isSilent())
            {
                float f = MathHelper.cos(this.animTime * ((float)Math.PI * 2F));
                float f1 = MathHelper.cos(this.prevAnimTime * ((float)Math.PI * 2F));

                if (f1 <= -0.3F && f >= -0.3F)
                {
                    this.worldObj.func_184134_a(this.posX, this.posY, this.posZ, SoundEvents.entity_enderdragon_flap, this.getSoundCategory(), 5.0F, 0.8F + this.rand.nextFloat() * 0.3F, false);
                }

                if (!this.phaseManager.func_188756_a().func_188654_a() && --this.field_184678_bK < 0)
                {
                    this.worldObj.func_184134_a(this.posX, this.posY, this.posZ, SoundEvents.entity_enderdragon_growl, this.getSoundCategory(), 2.5F, 0.8F + this.rand.nextFloat() * 0.3F, false);
                    this.field_184678_bK = 200 + this.rand.nextInt(200);
                }
            }
        }

        this.prevAnimTime = this.animTime;

        if (this.getHealth() <= 0.0F)
        {
            float f13 = (this.rand.nextFloat() - 0.5F) * 8.0F;
            float f15 = (this.rand.nextFloat() - 0.5F) * 4.0F;
            float f17 = (this.rand.nextFloat() - 0.5F) * 8.0F;
            this.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_LARGE, this.posX + (double)f13, this.posY + 2.0D + (double)f15, this.posZ + (double)f17, 0.0D, 0.0D, 0.0D, new int[0]);
        }
        else
        {
            this.updateDragonEnderCrystal();
            float f12 = 0.2F / (MathHelper.sqrt_double(this.motionX * this.motionX + this.motionZ * this.motionZ) * 10.0F + 1.0F);
            f12 = f12 * (float)Math.pow(2.0D, this.motionY);

            if (this.phaseManager.func_188756_a().func_188654_a())
            {
                this.animTime += 0.1F;
            }
            else if (this.slowed)
            {
                this.animTime += f12 * 0.5F;
            }
            else
            {
                this.animTime += f12;
            }

            this.rotationYaw = MathHelper.wrapAngleTo180_float(this.rotationYaw);

            if (this.isAIDisabled())
            {
                this.animTime = 0.5F;
            }
            else
            {
                if (this.ringBufferIndex < 0)
                {
                    for (int i = 0; i < this.ringBuffer.length; ++i)
                    {
                        this.ringBuffer[i][0] = (double)this.rotationYaw;
                        this.ringBuffer[i][1] = this.posY;
                    }
                }

                if (++this.ringBufferIndex == this.ringBuffer.length)
                {
                    this.ringBufferIndex = 0;
                }

                this.ringBuffer[this.ringBufferIndex][0] = (double)this.rotationYaw;
                this.ringBuffer[this.ringBufferIndex][1] = this.posY;

                if (this.worldObj.isRemote)
                {
                    if (this.newPosRotationIncrements > 0)
                    {
                        double d5 = this.posX + (this.field_184623_bh - this.posX) / (double)this.newPosRotationIncrements;
                        double d0 = this.posY + (this.field_184624_bi - this.posY) / (double)this.newPosRotationIncrements;
                        double d1 = this.posZ + (this.field_184625_bj - this.posZ) / (double)this.newPosRotationIncrements;
                        double d2 = MathHelper.wrapAngleTo180_double(this.field_184626_bk - (double)this.rotationYaw);
                        this.rotationYaw = (float)((double)this.rotationYaw + d2 / (double)this.newPosRotationIncrements);
                        this.rotationPitch = (float)((double)this.rotationPitch + (this.newPosX - (double)this.rotationPitch) / (double)this.newPosRotationIncrements);
                        --this.newPosRotationIncrements;
                        this.setPosition(d5, d0, d1);
                        this.setRotation(this.rotationYaw, this.rotationPitch);
                    }

                    this.phaseManager.func_188756_a().func_188657_b();
                }
                else
                {
                    IPhase iphase = this.phaseManager.func_188756_a();
                    iphase.func_188659_c();

                    if (this.phaseManager.func_188756_a() != iphase)
                    {
                        iphase = this.phaseManager.func_188756_a();
                        iphase.func_188659_c();
                    }

                    Vec3d vec3d = iphase.func_188650_g();

                    if (vec3d != null)
                    {
                        double d6 = vec3d.xCoord - this.posX;
                        double d7 = vec3d.yCoord - this.posY;
                        double d8 = vec3d.zCoord - this.posZ;
                        double d3 = d6 * d6 + d7 * d7 + d8 * d8;
                        float f6 = iphase.func_188651_f();
                        d7 = MathHelper.clamp_double(d7 / (double)MathHelper.sqrt_double(d6 * d6 + d8 * d8), (double)(-f6), (double)f6);
                        this.motionY += d7 * 0.10000000149011612D;
                        this.rotationYaw = MathHelper.wrapAngleTo180_float(this.rotationYaw);
                        double d4 = MathHelper.clamp_double(MathHelper.wrapAngleTo180_double(180.0D - MathHelper.atan2(d6, d8) * (180D / Math.PI) - (double)this.rotationYaw), -50.0D, 50.0D);
                        Vec3d vec3d1 = (new Vec3d(vec3d.xCoord - this.posX, vec3d.yCoord - this.posY, vec3d.zCoord - this.posZ)).normalize();
                        Vec3d vec3d2 = (new Vec3d((double)MathHelper.sin(this.rotationYaw * 0.017453292F), this.motionY, (double)(-MathHelper.cos(this.rotationYaw * 0.017453292F)))).normalize();
                        float f8 = Math.max(((float)vec3d2.dotProduct(vec3d1) + 0.5F) / 1.5F, 0.0F);
                        this.randomYawVelocity *= 0.8F;
                        this.randomYawVelocity = (float)((double)this.randomYawVelocity + d4 * (double)iphase.func_188653_h());
                        this.rotationYaw += this.randomYawVelocity * 0.1F;
                        float f9 = (float)(2.0D / (d3 + 1.0D));
                        float f10 = 0.06F;
                        this.moveFlying(0.0F, -1.0F, f10 * (f8 * f9 + (1.0F - f9)));

                        if (this.slowed)
                        {
                            this.moveEntity(this.motionX * 0.800000011920929D, this.motionY * 0.800000011920929D, this.motionZ * 0.800000011920929D);
                        }
                        else
                        {
                            this.moveEntity(this.motionX, this.motionY, this.motionZ);
                        }

                        Vec3d vec3d3 = (new Vec3d(this.motionX, this.motionY, this.motionZ)).normalize();
                        float f11 = ((float)vec3d3.dotProduct(vec3d2) + 1.0F) / 2.0F;
                        f11 = 0.8F + 0.15F * f11;
                        this.motionX *= (double)f11;
                        this.motionZ *= (double)f11;
                        this.motionY *= 0.9100000262260437D;
                    }
                }

                this.renderYawOffset = this.rotationYaw;
                this.dragonPartHead.width = this.dragonPartHead.height = 1.0F;
                this.dragonPartNeck.width = this.dragonPartNeck.height = 3.0F;
                this.dragonPartTail1.width = this.dragonPartTail1.height = 2.0F;
                this.dragonPartTail2.width = this.dragonPartTail2.height = 2.0F;
                this.dragonPartTail3.width = this.dragonPartTail3.height = 2.0F;
                this.dragonPartBody.height = 3.0F;
                this.dragonPartBody.width = 5.0F;
                this.dragonPartWing1.height = 2.0F;
                this.dragonPartWing1.width = 4.0F;
                this.dragonPartWing2.height = 3.0F;
                this.dragonPartWing2.width = 4.0F;
                float f14 = (float)(this.getMovementOffsets(5, 1.0F)[1] - this.getMovementOffsets(10, 1.0F)[1]) * 10.0F * 0.017453292F;
                float f16 = MathHelper.cos(f14);
                float f18 = MathHelper.sin(f14);
                float f2 = this.rotationYaw * 0.017453292F;
                float f19 = MathHelper.sin(f2);
                float f3 = MathHelper.cos(f2);
                this.dragonPartBody.onUpdate();
                this.dragonPartBody.setLocationAndAngles(this.posX + (double)(f19 * 0.5F), this.posY, this.posZ - (double)(f3 * 0.5F), 0.0F, 0.0F);
                this.dragonPartWing1.onUpdate();
                this.dragonPartWing1.setLocationAndAngles(this.posX + (double)(f3 * 4.5F), this.posY + 2.0D, this.posZ + (double)(f19 * 4.5F), 0.0F, 0.0F);
                this.dragonPartWing2.onUpdate();
                this.dragonPartWing2.setLocationAndAngles(this.posX - (double)(f3 * 4.5F), this.posY + 2.0D, this.posZ - (double)(f19 * 4.5F), 0.0F, 0.0F);

                if (!this.worldObj.isRemote && this.hurtTime == 0)
                {
                    this.collideWithEntities(this.worldObj.getEntitiesWithinAABBExcludingEntity(this, this.dragonPartWing1.getEntityBoundingBox().expand(4.0D, 2.0D, 4.0D).offset(0.0D, -2.0D, 0.0D)));
                    this.collideWithEntities(this.worldObj.getEntitiesWithinAABBExcludingEntity(this, this.dragonPartWing2.getEntityBoundingBox().expand(4.0D, 2.0D, 4.0D).offset(0.0D, -2.0D, 0.0D)));
                    this.attackEntitiesInList(this.worldObj.getEntitiesWithinAABBExcludingEntity(this, this.dragonPartHead.getEntityBoundingBox().func_186662_g(1.0D)));
                    this.attackEntitiesInList(this.worldObj.getEntitiesWithinAABBExcludingEntity(this, this.dragonPartNeck.getEntityBoundingBox().func_186662_g(1.0D)));
                }

                double[] adouble = this.getMovementOffsets(5, 1.0F);
                float f4 = MathHelper.sin(this.rotationYaw * 0.017453292F - this.randomYawVelocity * 0.01F);
                float f20 = MathHelper.cos(this.rotationYaw * 0.017453292F - this.randomYawVelocity * 0.01F);
                this.dragonPartHead.onUpdate();
                this.dragonPartNeck.onUpdate();
                float f5 = this.func_184662_q(1.0F);
                this.dragonPartHead.setLocationAndAngles(this.posX + (double)(f4 * 6.5F * f16), this.posY + (double)f5 + (double)(f18 * 6.5F), this.posZ - (double)(f20 * 6.5F * f16), 0.0F, 0.0F);
                this.dragonPartNeck.setLocationAndAngles(this.posX + (double)(f4 * 5.5F * f16), this.posY + (double)f5 + (double)(f18 * 5.5F), this.posZ - (double)(f20 * 5.5F * f16), 0.0F, 0.0F);

                for (int j = 0; j < 3; ++j)
                {
                    EntityDragonPart entitydragonpart = null;

                    if (j == 0)
                    {
                        entitydragonpart = this.dragonPartTail1;
                    }

                    if (j == 1)
                    {
                        entitydragonpart = this.dragonPartTail2;
                    }

                    if (j == 2)
                    {
                        entitydragonpart = this.dragonPartTail3;
                    }

                    double[] adouble1 = this.getMovementOffsets(12 + j * 2, 1.0F);
                    float f21 = this.rotationYaw * 0.017453292F + this.simplifyAngle(adouble1[0] - adouble[0]) * 0.017453292F;
                    float f22 = MathHelper.sin(f21);
                    float f7 = MathHelper.cos(f21);
                    float f23 = 1.5F;
                    float f24 = (float)(j + 1) * 2.0F;
                    entitydragonpart.onUpdate();
                    entitydragonpart.setLocationAndAngles(this.posX - (double)((f19 * f23 + f22 * f24) * f16), this.posY + (adouble1[1] - adouble[1]) - (double)((f24 + f23) * f18) + 1.5D, this.posZ + (double)((f3 * f23 + f7 * f24) * f16), 0.0F, 0.0F);
                }

                if (!this.worldObj.isRemote)
                {
                    this.slowed = this.destroyBlocksInAABB(this.dragonPartHead.getEntityBoundingBox()) | this.destroyBlocksInAABB(this.dragonPartNeck.getEntityBoundingBox()) | this.destroyBlocksInAABB(this.dragonPartBody.getEntityBoundingBox());

                    if (this.fightManager != null)
                    {
                        this.fightManager.func_186099_b(this);
                    }
                }
            }
        }
    }

    private float func_184662_q(float p_184662_1_)
    {
        double d0 = 0.0D;

        if (this.phaseManager.func_188756_a().func_188654_a())
        {
            d0 = -1.0D;
        }
        else
        {
            double[] adouble = this.getMovementOffsets(5, 1.0F);
            double[] adouble1 = this.getMovementOffsets(0, 1.0F);
            d0 = adouble[1] - adouble1[0];
        }

        return (float)d0;
    }

    /**
     * Updates the state of the enderdragon's current endercrystal.
     */
    private void updateDragonEnderCrystal()
    {
        if (this.healingEnderCrystal != null)
        {
            if (this.healingEnderCrystal.isDead)
            {
                this.healingEnderCrystal = null;
            }
            else if (this.ticksExisted % 10 == 0 && this.getHealth() < this.getMaxHealth())
            {
                this.setHealth(this.getHealth() + 1.0F);
            }
        }

        if (this.rand.nextInt(10) == 0)
        {
            List<EntityEnderCrystal> list = this.worldObj.<EntityEnderCrystal>getEntitiesWithinAABB(EntityEnderCrystal.class, this.getEntityBoundingBox().func_186662_g(32.0D));
            EntityEnderCrystal entityendercrystal = null;
            double d0 = Double.MAX_VALUE;

            for (EntityEnderCrystal entityendercrystal1 : list)
            {
                double d1 = entityendercrystal1.getDistanceSqToEntity(this);

                if (d1 < d0)
                {
                    d0 = d1;
                    entityendercrystal = entityendercrystal1;
                }
            }

            this.healingEnderCrystal = entityendercrystal;
        }
    }

    /**
     * Pushes all entities inside the list away from the enderdragon.
     */
    private void collideWithEntities(List<Entity> p_70970_1_)
    {
        double d0 = (this.dragonPartBody.getEntityBoundingBox().minX + this.dragonPartBody.getEntityBoundingBox().maxX) / 2.0D;
        double d1 = (this.dragonPartBody.getEntityBoundingBox().minZ + this.dragonPartBody.getEntityBoundingBox().maxZ) / 2.0D;

        for (Entity entity : p_70970_1_)
        {
            if (entity instanceof EntityLivingBase)
            {
                double d2 = entity.posX - d0;
                double d3 = entity.posZ - d1;
                double d4 = d2 * d2 + d3 * d3;
                entity.addVelocity(d2 / d4 * 4.0D, 0.20000000298023224D, d3 / d4 * 4.0D);

                if (!this.phaseManager.func_188756_a().func_188654_a() && ((EntityLivingBase)entity).getRevengeTimer() < entity.ticksExisted - 2)
                {
                    entity.attackEntityFrom(DamageSource.causeMobDamage(this), 5.0F);
                    this.applyEnchantments(this, entity);
                }
            }
        }
    }

    /**
     * Attacks all entities inside this list, dealing 5 hearts of damage.
     */
    private void attackEntitiesInList(List<Entity> p_70971_1_)
    {
        for (int i = 0; i < p_70971_1_.size(); ++i)
        {
            Entity entity = (Entity)p_70971_1_.get(i);

            if (entity instanceof EntityLivingBase)
            {
                entity.attackEntityFrom(DamageSource.causeMobDamage(this), 10.0F);
                this.applyEnchantments(this, entity);
            }
        }
    }

    /**
     * Simplifies the value of a number by adding/subtracting 180 to the point that the number is between -180 and 180.
     */
    private float simplifyAngle(double p_70973_1_)
    {
        return (float)MathHelper.wrapAngleTo180_double(p_70973_1_);
    }

    /**
     * Destroys all blocks that aren't associated with 'The End' inside the given bounding box.
     */
    private boolean destroyBlocksInAABB(AxisAlignedBB p_70972_1_)
    {
        int i = MathHelper.floor_double(p_70972_1_.minX);
        int j = MathHelper.floor_double(p_70972_1_.minY);
        int k = MathHelper.floor_double(p_70972_1_.minZ);
        int l = MathHelper.floor_double(p_70972_1_.maxX);
        int i1 = MathHelper.floor_double(p_70972_1_.maxY);
        int j1 = MathHelper.floor_double(p_70972_1_.maxZ);
        boolean flag = false;
        boolean flag1 = false;

        for (int k1 = i; k1 <= l; ++k1)
        {
            for (int l1 = j; l1 <= i1; ++l1)
            {
                for (int i2 = k; i2 <= j1; ++i2)
                {
                    BlockPos blockpos = new BlockPos(k1, l1, i2);
                    IBlockState iblockstate = this.worldObj.getBlockState(blockpos);
                    Block block = iblockstate.getBlock();

                    if (iblockstate.getMaterial() != Material.air && iblockstate.getMaterial() != Material.fire)
                    {
                        if (!this.worldObj.getGameRules().getBoolean("mobGriefing"))
                        {
                            flag = true;
                        }
                        else if (block != Blocks.barrier && block != Blocks.obsidian && block != Blocks.end_stone && block != Blocks.bedrock && block != Blocks.end_portal && block != Blocks.end_portal_frame)
                        {
                            if (block != Blocks.command_block && block != Blocks.repeating_command_block && block != Blocks.chain_command_block && block != Blocks.iron_bars && block != Blocks.end_gateway)
                            {
                                flag1 = this.worldObj.setBlockToAir(blockpos) || flag1;
                            }
                            else
                            {
                                flag = true;
                            }
                        }
                        else
                        {
                            flag = true;
                        }
                    }
                }
            }
        }

        if (flag1)
        {
            double d0 = p_70972_1_.minX + (p_70972_1_.maxX - p_70972_1_.minX) * (double)this.rand.nextFloat();
            double d1 = p_70972_1_.minY + (p_70972_1_.maxY - p_70972_1_.minY) * (double)this.rand.nextFloat();
            double d2 = p_70972_1_.minZ + (p_70972_1_.maxZ - p_70972_1_.minZ) * (double)this.rand.nextFloat();
            this.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_LARGE, d0, d1, d2, 0.0D, 0.0D, 0.0D, new int[0]);
        }

        return flag;
    }

    public boolean attackEntityFromPart(EntityDragonPart dragonPart, DamageSource source, float p_70965_3_)
    {
        p_70965_3_ = this.phaseManager.func_188756_a().func_188656_a(dragonPart, source, p_70965_3_);

        if (dragonPart != this.dragonPartHead)
        {
            p_70965_3_ = p_70965_3_ / 4.0F + Math.min(p_70965_3_, 1.0F);
        }

        if (p_70965_3_ < 0.01F)
        {
            return false;
        }
        else
        {
            if (source.getEntity() instanceof EntityPlayer || source.isExplosion())
            {
                float f = this.getHealth();
                this.attackDragonFrom(source, p_70965_3_);

                if (this.getHealth() <= 0.0F && !this.phaseManager.func_188756_a().func_188654_a())
                {
                    this.setHealth(1.0F);
                    this.phaseManager.func_188758_a(PhaseList.DYING);
                }

                if (this.phaseManager.func_188756_a().func_188654_a())
                {
                    this.field_184679_bL = (int)((float)this.field_184679_bL + (f - this.getHealth()));

                    if ((float)this.field_184679_bL > 0.25F * this.getMaxHealth())
                    {
                        this.field_184679_bL = 0;
                        this.phaseManager.func_188758_a(PhaseList.TAKEOFF);
                    }
                }
            }

            return true;
        }
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        if (source instanceof EntityDamageSource && ((EntityDamageSource)source).getIsThornsDamage())
        {
            this.attackEntityFromPart(this.dragonPartBody, source, amount);
        }

        return false;
    }

    /**
     * Provides a way to cause damage to an ender dragon.
     */
    protected boolean attackDragonFrom(DamageSource source, float amount)
    {
        return super.attackEntityFrom(source, amount);
    }

    /**
     * Called by the /kill command.
     */
    public void onKillCommand()
    {
        this.setDead();

        if (this.fightManager != null)
        {
            this.fightManager.func_186099_b(this);
            this.fightManager.processDragonDeath(this);
        }
    }

    /**
     * handles entity death timer, experience orb and particle creation
     */
    protected void onDeathUpdate()
    {
        if (this.fightManager != null)
        {
            this.fightManager.func_186099_b(this);
        }

        ++this.deathTicks;

        if (this.deathTicks >= 180 && this.deathTicks <= 200)
        {
            float f = (this.rand.nextFloat() - 0.5F) * 8.0F;
            float f1 = (this.rand.nextFloat() - 0.5F) * 4.0F;
            float f2 = (this.rand.nextFloat() - 0.5F) * 8.0F;
            this.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_HUGE, this.posX + (double)f, this.posY + 2.0D + (double)f1, this.posZ + (double)f2, 0.0D, 0.0D, 0.0D, new int[0]);
        }

        boolean flag = this.worldObj.getGameRules().getBoolean("doMobLoot");
        int i = 500;

        if (this.fightManager != null && !this.fightManager.func_186102_d())
        {
            i = 12000;
        }

        if (!this.worldObj.isRemote)
        {
            if (this.deathTicks > 150 && this.deathTicks % 5 == 0 && flag)
            {
                this.func_184668_a(MathHelper.floor_float((float)i * 0.08F));
            }

            if (this.deathTicks == 1)
            {
                this.worldObj.playBroadcastSound(1028, new BlockPos(this), 0);
            }
        }

        this.moveEntity(0.0D, 0.10000000149011612D, 0.0D);
        this.renderYawOffset = this.rotationYaw += 20.0F;

        if (this.deathTicks == 200 && !this.worldObj.isRemote)
        {
            if (flag)
            {
                this.func_184668_a(MathHelper.floor_float((float)i * 0.2F));
            }

            if (this.fightManager != null)
            {
                this.fightManager.processDragonDeath(this);
            }

            this.setDead();
        }
    }

    private void func_184668_a(int p_184668_1_)
    {
        while (p_184668_1_ > 0)
        {
            int i = EntityXPOrb.getXPSplit(p_184668_1_);
            p_184668_1_ -= i;
            this.worldObj.spawnEntityInWorld(new EntityXPOrb(this.worldObj, this.posX, this.posY, this.posZ, i));
        }
    }

    public int func_184671_o()
    {
        if (this.field_184680_bM[0] == null)
        {
            int i = 0;
            int j = 0;
            int k = 0;
            int l = 0;

            for (int i1 = 0; i1 < 24; ++i1)
            {
                int j1 = 5;

                if (i1 < 12)
                {
                    i = (int)(60.0F * MathHelper.cos(2.0F * (-(float)Math.PI + 0.2617994F * (float)i1)));
                    k = (int)(60.0F * MathHelper.sin(2.0F * (-(float)Math.PI + 0.2617994F * (float)i1)));
                }
                else if (i1 < 20)
                {
                    l = i1 - 12;
                    i = (int)(40.0F * MathHelper.cos(2.0F * (-(float)Math.PI + 0.3926991F * (float)l)));
                    k = (int)(40.0F * MathHelper.sin(2.0F * (-(float)Math.PI + 0.3926991F * (float)l)));
                    j1 += 10;
                }
                else
                {
                    l = i1 - 20;
                    i = (int)(20.0F * MathHelper.cos(2.0F * (-(float)Math.PI + ((float)Math.PI / 4F) * (float)l)));
                    k = (int)(20.0F * MathHelper.sin(2.0F * (-(float)Math.PI + ((float)Math.PI / 4F) * (float)l)));
                }

                j = Math.max(this.worldObj.getSeaLevel() + 10, this.worldObj.getTopSolidOrLiquidBlock(new BlockPos(i, 0, k)).getY() + j1);
                this.field_184680_bM[i1] = new PathPoint(i, j, k);
            }

            this.field_184681_bN[0] = 6146;
            this.field_184681_bN[1] = 8197;
            this.field_184681_bN[2] = 8202;
            this.field_184681_bN[3] = 16404;
            this.field_184681_bN[4] = 32808;
            this.field_184681_bN[5] = 32848;
            this.field_184681_bN[6] = 65696;
            this.field_184681_bN[7] = 131392;
            this.field_184681_bN[8] = 131712;
            this.field_184681_bN[9] = 263424;
            this.field_184681_bN[10] = 526848;
            this.field_184681_bN[11] = 525313;
            this.field_184681_bN[12] = 1581057;
            this.field_184681_bN[13] = 3166214;
            this.field_184681_bN[14] = 2138120;
            this.field_184681_bN[15] = 6373424;
            this.field_184681_bN[16] = 4358208;
            this.field_184681_bN[17] = 12910976;
            this.field_184681_bN[18] = 9044480;
            this.field_184681_bN[19] = 9706496;
            this.field_184681_bN[20] = 15216640;
            this.field_184681_bN[21] = 13688832;
            this.field_184681_bN[22] = 11763712;
            this.field_184681_bN[23] = 8257536;
        }

        return this.func_184663_l(this.posX, this.posY, this.posZ);
    }

    public int func_184663_l(double p_184663_1_, double p_184663_3_, double p_184663_5_)
    {
        float f = 10000.0F;
        int i = 0;
        PathPoint pathpoint = new PathPoint(MathHelper.floor_double(p_184663_1_), MathHelper.floor_double(p_184663_3_), MathHelper.floor_double(p_184663_5_));
        int j = 0;

        if (this.fightManager == null || this.fightManager.func_186092_c() == 0)
        {
            j = 12;
        }

        for (int k = j; k < 24; ++k)
        {
            if (this.field_184680_bM[k] != null)
            {
                float f1 = this.field_184680_bM[k].distanceToSquared(pathpoint);

                if (f1 < f)
                {
                    f = f1;
                    i = k;
                }
            }
        }

        return i;
    }

    public PathEntity func_184666_a(int p_184666_1_, int p_184666_2_, PathPoint p_184666_3_)
    {
        for (int i = 0; i < 24; ++i)
        {
            PathPoint pathpoint = this.field_184680_bM[i];
            pathpoint.visited = false;
            pathpoint.distanceToTarget = 0.0F;
            pathpoint.totalPathDistance = 0.0F;
            pathpoint.distanceToNext = 0.0F;
            pathpoint.previous = null;
            pathpoint.index = -1;
        }

        PathPoint pathpoint4 = this.field_184680_bM[p_184666_1_];
        PathPoint pathpoint5 = this.field_184680_bM[p_184666_2_];
        pathpoint4.totalPathDistance = 0.0F;
        pathpoint4.distanceToNext = pathpoint4.distanceTo(pathpoint5);
        pathpoint4.distanceToTarget = pathpoint4.distanceToNext;
        this.field_184682_bO.clearPath();
        this.field_184682_bO.addPoint(pathpoint4);
        PathPoint pathpoint1 = pathpoint4;
        int j = 0;

        if (this.fightManager == null || this.fightManager.func_186092_c() == 0)
        {
            j = 12;
        }

        while (!this.field_184682_bO.isPathEmpty())
        {
            PathPoint pathpoint2 = this.field_184682_bO.dequeue();

            if (pathpoint2.equals(pathpoint5))
            {
                if (p_184666_3_ != null)
                {
                    p_184666_3_.previous = pathpoint5;
                    pathpoint5 = p_184666_3_;
                }

                return this.func_184669_a(pathpoint4, pathpoint5);
            }

            if (pathpoint2.distanceTo(pathpoint5) < pathpoint1.distanceTo(pathpoint5))
            {
                pathpoint1 = pathpoint2;
            }

            pathpoint2.visited = true;
            int k = 0;

            for (int l = 0; l < 24; ++l)
            {
                if (this.field_184680_bM[l] == pathpoint2)
                {
                    k = l;
                    break;
                }
            }

            for (int i1 = j; i1 < 24; ++i1)
            {
                if ((this.field_184681_bN[k] & 1 << i1) > 0)
                {
                    PathPoint pathpoint3 = this.field_184680_bM[i1];

                    if (!pathpoint3.visited)
                    {
                        float f = pathpoint2.totalPathDistance + pathpoint2.distanceTo(pathpoint3);

                        if (!pathpoint3.isAssigned() || f < pathpoint3.totalPathDistance)
                        {
                            pathpoint3.previous = pathpoint2;
                            pathpoint3.totalPathDistance = f;
                            pathpoint3.distanceToNext = pathpoint3.distanceTo(pathpoint5);

                            if (pathpoint3.isAssigned())
                            {
                                this.field_184682_bO.changeDistance(pathpoint3, pathpoint3.totalPathDistance + pathpoint3.distanceToNext);
                            }
                            else
                            {
                                pathpoint3.distanceToTarget = pathpoint3.totalPathDistance + pathpoint3.distanceToNext;
                                this.field_184682_bO.addPoint(pathpoint3);
                            }
                        }
                    }
                }
            }
        }

        if (pathpoint1 == pathpoint4)
        {
            return null;
        }
        else
        {
            LOGGER.debug("Failed to find path from {} to {}", new Object[] {Integer.valueOf(p_184666_1_), Integer.valueOf(p_184666_2_)});

            if (p_184666_3_ != null)
            {
                p_184666_3_.previous = pathpoint1;
                pathpoint1 = p_184666_3_;
            }

            return this.func_184669_a(pathpoint4, pathpoint1);
        }
    }

    private PathEntity func_184669_a(PathPoint p_184669_1_, PathPoint p_184669_2_)
    {
        int i = 1;

        for (PathPoint pathpoint = p_184669_2_; pathpoint.previous != null; pathpoint = pathpoint.previous)
        {
            ++i;
        }

        PathPoint[] apathpoint = new PathPoint[i];
        PathPoint pathpoint1 = p_184669_2_;
        --i;

        for (apathpoint[i] = p_184669_2_; pathpoint1.previous != null; apathpoint[i] = pathpoint1)
        {
            pathpoint1 = pathpoint1.previous;
            --i;
        }

        return new PathEntity(apathpoint);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setInteger("DragonPhase", this.phaseManager.func_188756_a().func_188652_i().getId());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.hasKey("DragonPhase"))
        {
            this.phaseManager.func_188758_a(PhaseList.func_188738_a(tagCompund.getInteger("DragonPhase")));
        }
    }

    /**
     * Makes the entity despawn if requirements are reached
     */
    protected void despawnEntity()
    {
    }

    /**
     * Return the Entity parts making up this Entity (currently only for dragons)
     */
    public Entity[] getParts()
    {
        return this.dragonPartArray;
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return false;
    }

    public World getWorld()
    {
        return this.worldObj;
    }

    public SoundCategory getSoundCategory()
    {
        return SoundCategory.HOSTILE;
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_enderdragon_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_enderdragon_hurt;
    }

    /**
     * Returns the volume for the sounds this mob makes.
     */
    protected float getSoundVolume()
    {
        return 5.0F;
    }

    public float func_184667_a(int p_184667_1_, double[] p_184667_2_, double[] p_184667_3_)
    {
        IPhase iphase = this.phaseManager.func_188756_a();
        PhaseList <? extends IPhase > phaselist = iphase.func_188652_i();
        double d0;

        if (phaselist != PhaseList.LANDING && phaselist != PhaseList.TAKEOFF)
        {
            if (iphase.func_188654_a())
            {
                d0 = (double)p_184667_1_;
            }
            else if (p_184667_1_ == 6)
            {
                d0 = 0.0D;
            }
            else
            {
                d0 = p_184667_3_[1] - p_184667_2_[1];
            }
        }
        else
        {
            BlockPos blockpos = this.worldObj.getTopSolidOrLiquidBlock(WorldGenEndPodium.field_186139_a);
            float f = Math.max(MathHelper.sqrt_double(this.getDistanceSqToCenter(blockpos)) / 4.0F, 1.0F);
            d0 = (double)((float)p_184667_1_ / f);
        }

        return (float)d0;
    }

    public Vec3d func_184665_a(float p_184665_1_)
    {
        IPhase iphase = this.phaseManager.func_188756_a();
        PhaseList <? extends IPhase > phaselist = iphase.func_188652_i();
        Vec3d vec3d;

        if (phaselist != PhaseList.LANDING && phaselist != PhaseList.TAKEOFF)
        {
            if (iphase.func_188654_a())
            {
                float f4 = this.rotationPitch;
                float f5 = 1.5F;
                this.rotationPitch = -6.0F * f5 * 5.0F;
                vec3d = this.getLook(p_184665_1_);
                this.rotationPitch = f4;
            }
            else
            {
                vec3d = this.getLook(p_184665_1_);
            }
        }
        else
        {
            BlockPos blockpos = this.worldObj.getTopSolidOrLiquidBlock(WorldGenEndPodium.field_186139_a);
            float f = Math.max(MathHelper.sqrt_double(this.getDistanceSqToCenter(blockpos)) / 4.0F, 1.0F);
            float f1 = 6.0F / f;
            float f2 = this.rotationPitch;
            float f3 = 1.5F;
            this.rotationPitch = -f1 * f3 * 5.0F;
            vec3d = this.getLook(p_184665_1_);
            this.rotationPitch = f2;
        }

        return vec3d;
    }

    public void func_184672_a(EntityEnderCrystal p_184672_1_, BlockPos p_184672_2_, DamageSource p_184672_3_)
    {
        EntityPlayer entityplayer;

        if (p_184672_3_.getEntity() instanceof EntityPlayer)
        {
            entityplayer = (EntityPlayer)p_184672_3_.getEntity();
        }
        else
        {
            entityplayer = this.worldObj.func_184139_a(p_184672_2_, 64.0D, 64.0D);
        }

        if (p_184672_1_ == this.healingEnderCrystal)
        {
            this.attackEntityFromPart(this.dragonPartHead, DamageSource.func_188405_b(entityplayer), 10.0F);
        }

        this.phaseManager.func_188756_a().func_188655_a(p_184672_1_, p_184672_2_, p_184672_3_, entityplayer);
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        if (PHASE.equals(key) && this.worldObj.isRemote)
        {
            this.phaseManager.func_188758_a(PhaseList.func_188738_a(((Integer)this.getDataManager().get(PHASE)).intValue()));
        }

        super.notifyDataManagerChange(key);
    }

    public PhaseManager getPhaseManager()
    {
        return this.phaseManager;
    }

    public DragonFightManager getFightManager()
    {
        return this.fightManager;
    }

    /**
     * adds a PotionEffect to the entity
     */
    public void addPotionEffect(PotionEffect potioneffectIn)
    {
    }

    protected boolean canBeRidden(Entity entityIn)
    {
        return false;
    }

    public boolean func_184222_aU()
    {
        return false;
    }
}
