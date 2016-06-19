package net.minecraft.entity.item;

import net.minecraft.block.BlockFurnace;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class EntityMinecartFurnace extends EntityMinecart
{
    private static final DataParameter<Boolean> field_184275_c = EntityDataManager.<Boolean>createKey(EntityMinecartFurnace.class, DataSerializers.BOOLEAN);
    private int fuel;
    public double pushX;
    public double pushZ;

    public EntityMinecartFurnace(World worldIn)
    {
        super(worldIn);
    }

    public EntityMinecartFurnace(World worldIn, double x, double y, double z)
    {
        super(worldIn, x, y, z);
    }

    public EntityMinecart.Type func_184264_v()
    {
        return EntityMinecart.Type.FURNACE;
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(field_184275_c, Boolean.valueOf(false));
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (this.fuel > 0)
        {
            --this.fuel;
        }

        if (this.fuel <= 0)
        {
            this.pushX = this.pushZ = 0.0D;
        }

        this.setMinecartPowered(this.fuel > 0);

        if (this.isMinecartPowered() && this.rand.nextInt(4) == 0)
        {
            this.worldObj.spawnParticle(EnumParticleTypes.SMOKE_LARGE, this.posX, this.posY + 0.8D, this.posZ, 0.0D, 0.0D, 0.0D, new int[0]);
        }
    }

    /**
     * Get's the maximum speed for a minecart
     */
    protected double getMaximumSpeed()
    {
        return 0.2D;
    }

    public void killMinecart(DamageSource source)
    {
        super.killMinecart(source);

        if (!source.isExplosion() && this.worldObj.getGameRules().getBoolean("doEntityDrops"))
        {
            this.entityDropItem(new ItemStack(Blocks.furnace, 1), 0.0F);
        }
    }

    protected void func_180460_a(BlockPos p_180460_1_, IBlockState p_180460_2_)
    {
        super.func_180460_a(p_180460_1_, p_180460_2_);
        double d0 = this.pushX * this.pushX + this.pushZ * this.pushZ;

        if (d0 > 1.0E-4D && this.motionX * this.motionX + this.motionZ * this.motionZ > 0.001D)
        {
            d0 = (double)MathHelper.sqrt_double(d0);
            this.pushX /= d0;
            this.pushZ /= d0;

            if (this.pushX * this.motionX + this.pushZ * this.motionZ < 0.0D)
            {
                this.pushX = 0.0D;
                this.pushZ = 0.0D;
            }
            else
            {
                double d1 = d0 / this.getMaximumSpeed();
                this.pushX *= d1;
                this.pushZ *= d1;
            }
        }
    }

    protected void applyDrag()
    {
        double d0 = this.pushX * this.pushX + this.pushZ * this.pushZ;

        if (d0 > 1.0E-4D)
        {
            d0 = (double)MathHelper.sqrt_double(d0);
            this.pushX /= d0;
            this.pushZ /= d0;
            double d1 = 1.0D;
            this.motionX *= 0.800000011920929D;
            this.motionY *= 0.0D;
            this.motionZ *= 0.800000011920929D;
            this.motionX += this.pushX * d1;
            this.motionZ += this.pushZ * d1;
        }
        else
        {
            this.motionX *= 0.9800000190734863D;
            this.motionY *= 0.0D;
            this.motionZ *= 0.9800000190734863D;
        }

        super.applyDrag();
    }

    public boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        if (p_184230_2_ != null && p_184230_2_.getItem() == Items.coal && this.fuel + 3600 <= 32000)
        {
            if (!p_184230_1_.capabilities.isCreativeMode)
            {
                --p_184230_2_.stackSize;
            }

            this.fuel += 3600;
        }

        this.pushX = this.posX - p_184230_1_.posX;
        this.pushZ = this.posZ - p_184230_1_.posZ;
        return true;
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setDouble("PushX", this.pushX);
        tagCompound.setDouble("PushZ", this.pushZ);
        tagCompound.setShort("Fuel", (short)this.fuel);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.pushX = tagCompund.getDouble("PushX");
        this.pushZ = tagCompund.getDouble("PushZ");
        this.fuel = tagCompund.getShort("Fuel");
    }

    protected boolean isMinecartPowered()
    {
        return ((Boolean)this.dataWatcher.get(field_184275_c)).booleanValue();
    }

    protected void setMinecartPowered(boolean p_94107_1_)
    {
        this.dataWatcher.set(field_184275_c, Boolean.valueOf(p_94107_1_));
    }

    public IBlockState getDefaultDisplayTile()
    {
        return (this.isMinecartPowered() ? Blocks.lit_furnace : Blocks.furnace).getDefaultState().withProperty(BlockFurnace.FACING, EnumFacing.NORTH);
    }
}
