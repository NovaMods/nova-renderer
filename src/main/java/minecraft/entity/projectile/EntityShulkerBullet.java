package net.minecraft.entity.projectile;

import com.google.common.collect.Lists;
import java.util.List;
import java.util.UUID;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.MobEffects;
import net.minecraft.init.SoundEvents;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;

public class EntityShulkerBullet extends Entity
{
    private EntityLivingBase field_184570_a;
    private Entity field_184571_b;
    private EnumFacing field_184573_c;
    private int field_184575_d;
    private double field_184577_e;
    private double field_184578_f;
    private double field_184579_g;
    private UUID field_184580_h;
    private BlockPos field_184572_as;
    private UUID field_184574_at;
    private BlockPos field_184576_au;

    public EntityShulkerBullet(World worldIn)
    {
        super(worldIn);
        this.setSize(0.3125F, 0.3125F);
        this.noClip = true;
    }

    public SoundCategory getSoundCategory()
    {
        return SoundCategory.HOSTILE;
    }

    public EntityShulkerBullet(World worldIn, double x, double y, double z, double p_i46771_8_, double p_i46771_10_, double p_i46771_12_)
    {
        this(worldIn);
        this.setLocationAndAngles(x, y, z, this.rotationYaw, this.rotationPitch);
        this.motionX = p_i46771_8_;
        this.motionY = p_i46771_10_;
        this.motionZ = p_i46771_12_;
    }

    public EntityShulkerBullet(World worldIn, EntityLivingBase p_i46772_2_, Entity p_i46772_3_, EnumFacing.Axis p_i46772_4_)
    {
        this(worldIn);
        this.field_184570_a = p_i46772_2_;
        BlockPos blockpos = new BlockPos(p_i46772_2_);
        double d0 = (double)blockpos.getX() + 0.5D;
        double d1 = (double)blockpos.getY() + 0.5D;
        double d2 = (double)blockpos.getZ() + 0.5D;
        this.setLocationAndAngles(d0, d1, d2, this.rotationYaw, this.rotationPitch);
        this.field_184571_b = p_i46772_3_;
        this.field_184573_c = EnumFacing.UP;
        this.func_184569_a(p_i46772_4_);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        if (this.field_184570_a != null)
        {
            BlockPos blockpos = new BlockPos(this.field_184570_a);
            NBTTagCompound nbttagcompound = NBTUtil.createUUIDTag(this.field_184570_a.getUniqueID());
            nbttagcompound.setInteger("X", blockpos.getX());
            nbttagcompound.setInteger("Y", blockpos.getY());
            nbttagcompound.setInteger("Z", blockpos.getZ());
            tagCompound.setTag("Owner", nbttagcompound);
        }

        if (this.field_184571_b != null)
        {
            BlockPos blockpos1 = new BlockPos(this.field_184571_b);
            NBTTagCompound nbttagcompound1 = NBTUtil.createUUIDTag(this.field_184571_b.getUniqueID());
            nbttagcompound1.setInteger("X", blockpos1.getX());
            nbttagcompound1.setInteger("Y", blockpos1.getY());
            nbttagcompound1.setInteger("Z", blockpos1.getZ());
            tagCompound.setTag("Target", nbttagcompound1);
        }

        if (this.field_184573_c != null)
        {
            tagCompound.setInteger("Dir", this.field_184573_c.getIndex());
        }

        tagCompound.setInteger("Steps", this.field_184575_d);
        tagCompound.setDouble("TXD", this.field_184577_e);
        tagCompound.setDouble("TYD", this.field_184578_f);
        tagCompound.setDouble("TZD", this.field_184579_g);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        this.field_184575_d = tagCompund.getInteger("Steps");
        this.field_184577_e = tagCompund.getDouble("TXD");
        this.field_184578_f = tagCompund.getDouble("TYD");
        this.field_184579_g = tagCompund.getDouble("TZD");

        if (tagCompund.hasKey("Dir", 99))
        {
            this.field_184573_c = EnumFacing.getFront(tagCompund.getInteger("Dir"));
        }

        if (tagCompund.hasKey("Owner", 10))
        {
            NBTTagCompound nbttagcompound = tagCompund.getCompoundTag("Owner");
            this.field_184580_h = NBTUtil.getUUIDFromTag(nbttagcompound);
            this.field_184572_as = new BlockPos(nbttagcompound.getInteger("X"), nbttagcompound.getInteger("Y"), nbttagcompound.getInteger("Z"));
        }

        if (tagCompund.hasKey("Target", 10))
        {
            NBTTagCompound nbttagcompound1 = tagCompund.getCompoundTag("Target");
            this.field_184574_at = NBTUtil.getUUIDFromTag(nbttagcompound1);
            this.field_184576_au = new BlockPos(nbttagcompound1.getInteger("X"), nbttagcompound1.getInteger("Y"), nbttagcompound1.getInteger("Z"));
        }
    }

    protected void entityInit()
    {
    }

    private void func_184568_a(EnumFacing p_184568_1_)
    {
        this.field_184573_c = p_184568_1_;
    }

