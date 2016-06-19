package net.minecraft.item;

import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityHanging;
import net.minecraft.entity.item.EntityItemFrame;
import net.minecraft.entity.item.EntityPainting;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemHangingEntity extends Item
{
    private final Class <? extends EntityHanging > hangingEntityClass;

    public ItemHangingEntity(Class <? extends EntityHanging > entityClass)
    {
        this.hangingEntityClass = entityClass;
        this.setCreativeTab(CreativeTabs.tabDecorations);
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        BlockPos blockpos = pos.offset(hitX);

        if (hitX != EnumFacing.DOWN && hitX != EnumFacing.UP && playerIn.canPlayerEdit(blockpos, hitX, stack))
        {
            EntityHanging entityhanging = this.createEntity(worldIn, blockpos, hitX);

            if (entityhanging != null && entityhanging.onValidSurface())
            {
                if (!worldIn.isRemote)
                {
                    entityhanging.func_184523_o();
                    worldIn.spawnEntityInWorld(entityhanging);
                }

                --stack.stackSize;
            }

            return EnumActionResult.SUCCESS;
        }
        else
        {
            return EnumActionResult.FAIL;
        }
    }

    private EntityHanging createEntity(World worldIn, BlockPos pos, EnumFacing clickedSide)
    {
        return (EntityHanging)(this.hangingEntityClass == EntityPainting.class ? new EntityPainting(worldIn, pos, clickedSide) : (this.hangingEntityClass == EntityItemFrame.class ? new EntityItemFrame(worldIn, pos, clickedSide) : null));
    }
}
