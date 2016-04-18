package net.minecraft.entity.effect;

import java.util.List;
import net.minecraft.block.material.Material;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.EnumDifficulty;
import net.minecraft.world.World;

public class EntityLightningBolt extends EntityWeatherEffect
{
    /**
     * Declares which state the lightning bolt is in. Whether it's in the air, hit the ground, etc.
     */
    private int lightningState;

    /**
     * A random long that is used to change the vertex of the lightning rendered in RenderLightningBolt
     */
    public long boltVertex;

    /**
     * Determines the time before the EntityLightningBolt is destroyed. It is a random integer decremented over time.
     */
    private int boltLivingTime;
    private final boolean isEffect;

    public EntityLightningBolt(World p_i46780_1_, double p_i46780_2_, double p_i46780_4_, double p_i46780_6_, boolean p_i46780_8_)
    {
        super(p_i46780_1_);
        this.setLocationAndAngles(p_i46780_2_, p_i46780_4_, p_i46780_6_, 0.0F, 0.0F);
        this.lightningState = 2;
        this.boltVertex = this.rand.nextLong();
        this.boltLivingTime = this.rand.nextInt(3) + 1;
        this.isEffect = p_i46780_8_;
        BlockPos blockpos = new BlockPos(this);

        if (!p_i46780_8_ && !p_i46780_1_.isRemote && p_i46780_1_.getGameRules().getBoolean("doFireTick") && (p_i46780_1_.getDifficulty() == EnumDifficulty.NORMAL || p_i46780_1_.getDifficulty() == EnumDifficulty.HARD) && p_i46780_1_.isAreaLoaded(blockpos, 10))
        {
            if (p_i46780_1_.getBlockState(blockpos).getMaterial() == Material.air && Blocks.fire.canPlaceBlockAt(p_i46780_1_, blockpos))
            {
                p_i46780_1_.setBlockState(blockpos, Blocks.fire.getDefaultState());
            }

            for (int i = 0; i < 4; ++i)
            {
                BlockPos blockpos1 = blockpos.add(this.rand.nextInt(3) - 1, this.rand.nextInt(3) - 1, this.rand.nextInt(3) - 1);

                if (p_i46780_1_.getBlockState(blockpos1).getMaterial() == Material.air && Blocks.fire.canPlaceBlockAt(p_i46780_1_, blockpos1))
                {
                    p_i46780_1_.setBlockState(blockpos1, Blocks.fire.getDefaultState());
                }
            }
        }
    }

    public SoundCategory getSoundCategory()
    {
        return SoundCategory.WEATHER;
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (this.lightningState == 2)
        {
            this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, SoundEvents.entity_lightning_thunder, SoundCategory.WEATHER, 10000.0F, 0.8F + this.rand.nextFloat() * 0.2F);
            this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, SoundEvents.entity_lightning_impact, SoundCategory.WEATHER, 2.0F, 0.5F + this.rand.nextFloat() * 0.2F);
        }

        --this.lightningState;

        if (this.lightningState < 0)
        {
            if (this.boltLivingTime == 0)
            {
                this.setDead();
            }
            else if (this.lightningState < -this.rand.nextInt(10))
            {
                --this.boltLivingTime;
                this.lightningState = 1;

                if (!this.isEffect && !this.worldObj.isRemote)
                {
                    this.boltVertex = this.rand.nextLong();
                    BlockPos blockpos = new BlockPos(this);

                    if (this.worldObj.getGameRules().getBoolean("doFireTick") && this.worldObj.isAreaLoaded(blockpos, 10) && this.worldObj.getBlockState(blockpos).getMaterial() == Material.air && Blocks.fire.canPlaceBlockAt(this.worldObj, blockpos))
                    {
                        this.worldObj.setBlockState(blockpos, Blocks.fire.getDefaultState());
                    }
                }
            }
        }

        if (this.lightningState >= 0)
        {
            if (this.worldObj.isRemote)
            {
                this.worldObj.setLastLightningBolt(2);
            }
            else if (!this.isEffect)
            {
                double d0 = 3.0D;
                List<Entity> list = this.worldObj.getEntitiesWithinAABBExcludingEntity(this, new AxisAlignedBB(this.posX - d0, this.posY - d0, this.posZ - d0, this.posX + d0, this.posY + 6.0D + d0, this.posZ + d0));

                for (int i = 0; i < list.size(); ++i)
                {
                    Entity entity = (Entity)list.get(i);
                    entity.onStruckByLightning(this);
                }
            }
        }
    }

    protected void entityInit()
    {
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
    }
}
