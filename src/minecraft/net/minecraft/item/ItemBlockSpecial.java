package net.minecraft.item;

import net.minecraft.block.Block;
import net.minecraft.block.BlockSnow;
import net.minecraft.block.SoundType;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemBlockSpecial extends Item
{
    private Block block;

    public ItemBlockSpecial(Block block)
    {
        this.block = block;
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos);
        Block block = iblockstate.getBlock();

        if (block == Blocks.snow_layer && ((Integer)iblockstate.getValue(BlockSnow.LAYERS)).intValue() < 1)
        {
            hitX = EnumFacing.UP;
        }
        else if (!block.isReplaceable(worldIn, pos))
        {
            pos = pos.offset(hitX);
        }

        if (playerIn.canPlayerEdit(pos, hitX, stack) && stack.stackSize != 0 && worldIn.canBlockBePlaced(this.block, pos, false, hitX, (Entity)null, stack))
        {
            IBlockState iblockstate1 = this.block.onBlockPlaced(worldIn, pos, hitX, hitY, hitZ, p_180614_9_, 0, playerIn);

            if (!worldIn.setBlockState(pos, iblockstate1, 11))
            {
                return EnumActionResult.FAIL;
            }
            else
            {
                iblockstate1 = worldIn.getBlockState(pos);

                if (iblockstate1.getBlock() == this.block)
                {
                    ItemBlock.setTileEntityNBT(worldIn, playerIn, pos, stack);
                    iblockstate1.getBlock().onBlockPlacedBy(worldIn, pos, iblockstate1, playerIn, stack);
                }

                SoundType soundtype = this.block.getStepSound();
                worldIn.func_184133_a(playerIn, pos, soundtype.func_185841_e(), SoundCategory.BLOCKS, (soundtype.func_185843_a() + 1.0F) / 2.0F, soundtype.func_185847_b() * 0.8F);
                --stack.stackSize;
                return EnumActionResult.SUCCESS;
            }
        }
        else
        {
            return EnumActionResult.FAIL;
        }
    }
}
