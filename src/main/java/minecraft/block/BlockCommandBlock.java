package net.minecraft.block;

import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyDirection;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.tileentity.CommandBlockBaseLogic;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityCommandBlock;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.StringUtils;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class BlockCommandBlock extends BlockContainer
{
    public static final PropertyDirection FACING = BlockDirectional.FACING;
    public static final PropertyBool CONDITIONAL = PropertyBool.create("conditional");

    public BlockCommandBlock(MapColor p_i46688_1_)
    {
        super(Material.iron, p_i46688_1_);
        this.setDefaultState(this.blockState.getBaseState().withProperty(FACING, EnumFacing.NORTH).withProperty(CONDITIONAL, Boolean.valueOf(false)));
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        TileEntityCommandBlock tileentitycommandblock = new TileEntityCommandBlock();
        tileentitycommandblock.setAuto(this == Blocks.chain_command_block);
        return tileentitycommandblock;
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (!worldIn.isRemote)
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityCommandBlock)
            {
                TileEntityCommandBlock tileentitycommandblock = (TileEntityCommandBlock)tileentity;
                boolean flag = worldIn.isBlockPowered(pos);
                boolean flag1 = tileentitycommandblock.isPowered();
                boolean flag2 = tileentitycommandblock.isAuto();

                if (flag && !flag1)
                {
                    tileentitycommandblock.setPowered(true);

                    if (tileentitycommandblock.func_184251_i() != TileEntityCommandBlock.Mode.SEQUENCE && !flag2)
                    {
                        boolean flag3 = !tileentitycommandblock.func_184258_j() || this.func_185562_e(worldIn, pos, state);
                        tileentitycommandblock.setConditionMet(flag3);
                        worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn));

                        if (flag3)
                        {
                            this.func_185563_c(worldIn, pos);
                        }
                    }
                }
                else if (!flag && flag1)
                {
                    tileentitycommandblock.setPowered(false);
                }
            }
        }
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (!worldIn.isRemote)
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityCommandBlock)
            {
                TileEntityCommandBlock tileentitycommandblock = (TileEntityCommandBlock)tileentity;
                CommandBlockBaseLogic commandblockbaselogic = tileentitycommandblock.getCommandBlockLogic();
                boolean flag = !StringUtils.isNullOrEmpty(commandblockbaselogic.getCommand());
                TileEntityCommandBlock.Mode tileentitycommandblock$mode = tileentitycommandblock.func_184251_i();
                boolean flag1 = !tileentitycommandblock.func_184258_j() || this.func_185562_e(worldIn, pos, state);
                boolean flag2 = tileentitycommandblock.isConditionMet();
                boolean flag3 = false;

                if (tileentitycommandblock$mode != TileEntityCommandBlock.Mode.SEQUENCE && flag2 && flag)
                {
                    commandblockbaselogic.trigger(worldIn);
                    flag3 = true;
                }

                if (tileentitycommandblock.isPowered() || tileentitycommandblock.isAuto())
                {
                    if (tileentitycommandblock$mode == TileEntityCommandBlock.Mode.SEQUENCE && flag1 && flag)
                    {
                        commandblockbaselogic.trigger(worldIn);
                        flag3 = true;
                    }

                    if (tileentitycommandblock$mode == TileEntityCommandBlock.Mode.AUTO)
                    {
                        worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn));

                        if (flag1)
                        {
                            this.func_185563_c(worldIn, pos);
                        }
                    }
                }

                if (!flag3)
                {
                    commandblockbaselogic.func_184167_a(0);
                }

                tileentitycommandblock.setConditionMet(flag1);
                worldIn.updateComparatorOutputLevel(pos, this);
            }
        }
    }

    public boolean func_185562_e(World p_185562_1_, BlockPos p_185562_2_, IBlockState p_185562_3_)
    {
        EnumFacing enumfacing = (EnumFacing)p_185562_3_.getValue(FACING);
        TileEntity tileentity = p_185562_1_.getTileEntity(p_185562_2_.offset(enumfacing.getOpposite()));
        return tileentity instanceof TileEntityCommandBlock && ((TileEntityCommandBlock)tileentity).getCommandBlockLogic().getSuccessCount() > 0;
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return 1;
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        TileEntity tileentity = worldIn.getTileEntity(pos);

        if (tileentity instanceof TileEntityCommandBlock)
        {
            if (!playerIn.capabilities.isCreativeMode)
            {
                return false;
            }
            else
            {
                playerIn.func_184824_a((TileEntityCommandBlock)tileentity);
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    public boolean hasComparatorInputOverride(IBlockState state)
    {
        return true;
    }

    public int getComparatorInputOverride(IBlockState worldIn, World pos, BlockPos p_180641_3_)
    {
        TileEntity tileentity = pos.getTileEntity(p_180641_3_);
        return tileentity instanceof TileEntityCommandBlock ? ((TileEntityCommandBlock)tileentity).getCommandBlockLogic().getSuccessCount() : 0;
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
        TileEntity tileentity = worldIn.getTileEntity(pos);

        if (tileentity instanceof TileEntityCommandBlock)
        {
            TileEntityCommandBlock tileentitycommandblock = (TileEntityCommandBlock)tileentity;
            CommandBlockBaseLogic commandblockbaselogic = tileentitycommandblock.getCommandBlockLogic();

            if (stack.hasDisplayName())
            {
                commandblockbaselogic.setName(stack.getDisplayName());
            }

            if (!worldIn.isRemote)
            {
                NBTTagCompound nbttagcompound = stack.getTagCompound();

                if (nbttagcompound == null || !nbttagcompound.hasKey("BlockEntityTag", 10))
                {
                    commandblockbaselogic.setTrackOutput(worldIn.getGameRules().getBoolean("sendCommandFeedback"));
                    tileentitycommandblock.setAuto(this == Blocks.chain_command_block);
                }

                if (tileentitycommandblock.func_184251_i() == TileEntityCommandBlock.Mode.SEQUENCE)
                {
                    boolean flag = worldIn.isBlockPowered(pos);
                    tileentitycommandblock.setPowered(flag);
                }
            }
        }
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 0;
    }

    /**
     * The type of render function called. 3 for standard block models, 2 for TESR's, 1 for liquids, -1 is no render
     */
    public EnumBlockRenderType getRenderType(IBlockState state)
    {
        return EnumBlockRenderType.MODEL;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(FACING, EnumFacing.getFront(meta & 7)).withProperty(CONDITIONAL, Boolean.valueOf((meta & 8) != 0));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((EnumFacing)state.getValue(FACING)).getIndex() | (((Boolean)state.getValue(CONDITIONAL)).booleanValue() ? 8 : 0);
    }

    /**
     * Returns the blockstate with the given rotation from the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withRotation(IBlockState state, Rotation rot)
    {
        return state.withProperty(FACING, rot.func_185831_a((EnumFacing)state.getValue(FACING)));
    }

    /**
     * Returns the blockstate with the given mirror of the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
    {
        return state.withRotation(mirrorIn.func_185800_a((EnumFacing)state.getValue(FACING)));
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {FACING, CONDITIONAL});
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        return this.getDefaultState().withProperty(FACING, BlockPistonBase.func_185647_a(pos, placer)).withProperty(CONDITIONAL, Boolean.valueOf(false));
    }

    public void func_185563_c(World p_185563_1_, BlockPos p_185563_2_)
    {
        IBlockState iblockstate = p_185563_1_.getBlockState(p_185563_2_);

        if (iblockstate.getBlock() == Blocks.command_block || iblockstate.getBlock() == Blocks.repeating_command_block)
        {
            BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos(p_185563_2_);
            blockpos$mutableblockpos.func_185337_c((EnumFacing)iblockstate.getValue(FACING));

            for (TileEntity tileentity = p_185563_1_.getTileEntity(blockpos$mutableblockpos); tileentity instanceof TileEntityCommandBlock; tileentity = p_185563_1_.getTileEntity(blockpos$mutableblockpos))
            {
                TileEntityCommandBlock tileentitycommandblock = (TileEntityCommandBlock)tileentity;

                if (tileentitycommandblock.func_184251_i() != TileEntityCommandBlock.Mode.SEQUENCE)
                {
                    break;
                }

                IBlockState iblockstate1 = p_185563_1_.getBlockState(blockpos$mutableblockpos);
                Block block = iblockstate1.getBlock();

                if (block != Blocks.chain_command_block || p_185563_1_.func_184145_b(blockpos$mutableblockpos, block))
                {
                    break;
                }

                p_185563_1_.scheduleUpdate(new BlockPos(blockpos$mutableblockpos), block, this.tickRate(p_185563_1_));
                blockpos$mutableblockpos.func_185337_c((EnumFacing)iblockstate1.getValue(FACING));
            }
        }
    }
}
