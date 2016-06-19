package net.minecraft.item;

import net.minecraft.block.Block;
import net.minecraft.block.BlockSlab;
import net.minecraft.block.SoundType;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemSlab extends ItemBlock
{
    private final BlockSlab singleSlab;
    private final BlockSlab doubleSlab;

    public ItemSlab(Block block, BlockSlab singleSlab, BlockSlab doubleSlab)
    {
        super(block);
        this.singleSlab = singleSlab;
        this.doubleSlab = doubleSlab;
        this.setMaxDamage(0);
        this.setHasSubtypes(true);
    }

    /**
     * Converts the given ItemStack damage value into a metadata value to be placed in the world when this Item is
     * placed as a Block (mostly used with ItemBlocks).
     */
    public int getMetadata(int damage)
    {
        return damage;
    }

    /**
     * Returns the unlocalized name of this item. This version accepts an ItemStack so different stacks can have
     * different names based on their damage or NBT.
     */
    public String getUnlocalizedName(ItemStack stack)
    {
        return this.singleSlab.getUnlocalizedName(stack.getMetadata());
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        if (stack.stackSize != 0 && playerIn.canPlayerEdit(pos.offset(hitX), hitX, stack))
        {
            Comparable<?> comparable = this.singleSlab.func_185674_a(stack);
            IBlockState iblockstate = worldIn.getBlockState(pos);

            if (iblockstate.getBlock() == this.singleSlab)
            {
                IProperty<?> iproperty = this.singleSlab.getVariantProperty();
                Comparable<?> comparable1 = iblockstate.getValue(iproperty);
                BlockSlab.EnumBlockHalf blockslab$enumblockhalf = (BlockSlab.EnumBlockHalf)iblockstate.getValue(BlockSlab.HALF);

                if ((hitX == EnumFacing.UP && blockslab$enumblockhalf == BlockSlab.EnumBlockHalf.BOTTOM || hitX == EnumFacing.DOWN && blockslab$enumblockhalf == BlockSlab.EnumBlockHalf.TOP) && comparable1 == comparable)
                {
                    IBlockState iblockstate1 = this.func_185055_a(iproperty, comparable1);
                    AxisAlignedBB axisalignedbb = iblockstate1.getSelectedBoundingBox(worldIn, pos);

                    if (axisalignedbb != Block.NULL_AABB && worldIn.checkNoEntityCollision(axisalignedbb.offset(pos)) && worldIn.setBlockState(pos, iblockstate1, 11))
                    {
                        SoundType soundtype = this.doubleSlab.getStepSound();
                        worldIn.func_184133_a(playerIn, pos, soundtype.func_185841_e(), SoundCategory.BLOCKS, (soundtype.func_185843_a() + 1.0F) / 2.0F, soundtype.func_185847_b() * 0.8F);
                        --stack.stackSize;
                    }

                    return EnumActionResult.SUCCESS;
                }
            }

            return this.tryPlace(playerIn, stack, worldIn, pos.offset(hitX), comparable) ? EnumActionResult.SUCCESS : super.onItemUse(stack, playerIn, worldIn, pos, side, hitX, hitY, hitZ, p_180614_9_);
        }
        else
        {
            return EnumActionResult.FAIL;
        }
    }

    public boolean canPlaceBlockOnSide(World worldIn, BlockPos pos, EnumFacing side, EntityPlayer player, ItemStack stack)
    {
        BlockPos blockpos = pos;
        IProperty<?> iproperty = this.singleSlab.getVariantProperty();
        Comparable<?> comparable = this.singleSlab.func_185674_a(stack);
        IBlockState iblockstate = worldIn.getBlockState(pos);

        if (iblockstate.getBlock() == this.singleSlab)
        {
            boolean flag = iblockstate.getValue(BlockSlab.HALF) == BlockSlab.EnumBlockHalf.TOP;

            if ((side == EnumFacing.UP && !flag || side == EnumFacing.DOWN && flag) && comparable == iblockstate.getValue(iproperty))
            {
                return true;
            }
        }

        pos = pos.offset(side);
        IBlockState iblockstate1 = worldIn.getBlockState(pos);
        return iblockstate1.getBlock() == this.singleSlab && comparable == iblockstate1.getValue(iproperty) ? true : super.canPlaceBlockOnSide(worldIn, blockpos, side, player, stack);
    }

    private boolean tryPlace(EntityPlayer stack, ItemStack worldIn, World pos, BlockPos variantInStack, Object p_180615_5_)
    {
        IBlockState iblockstate = pos.getBlockState(variantInStack);

        if (iblockstate.getBlock() == this.singleSlab)
        {
            Comparable<?> comparable = iblockstate.getValue(this.singleSlab.getVariantProperty());

            if (comparable == p_180615_5_)
            {
                IBlockState iblockstate1 = this.func_185055_a(this.singleSlab.getVariantProperty(), comparable);
                AxisAlignedBB axisalignedbb = iblockstate1.getSelectedBoundingBox(pos, variantInStack);

                if (axisalignedbb != Block.NULL_AABB && pos.checkNoEntityCollision(axisalignedbb.offset(variantInStack)) && pos.setBlockState(variantInStack, iblockstate1, 11))
                {
                    SoundType soundtype = this.doubleSlab.getStepSound();
                    pos.func_184133_a(stack, variantInStack, soundtype.func_185841_e(), SoundCategory.BLOCKS, (soundtype.func_185843_a() + 1.0F) / 2.0F, soundtype.func_185847_b() * 0.8F);
                    --worldIn.stackSize;
                }

                return true;
            }
        }

        return false;
    }

    protected <T extends Comparable<T>> IBlockState func_185055_a(IProperty<T> p_185055_1_, Comparable<?> p_185055_2_)
    {
        return this.doubleSlab.getDefaultState().withProperty((IProperty)p_185055_1_, p_185055_2_);
    }
}
