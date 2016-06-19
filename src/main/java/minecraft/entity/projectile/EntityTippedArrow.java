package net.minecraft.entity.projectile;

import com.google.common.collect.Sets;
import java.util.Collection;
import java.util.Set;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Items;
import net.minecraft.init.PotionTypes;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.potion.PotionEffect;
import net.minecraft.potion.PotionType;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.World;

public class EntityTippedArrow extends EntityArrow
{
    private static final DataParameter<Integer> COLOR = EntityDataManager.<Integer>createKey(EntityTippedArrow.class, DataSerializers.VARINT);
    private PotionType field_184560_g = PotionTypes.empty;
    private final Set<PotionEffect> field_184561_h = Sets.<PotionEffect>newHashSet();

    public EntityTippedArrow(World worldIn)
    {
        super(worldIn);
    }

    public EntityTippedArrow(World worldIn, double x, double y, double z)
    {
        super(worldIn, x, y, z);
    }

    public EntityTippedArrow(World worldIn, EntityLivingBase shooter)
    {
        super(worldIn, shooter);
    }

    public void func_184555_a(ItemStack p_184555_1_)
    {
        if (p_184555_1_.getItem() == Items.tipped_arrow)
        {
            this.field_184560_g = PotionUtils.func_185187_c(p_184555_1_.getTagCompound());
            Collection<PotionEffect> collection = PotionUtils.func_185190_b(p_184555_1_);

            if (!collection.isEmpty())
            {
                for (PotionEffect potioneffect : collection)
                {
                    this.field_184561_h.add(new PotionEffect(potioneffect));
                }
            }

            this.dataWatcher.set(COLOR, Integer.valueOf(PotionUtils.func_185181_a(PotionUtils.func_185186_a(this.field_184560_g, collection))));
        }
        else if (p_184555_1_.getItem() == Items.arrow)
        {
            this.field_184560_g = PotionTypes.empty;
            this.field_184561_h.clear();
            this.dataWatcher.set(COLOR, Integer.valueOf(0));
        }
    }

    public void func_184558_a(PotionEffect p_184558_1_)
    {
        this.field_184561_h.add(p_184558_1_);
        this.getDataManager().set(COLOR, Integer.valueOf(PotionUtils.func_185181_a(PotionUtils.func_185186_a(this.field_184560_g, this.field_184561_h))));
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(COLOR, Integer.valueOf(0));
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (this.worldObj.isRemote)
        {
            if (this.inGround)
            {
                if (this.field_184552_b % 5 == 0)
                {
                    this.func_184556_b(1);
                }
            }
            else
            {
                this.func_184556_b(2);
            }
        }
        else if (this.inGround && this.field_184552_b != 0 && !this.field_184561_h.isEmpty() && this.field_184552_b >= 600)
        {
            this.worldObj.setEntityState(this, (byte)0);
            this.field_184560_g = PotionTypes.empty;
            this.field_184561_h.clear();
            this.dataWatcher.set(COLOR, Integer.valueOf(0));
        }
    }

    private void func_184556_b(int p_184556_1_)
    {
        int i = this.func_184557_n();

        if (i != 0 && p_184556_1_ > 0)
        {
            double d0 = (double)(i >> 16 & 255) / 255.0D;
            double d1 = (double)(i >> 8 & 255) / 255.0D;
            double d2 = (double)(i >> 0 & 255) / 255.0D;

            for (int j = 0; j < p_184556_1_; ++j)
            {
                this.worldObj.spawnParticle(EnumParticleTypes.SPELL_MOB, this.posX + (this.rand.nextDouble() - 0.5D) * (double)this.width, this.posY + this.rand.nextDouble() * (double)this.height, this.posZ + (this.rand.nextDouble() - 0.5D) * (double)this.width, d0, d1, d2, new int[0]);
            }
        }
    }

    public int func_184557_n()
    {
        return ((Integer)this.dataWatcher.get(COLOR)).intValue();
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);

        if (this.field_184560_g != PotionTypes.empty && this.field_184560_g != null)
        {
            tagCompound.setString("Potion", ((ResourceLocation)PotionType.REGISTRY.getNameForObject(this.field_184560_g)).toString());
        }

        if (!this.field_184561_h.isEmpty())
        {
            NBTTagList nbttaglist = new NBTTagList();

            for (PotionEffect potioneffect : this.field_184561_h)
            {
                nbttaglist.appendTag(potioneffect.writeCustomPotionEffectToNBT(new NBTTagCompound()));
            }

            tagCompound.setTag("CustomPotionEffects", nbttaglist);
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.hasKey("Potion", 8))
        {
            this.field_184560_g = PotionUtils.func_185187_c(tagCompund);
        }

        for (PotionEffect potioneffect : PotionUtils.func_185192_b(tagCompund))
        {
            this.func_184558_a(potioneffect);
        }

        if (this.field_184560_g != PotionTypes.empty || !this.field_184561_h.isEmpty())
        {
            this.dataWatcher.set(COLOR, Integer.valueOf(PotionUtils.func_185181_a(PotionUtils.func_185186_a(this.field_184560_g, this.field_184561_h))));
        }
    }

    protected void func_184548_a(EntityLivingBase p_184548_1_)
    {
        super.func_184548_a(p_184548_1_);

        for (PotionEffect potioneffect : this.field_184560_g.getEffects())
        {
            p_184548_1_.addPotionEffect(new PotionEffect(potioneffect.func_188419_a(), potioneffect.getDuration() / 8, potioneffect.getAmplifier(), potioneffect.getIsAmbient(), potioneffect.func_188418_e()));
        }

        if (!this.field_184561_h.isEmpty())
        {
            for (PotionEffect potioneffect1 : this.field_184561_h)
            {
                p_184548_1_.addPotionEffect(potioneffect1);
            }
        }
    }

    protected ItemStack func_184550_j()
    {
        if (this.field_184561_h.isEmpty() && this.field_184560_g == PotionTypes.empty)
        {
            return new ItemStack(Items.arrow);
        }
        else
        {
            ItemStack itemstack = new ItemStack(Items.tipped_arrow);
            PotionUtils.func_185188_a(itemstack, this.field_184560_g);
            PotionUtils.func_185184_a(itemstack, this.field_184561_h);
            return itemstack;
        }
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 0)
        {
            int i = this.func_184557_n();

            if (i > 0)
            {
                double d0 = (double)(i >> 16 & 255) / 255.0D;
                double d1 = (double)(i >> 8 & 255) / 255.0D;
                double d2 = (double)(i >> 0 & 255) / 255.0D;

                for (int j = 0; j < 20; ++j)
                {
                    this.worldObj.spawnParticle(EnumParticleTypes.SPELL_MOB, this.posX + (this.rand.nextDouble() - 0.5D) * (double)this.width, this.posY + this.rand.nextDouble() * (double)this.height, this.posZ + (this.rand.nextDouble() - 0.5D) * (double)this.width, d0, d1, d2, new int[0]);
                }
            }
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }
}
