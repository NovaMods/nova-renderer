package net.minecraft.item;

import net.minecraft.block.Block;
import net.minecraft.block.BlockLiquid;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.World;

public class ItemBucket extends Item
{
    /** field for checking if the bucket has been filled. */
    private Block isFull;

    public ItemBucket(Block containedBlock)
    {
        this.maxStackSize = 1;
        this.isFull = containedBlock;
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        boolean flag = this.isFull == Blocks.air;
        RayTraceResult raytraceresult = this.getMovingObjectPositionFromPlayer(worldIn, playerIn, flag);

        if (raytraceresult == null)
        {
            return new ActionResult(EnumActionResult.PASS, itemStackIn);
        }
        else if (raytraceresult.typeOfHit != RayTraceResult.Type.BLOCK)
        {
            return new ActionResult(EnumActionResult.PASS, itemStackIn);
        }
        else
        {
            BlockPos blockpos = raytraceresult.getBlockPos();

            if (!worldIn.isBlockModifiable(playerIn, blockpos))
            {
                return new ActionResult(EnumActionResult.FAIL, itemStackIn);
            }
            else if (flag)
            {
                if (!playerIn.canPlayerEdit(blockpos.offset(raytraceresult.sideHit), raytraceresult.sideHit, itemStackIn))
                {
                    return new ActionResult(EnumActionResult.FAIL, itemStackIn);
                }
                else
                {
                    IBlockState iblockstate = worldIn.getBlockState(blockpos);
                    Material material = iblockstate.getMaterial();

                    if (material == Material.water && ((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue() == 0)
                    {
                        worldIn.setBlockState(blockpos, Blocks.air.getDefaultState(), 11);
                        playerIn.triggerAchievement(StatList.func_188057_b(this));
                        playerIn.playSound(SoundEvents.item_bucket_fill, 1.0F, 1.0F);
                        return new ActionResult(EnumActionResult.SUCCESS, this.fillBucket(itemStackIn, playerIn, Items.water_bucket));
                    }
                    else if (material == Material.lava && ((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue() == 0)
                    {
                        playerIn.playSound(SoundEvents.item_bucket_fill_lava, 1.0F, 1.0F);
                        worldIn.setBlockState(blockpos, Blocks.air.getDefaultState(), 11);
                        playerIn.triggerAchievement(StatList.func_188057_b(this));
                        return new ActionResult(EnumActionResult.SUCCESS, this.fillBucket(itemStackIn, playerIn, Items.lava_bucket));
                    }
                    else
                    {
                        return new ActionResult(EnumActionResult.FAIL, itemStackIn);
                    }
                }
            }
            else
            {
                boolean flag1 = worldIn.getBlockState(blockpos).getBlock().isReplaceable(worldIn, blockpos);
                BlockPos blockpos1 = flag1 && raytraceresult.sideHit == EnumFacing.UP ? blockpos : blockpos.offset(raytraceresult.sideHit);

                if (!playerIn.canPlayerEdit(blockpos1, raytraceresult.sideHit, itemStackIn))
                {
                    return new ActionResult(EnumActionResult.FAIL, itemStackIn);
                }
                else if (this.tryPlaceContainedLiquid(playerIn, worldIn, blockpos1))
                {
                    playerIn.triggerAchievement(StatList.func_188057_b(this));
                    return !playerIn.capabilities.isCreativeMode ? new ActionResult(EnumActionResult.SUCCESS, new ItemStack(Items.bucket)) : new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
                }
                else
                {
                    return new ActionResult(EnumActionResult.FAIL, itemStackIn);
                }
            }
        }
    }

    private ItemStack fillBucket(ItemStack emptyBuckets, EntityPlayer player, Item fullBucket)
    {
        if (player.capabilities.isCreativeMode)
        {
            return emptyBuckets;
        }
        else if (--emptyBuckets.stackSize <= 0)
        {
            return new ItemStack(fullBucket);
        }
        else
        {
            if (!player.inventory.addItemStackToInventory(new ItemStack(fullBucket)))
            {
                player.dropPlayerItemWithRandomChoice(new ItemStack(fullBucket), false);
            }

            return emptyBuckets;
        }
    }

    public boolean tryPlaceContainedLiquid(EntityPlayer worldIn, World pos, BlockPos p_180616_3_)
    {
        if (this.isFull == Blocks.air)
        {
            return false;
        }
        else
        {
            IBlockState iblockstate = pos.getBlockState(p_180616_3_);
            Material material = iblockstate.getMaterial();
            boolean flag = !material.isSolid();
            boolean flag1 = iblockstate.getBlock().isReplaceable(pos, p_180616_3_);

            if (!pos.isAirBlock(p_180616_3_) && !flag && !flag1)
            {
                return false;
            }
            else
            {
                if (pos.provider.doesWaterVaporize() && this.isFull == Blocks.flowing_water)
                {
                    int l = p_180616_3_.getX();
                    int i = p_180616_3_.getY();
                    int j = p_180616_3_.getZ();
                    pos.func_184133_a(worldIn, p_180616_3_, SoundEvents.block_fire_extinguish, SoundCategory.BLOCKS, 0.5F, 2.6F + (pos.rand.nextFloat() - pos.rand.nextFloat()) * 0.8F);

                    for (int k = 0; k < 8; ++k)
                    {
                        pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, (double)l + Math.random(), (double)i + Math.random(), (double)j + Math.random(), 0.0D, 0.0D, 0.0D, new int[0]);
                    }
                }
                else
                {
                    if (!pos.isRemote && (flag || flag1) && !material.isLiquid())
                    {
                        pos.destroyBlock(p_180616_3_, true);
                    }

                    SoundEvent soundevent = this.isFull == Blocks.flowing_lava ? SoundEvents.item_bucket_empty_lava : SoundEvents.item_bucket_empty;
                    pos.func_184133_a(worldIn, p_180616_3_, soundevent, SoundCategory.BLOCKS, 1.0F, 1.0F);
                    pos.setBlockState(p_180616_3_, this.isFull.getDefaultState(), 11);
                }

                return true;
            }
        }
    }
}
