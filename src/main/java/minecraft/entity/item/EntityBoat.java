package net.minecraft.entity.item;

import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.block.BlockLiquid;
import net.minecraft.block.BlockPlanks;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.passive.EntityAnimal;
import net.minecraft.entity.passive.EntityWaterMob;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.network.play.client.CPacketSteerBoat;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntityDamageSourceIndirect;
import net.minecraft.util.EntitySelectors;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class EntityBoat extends Entity
{
    private static final DataParameter<Integer> TIME_SINCE_HIT = EntityDataManager.<Integer>createKey(EntityBoat.class, DataSerializers.VARINT);
    private static final DataParameter<Integer> FORWARD_DIRECTION = EntityDataManager.<Integer>createKey(EntityBoat.class, DataSerializers.VARINT);
    private static final DataParameter<Float> DAMAGE_TAKEN = EntityDataManager.<Float>createKey(EntityBoat.class, DataSerializers.FLOAT);
    private static final DataParameter<Integer> BOAT_TYPE = EntityDataManager.<Integer>createKey(EntityBoat.class, DataSerializers.VARINT);
    private static final DataParameter<Boolean>[] field_184468_e = new DataParameter[] {EntityDataManager.createKey(EntityBoat.class, DataSerializers.BOOLEAN), EntityDataManager.createKey(EntityBoat.class, DataSerializers.BOOLEAN)};
    private float[] field_184470_f;
    private float field_184472_g;
    private float field_184474_h;
    private float field_184475_as;
    private int field_184476_at;
    private double boatPitch;
    private double field_184477_av;
    private double field_184478_aw;
    private double boatYaw;
    private double field_184479_ay;
    private boolean field_184480_az;
    private boolean field_184459_aA;
    private boolean field_184461_aB;
    private boolean field_184463_aC;
    private double field_184465_aD;
    private float field_184467_aE;
    private EntityBoat.Status field_184469_aF;
    private EntityBoat.Status field_184471_aG;
    private double field_184473_aH;

    public EntityBoat(World worldIn)
    {
        super(worldIn);
        this.field_184470_f = new float[2];
        this.preventEntitySpawning = true;
        this.setSize(1.375F, 0.5625F);
    }

    public EntityBoat(World worldIn, double x, double y, double z)
    {
        this(worldIn);
        this.setPosition(x, y, z);
        this.motionX = 0.0D;
        this.motionY = 0.0D;
        this.motionZ = 0.0D;
        this.prevPosX = x;
        this.prevPosY = y;
        this.prevPosZ = z;
    }

    /**
     * returns if this entity triggers Block.onEntityWalking on the blocks they walk on. used for spiders and wolves to
     * prevent them from trampling crops
     */
    protected boolean canTriggerWalking()
    {
        return false;
    }

    protected void entityInit()
    {
        this.dataWatcher.register(TIME_SINCE_HIT, Integer.valueOf(0));
        this.dataWatcher.register(FORWARD_DIRECTION, Integer.valueOf(1));
        this.dataWatcher.register(DAMAGE_TAKEN, Float.valueOf(0.0F));
        this.dataWatcher.register(BOAT_TYPE, Integer.valueOf(EntityBoat.Type.OAK.ordinal()));

        for (int i = 0; i < field_184468_e.length; ++i)
        {
            this.dataWatcher.register(field_184468_e[i], Boolean.valueOf(false));
        }
    }

    /**
     * Returns a boundingBox used to collide the entity with other entities and blocks. This enables the entity to be
     * pushable on contact, like boats or minecarts.
     */
    public AxisAlignedBB getCollisionBox(Entity entityIn)
    {
        return entityIn.getEntityBoundingBox();
    }

    /**
     * Returns the collision bounding box for this entity
     */
    public AxisAlignedBB getCollisionBoundingBox()
    {
        return this.getEntityBoundingBox();
    }

    /**
     * Returns true if this entity should push and be pushed by other entities when colliding.
     */
    public boolean canBePushed()
    {
        return true;
    }

    /**
     * Returns the Y offset from the entity's position for any entity riding this one.
     */
    public double getMountedYOffset()
    {
        return -0.1D;
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
        else if (!this.worldObj.isRemote && !this.isDead)
        {
            if (source instanceof EntityDamageSourceIndirect && source.getEntity() != null && this.isPassenger(source.getEntity()))
            {
                return false;
            }
            else
            {
                this.setForwardDirection(-this.getForwardDirection());
                this.setTimeSinceHit(10);
                this.setDamageTaken(this.getDamageTaken() + amount * 10.0F);
                this.setBeenAttacked();
                boolean flag = source.getEntity() instanceof EntityPlayer && ((EntityPlayer)source.getEntity()).capabilities.isCreativeMode;

                if (flag || this.getDamageTaken() > 40.0F)
                {
                    if (!flag && this.worldObj.getGameRules().getBoolean("doEntityDrops"))
                    {
                        this.dropItemWithOffset(this.func_184455_j(), 1, 0.0F);
                    }

                    this.setDead();
                }

                return true;
            }
        }
        else
        {
            return true;
        }
    }

    /**
     * Applies a velocity to each of the entities pushing them away from each other. Args: entity
     */
    public void applyEntityCollision(Entity entityIn)
    {
        if (entityIn instanceof EntityBoat)
        {
            if (entityIn.getEntityBoundingBox().minY < this.getEntityBoundingBox().maxY)
            {
                super.applyEntityCollision(entityIn);
            }
        }
        else if (entityIn.getEntityBoundingBox().minY <= this.getEntityBoundingBox().minY)
        {
            super.applyEntityCollision(entityIn);
        }
    }

    public Item func_184455_j()
    {
        switch (this.func_184453_r())
        {
            case OAK:
            default:
                return Items.boat;

            case SPRUCE:
                return Items.spruce_boat;

            case BIRCH:
                return Items.birch_boat;

            case JUNGLE:
                return Items.jungle_boat;

            case ACACIA:
                return Items.acacia_boat;

            case DARK_OAK:
                return Items.dark_oak_boat;
        }
    }

    /**
     * Setups the entity to do the hurt animation. Only used by packets in multiplayer.
     */
    public void performHurtAnimation()
    {
        this.setForwardDirection(-this.getForwardDirection());
        this.setTimeSinceHit(10);
        this.setDamageTaken(this.getDamageTaken() * 11.0F);
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return !this.isDead;
    }

    public void setPositionAndRotation2(double x, double y, double z, float yaw, float pitch, int posRotationIncrements, boolean p_180426_10_)
    {
        this.boatPitch = x;
        this.field_184477_av = y;
        this.field_184478_aw = z;
        this.boatYaw = (double)yaw;
        this.field_184479_ay = (double)pitch;
        this.field_184476_at = 10;
    }

    public EnumFacing func_184172_bi()
    {
        return this.getHorizontalFacing().rotateY();
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        this.field_184471_aG = this.field_184469_aF;
        this.field_184469_aF = this.func_184449_t();

        if (this.field_184469_aF != EntityBoat.Status.UNDER_WATER && this.field_184469_aF != EntityBoat.Status.UNDER_FLOWING_WATER)
        {
            this.field_184474_h = 0.0F;
        }
        else
        {
            ++this.field_184474_h;
        }

        if (!this.worldObj.isRemote && this.field_184474_h >= 60.0F)
        {
            this.removePassengers();
        }

        if (this.getTimeSinceHit() > 0)
        {
            this.setTimeSinceHit(this.getTimeSinceHit() - 1);
        }

        if (this.getDamageTaken() > 0.0F)
        {
            this.setDamageTaken(this.getDamageTaken() - 1.0F);
        }

        this.prevPosX = this.posX;
        this.prevPosY = this.posY;
        this.prevPosZ = this.posZ;
        super.onUpdate();
        this.func_184447_s();

        if (this.func_184186_bw())
        {
            if (this.getPassengers().size() == 0 || !(this.getPassengers().get(0) instanceof EntityPlayer))
            {
                this.func_184445_a(false, false);
            }

            this.func_184450_w();

            if (this.worldObj.isRemote)
            {
                this.func_184443_x();
                this.worldObj.sendPacketToServer(new CPacketSteerBoat(this.func_184457_a(0), this.func_184457_a(1)));
            }

            this.moveEntity(this.motionX, this.motionY, this.motionZ);
        }
        else
        {
            this.motionX = 0.0D;
            this.motionY = 0.0D;
            this.motionZ = 0.0D;
        }

        for (int i = 0; i <= 1; ++i)
        {
            if (this.func_184457_a(i))
            {
                this.field_184470_f[i] = (float)((double)this.field_184470_f[i] + 0.01D);
            }
            else
            {
                this.field_184470_f[i] = 0.0F;
            }
        }

        this.doBlockCollisions();
        List<Entity> list = this.worldObj.getEntitiesInAABBexcluding(this, this.getEntityBoundingBox().expand(0.20000000298023224D, -0.009999999776482582D, 0.20000000298023224D), EntitySelectors.<Entity>func_188442_a(this));

        if (!list.isEmpty())
        {
            boolean flag = !this.worldObj.isRemote && !(this.getControllingPassenger() instanceof EntityPlayer);

            for (int j = 0; j < list.size(); ++j)
            {
                Entity entity = (Entity)list.get(j);

                if (!entity.isPassenger(this))
                {
                    if (flag && this.getPassengers().size() < 2 && !entity.isRiding() && entity.width < this.width && entity instanceof EntityLivingBase && !(entity instanceof EntityWaterMob) && !(entity instanceof EntityPlayer))
                    {
                        entity.startRiding(this);
                    }
                    else
                    {
                        this.applyEntityCollision(entity);
                    }
                }
            }
        }
    }

    private void func_184447_s()
    {
        if (this.field_184476_at > 0 && !this.func_184186_bw())
        {
            double d0 = this.posX + (this.boatPitch - this.posX) / (double)this.field_184476_at;
            double d1 = this.posY + (this.field_184477_av - this.posY) / (double)this.field_184476_at;
            double d2 = this.posZ + (this.field_184478_aw - this.posZ) / (double)this.field_184476_at;
            double d3 = MathHelper.wrapAngleTo180_double(this.boatYaw - (double)this.rotationYaw);
            this.rotationYaw = (float)((double)this.rotationYaw + d3 / (double)this.field_184476_at);
            this.rotationPitch = (float)((double)this.rotationPitch + (this.field_184479_ay - (double)this.rotationPitch) / (double)this.field_184476_at);
            --this.field_184476_at;
            this.setPosition(d0, d1, d2);
            this.setRotation(this.rotationYaw, this.rotationPitch);
        }
    }

    public void func_184445_a(boolean p_184445_1_, boolean p_184445_2_)
    {
        this.dataWatcher.set(field_184468_e[0], Boolean.valueOf(p_184445_1_));
        this.dataWatcher.set(field_184468_e[1], Boolean.valueOf(p_184445_2_));
    }

    public float func_184448_a(int p_184448_1_, float p_184448_2_)
    {
        return this.func_184457_a(p_184448_1_) ? (float)MathHelper.denormalizeClamp((double)this.field_184470_f[p_184448_1_] - 0.01D, (double)this.field_184470_f[p_184448_1_], (double)p_184448_2_) : 0.0F;
    }

    private EntityBoat.Status func_184449_t()
    {
        EntityBoat.Status entityboat$status = this.func_184444_v();

        if (entityboat$status != null)
        {
            this.field_184465_aD = this.getEntityBoundingBox().maxY;
            return entityboat$status;
        }
        else if (this.func_184446_u())
        {
            return EntityBoat.Status.IN_WATER;
        }
        else
        {
            float f = this.func_184441_l();

            if (f > 0.0F)
            {
                this.field_184467_aE = f;
                return EntityBoat.Status.ON_LAND;
            }
            else
            {
                return EntityBoat.Status.IN_AIR;
            }
        }
    }

    public float func_184451_k()
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
        int i = MathHelper.floor_double(axisalignedbb.minX);
        int j = MathHelper.ceiling_double_int(axisalignedbb.maxX);
        int k = MathHelper.floor_double(axisalignedbb.maxY);
        int l = MathHelper.ceiling_double_int(axisalignedbb.maxY - this.field_184473_aH);
        int i1 = MathHelper.floor_double(axisalignedbb.minZ);
        int j1 = MathHelper.ceiling_double_int(axisalignedbb.maxZ);
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        try
        {
            label78:

            for (int k1 = k; k1 < l; ++k1)
            {
                float f = 0.0F;
                int l1 = i;

                while (true)
                {
                    if (l1 >= j)
                    {
                        if (f < 1.0F)
                        {
                            float f2 = (float)blockpos$pooledmutableblockpos.getY() + f;
                            return f2;
                        }

                        break;
                    }

                    for (int i2 = i1; i2 < j1; ++i2)
                    {
                        blockpos$pooledmutableblockpos.set(l1, k1, i2);
                        IBlockState iblockstate = this.worldObj.getBlockState(blockpos$pooledmutableblockpos);

                        if (iblockstate.getMaterial() == Material.water)
                        {
                            f = Math.max(f, func_184456_a(iblockstate, this.worldObj, blockpos$pooledmutableblockpos));
                        }

                        if (f >= 1.0F)
                        {
                            continue label78;
                        }
                    }

                    ++l1;
                }
            }

            float f1 = (float)(l + 1);
            return f1;
        }
        finally
        {
            blockpos$pooledmutableblockpos.release();
        }
    }

    public float func_184441_l()
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
        AxisAlignedBB axisalignedbb1 = new AxisAlignedBB(axisalignedbb.minX, axisalignedbb.minY - 0.001D, axisalignedbb.minZ, axisalignedbb.maxX, axisalignedbb.minY, axisalignedbb.maxZ);
        int i = MathHelper.floor_double(axisalignedbb1.minX) - 1;
        int j = MathHelper.ceiling_double_int(axisalignedbb1.maxX) + 1;
        int k = MathHelper.floor_double(axisalignedbb1.minY) - 1;
        int l = MathHelper.ceiling_double_int(axisalignedbb1.maxY) + 1;
        int i1 = MathHelper.floor_double(axisalignedbb1.minZ) - 1;
        int j1 = MathHelper.ceiling_double_int(axisalignedbb1.maxZ) + 1;
        List<AxisAlignedBB> list = Lists.<AxisAlignedBB>newArrayList();
        float f = 0.0F;
        int k1 = 0;
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        try
        {
            for (int l1 = i; l1 < j; ++l1)
            {
                for (int i2 = i1; i2 < j1; ++i2)
                {
                    int j2 = (l1 != i && l1 != j - 1 ? 0 : 1) + (i2 != i1 && i2 != j1 - 1 ? 0 : 1);

                    if (j2 != 2)
                    {
                        for (int k2 = k; k2 < l; ++k2)
                        {
                            if (j2 <= 0 || k2 != k && k2 != l - 1)
                            {
                                blockpos$pooledmutableblockpos.set(l1, k2, i2);
                                IBlockState iblockstate = this.worldObj.getBlockState(blockpos$pooledmutableblockpos);
                                iblockstate.func_185908_a(this.worldObj, blockpos$pooledmutableblockpos, axisalignedbb1, list, this);

                                if (!list.isEmpty())
                                {
                                    f += iblockstate.getBlock().slipperiness;
                                    ++k1;
                                }

                                list.clear();
                            }
                        }
                    }
                }
            }
        }
        finally
        {
            blockpos$pooledmutableblockpos.release();
        }

        return f / (float)k1;
    }

    private boolean func_184446_u()
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
        int i = MathHelper.floor_double(axisalignedbb.minX);
        int j = MathHelper.ceiling_double_int(axisalignedbb.maxX);
        int k = MathHelper.floor_double(axisalignedbb.minY);
        int l = MathHelper.ceiling_double_int(axisalignedbb.minY + 0.001D);
        int i1 = MathHelper.floor_double(axisalignedbb.minZ);
        int j1 = MathHelper.ceiling_double_int(axisalignedbb.maxZ);
        boolean flag = false;
        this.field_184465_aD = Double.MIN_VALUE;
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        try
        {
            for (int k1 = i; k1 < j; ++k1)
            {
                for (int l1 = k; l1 < l; ++l1)
                {
                    for (int i2 = i1; i2 < j1; ++i2)
                    {
                        blockpos$pooledmutableblockpos.set(k1, l1, i2);
                        IBlockState iblockstate = this.worldObj.getBlockState(blockpos$pooledmutableblockpos);

                        if (iblockstate.getMaterial() == Material.water)
                        {
                            float f = func_184452_b(iblockstate, this.worldObj, blockpos$pooledmutableblockpos);
                            this.field_184465_aD = Math.max((double)f, this.field_184465_aD);
                            flag |= axisalignedbb.minY < (double)f;
                        }
                    }
                }
            }
        }
        finally
        {
            blockpos$pooledmutableblockpos.release();
        }

        return flag;
    }

    private EntityBoat.Status func_184444_v()
    {
        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox();
        double d0 = axisalignedbb.maxY + 0.001D;
        int i = MathHelper.floor_double(axisalignedbb.minX);
        int j = MathHelper.ceiling_double_int(axisalignedbb.maxX);
        int k = MathHelper.floor_double(axisalignedbb.maxY);
        int l = MathHelper.ceiling_double_int(d0);
        int i1 = MathHelper.floor_double(axisalignedbb.minZ);
        int j1 = MathHelper.ceiling_double_int(axisalignedbb.maxZ);
        boolean flag = false;
        BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();

        try
        {
            for (int k1 = i; k1 < j; ++k1)
            {
                for (int l1 = k; l1 < l; ++l1)
                {
                    for (int i2 = i1; i2 < j1; ++i2)
                    {
                        blockpos$pooledmutableblockpos.set(k1, l1, i2);
                        IBlockState iblockstate = this.worldObj.getBlockState(blockpos$pooledmutableblockpos);

                        if (iblockstate.getMaterial() == Material.water && d0 < (double)func_184452_b(iblockstate, this.worldObj, blockpos$pooledmutableblockpos))
                        {
                            if (((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue() != 0)
                            {
                                EntityBoat.Status entityboat$status = EntityBoat.Status.UNDER_FLOWING_WATER;
                                return entityboat$status;
                            }

                            flag = true;
                        }
                    }
                }
            }
        }
        finally
        {
            blockpos$pooledmutableblockpos.release();
        }

        return flag ? EntityBoat.Status.UNDER_WATER : null;
    }

    public static float func_184456_a(IBlockState p_184456_0_, IBlockAccess p_184456_1_, BlockPos p_184456_2_)
    {
        int i = ((Integer)p_184456_0_.getValue(BlockLiquid.LEVEL)).intValue();
        return (i & 7) == 0 && p_184456_1_.getBlockState(p_184456_2_.up()).getMaterial() == Material.water ? 1.0F : 1.0F - BlockLiquid.getLiquidHeightPercent(i);
    }

    public static float func_184452_b(IBlockState p_184452_0_, IBlockAccess p_184452_1_, BlockPos p_184452_2_)
    {
        return (float)p_184452_2_.getY() + func_184456_a(p_184452_0_, p_184452_1_, p_184452_2_);
    }

    private void func_184450_w()
    {
        double d0 = -0.03999999910593033D;
        double d1 = d0;
        double d2 = 0.0D;
        this.field_184472_g = 0.05F;

        if (this.field_184471_aG == EntityBoat.Status.IN_AIR && this.field_184469_aF != EntityBoat.Status.IN_AIR && this.field_184469_aF != EntityBoat.Status.ON_LAND)
        {
            this.field_184465_aD = this.getEntityBoundingBox().minY + (double)this.height;
            this.setPosition(this.posX, (double)(this.func_184451_k() - this.height) + 0.101D, this.posZ);
            this.motionY = 0.0D;
            this.field_184473_aH = 0.0D;
            this.field_184469_aF = EntityBoat.Status.IN_WATER;
        }
        else
        {
            if (this.field_184469_aF == EntityBoat.Status.IN_WATER)
            {
                d2 = (this.field_184465_aD - this.getEntityBoundingBox().minY) / (double)this.height;
                this.field_184472_g = 0.9F;
            }
            else if (this.field_184469_aF == EntityBoat.Status.UNDER_FLOWING_WATER)
            {
                d1 = -7.0E-4D;
                this.field_184472_g = 0.9F;
            }
            else if (this.field_184469_aF == EntityBoat.Status.UNDER_WATER)
            {
                d2 = 0.009999999776482582D;
                this.field_184472_g = 0.45F;
            }
            else if (this.field_184469_aF == EntityBoat.Status.IN_AIR)
            {
                this.field_184472_g = 0.9F;
            }
            else if (this.field_184469_aF == EntityBoat.Status.ON_LAND)
            {
                this.field_184472_g = this.field_184467_aE;

                if (this.getControllingPassenger() instanceof EntityPlayer)
                {
                    this.field_184467_aE /= 2.0F;
                }
            }

            this.motionX *= (double)this.field_184472_g;
            this.motionZ *= (double)this.field_184472_g;
            this.field_184475_as *= this.field_184472_g;
            this.motionY += d1;

            if (d2 > 0.0D)
            {
                double d3 = 0.65D;
                this.motionY += d2 * (-d0 / 0.65D);
                double d4 = 0.75D;
                this.motionY *= 0.75D;
            }
        }
    }

    private void func_184443_x()
    {
        if (this.isBeingRidden())
        {
            float f = 0.0F;

            if (this.field_184480_az)
            {
                this.field_184475_as += -1.0F;
            }

            if (this.field_184459_aA)
            {
                ++this.field_184475_as;
            }

            if (this.field_184459_aA != this.field_184480_az && !this.field_184461_aB && !this.field_184463_aC)
            {
                f += 0.005F;
            }

            this.rotationYaw += this.field_184475_as;

            if (this.field_184461_aB)
            {
                f += 0.04F;
            }

            if (this.field_184463_aC)
            {
                f -= 0.005F;
            }

            this.motionX += (double)(MathHelper.sin(-this.rotationYaw * 0.017453292F) * f);
            this.motionZ += (double)(MathHelper.cos(this.rotationYaw * 0.017453292F) * f);
            this.func_184445_a(this.field_184459_aA || this.field_184461_aB, this.field_184480_az || this.field_184461_aB);
        }
    }

    public void updatePassenger(Entity passenger)
    {
        if (this.isPassenger(passenger))
        {
            float f = 0.0F;
            float f1 = (float)((this.isDead ? 0.009999999776482582D : this.getMountedYOffset()) + passenger.getYOffset());

            if (this.getPassengers().size() > 1)
            {
                int i = this.getPassengers().indexOf(passenger);

                if (i == 0)
                {
                    f = 0.2F;
                }
                else
                {
                    f = -0.6F;
                }

                if (passenger instanceof EntityAnimal)
                {
                    f = (float)((double)f + 0.2D);
                }
            }

            Vec3d vec3d = (new Vec3d((double)f, 0.0D, 0.0D)).rotateYaw(-this.rotationYaw * 0.017453292F - ((float)Math.PI / 2F));
            passenger.setPosition(this.posX + vec3d.xCoord, this.posY + (double)f1, this.posZ + vec3d.zCoord);
            passenger.rotationYaw += this.field_184475_as;
            passenger.setRotationYawHead(passenger.getRotationYawHead() + this.field_184475_as);
            this.func_184454_a(passenger);

            if (passenger instanceof EntityAnimal && this.getPassengers().size() > 1)
            {
                int j = passenger.getEntityId() % 2 == 0 ? 90 : 270;
                passenger.setRenderYawOffset(((EntityAnimal)passenger).renderYawOffset + (float)j);
                passenger.setRotationYawHead(passenger.getRotationYawHead() + (float)j);
            }
        }
    }

    protected void func_184454_a(Entity p_184454_1_)
    {
        p_184454_1_.setRenderYawOffset(this.rotationYaw);
        float f = MathHelper.wrapAngleTo180_float(p_184454_1_.rotationYaw - this.rotationYaw);
        float f1 = MathHelper.clamp_float(f, -105.0F, 105.0F);
        p_184454_1_.prevRotationYaw += f1 - f;
        p_184454_1_.rotationYaw += f1 - f;
        p_184454_1_.setRotationYawHead(p_184454_1_.rotationYaw);
    }

    public void func_184190_l(Entity p_184190_1_)
    {
        this.func_184454_a(p_184190_1_);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        tagCompound.setString("Type", this.func_184453_r().func_184980_a());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        if (tagCompund.hasKey("Type", 8))
        {
            this.func_184458_a(EntityBoat.Type.func_184981_a(tagCompund.getString("Type")));
        }
    }

    public boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        if (!this.worldObj.isRemote && !p_184230_1_.isSneaking() && this.field_184474_h < 60.0F)
        {
            p_184230_1_.startRiding(this);
        }

        return true;
    }

    protected void updateFallState(double y, boolean onGroundIn, IBlockState state, BlockPos pos)
    {
        this.field_184473_aH = this.motionY;

        if (!this.isRiding())
        {
            if (onGroundIn)
            {
                if (this.fallDistance > 3.0F)
                {
                    if (this.field_184469_aF != EntityBoat.Status.ON_LAND)
                    {
                        this.fallDistance = 0.0F;
                        return;
                    }

                    this.fall(this.fallDistance, 1.0F);

                    if (!this.worldObj.isRemote && !this.isDead)
                    {
                        this.setDead();

                        if (this.worldObj.getGameRules().getBoolean("doEntityDrops"))
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                this.entityDropItem(new ItemStack(Item.getItemFromBlock(Blocks.planks), 1, this.func_184453_r().func_184982_b()), 0.0F);
                            }

                            for (int j = 0; j < 2; ++j)
                            {
                                this.dropItemWithOffset(Items.stick, 1, 0.0F);
                            }
                        }
                    }
                }

                this.fallDistance = 0.0F;
            }
            else if (this.worldObj.getBlockState((new BlockPos(this)).down()).getMaterial() != Material.water && y < 0.0D)
            {
                this.fallDistance = (float)((double)this.fallDistance - y);
            }
        }
    }

    public boolean func_184457_a(int p_184457_1_)
    {
        return ((Boolean)this.dataWatcher.get(field_184468_e[p_184457_1_])).booleanValue() && this.getControllingPassenger() != null;
    }

    /**
     * Sets the damage taken from the last hit.
     */
    public void setDamageTaken(float p_70266_1_)
    {
        this.dataWatcher.set(DAMAGE_TAKEN, Float.valueOf(p_70266_1_));
    }

    /**
     * Gets the damage taken from the last hit.
     */
    public float getDamageTaken()
    {
        return ((Float)this.dataWatcher.get(DAMAGE_TAKEN)).floatValue();
    }

    /**
     * Sets the time to count down from since the last time entity was hit.
     */
    public void setTimeSinceHit(int p_70265_1_)
    {
        this.dataWatcher.set(TIME_SINCE_HIT, Integer.valueOf(p_70265_1_));
    }

    /**
     * Gets the time since the last hit.
     */
    public int getTimeSinceHit()
    {
        return ((Integer)this.dataWatcher.get(TIME_SINCE_HIT)).intValue();
    }

    /**
     * Sets the forward direction of the entity.
     */
    public void setForwardDirection(int p_70269_1_)
    {
        this.dataWatcher.set(FORWARD_DIRECTION, Integer.valueOf(p_70269_1_));
    }

    /**
     * Gets the forward direction of the entity.
     */
    public int getForwardDirection()
    {
        return ((Integer)this.dataWatcher.get(FORWARD_DIRECTION)).intValue();
    }

    public void func_184458_a(EntityBoat.Type p_184458_1_)
    {
        this.dataWatcher.set(BOAT_TYPE, Integer.valueOf(p_184458_1_.ordinal()));
    }

    public EntityBoat.Type func_184453_r()
    {
        return EntityBoat.Type.func_184979_a(((Integer)this.dataWatcher.get(BOAT_TYPE)).intValue());
    }

    protected boolean canFitPassenger(Entity passenger)
    {
        return this.getPassengers().size() < 2;
    }

    /**
     * For vehicles, the first passenger is generally considered the controller and "drives" the vehicle. For example,
     * Pigs, Horses, and Boats are generally "steered" by the controlling passenger.
     */
    public Entity getControllingPassenger()
    {
        List<Entity> list = this.getPassengers();
        return list.isEmpty() ? null : (Entity)list.get(0);
    }

    public void func_184442_a(boolean p_184442_1_, boolean p_184442_2_, boolean p_184442_3_, boolean p_184442_4_)
    {
        this.field_184480_az = p_184442_1_;
        this.field_184459_aA = p_184442_2_;
        this.field_184461_aB = p_184442_3_;
        this.field_184463_aC = p_184442_4_;
    }

    public static enum Status
    {
        IN_WATER,
        UNDER_WATER,
        UNDER_FLOWING_WATER,
        ON_LAND,
        IN_AIR;
    }

    public static enum Type
    {
        OAK(BlockPlanks.EnumType.OAK.getMetadata(), "oak"),
        SPRUCE(BlockPlanks.EnumType.SPRUCE.getMetadata(), "spruce"),
        BIRCH(BlockPlanks.EnumType.BIRCH.getMetadata(), "birch"),
        JUNGLE(BlockPlanks.EnumType.JUNGLE.getMetadata(), "jungle"),
        ACACIA(BlockPlanks.EnumType.ACACIA.getMetadata(), "acacia"),
        DARK_OAK(BlockPlanks.EnumType.DARK_OAK.getMetadata(), "dark_oak");

        private final String field_184990_g;
        private final int field_184991_h;

        private Type(int p_i47028_3_, String p_i47028_4_)
        {
            this.field_184990_g = p_i47028_4_;
            this.field_184991_h = p_i47028_3_;
        }

        public String func_184980_a()
        {
            return this.field_184990_g;
        }

        public int func_184982_b()
        {
            return this.field_184991_h;
        }

        public String toString()
        {
            return this.field_184990_g;
        }

        public static EntityBoat.Type func_184979_a(int p_184979_0_)
        {
            if (p_184979_0_ < 0 || p_184979_0_ >= values().length)
            {
                p_184979_0_ = 0;
            }

            return values()[p_184979_0_];
        }

        public static EntityBoat.Type func_184981_a(String p_184981_0_)
        {
            for (int i = 0; i < values().length; ++i)
            {
                if (values()[i].func_184980_a().equals(p_184981_0_))
                {
                    return values()[i];
                }
            }

            return values()[0];
        }
    }
}
