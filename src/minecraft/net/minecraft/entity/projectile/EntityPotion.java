package net.minecraft.entity.projectile;

import com.google.common.base.Optional;
import java.util.List;
import net.minecraft.entity.EntityAreaEffectCloud;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.PotionTypes;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.potion.Potion;
import net.minecraft.potion.PotionEffect;
import net.minecraft.potion.PotionType;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class EntityPotion extends EntityThrowable
{
    private static final DataParameter<Optional<ItemStack>> field_184545_d = EntityDataManager.<Optional<ItemStack>>createKey(EntityItem.class, DataSerializers.OPTIONAL_ITEM_STACK);
    private static final Logger field_184546_e = LogManager.getLogger();

    public EntityPotion(World worldIn)
    {
        super(worldIn);
    }

    public EntityPotion(World worldIn, EntityLivingBase throwerIn, ItemStack potionDamageIn)
    {
        super(worldIn, throwerIn);
        this.func_184541_a(potionDamageIn);
    }

    public EntityPotion(World worldIn, double x, double y, double z, ItemStack potionDamageIn)
    {
        super(worldIn, x, y, z);

        if (potionDamageIn != null)
        {
            this.func_184541_a(potionDamageIn);
        }
    }

    protected void entityInit()
    {
        this.getDataManager().register(field_184545_d, Optional.<ItemStack>absent());
    }

    public ItemStack func_184543_l()
    {
        ItemStack itemstack = (ItemStack)((Optional)this.getDataManager().get(field_184545_d)).orNull();

        if (itemstack == null || itemstack.getItem() != Items.splash_potion && itemstack.getItem() != Items.lingering_potion)
        {
            if (this.worldObj != null)
            {
                field_184546_e.error("ThrownPotion entity " + this.getEntityId() + " has no item?!");
            }

            return new ItemStack(Items.splash_potion);
        }
        else
        {
            return itemstack;
        }
    }

    public void func_184541_a(ItemStack p_184541_1_)
    {
        this.getDataManager().set(field_184545_d, Optional.fromNullable(p_184541_1_));
        this.getDataManager().setDirty(field_184545_d);
    }

    /**
     * Gets the amount of gravity to apply to the thrown entity with each tick.
     */
    protected float getGravityVelocity()
    {
        return 0.05F;
    }

    /**
     * Called when this EntityThrowable hits a block or entity.
     */
    protected void onImpact(RayTraceResult result)
    {
        if (!this.worldObj.isRemote)
        {
            ItemStack itemstack = this.func_184543_l();
            PotionType potiontype = PotionUtils.func_185191_c(itemstack);
            List<PotionEffect> list = PotionUtils.getEffectsFromStack(itemstack);

            if (result.typeOfHit == RayTraceResult.Type.BLOCK && potiontype == PotionTypes.water && list.isEmpty())
            {
                BlockPos blockpos = result.getBlockPos().offset(result.sideHit);
                this.func_184542_a(blockpos);

                for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
                {
                    this.func_184542_a(blockpos.offset(enumfacing));
                }

                this.worldObj.playAuxSFX(2002, new BlockPos(this), PotionType.getID(potiontype));
                this.setDead();
            }
            else
            {
                if (!list.isEmpty())
                {
                    if (this.func_184544_n())
                    {
                        EntityAreaEffectCloud entityareaeffectcloud = new EntityAreaEffectCloud(this.worldObj, this.posX, this.posY, this.posZ);
                        entityareaeffectcloud.func_184481_a(this.getThrower());
                        entityareaeffectcloud.setRadius(3.0F);
                        entityareaeffectcloud.func_184495_b(-0.5F);
                        entityareaeffectcloud.func_184485_d(10);
                        entityareaeffectcloud.func_184487_c(-entityareaeffectcloud.getRadius() / (float)entityareaeffectcloud.func_184489_o());
                        entityareaeffectcloud.func_184484_a(potiontype);

                        for (PotionEffect potioneffect : PotionUtils.func_185190_b(itemstack))
                        {
                            entityareaeffectcloud.func_184496_a(new PotionEffect(potioneffect.func_188419_a(), potioneffect.getDuration(), potioneffect.getAmplifier()));
                        }

                        this.worldObj.spawnEntityInWorld(entityareaeffectcloud);
                    }
                    else
                    {
                        AxisAlignedBB axisalignedbb = this.getEntityBoundingBox().expand(4.0D, 2.0D, 4.0D);
                        List<EntityLivingBase> list1 = this.worldObj.<EntityLivingBase>getEntitiesWithinAABB(EntityLivingBase.class, axisalignedbb);

                        if (!list1.isEmpty())
                        {
                            for (EntityLivingBase entitylivingbase : list1)
                            {
                                if (entitylivingbase.func_184603_cC())
                                {
                                    double d0 = this.getDistanceSqToEntity(entitylivingbase);

                                    if (d0 < 16.0D)
                                    {
                                        double d1 = 1.0D - Math.sqrt(d0) / 4.0D;

                                        if (entitylivingbase == result.entityHit)
                                        {
                                            d1 = 1.0D;
                                        }

                                        for (PotionEffect potioneffect1 : list)
                                        {
                                            Potion potion = potioneffect1.func_188419_a();

                                            if (potion.isInstant())
                                            {
                                                potion.affectEntity(this, this.getThrower(), entitylivingbase, potioneffect1.getAmplifier(), d1);
                                            }
                                            else
                                            {
                                                int i = (int)(d1 * (double)potioneffect1.getDuration() + 0.5D);

                                                if (i > 20)
                                                {
                                                    entitylivingbase.addPotionEffect(new PotionEffect(potion, i, potioneffect1.getAmplifier()));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                this.worldObj.playAuxSFX(2002, new BlockPos(this), PotionType.getID(potiontype));
                this.setDead();
            }
        }
    }

    private boolean func_184544_n()
    {
        return this.func_184543_l().getItem() == Items.lingering_potion;
    }

    private void func_184542_a(BlockPos p_184542_1_)
    {
        if (this.worldObj.getBlockState(p_184542_1_).getBlock() == Blocks.fire)
        {
            this.worldObj.setBlockState(p_184542_1_, Blocks.air.getDefaultState(), 2);
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        ItemStack itemstack = ItemStack.loadItemStackFromNBT(tagCompund.getCompoundTag("Potion"));

        if (itemstack == null)
        {
            this.setDead();
        }
        else
        {
            this.func_184541_a(itemstack);
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        ItemStack itemstack = this.func_184543_l();

        if (itemstack != null)
        {
            tagCompound.setTag("Potion", itemstack.writeToNBT(new NBTTagCompound()));
        }
    }
}
