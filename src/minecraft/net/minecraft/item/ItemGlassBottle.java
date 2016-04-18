package net.minecraft.item;

import com.google.common.base.Predicate;
import java.util.List;
import net.minecraft.block.material.Material;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityAreaEffectCloud;
import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.World;

public class ItemGlassBottle extends Item
{
    public ItemGlassBottle()
    {
        this.setCreativeTab(CreativeTabs.tabBrewing);
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        List<EntityAreaEffectCloud> list = worldIn.<EntityAreaEffectCloud>getEntitiesWithinAABB(EntityAreaEffectCloud.class, playerIn.getEntityBoundingBox().func_186662_g(2.0D), new Predicate<EntityAreaEffectCloud>()
        {
            public boolean apply(EntityAreaEffectCloud p_apply_1_)
            {
                return p_apply_1_ != null && p_apply_1_.isEntityAlive() && p_apply_1_.func_184494_w() instanceof EntityDragon;
            }
        });

        if (!list.isEmpty())
        {
            EntityAreaEffectCloud entityareaeffectcloud = (EntityAreaEffectCloud)list.get(0);
            entityareaeffectcloud.setRadius(entityareaeffectcloud.getRadius() - 0.5F);
            worldIn.func_184148_a((EntityPlayer)null, playerIn.posX, playerIn.posY, playerIn.posZ, SoundEvents.item_bottle_fill_dragonbreath, SoundCategory.NEUTRAL, 1.0F, 1.0F);
            return new ActionResult(EnumActionResult.SUCCESS, this.func_185061_a(itemStackIn, playerIn, new ItemStack(Items.dragon_breath)));
        }
        else
        {
            RayTraceResult raytraceresult = this.getMovingObjectPositionFromPlayer(worldIn, playerIn, true);

            if (raytraceresult == null)
            {
                return new ActionResult(EnumActionResult.PASS, itemStackIn);
            }
            else
            {
                if (raytraceresult.typeOfHit == RayTraceResult.Type.BLOCK)
                {
                    BlockPos blockpos = raytraceresult.getBlockPos();

                    if (!worldIn.isBlockModifiable(playerIn, blockpos) || !playerIn.canPlayerEdit(blockpos.offset(raytraceresult.sideHit), raytraceresult.sideHit, itemStackIn))
                    {
                        return new ActionResult(EnumActionResult.PASS, itemStackIn);
                    }

                    if (worldIn.getBlockState(blockpos).getMaterial() == Material.water)
                    {
                        worldIn.func_184148_a(playerIn, playerIn.posX, playerIn.posY, playerIn.posZ, SoundEvents.item_bottle_fill, SoundCategory.NEUTRAL, 1.0F, 1.0F);
                        return new ActionResult(EnumActionResult.SUCCESS, this.func_185061_a(itemStackIn, playerIn, new ItemStack(Items.potionitem)));
                    }
                }

                return new ActionResult(EnumActionResult.PASS, itemStackIn);
            }
        }
    }

    protected ItemStack func_185061_a(ItemStack p_185061_1_, EntityPlayer p_185061_2_, ItemStack p_185061_3_)
    {
        --p_185061_1_.stackSize;
        p_185061_2_.triggerAchievement(StatList.func_188057_b(this));

        if (p_185061_1_.stackSize <= 0)
        {
            return p_185061_3_;
        }
        else
        {
            if (!p_185061_2_.inventory.addItemStackToInventory(p_185061_3_))
            {
                p_185061_2_.dropPlayerItemWithRandomChoice(p_185061_3_, false);
            }

            return p_185061_1_;
        }
    }
}
