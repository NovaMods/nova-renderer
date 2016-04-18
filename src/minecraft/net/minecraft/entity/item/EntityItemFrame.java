package net.minecraft.entity.item;

import com.google.common.base.Optional;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityHanging;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemMap;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.storage.MapData;

public class EntityItemFrame extends EntityHanging
{
    private static final DataParameter<Optional<ItemStack>> field_184525_c = EntityDataManager.<Optional<ItemStack>>createKey(EntityItemFrame.class, DataSerializers.OPTIONAL_ITEM_STACK);
    private static final DataParameter<Integer> ROTATION = EntityDataManager.<Integer>createKey(EntityItemFrame.class, DataSerializers.VARINT);

    /** Chance for this item frame's item to drop from the frame. */
    private float itemDropChance = 1.0F;

    public EntityItemFrame(World worldIn)
    {
        super(worldIn);
    }

    public EntityItemFrame(World worldIn, BlockPos p_i45852_2_, EnumFacing p_i45852_3_)
    {
        super(worldIn, p_i45852_2_);
        this.updateFacingWithBoundingBox(p_i45852_3_);
    }

    protected void entityInit()
    {
        this.getDataManager().register(field_184525_c, Optional.<ItemStack>absent());
        this.getDataManager().register(ROTATION, Integer.valueOf(0));
    }

    public float getCollisionBorderSize()
    {
        return 0.0F;
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
        else if (!source.isExplosion() && this.getDisplayedItem() != null)
        {
            if (!this.worldObj.isRemote)
            {
                this.dropItemOrSelf(source.getEntity(), false);
                this.playSound(SoundEvents.entity_itemframe_remove_item, 1.0F, 1.0F);
                this.setDisplayedItem((ItemStack)null);
            }

            return true;
        }
        else
        {
            return super.attackEntityFrom(source, amount);
        }
    }

    public int getWidthPixels()
    {
        return 12;
    }

    public int getHeightPixels()
    {
        return 12;
    }

    /**
     * Checks if the entity is in range to render by using the past in distance and comparing it to its average edge
     * length * 64 * renderDistanceWeight Args: distance
     */
    public boolean isInRangeToRenderDist(double distance)
    {
        double d0 = 16.0D;
        d0 = d0 * 64.0D * func_184183_bd();
        return distance < d0 * d0;
    }

    /**
     * Called when this entity is broken. Entity parameter may be null.
     */
    public void onBroken(Entity brokenEntity)
    {
        this.playSound(SoundEvents.entity_itemframe_break, 1.0F, 1.0F);
        this.dropItemOrSelf(brokenEntity, true);
    }

    public void func_184523_o()
    {
        this.playSound(SoundEvents.entity_itemframe_place, 1.0F, 1.0F);
    }

    public void dropItemOrSelf(Entity p_146065_1_, boolean p_146065_2_)
    {
        if (this.worldObj.getGameRules().getBoolean("doEntityDrops"))
        {
            ItemStack itemstack = this.getDisplayedItem();

            if (p_146065_1_ instanceof EntityPlayer)
            {
                EntityPlayer entityplayer = (EntityPlayer)p_146065_1_;

                if (entityplayer.capabilities.isCreativeMode)
                {
                    this.removeFrameFromMap(itemstack);
                    return;
                }
            }

            if (p_146065_2_)
            {
                this.entityDropItem(new ItemStack(Items.item_frame), 0.0F);
            }

            if (itemstack != null && this.rand.nextFloat() < this.itemDropChance)
            {
                itemstack = itemstack.copy();
                this.removeFrameFromMap(itemstack);
                this.entityDropItem(itemstack, 0.0F);
            }
        }
    }

