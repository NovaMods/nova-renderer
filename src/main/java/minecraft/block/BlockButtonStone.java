package net.minecraft.block;

import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class BlockButtonStone extends BlockButton
{
    protected BlockButtonStone()
    {
        super(false);
    }

    protected void func_185615_a(EntityPlayer p_185615_1_, World p_185615_2_, BlockPos p_185615_3_)
    {
        p_185615_2_.func_184133_a(p_185615_1_, p_185615_3_, SoundEvents.block_stone_button_click_on, SoundCategory.BLOCKS, 0.3F, 0.6F);
    }

    protected void func_185617_b(World p_185617_1_, BlockPos p_185617_2_)
    {
        p_185617_1_.func_184133_a((EntityPlayer)null, p_185617_2_, SoundEvents.block_stone_button_click_off, SoundCategory.BLOCKS, 0.3F, 0.5F);
    }
}
