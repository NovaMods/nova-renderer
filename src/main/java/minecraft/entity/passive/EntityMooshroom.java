package net.minecraft.entity.passive;

import net.minecraft.entity.EntityAgeable;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntityMooshroom extends EntityCow
{
    public EntityMooshroom(World worldIn)
    {
        super(worldIn);
        this.setSize(0.9F, 1.4F);
        this.spawnableBlock = Blocks.mycelium;
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (p_184645_3_ != null && p_184645_3_.getItem() == Items.bowl && this.getGrowingAge() >= 0 && !p_184645_1_.capabilities.isCreativeMode)
        {
            if (--p_184645_3_.stackSize == 0)
            {
                p_184645_1_.setHeldItem(p_184645_2_, new ItemStack(Items.mushroom_stew));
            }
            else if (!p_184645_1_.inventory.addItemStackToInventory(new ItemStack(Items.mushroom_stew)))
            {
                p_184645_1_.dropPlayerItemWithRandomChoice(new ItemStack(Items.mushroom_stew), false);
            }

            return true;
        }
        else if (p_184645_3_ != null && p_184645_3_.getItem() == Items.shears && this.getGrowingAge() >= 0)
        {
            this.setDead();
            this.worldObj.spawnParticle(EnumParticleTypes.EXPLOSION_LARGE, this.posX, this.posY + (double)(this.height / 2.0F), this.posZ, 0.0D, 0.0D, 0.0D, new int[0]);

            if (!this.worldObj.isRemote)
            {
                EntityCow entitycow = new EntityCow(this.worldObj);
                entitycow.setLocationAndAngles(this.posX, this.posY, this.posZ, this.rotationYaw, this.rotationPitch);
                entitycow.setHealth(this.getHealth());
                entitycow.renderYawOffset = this.renderYawOffset;

                if (this.hasCustomName())
                {
                    entitycow.setCustomNameTag(this.getCustomNameTag());
                }

                this.worldObj.spawnEntityInWorld(entitycow);

                for (int i = 0; i < 5; ++i)
                {
                    this.worldObj.spawnEntityInWorld(new EntityItem(this.worldObj, this.posX, this.posY + (double)this.height, this.posZ, new ItemStack(Blocks.red_mushroom)));
                }

                p_184645_3_.damageItem(1, p_184645_1_);
                this.playSound(SoundEvents.entity_mooshroom_shear, 1.0F, 1.0F);
            }

            return true;
        }
        else
        {
            return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
        }
    }

    public EntityMooshroom createChild(EntityAgeable ageable)
    {
        return new EntityMooshroom(this.worldObj);
    }

    protected ResourceLocation func_184647_J()
    {
        return LootTableList.ENTITIES_MUSHROOM_COW;
    }
}