    /**
     * Removes the dot representing this frame's position from the map when the item frame is broken.
     */
    private void removeFrameFromMap(ItemStack p_110131_1_)
    {
        if (p_110131_1_ != null)
        {
            if (p_110131_1_.getItem() == Items.filled_map)
            {
                MapData mapdata = ((ItemMap)p_110131_1_.getItem()).getMapData(p_110131_1_, this.worldObj);
                mapdata.mapDecorations.remove("frame-" + this.getEntityId());
            }

            p_110131_1_.setItemFrame((EntityItemFrame)null);
        }
    }

    public ItemStack getDisplayedItem()
    {
        return (ItemStack)((Optional)this.getDataManager().get(field_184525_c)).orNull();
    }

    public void setDisplayedItem(ItemStack p_82334_1_)
    {
        this.setDisplayedItemWithUpdate(p_82334_1_, true);
    }

    private void setDisplayedItemWithUpdate(ItemStack p_174864_1_, boolean p_174864_2_)
    {
        if (p_174864_1_ != null)
        {
            p_174864_1_ = p_174864_1_.copy();
            p_174864_1_.stackSize = 1;
            p_174864_1_.setItemFrame(this);
        }

        this.getDataManager().set(field_184525_c, Optional.fromNullable(p_174864_1_));
        this.getDataManager().setDirty(field_184525_c);

        if (p_174864_1_ != null)
        {
            this.playSound(SoundEvents.entity_itemframe_add_item, 1.0F, 1.0F);
        }

        if (p_174864_2_ && this.hangingPosition != null)
        {
            this.worldObj.updateComparatorOutputLevel(this.hangingPosition, Blocks.air);
        }
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        if (key.equals(field_184525_c))
        {
            ItemStack itemstack = this.getDisplayedItem();

            if (itemstack != null && itemstack.getItemFrame() != this)
            {
                itemstack.setItemFrame(this);
            }
        }
    }

    /**
     * Return the rotation of the item currently on this frame.
     */
    public int getRotation()
    {
        return ((Integer)this.getDataManager().get(ROTATION)).intValue();
    }

    public void setItemRotation(int rotationIn)
    {
        this.func_174865_a(rotationIn, true);
    }

    private void func_174865_a(int rotationIn, boolean p_174865_2_)
    {
        this.getDataManager().set(ROTATION, Integer.valueOf(rotationIn % 8));

        if (p_174865_2_ && this.hangingPosition != null)
        {
            this.worldObj.updateComparatorOutputLevel(this.hangingPosition, Blocks.air);
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        if (this.getDisplayedItem() != null)
        {
            tagCompound.setTag("Item", this.getDisplayedItem().writeToNBT(new NBTTagCompound()));
            tagCompound.setByte("ItemRotation", (byte)this.getRotation());
            tagCompound.setFloat("ItemDropChance", this.itemDropChance);
        }

        super.writeEntityToNBT(tagCompound);
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        NBTTagCompound nbttagcompound = tagCompund.getCompoundTag("Item");

        if (nbttagcompound != null && !nbttagcompound.hasNoTags())
        {
            this.setDisplayedItemWithUpdate(ItemStack.loadItemStackFromNBT(nbttagcompound), false);
            this.func_174865_a(tagCompund.getByte("ItemRotation"), false);

            if (tagCompund.hasKey("ItemDropChance", 99))
            {
                this.itemDropChance = tagCompund.getFloat("ItemDropChance");
            }
        }

        super.readEntityFromNBT(tagCompund);
    }

    public boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        if (this.getDisplayedItem() == null)
        {
            if (p_184230_2_ != null && !this.worldObj.isRemote)
            {
                this.setDisplayedItem(p_184230_2_);

                if (!p_184230_1_.capabilities.isCreativeMode)
                {
                    --p_184230_2_.stackSize;
                }
            }
        }
        else if (!this.worldObj.isRemote)
        {
            this.playSound(SoundEvents.entity_itemframe_rotate_item, 1.0F, 1.0F);
            this.setItemRotation(this.getRotation() + 1);
        }

        return true;
    }

    public int func_174866_q()
    {
        return this.getDisplayedItem() == null ? 0 : this.getRotation() % 8 + 1;
    }
}
