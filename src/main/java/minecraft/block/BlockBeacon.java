package net.minecraft.block;

import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityBeacon;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.HttpUtil;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;
import net.minecraft.world.chunk.Chunk;

public class BlockBeacon extends BlockContainer
{
    public BlockBeacon()
    {
        super(Material.glass, MapColor.diamondColor);
        this.setHardness(3.0F);
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    /**
     * Returns a new instance of a block's tile entity class. Called on placing the block.
     */
    public TileEntity createNewTileEntity(World worldIn, int meta)
    {
        return new TileEntityBeacon();
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (worldIn.isRemote)
        {
            return true;
        }
        else
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityBeacon)
            {
                playerIn.displayGUIChest((TileEntityBeacon)tileentity);
                playerIn.triggerAchievement(StatList.field_188082_P);
            }

            return true;
        }
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    /**
     * The type of render function called. 3 for standard block models, 2 for TESR's, 1 for liquids, -1 is no render
     */
    public EnumBlockRenderType getRenderType(IBlockState state)
    {
        return EnumBlockRenderType.MODEL;
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
        super.onBlockPlacedBy(worldIn, pos, state, placer, stack);

        if (stack.hasDisplayName())
        {
            TileEntity tileentity = worldIn.getTileEntity(pos);

            if (tileentity instanceof TileEntityBeacon)
            {
                ((TileEntityBeacon)tileentity).setName(stack.getDisplayName());
            }
        }
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        TileEntity tileentity = worldIn.getTileEntity(pos);

        if (tileentity instanceof TileEntityBeacon)
        {
            ((TileEntityBeacon)tileentity).updateBeacon();
            worldIn.addBlockEvent(pos, this, 1, 0);
        }
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    public static void updateColorAsync(final World worldIn, final BlockPos glassPos)
    {
        HttpUtil.field_180193_a.submit(new Runnable()
        {
            public void run()
            {
                Chunk chunk = worldIn.getChunkFromBlockCoords(glassPos);

                for (int i = glassPos.getY() - 1; i >= 0; --i)
                {
                    final BlockPos blockpos = new BlockPos(glassPos.getX(), i, glassPos.getZ());

                    if (!chunk.canSeeSky(blockpos))
                    {
                        break;
                    }

                    IBlockState iblockstate = worldIn.getBlockState(blockpos);

                    if (iblockstate.getBlock() == Blocks.beacon)
                    {
                        ((WorldServer)worldIn).addScheduledTask(new Runnable()
                        {
                            public void run()
                            {
                                TileEntity tileentity = worldIn.getTileEntity(blockpos);

                                if (tileentity instanceof TileEntityBeacon)
                                {
                                    ((TileEntityBeacon)tileentity).updateBeacon();
                                    worldIn.addBlockEvent(blockpos, Blocks.beacon, 1, 0);
                                }
                            }
                        });
                    }
                }
            }
        });
    }
}
