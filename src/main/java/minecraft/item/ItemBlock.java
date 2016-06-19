package net.minecraft.item;

import java.util.List;
import net.minecraft.block.Block;
import net.minecraft.block.SoundType;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.server.MinecraftServer;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class ItemBlock extends Item
{
    protected final Block block;

    public ItemBlock(Block block)
    {
        this.block = block;
    }

    /**
     * Sets the unlocalized name of this item to the string passed as the parameter, prefixed by "item."
     */
    public ItemBlock setUnlocalizedName(String unlocalizedName)
    {
        super.setUnlocalizedName(unlocalizedName);
        return this;
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        IBlockState iblockstate = worldIn.getBlockState(pos);
        Block block = iblockstate.getBlock();

        if (!block.isReplaceable(worldIn, pos))
        {
            pos = pos.offset(hitX);
        }

        if (stack.stackSize != 0 && playerIn.canPlayerEdit(pos, hitX, stack) && worldIn.canBlockBePlaced(this.block, pos, false, hitX, (Entity)null, stack))
        {
            int i = this.getMetadata(stack.getMetadata());
            IBlockState iblockstate1 = this.block.onBlockPlaced(worldIn, pos, hitX, hitY, hitZ, p_180614_9_, i, playerIn);

            if (worldIn.setBlockState(pos, iblockstate1, 11))
            {
                iblockstate1 = worldIn.getBlockState(pos);

                if (iblockstate1.getBlock() == this.block)
                {
                    setTileEntityNBT(worldIn, playerIn, pos, stack);
                    this.block.onBlockPlacedBy(worldIn, pos, iblockstate1, playerIn, stack);
                }

                SoundType soundtype = this.block.getStepSound();
                worldIn.func_184133_a(playerIn, pos, soundtype.func_185841_e(), SoundCategory.BLOCKS, (soundtype.func_185843_a() + 1.0F) / 2.0F, soundtype.func_185847_b() * 0.8F);
                --stack.stackSize;
            }

            return EnumActionResult.SUCCESS;
        }
        else
        {
            return EnumActionResult.FAIL;
        }
    }

    public static boolean setTileEntityNBT(World worldIn, EntityPlayer pos, BlockPos stack, ItemStack p_179224_3_)
    {
        MinecraftServer minecraftserver = worldIn.getMinecraftServer();

        if (minecraftserver == null)
        {
            return false;
        }
        else
        {
            if (p_179224_3_.hasTagCompound() && p_179224_3_.getTagCompound().hasKey("BlockEntityTag", 10))
            {
                TileEntity tileentity = worldIn.getTileEntity(stack);

                if (tileentity != null)
                {
                    if (!worldIn.isRemote && tileentity.func_183000_F() && (pos == null || !minecraftserver.getPlayerList().canSendCommands(pos.getGameProfile())))
                    {
                        return false;
                    }

                    NBTTagCompound nbttagcompound = new NBTTagCompound();
                    NBTTagCompound nbttagcompound1 = (NBTTagCompound)nbttagcompound.copy();
                    tileentity.writeToNBT(nbttagcompound);
                    NBTTagCompound nbttagcompound2 = (NBTTagCompound)p_179224_3_.getTagCompound().getTag("BlockEntityTag");
                    nbttagcompound.merge(nbttagcompound2);
                    nbttagcompound.setInteger("x", stack.getX());
                    nbttagcompound.setInteger("y", stack.getY());
                    nbttagcompound.setInteger("z", stack.getZ());

                    if (!nbttagcompound.equals(nbttagcompound1))
                    {
                        tileentity.readFromNBT(nbttagcompound);
                        tileentity.markDirty();
                        return true;
                    }
                }
            }

            return false;
        }
    }

    public boolean canPlaceBlockOnSide(World worldIn, BlockPos pos, EnumFacing side, EntityPlayer player, ItemStack stack)
    {
        Block block = worldIn.getBlockState(pos).getBlock();

        if (block == Blocks.snow_layer)
        {
            side = EnumFacing.UP;
        }
        else if (!block.isReplaceable(worldIn, pos))
        {
            pos = pos.offset(side);
        }

        return worldIn.canBlockBePlaced(this.block, pos, false, side, (Entity)null, stack);
    }

    /**
     * Returns the unlocalized name of this item. This version accepts an ItemStack so different stacks can have
     * different names based on their damage or NBT.
     */
    public String getUnlocalizedName(ItemStack stack)
    {
        return this.block.getUnlocalizedName();
    }

    /**
     * Returns the unlocalized name of this item.
     */
    public String getUnlocalizedName()
    {
        return this.block.getUnlocalizedName();
    }

    /**
     * gets the CreativeTab this item is displayed on
     */
    public CreativeTabs getCreativeTab()
    {
        return this.block.getCreativeTabToDisplayOn();
    }

    /**
     * returns a list of items with the same ID, but different meta (eg: dye returns 16 items)
     */
    public void getSubItems(Item itemIn, CreativeTabs tab, List<ItemStack> subItems)
    {
        this.block.getSubBlocks(itemIn, tab, subItems);
    }

    public Block getBlock()
    {
        return this.block;
    }
}