    private void func_184569_a(EnumFacing.Axis p_184569_1_)
    {
        double d0 = 0.5D;
        BlockPos blockpos;

        if (this.field_184571_b == null)
        {
            blockpos = (new BlockPos(this)).down();
        }
        else
        {
            d0 = (double)this.field_184571_b.height * 0.5D;
            blockpos = new BlockPos(this.field_184571_b.posX, this.field_184571_b.posY + d0, this.field_184571_b.posZ);
        }

        double d1 = (double)blockpos.getX() + 0.5D;
        double d2 = (double)blockpos.getY() + d0;
        double d3 = (double)blockpos.getZ() + 0.5D;
        EnumFacing enumfacing = null;

        if (blockpos.distanceSqToCenter(this.posX, this.posY, this.posZ) >= 4.0D)
        {
            BlockPos blockpos1 = new BlockPos(this);
            List<EnumFacing> list = Lists.<EnumFacing>newArrayList();

            if (p_184569_1_ != EnumFacing.Axis.X)
            {
                if (blockpos1.getX() < blockpos.getX() && this.worldObj.isAirBlock(blockpos1.east()))
                {
                    list.add(EnumFacing.EAST);
                }
                else if (blockpos1.getX() > blockpos.getX() && this.worldObj.isAirBlock(blockpos1.west()))
                {
                    list.add(EnumFacing.WEST);
                }
            }

            if (p_184569_1_ != EnumFacing.Axis.Y)
            {
                if (blockpos1.getY() < blockpos.getY() && this.worldObj.isAirBlock(blockpos1.up()))
                {
                    list.add(EnumFacing.UP);
                }
                else if (blockpos1.getY() > blockpos.getY() && this.worldObj.isAirBlock(blockpos1.down()))
                {
                    list.add(EnumFacing.DOWN);
                }
            }

            if (p_184569_1_ != EnumFacing.Axis.Z)
            {
                if (blockpos1.getZ() < blockpos.getZ() && this.worldObj.isAirBlock(blockpos1.south()))
                {
                    list.add(EnumFacing.SOUTH);
                }
                else if (blockpos1.getZ() > blockpos.getZ() && this.worldObj.isAirBlock(blockpos1.north()))
                {
                    list.add(EnumFacing.NORTH);
                }
            }

            enumfacing = EnumFacing.random(this.rand);

            if (list.isEmpty())
            {
                for (int i = 5; !this.worldObj.isAirBlock(blockpos1.offset(enumfacing)) && i > 0; --i)
                {
                    enumfacing = EnumFacing.random(this.rand);
                }
            }
            else
            {
                enumfacing = (EnumFacing)list.get(this.rand.nextInt(list.size()));
            }

            d1 = this.posX + (double)enumfacing.getFrontOffsetX();
            d2 = this.posY + (double)enumfacing.getFrontOffsetY();
            d3 = this.posZ + (double)enumfacing.getFrontOffsetZ();
        }

        this.func_184568_a(enumfacing);
        double d6 = d1 - this.posX;
        double d7 = d2 - this.posY;
        double d4 = d3 - this.posZ;
        double d5 = (double)MathHelper.sqrt_double(d6 * d6 + d7 * d7 + d4 * d4);

        if (d5 == 0.0D)
        {
            this.field_184577_e = 0.0D;
            this.field_184578_f = 0.0D;
            this.field_184579_g = 0.0D;
        }
        else
        {
            this.field_184577_e = d6 / d5 * 0.15D;
            this.field_184578_f = d7 / d5 * 0.15D;
            this.field_184579_g = d4 / d5 * 0.15D;
        }

        this.isAirBorne = true;
        this.field_184575_d = 10 + this.rand.nextInt(5) * 10;
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        if (!this.worldObj.isRemote && this.worldObj.getDifficulty() == EnumDifficulty.PEACEFUL)
        {
            this.setDead();
        }
        else
        {
            super.onUpdate();

            if (!this.worldObj.isRemote)
            {
                if (this.field_184571_b == null && this.field_184574_at != null)
                {
                    for (EntityLivingBase entitylivingbase : this.worldObj.getEntitiesWithinAABB(EntityLivingBase.class, new AxisAlignedBB(this.field_184576_au.add(-2, -2, -2), this.field_184576_au.add(2, 2, 2))))
                    {
                        if (entitylivingbase.getUniqueID().equals(this.field_184574_at))
                        {
                            this.field_184571_b = entitylivingbase;
                            break;
                        }
                    }

                    this.field_184574_at = null;
                }

                if (this.field_184570_a == null && this.field_184580_h != null)
                {
                    for (EntityLivingBase entitylivingbase1 : this.worldObj.getEntitiesWithinAABB(EntityLivingBase.class, new AxisAlignedBB(this.field_184572_as.add(-2, -2, -2), this.field_184572_as.add(2, 2, 2))))
                    {
                        if (entitylivingbase1.getUniqueID().equals(this.field_184580_h))
                        {
                            this.field_184570_a = entitylivingbase1;
                            break;
                        }
                    }

                    this.field_184580_h = null;
                }

                if (this.field_184571_b == null || !this.field_184571_b.isEntityAlive() || this.field_184571_b instanceof EntityPlayer && ((EntityPlayer)this.field_184571_b).isSpectator())
                {
                    this.motionY -= 0.04D;
                }
                else
                {
                    this.field_184577_e = MathHelper.clamp_double(this.field_184577_e * 1.025D, -1.0D, 1.0D);
                    this.field_184578_f = MathHelper.clamp_double(this.field_184578_f * 1.025D, -1.0D, 1.0D);
                    this.field_184579_g = MathHelper.clamp_double(this.field_184579_g * 1.025D, -1.0D, 1.0D);
                    this.motionX += (this.field_184577_e - this.motionX) * 0.2D;
                    this.motionY += (this.field_184578_f - this.motionY) * 0.2D;
                    this.motionZ += (this.field_184579_g - this.motionZ) * 0.2D;
                }

                RayTraceResult raytraceresult = ProjectileHelper.func_188802_a(this, true, false, this.field_184570_a);

                if (raytraceresult != null)
                {
                    this.func_184567_a(raytraceresult);
                }
            }

            this.setPosition(this.posX + this.motionX, this.posY + this.motionY, this.posZ + this.motionZ);
            ProjectileHelper.func_188803_a(this, 0.5F);

            if (this.worldObj.isRemote)
            {
                this.worldObj.spawnParticle(EnumParticleTypes.END_ROD, this.posX - this.motionX, this.posY - this.motionY + 0.15D, this.posZ - this.motionZ, 0.0D, 0.0D, 0.0D, new int[0]);
            }
            else if (this.field_184571_b != null && !this.field_184571_b.isDead)
            {
                if (this.field_184575_d > 0)
                {
                    --this.field_184575_d;

                    if (this.field_184575_d == 0)
                    {
                        this.func_184569_a(this.field_184573_c == null ? null : this.field_184573_c.getAxis());
                    }
                }

                if (this.field_184573_c != null)
                {
                    BlockPos blockpos = new BlockPos(this);
                    EnumFacing.Axis enumfacing$axis = this.field_184573_c.getAxis();

                    if (this.worldObj.isBlockNormalCube(blockpos.offset(this.field_184573_c), false))
                    {
                        this.func_184569_a(enumfacing$axis);
                    }
                    else
                    {
                        BlockPos blockpos1 = new BlockPos(this.field_184571_b);

                        if (enumfacing$axis == EnumFacing.Axis.X && blockpos.getX() == blockpos1.getX() || enumfacing$axis == EnumFacing.Axis.Z && blockpos.getZ() == blockpos1.getZ() || enumfacing$axis == EnumFacing.Axis.Y && blockpos.getY() == blockpos1.getY())
                        {
                            this.func_184569_a(enumfacing$axis);
                        }
                    }
                }
            }
        }
    }

