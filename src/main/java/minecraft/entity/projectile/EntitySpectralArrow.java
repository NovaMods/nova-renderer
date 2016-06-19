package net.minecraft.entity.projectile;

import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.world.World;

public class EntitySpectralArrow extends EntityArrow
{
    private int duration = 200;

    public EntitySpectralArrow(World worldIn)
    {
        super(worldIn);
    }

    public EntitySpectralArrow(World worldIn, EntityLivingBase shooter)
    {
        super(worldIn, shooter);
    }

    public EntitySpectralArrow(World worldIn, double x, double y, double z)
    {
        super(worldIn, x, y, z);
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (this.worldObj.isRemote && !this.inGround)
        {
            this.worldObj.spawnParticle(EnumParticleTypes.SPELL_INSTANT, this.posX, this.posY, this.posZ, 0.0D, 0.0D, 0.0D, new int[0]);
        }
    }

    protected ItemStack func_184550_j()
    {
        return new ItemStack(Items.spectral_arrow);
    }

    protected void func_184548_a(EntityLivingBase p_184548_1_)
    {
        super.func_184548_a(p_184548_1_);
        PotionEffect potioneffect = new PotionEffect(MobEffects.glowing, this.duration, 0);
        p_184548_1_.addPotionEffect(potioneffect);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.hasKey("Duration"))
        {
            this.duration = tagCompund.getInteger("Duration");
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setInteger("Duration", this.duration);
    }
}
