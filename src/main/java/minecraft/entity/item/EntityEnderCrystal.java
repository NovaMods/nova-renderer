package net.minecraft.entity.item;

import com.google.common.base.Optional;
import net.minecraft.entity.Entity;
import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.WorldProviderEnd;
import net.minecraft.world.end.DragonFightManager;

public class EntityEnderCrystal extends Entity
{
    private static final DataParameter<Optional<BlockPos>> BEAM_TARGET = EntityDataManager.<Optional<BlockPos>>createKey(EntityEnderCrystal.class, DataSerializers.OPTIONAL_BLOCK_POS);
    private static final DataParameter<Boolean> SHOW_BOTTOM = EntityDataManager.<Boolean>createKey(EntityEnderCrystal.class, DataSerializers.BOOLEAN);

    /** Used to create the rotation animation when rendering the crystal. */
    public int innerRotation;

    public EntityEnderCrystal(World worldIn)
    {
        super(worldIn);
        this.preventEntitySpawning = true;
        this.setSize(2.0F, 2.0F);
        this.innerRotation = this.rand.nextInt(100000);
    }

    public EntityEnderCrystal(World worldIn, double x, double y, double z)
    {
        this(worldIn);
        this.setPosition(x, y, z);
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
        this.getDataManager().register(BEAM_TARGET, Optional.<BlockPos>absent());
        this.getDataManager().register(SHOW_BOTTOM, Boolean.valueOf(true));
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        this.prevPosX = this.posX;
        this.prevPosY = this.posY;
        this.prevPosZ = this.posZ;
        ++this.innerRotation;

        if (!this.worldObj.isRemote)
        {
            BlockPos blockpos = new BlockPos(this);

            if (this.worldObj.provider instanceof WorldProviderEnd && this.worldObj.getBlockState(blockpos).getBlock() != Blocks.fire)
            {
                this.worldObj.setBlockState(blockpos, Blocks.fire.getDefaultState());
            }
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        if (this.getBeamTarget() != null)
        {
            tagCompound.setTag("BeamTarget", NBTUtil.createPosTag(this.getBeamTarget()));
        }

        tagCompound.setBoolean("ShowBottom", this.shouldShowBottom());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        if (tagCompund.hasKey("BeamTarget", 10))
        {
            this.setBeamTarget(NBTUtil.getPosFromTag(tagCompund.getCompoundTag("BeamTarget")));
        }

        if (tagCompund.hasKey("ShowBottom", 1))
        {
            this.setShowBottom(tagCompund.getBoolean("ShowBottom"));
        }
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
        if (this.isEntityInvulnerable(source))
        {
            return false;
        }
        else if (source.getEntity() instanceof EntityDragon)
        {
            return false;
        }
        else
        {
            if (!this.isDead && !this.worldObj.isRemote)
            {
                this.setDead();

                if (!this.worldObj.isRemote)
                {
                    this.worldObj.createExplosion((Entity)null, this.posX, this.posY, this.posZ, 6.0F, true);
                    this.func_184519_a(source);
                }
            }

            return true;
        }
    }

    /**
     * Called by the /kill command.
     */
    public void onKillCommand()
    {
        this.func_184519_a(DamageSource.generic);
        super.onKillCommand();
    }

    private void func_184519_a(DamageSource source)
    {
        if (this.worldObj.provider instanceof WorldProviderEnd)
        {
            WorldProviderEnd worldproviderend = (WorldProviderEnd)this.worldObj.provider;
            DragonFightManager dragonfightmanager = worldproviderend.getDragonFightManager();

            if (dragonfightmanager != null)
            {
                dragonfightmanager.func_186090_a(this, source);
            }
        }
    }

    public void setBeamTarget(BlockPos beamTarget)
    {
        this.getDataManager().set(BEAM_TARGET, Optional.fromNullable(beamTarget));
    }

    public BlockPos getBeamTarget()
    {
        return (BlockPos)((Optional)this.getDataManager().get(BEAM_TARGET)).orNull();
    }

    public void setShowBottom(boolean showBottom)
    {
        this.getDataManager().set(SHOW_BOTTOM, Boolean.valueOf(showBottom));
    }

    public boolean shouldShowBottom()
    {
        return ((Boolean)this.getDataManager().get(SHOW_BOTTOM)).booleanValue();
    }

    /**
     * Checks if the entity is in range to render by using the past in distance and comparing it to its average edge
     * length * 64 * renderDistanceWeight Args: distance
     */
    public boolean isInRangeToRenderDist(double distance)
    {
        return super.isInRangeToRenderDist(distance) || this.getBeamTarget() != null;
    }
}
