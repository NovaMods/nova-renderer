package net.minecraft.item;

import java.util.List;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.WorldProviderEnd;
import net.minecraft.world.end.DragonFightManager;

public class ItemEndCrystal extends Item
{
    public ItemEndCrystal()
    {
        this.setUnlocalizedName("end_crystal");
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos);

        if (iblockstate.getBlock() != Blocks.obsidian && iblockstate.getBlock() != Blocks.bedrock)
        {
            return EnumActionResult.FAIL;
        }
        else
        {
            BlockPos blockpos = pos.up();

            if (!playerIn.canPlayerEdit(blockpos, hitX, stack))
            {
                return EnumActionResult.FAIL;
            }
            else
            {
                BlockPos blockpos1 = blockpos.up();
                boolean flag = !worldIn.isAirBlock(blockpos) && !worldIn.getBlockState(blockpos).getBlock().isReplaceable(worldIn, blockpos);
                flag = flag | (!worldIn.isAirBlock(blockpos1) && !worldIn.getBlockState(blockpos1).getBlock().isReplaceable(worldIn, blockpos1));

                if (flag)
                {
                    return EnumActionResult.FAIL;
                }
                else
                {
                    double d0 = (double)blockpos.getX();
                    double d1 = (double)blockpos.getY();
                    double d2 = (double)blockpos.getZ();
                    List<Entity> list = worldIn.getEntitiesWithinAABBExcludingEntity((Entity)null, new AxisAlignedBB(d0, d1, d2, d0 + 1.0D, d1 + 2.0D, d2 + 1.0D));

                    if (!list.isEmpty())
                    {
                        return EnumActionResult.FAIL;
                    }
                    else
                    {
                        if (!worldIn.isRemote)
                        {
                            EntityEnderCrystal entityendercrystal = new EntityEnderCrystal(worldIn, (double)((float)pos.getX() + 0.5F), (double)(pos.getY() + 1), (double)((float)pos.getZ() + 0.5F));
                            entityendercrystal.setShowBottom(false);
                            worldIn.spawnEntityInWorld(entityendercrystal);

                            if (worldIn.provider instanceof WorldProviderEnd)
                            {
                                DragonFightManager dragonfightmanager = ((WorldProviderEnd)worldIn.provider).getDragonFightManager();
                                dragonfightmanager.func_186106_e();
                            }
                        }

                        --stack.stackSize;
                        return EnumActionResult.SUCCESS;
                    }
                }
            }
        }
    }

    public boolean hasEffect(ItemStack stack)
    {
        return true;
    }
}
