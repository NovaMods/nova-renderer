package net.minecraft.item;

import net.minecraft.block.BlockEndPortalFrame;
import net.minecraft.block.state.IBlockState;
import net.minecraft.block.state.pattern.BlockPattern;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.item.EntityEnderEye;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.stats.StatList;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;

public class ItemEnderEye extends Item
{
    public ItemEnderEye()
    {
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos);

        if (playerIn.canPlayerEdit(pos.offset(hitX), hitX, stack) && iblockstate.getBlock() == Blocks.end_portal_frame && !((Boolean)iblockstate.getValue(BlockEndPortalFrame.EYE)).booleanValue())
        {
            if (worldIn.isRemote)
            {
                return EnumActionResult.SUCCESS;
            }
            else
            {
                worldIn.setBlockState(pos, iblockstate.withProperty(BlockEndPortalFrame.EYE, Boolean.valueOf(true)), 2);
                worldIn.updateComparatorOutputLevel(pos, Blocks.end_portal_frame);
                --stack.stackSize;

                for (int i = 0; i < 16; ++i)
                {
                    double d0 = (double)((float)pos.getX() + (5.0F + itemRand.nextFloat() * 6.0F) / 16.0F);
                    double d1 = (double)((float)pos.getY() + 0.8125F);
                    double d2 = (double)((float)pos.getZ() + (5.0F + itemRand.nextFloat() * 6.0F) / 16.0F);
                    double d3 = 0.0D;
                    double d4 = 0.0D;
                    double d5 = 0.0D;
                    worldIn.spawnParticle(EnumParticleTypes.SMOKE_NORMAL, d0, d1, d2, 0.0D, 0.0D, 0.0D, new int[0]);
                }

                EnumFacing enumfacing = (EnumFacing)iblockstate.getValue(BlockEndPortalFrame.FACING);
                BlockPattern.PatternHelper blockpattern$patternhelper = BlockEndPortalFrame.func_185661_e().match(worldIn, pos);

                if (blockpattern$patternhelper != null)
                {
                    BlockPos blockpos = blockpattern$patternhelper.getPos().add(-3, 0, -3);

                    for (int k = 0; k < 3; ++k)
                    {
                        for (int j = 0; j < 3; ++j)
                        {
                            worldIn.setBlockState(blockpos.add(k, 0, j), Blocks.end_portal.getDefaultState(), 2);
                        }
                    }
                }

                return EnumActionResult.SUCCESS;
            }
        }
        else
        {
            return EnumActionResult.FAIL;
        }
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        RayTraceResult raytraceresult = this.getMovingObjectPositionFromPlayer(worldIn, playerIn, false);

        if (raytraceresult != null && raytraceresult.typeOfHit == RayTraceResult.Type.BLOCK && worldIn.getBlockState(raytraceresult.getBlockPos()).getBlock() == Blocks.end_portal_frame)
        {
            return new ActionResult(EnumActionResult.PASS, itemStackIn);
        }
        else
        {
            if (!worldIn.isRemote)
            {
                BlockPos blockpos = ((WorldServer)worldIn).getChunkProvider().getStrongholdGen(worldIn, "Stronghold", new BlockPos(playerIn));

                if (blockpos != null)
                {
                    EntityEnderEye entityendereye = new EntityEnderEye(worldIn, playerIn.posX, playerIn.posY + (double)(playerIn.height / 2.0F), playerIn.posZ);
                    entityendereye.moveTowards(blockpos);
                    worldIn.spawnEntityInWorld(entityendereye);
                    worldIn.func_184148_a((EntityPlayer)null, playerIn.posX, playerIn.posY, playerIn.posZ, SoundEvents.entity_endereye_launch, SoundCategory.NEUTRAL, 0.5F, 0.4F / (itemRand.nextFloat() * 0.4F + 0.8F));
                    worldIn.playAuxSFXAtEntity((EntityPlayer)null, 1003, new BlockPos(playerIn), 0);

                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        --itemStackIn.stackSize;
                    }

                    playerIn.triggerAchievement(StatList.func_188057_b(this));
                    return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
                }
            }

            return new ActionResult(EnumActionResult.FAIL, itemStackIn);
        }
    }
}
