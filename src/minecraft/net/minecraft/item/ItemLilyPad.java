package net.minecraft.item;

import net.minecraft.block.Block;
import net.minecraft.block.BlockLiquid;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.World;

public class ItemLilyPad extends ItemColored
{
    public ItemLilyPad(Block block)
    {
        super(block, false);
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
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
                    return new ActionResult(EnumActionResult.FAIL, itemStackIn);
                }

                BlockPos blockpos1 = blockpos.up();
                IBlockState iblockstate = worldIn.getBlockState(blockpos);

                if (iblockstate.getMaterial() == Material.water && ((Integer)iblockstate.getValue(BlockLiquid.LEVEL)).intValue() == 0 && worldIn.isAirBlock(blockpos1))
                {
                    worldIn.setBlockState(blockpos1, Blocks.waterlily.getDefaultState(), 11);

                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        --itemStackIn.stackSize;
                    }

                    playerIn.triggerAchievement(StatList.func_188057_b(this));
                    worldIn.func_184133_a(playerIn, blockpos, SoundEvents.block_waterlily_place, SoundCategory.BLOCKS, 1.0F, 1.0F);
                    return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
                }
            }

            return new ActionResult(EnumActionResult.FAIL, itemStackIn);
        }
    }
}
