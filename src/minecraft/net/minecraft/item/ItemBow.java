package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Enchantments;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundCategory;
import net.minecraft.world.World;

public class ItemBow extends Item
{
    public ItemBow()
    {
        this.maxStackSize = 1;
        this.setMaxDamage(384);
        this.setCreativeTab(CreativeTabs.tabCombat);
        this.addPropertyOverride(new ResourceLocation("pull"), new IItemPropertyGetter()
        {
            public float apply(ItemStack stack, World worldIn, EntityLivingBase entityIn)
            {
                if (entityIn == null)
                {
                    return 0.0F;
                }
                else
                {
                    ItemStack itemstack = entityIn.func_184607_cu();
                    return itemstack != null && itemstack.getItem() == Items.bow ? (float)(stack.getMaxItemUseDuration() - entityIn.getItemInUseCount()) / 20.0F : 0.0F;
                }
            }
        });
        this.addPropertyOverride(new ResourceLocation("pulling"), new IItemPropertyGetter()
        {
            public float apply(ItemStack stack, World worldIn, EntityLivingBase entityIn)
            {
                return entityIn != null && entityIn.func_184587_cr() && entityIn.func_184607_cu() == stack ? 1.0F : 0.0F;
            }
        });
    }

    private ItemStack func_185060_a(EntityPlayer p_185060_1_)
    {
        if (this.func_185058_h_(p_185060_1_.getHeldItem(EnumHand.OFF_HAND)))
        {
            return p_185060_1_.getHeldItem(EnumHand.OFF_HAND);
        }
        else if (this.func_185058_h_(p_185060_1_.getHeldItem(EnumHand.MAIN_HAND)))
        {
            return p_185060_1_.getHeldItem(EnumHand.MAIN_HAND);
        }
        else
        {
            for (int i = 0; i < p_185060_1_.inventory.getSizeInventory(); ++i)
            {
                ItemStack itemstack = p_185060_1_.inventory.getStackInSlot(i);

                if (this.func_185058_h_(itemstack))
                {
                    return itemstack;
                }
            }

            return null;
        }
    }

    protected boolean func_185058_h_(ItemStack p_185058_1_)
    {
        return p_185058_1_ != null && p_185058_1_.getItem() instanceof ItemArrow;
    }

    /**
     * Called when the player stops using an Item (stops holding the right mouse button).
     */
    public void onPlayerStoppedUsing(ItemStack stack, World worldIn, EntityLivingBase playerIn, int timeLeft)
    {
        if (playerIn instanceof EntityPlayer)
        {
            EntityPlayer entityplayer = (EntityPlayer)playerIn;
            boolean flag = entityplayer.capabilities.isCreativeMode || EnchantmentHelper.getEnchantmentLevel(Enchantments.infinity, stack) > 0;
            ItemStack itemstack = this.func_185060_a(entityplayer);

            if (itemstack != null || flag)
            {
                if (itemstack == null)
                {
                    itemstack = new ItemStack(Items.arrow);
                }

                int i = this.getMaxItemUseDuration(stack) - timeLeft;
                float f = func_185059_b(i);

                if ((double)f >= 0.1D)
                {
                    boolean flag1 = flag && itemstack.getItem() == Items.arrow;

                    if (!worldIn.isRemote)
                    {
                        ItemArrow itemarrow = (ItemArrow)((ItemArrow)(itemstack.getItem() instanceof ItemArrow ? itemstack.getItem() : Items.arrow));
                        EntityArrow entityarrow = itemarrow.func_185052_a(worldIn, itemstack, entityplayer);
                        entityarrow.func_184547_a(entityplayer, entityplayer.rotationPitch, entityplayer.rotationYaw, 0.0F, f * 3.0F, 1.0F);

                        if (f == 1.0F)
                        {
                            entityarrow.setIsCritical(true);
                        }

                        int j = EnchantmentHelper.getEnchantmentLevel(Enchantments.power, stack);

                        if (j > 0)
                        {
                            entityarrow.setDamage(entityarrow.getDamage() + (double)j * 0.5D + 0.5D);
                        }

                        int k = EnchantmentHelper.getEnchantmentLevel(Enchantments.punch, stack);

                        if (k > 0)
                        {
                            entityarrow.setKnockbackStrength(k);
                        }

                        if (EnchantmentHelper.getEnchantmentLevel(Enchantments.flame, stack) > 0)
                        {
                            entityarrow.setFire(100);
                        }

                        stack.damageItem(1, entityplayer);

                        if (flag1)
                        {
                            entityarrow.canBePickedUp = EntityArrow.PickupStatus.CREATIVE_ONLY;
                        }

                        worldIn.spawnEntityInWorld(entityarrow);
                    }

                    worldIn.func_184148_a((EntityPlayer)null, entityplayer.posX, entityplayer.posY, entityplayer.posZ, SoundEvents.entity_arrow_shoot, SoundCategory.NEUTRAL, 1.0F, 1.0F / (itemRand.nextFloat() * 0.4F + 1.2F) + f * 0.5F);

                    if (!flag1)
                    {
                        --itemstack.stackSize;

                        if (itemstack.stackSize == 0)
                        {
                            entityplayer.inventory.func_184437_d(itemstack);
                        }
                    }

                    entityplayer.triggerAchievement(StatList.func_188057_b(this));
                }
            }
        }
    }

    public static float func_185059_b(int p_185059_0_)
    {
        float f = (float)p_185059_0_ / 20.0F;
        f = (f * f + f * 2.0F) / 3.0F;

        if (f > 1.0F)
        {
            f = 1.0F;
        }

        return f;
    }

    /**
     * How long it takes to use or consume an item
     */
    public int getMaxItemUseDuration(ItemStack stack)
    {
        return 72000;
    }

    /**
     * returns the action that specifies what animation to play when the items is being used
     */
    public EnumAction getItemUseAction(ItemStack stack)
    {
        return EnumAction.BOW;
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        boolean flag = this.func_185060_a(playerIn) != null;

        if (!playerIn.capabilities.isCreativeMode && !flag)
        {
            return !flag ? new ActionResult(EnumActionResult.FAIL, itemStackIn) : new ActionResult(EnumActionResult.PASS, itemStackIn);
        }
        else
        {
            playerIn.func_184598_c(hand);
            return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
        }
    }

    /**
     * Return the enchantability factor of the item, most of the time is based on material.
     */
    public int getItemEnchantability()
    {
        return 1;
    }
}
