package net.minecraft.entity.passive;

import com.google.common.base.Optional;
import java.util.UUID;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IEntityOwnable;
import net.minecraft.entity.ai.EntityAISit;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.scoreboard.Team;
import net.minecraft.server.management.PreYggdrasilConverter;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.world.World;

public abstract class EntityTameable extends EntityAnimal implements IEntityOwnable
{
    protected static final DataParameter<Byte> TAMED = EntityDataManager.<Byte>createKey(EntityTameable.class, DataSerializers.BYTE);
    protected static final DataParameter<Optional<UUID>> field_184756_bw = EntityDataManager.<Optional<UUID>>createKey(EntityTameable.class, DataSerializers.OPTIONAL_UNIQUE_ID);
    protected EntityAISit aiSit;

    public EntityTameable(World worldIn)
    {
        super(worldIn);
        this.setupTamedAI();
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(TAMED, Byte.valueOf((byte)0));
        this.dataWatcher.register(field_184756_bw, Optional.<UUID>absent());
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);

        if (this.getOwnerId() == null)
        {
            tagCompound.setString("OwnerUUID", "");
        }
        else
        {
            tagCompound.setString("OwnerUUID", this.getOwnerId().toString());
        }

        tagCompound.setBoolean("Sitting", this.isSitting());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        String s = "";

        if (tagCompund.hasKey("OwnerUUID", 8))
        {
            s = tagCompund.getString("OwnerUUID");
        }
        else
        {
            String s1 = tagCompund.getString("Owner");
            s = PreYggdrasilConverter.func_187473_a(this.getServer(), s1);
        }

        if (!s.isEmpty())
        {
            try
            {
                this.setOwnerId(UUID.fromString(s));
                this.setTamed(true);
            }
            catch (Throwable var4)
            {
                this.setTamed(false);
            }
        }

        if (this.aiSit != null)
        {
            this.aiSit.setSitting(tagCompund.getBoolean("Sitting"));
        }

        this.setSitting(tagCompund.getBoolean("Sitting"));
    }

    public boolean func_184652_a(EntityPlayer p_184652_1_)
    {
        return this.isTamed() && this.isOwner(p_184652_1_);
    }

    /**
     * Play the taming effect, will either be hearts or smoke depending on status
     */
    protected void playTameEffect(boolean play)
    {
        EnumParticleTypes enumparticletypes = EnumParticleTypes.HEART;

        if (!play)
        {
            enumparticletypes = EnumParticleTypes.SMOKE_NORMAL;
        }

        for (int i = 0; i < 7; ++i)
        {
            double d0 = this.rand.nextGaussian() * 0.02D;
            double d1 = this.rand.nextGaussian() * 0.02D;
            double d2 = this.rand.nextGaussian() * 0.02D;
            this.worldObj.spawnParticle(enumparticletypes, this.posX + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, this.posY + 0.5D + (double)(this.rand.nextFloat() * this.height), this.posZ + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, d0, d1, d2, new int[0]);
        }
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 7)
        {
            this.playTameEffect(true);
        }
        else if (id == 6)
        {
            this.playTameEffect(false);
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    public boolean isTamed()
    {
        return (((Byte)this.dataWatcher.get(TAMED)).byteValue() & 4) != 0;
    }

    public void setTamed(boolean tamed)
    {
        byte b0 = ((Byte)this.dataWatcher.get(TAMED)).byteValue();

        if (tamed)
        {
            this.dataWatcher.set(TAMED, Byte.valueOf((byte)(b0 | 4)));
        }
        else
        {
            this.dataWatcher.set(TAMED, Byte.valueOf((byte)(b0 & -5)));
        }

        this.setupTamedAI();
    }

    protected void setupTamedAI()
    {
    }

    public boolean isSitting()
    {
        return (((Byte)this.dataWatcher.get(TAMED)).byteValue() & 1) != 0;
    }

    public void setSitting(boolean sitting)
    {
        byte b0 = ((Byte)this.dataWatcher.get(TAMED)).byteValue();

        if (sitting)
        {
            this.dataWatcher.set(TAMED, Byte.valueOf((byte)(b0 | 1)));
        }
        else
        {
            this.dataWatcher.set(TAMED, Byte.valueOf((byte)(b0 & -2)));
        }
    }

    public UUID getOwnerId()
    {
        return (UUID)((Optional)this.dataWatcher.get(field_184756_bw)).orNull();
    }

    public void setOwnerId(UUID p_184754_1_)
    {
        this.dataWatcher.set(field_184756_bw, Optional.fromNullable(p_184754_1_));
    }

    public EntityLivingBase getOwner()
    {
        try
        {
            UUID uuid = this.getOwnerId();
            return uuid == null ? null : this.worldObj.getPlayerEntityByUUID(uuid);
        }
        catch (IllegalArgumentException var2)
        {
            return null;
        }
    }

    public boolean isOwner(EntityLivingBase entityIn)
    {
        return entityIn == this.getOwner();
    }

    /**
     * Returns the AITask responsible of the sit logic
     */
    public EntityAISit getAISit()
    {
        return this.aiSit;
    }

    public boolean shouldAttackEntity(EntityLivingBase p_142018_1_, EntityLivingBase p_142018_2_)
    {
        return true;
    }

    public Team getTeam()
    {
        if (this.isTamed())
        {
            EntityLivingBase entitylivingbase = this.getOwner();

            if (entitylivingbase != null)
            {
                return entitylivingbase.getTeam();
            }
        }

        return super.getTeam();
    }

    public boolean func_184191_r(Entity p_184191_1_)
    {
        if (this.isTamed())
        {
            EntityLivingBase entitylivingbase = this.getOwner();

            if (p_184191_1_ == entitylivingbase)
            {
                return true;
            }

            if (entitylivingbase != null)
            {
                return entitylivingbase.func_184191_r(p_184191_1_);
            }
        }

        return super.func_184191_r(p_184191_1_);
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        if (!this.worldObj.isRemote && this.worldObj.getGameRules().getBoolean("showDeathMessages") && this.getOwner() instanceof EntityPlayerMP)
        {
            this.getOwner().addChatMessage(this.getCombatTracker().getDeathMessage());
        }

        super.onDeath(cause);
    }
}