    /**
     * Returns true if the entity is on fire. Used by render to add the fire effect on rendering.
     */
    public boolean isBurning()
    {
        return false;
    }

    /**
     * Checks if the entity is in range to render by using the past in distance and comparing it to its average edge
     * length * 64 * renderDistanceWeight Args: distance
     */
    public boolean isInRangeToRenderDist(double distance)
    {
        return distance < 16384.0D;
    }

    /**
     * Gets how bright this entity is.
     */
    public float getBrightness(float partialTicks)
    {
        return 1.0F;
    }

    public int getBrightnessForRender(float partialTicks)
    {
        return 15728880;
    }

    protected void func_184567_a(RayTraceResult p_184567_1_)
    {
        if (p_184567_1_.entityHit == null)
        {
            ((WorldServer)this.worldObj).spawnParticle(EnumParticleTypes.EXPLOSION_LARGE, this.posX, this.posY, this.posZ, 2, 0.2D, 0.2D, 0.2D, 0.0D, new int[0]);
            this.playSound(SoundEvents.entity_shulker_bullet_hit, 1.0F, 1.0F);
        }
        else
        {
            boolean flag = p_184567_1_.entityHit.attackEntityFrom(DamageSource.func_188403_a(this, this.field_184570_a).setProjectile(), 4.0F);

            if (flag)
            {
                this.applyEnchantments(this.field_184570_a, p_184567_1_.entityHit);

                if (p_184567_1_.entityHit instanceof EntityLivingBase)
                {
                    ((EntityLivingBase)p_184567_1_.entityHit).addPotionEffect(new PotionEffect(MobEffects.levitation, 200));
                }
            }
        }

        this.setDead();
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return true;
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        if (!this.worldObj.isRemote)
        {
            this.playSound(SoundEvents.entity_shulker_bullet_hurt, 1.0F, 1.0F);
            ((WorldServer)this.worldObj).spawnParticle(EnumParticleTypes.CRIT, this.posX, this.posY, this.posZ, 15, 0.2D, 0.2D, 0.2D, 0.0D, new int[0]);
            this.setDead();
        }

        return true;
    }
}
