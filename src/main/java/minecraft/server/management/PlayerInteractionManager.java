package net.minecraft.server.management;

import net.minecraft.block.Block;
import net.minecraft.block.BlockChest;
import net.minecraft.block.BlockCommandBlock;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.inventory.IInventory;
import net.minecraft.item.ItemBlock;
import net.minecraft.item.ItemStack;
import net.minecraft.item.ItemSword;
import net.minecraft.network.play.server.SPacketBlockChange;
import net.minecraft.network.play.server.SPacketPlayerListItem;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityChest;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.ILockableContainer;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;
import net.minecraft.world.WorldSettings;

public class PlayerInteractionManager
{
    /** The world object that this object is connected to. */
    public World theWorld;

    /** The EntityPlayerMP object that this object is connected to. */
    public EntityPlayerMP thisPlayerMP;
    private WorldSettings.GameType gameType = WorldSettings.GameType.NOT_SET;

    /** True if the player is destroying a block */
    private boolean isDestroyingBlock;
    private int initialDamage;
    private BlockPos field_180240_f = BlockPos.ORIGIN;
    private int curblockDamage;

    /**
     * Set to true when the "finished destroying block" packet is received but the block wasn't fully damaged yet. The
     * block will not be destroyed while this is false.
     */
    private boolean receivedFinishDiggingPacket;
    private BlockPos field_180241_i = BlockPos.ORIGIN;
    private int initialBlockDamage;
    private int durabilityRemainingOnBlock = -1;

    public PlayerInteractionManager(World worldIn)
    {
        this.theWorld = worldIn;
    }

    public void setGameType(WorldSettings.GameType type)
    {
        this.gameType = type;
        type.configurePlayerCapabilities(this.thisPlayerMP.capabilities);
        this.thisPlayerMP.sendPlayerAbilities();
        this.thisPlayerMP.mcServer.getPlayerList().sendPacketToAllPlayers(new SPacketPlayerListItem(SPacketPlayerListItem.Action.UPDATE_GAME_MODE, new EntityPlayerMP[] {this.thisPlayerMP}));
        this.theWorld.updateAllPlayersSleepingFlag();
    }

    public WorldSettings.GameType getGameType()
    {
        return this.gameType;
    }

    public boolean survivalOrAdventure()
    {
        return this.gameType.isSurvivalOrAdventure();
    }

    /**
     * Get if we are in creative game mode.
     */
    public boolean isCreative()
    {
        return this.gameType.isCreative();
    }

    /**
     * if the gameType is currently NOT_SET then change it to par1
     */
    public void initializeGameType(WorldSettings.GameType type)
    {
        if (this.gameType == WorldSettings.GameType.NOT_SET)
        {
            this.gameType = type;
        }

        this.setGameType(this.gameType);
    }

    public void updateBlockRemoving()
    {
        ++this.curblockDamage;

        if (this.receivedFinishDiggingPacket)
        {
            int i = this.curblockDamage - this.initialBlockDamage;
            IBlockState iblockstate = this.theWorld.getBlockState(this.field_180241_i);
            Block block = iblockstate.getBlock();

            if (iblockstate.getMaterial() == Material.air)
            {
                this.receivedFinishDiggingPacket = false;
            }
            else
            {
                float f = iblockstate.getPlayerRelativeBlockHardness(this.thisPlayerMP, this.thisPlayerMP.worldObj, this.field_180241_i) * (float)(i + 1);
                int j = (int)(f * 10.0F);

                if (j != this.durabilityRemainingOnBlock)
                {
                    this.theWorld.sendBlockBreakProgress(this.thisPlayerMP.getEntityId(), this.field_180241_i, j);
                    this.durabilityRemainingOnBlock = j;
                }

                if (f >= 1.0F)
                {
                    this.receivedFinishDiggingPacket = false;
                    this.tryHarvestBlock(this.field_180241_i);
                }
            }
        }
        else if (this.isDestroyingBlock)
        {
            IBlockState iblockstate1 = this.theWorld.getBlockState(this.field_180240_f);
            Block block1 = iblockstate1.getBlock();

            if (iblockstate1.getMaterial() == Material.air)
            {
                this.theWorld.sendBlockBreakProgress(this.thisPlayerMP.getEntityId(), this.field_180240_f, -1);
                this.durabilityRemainingOnBlock = -1;
                this.isDestroyingBlock = false;
            }
            else
            {
                int k = this.curblockDamage - this.initialDamage;
                float f1 = iblockstate1.getPlayerRelativeBlockHardness(this.thisPlayerMP, this.thisPlayerMP.worldObj, this.field_180241_i) * (float)(k + 1);
                int l = (int)(f1 * 10.0F);

                if (l != this.durabilityRemainingOnBlock)
                {
                    this.theWorld.sendBlockBreakProgress(this.thisPlayerMP.getEntityId(), this.field_180240_f, l);
                    this.durabilityRemainingOnBlock = l;
                }
            }
        }
    }

    /**
     * If not creative, it calls sendBlockBreakProgress until the block is broken first. tryHarvestBlock can also be the
     * result of this call.
     */
    public void onBlockClicked(BlockPos pos, EnumFacing side)
    {
        if (this.isCreative())
        {
            if (!this.theWorld.extinguishFire((EntityPlayer)null, pos, side))
            {
                this.tryHarvestBlock(pos);
            }
        }
        else
        {
            IBlockState iblockstate = this.theWorld.getBlockState(pos);
            Block block = iblockstate.getBlock();

            if (this.gameType.isAdventure())
            {
                if (this.gameType == WorldSettings.GameType.SPECTATOR)
                {
                    return;
                }

                if (!this.thisPlayerMP.isAllowEdit())
                {
                    ItemStack itemstack = this.thisPlayerMP.getHeldItemMainhand();

                    if (itemstack == null)
                    {
                        return;
                    }

                    if (!itemstack.canDestroy(block))
                    {
                        return;
                    }
                }
            }

            this.theWorld.extinguishFire((EntityPlayer)null, pos, side);
            this.initialDamage = this.curblockDamage;
            float f = 1.0F;

            if (iblockstate.getMaterial() != Material.air)
            {
                block.onBlockClicked(this.theWorld, pos, this.thisPlayerMP);
                f = iblockstate.getPlayerRelativeBlockHardness(this.thisPlayerMP, this.thisPlayerMP.worldObj, pos);
            }

            if (iblockstate.getMaterial() != Material.air && f >= 1.0F)
            {
                this.tryHarvestBlock(pos);
            }
            else
            {
                this.isDestroyingBlock = true;
                this.field_180240_f = pos;
                int i = (int)(f * 10.0F);
                this.theWorld.sendBlockBreakProgress(this.thisPlayerMP.getEntityId(), pos, i);
                this.durabilityRemainingOnBlock = i;
            }
        }
    }

    public void blockRemoving(BlockPos pos)
    {
        if (pos.equals(this.field_180240_f))
        {
            int i = this.curblockDamage - this.initialDamage;
            IBlockState iblockstate = this.theWorld.getBlockState(pos);

            if (iblockstate.getMaterial() != Material.air)
            {
                float f = iblockstate.getPlayerRelativeBlockHardness(this.thisPlayerMP, this.thisPlayerMP.worldObj, pos) * (float)(i + 1);

                if (f >= 0.7F)
                {
                    this.isDestroyingBlock = false;
                    this.theWorld.sendBlockBreakProgress(this.thisPlayerMP.getEntityId(), pos, -1);
                    this.tryHarvestBlock(pos);
                }
                else if (!this.receivedFinishDiggingPacket)
                {
                    this.isDestroyingBlock = false;
                    this.receivedFinishDiggingPacket = true;
                    this.field_180241_i = pos;
                    this.initialBlockDamage = this.initialDamage;
                }
            }
        }
    }

    /**
     * Stops the block breaking process
     */
    public void cancelDestroyingBlock()
    {
        this.isDestroyingBlock = false;
        this.theWorld.sendBlockBreakProgress(this.thisPlayerMP.getEntityId(), this.field_180240_f, -1);
    }

    /**
     * Removes a block and triggers the appropriate events
     */
    private boolean removeBlock(BlockPos pos)
    {
        IBlockState iblockstate = this.theWorld.getBlockState(pos);
        iblockstate.getBlock().onBlockHarvested(this.theWorld, pos, iblockstate, this.thisPlayerMP);
        boolean flag = this.theWorld.setBlockToAir(pos);

        if (flag)
        {
            iblockstate.getBlock().onBlockDestroyedByPlayer(this.theWorld, pos, iblockstate);
        }

        return flag;
    }

    /**
     * Attempts to harvest a block
     */
    public boolean tryHarvestBlock(BlockPos pos)
    {
        if (this.gameType.isCreative() && this.thisPlayerMP.getHeldItemMainhand() != null && this.thisPlayerMP.getHeldItemMainhand().getItem() instanceof ItemSword)
        {
            return false;
        }
        else
        {
            IBlockState iblockstate = this.theWorld.getBlockState(pos);
            TileEntity tileentity = this.theWorld.getTileEntity(pos);

            if (iblockstate.getBlock() instanceof BlockCommandBlock && !this.thisPlayerMP.canCommandSenderUseCommand(2, ""))
            {
                this.theWorld.notifyBlockUpdate(pos, iblockstate, iblockstate, 3);
                return false;
            }
            else
            {
                if (this.gameType.isAdventure())
                {
                    if (this.gameType == WorldSettings.GameType.SPECTATOR)
                    {
                        return false;
                    }

                    if (!this.thisPlayerMP.isAllowEdit())
                    {
                        ItemStack itemstack = this.thisPlayerMP.getHeldItemMainhand();

                        if (itemstack == null)
                        {
                            return false;
                        }

                        if (!itemstack.canDestroy(iblockstate.getBlock()))
                        {
                            return false;
                        }
                    }
                }

                this.theWorld.playAuxSFXAtEntity(this.thisPlayerMP, 2001, pos, Block.getStateId(iblockstate));
                boolean flag1 = this.removeBlock(pos);

                if (this.isCreative())
                {
                    this.thisPlayerMP.playerNetServerHandler.sendPacket(new SPacketBlockChange(this.theWorld, pos));
                }
                else
                {
                    ItemStack itemstack1 = this.thisPlayerMP.getHeldItemMainhand();
                    ItemStack itemstack2 = itemstack1 == null ? null : itemstack1.copy();
                    boolean flag = this.thisPlayerMP.func_184823_b(iblockstate);

                    if (itemstack1 != null)
                    {
                        itemstack1.onBlockDestroyed(this.theWorld, iblockstate, pos, this.thisPlayerMP);

                        if (itemstack1.stackSize == 0)
                        {
                            this.thisPlayerMP.setHeldItem(EnumHand.MAIN_HAND, (ItemStack)null);
                        }
                    }

                    if (flag1 && flag)
                    {
                        iblockstate.getBlock().harvestBlock(this.theWorld, this.thisPlayerMP, pos, iblockstate, tileentity, itemstack2);
                    }
                }

                return flag1;
            }
        }
    }

    public EnumActionResult func_187250_a(EntityPlayer p_187250_1_, World p_187250_2_, ItemStack p_187250_3_, EnumHand p_187250_4_)
    {
        if (this.gameType == WorldSettings.GameType.SPECTATOR)
        {
            return EnumActionResult.PASS;
        }
        else if (p_187250_1_.func_184811_cZ().hasCooldown(p_187250_3_.getItem()))
        {
            return EnumActionResult.PASS;
        }
        else
        {
            int i = p_187250_3_.stackSize;
            int j = p_187250_3_.getMetadata();
            ActionResult<ItemStack> actionresult = p_187250_3_.useItemRightClick(p_187250_2_, p_187250_1_, p_187250_4_);
            ItemStack itemstack = (ItemStack)actionresult.func_188398_b();

            if (itemstack == p_187250_3_ && itemstack.stackSize == i && itemstack.getMaxItemUseDuration() <= 0 && itemstack.getMetadata() == j)
            {
                return actionresult.func_188397_a();
            }
            else
            {
                p_187250_1_.setHeldItem(p_187250_4_, itemstack);

                if (this.isCreative())
                {
                    itemstack.stackSize = i;

                    if (itemstack.isItemStackDamageable())
                    {
                        itemstack.setItemDamage(j);
                    }
                }

                if (itemstack.stackSize == 0)
                {
                    p_187250_1_.setHeldItem(p_187250_4_, (ItemStack)null);
                }

                if (!p_187250_1_.func_184587_cr())
                {
                    ((EntityPlayerMP)p_187250_1_).sendContainerToPlayer(p_187250_1_.inventoryContainer);
                }

                return actionresult.func_188397_a();
            }
        }
    }

    public EnumActionResult func_187251_a(EntityPlayer p_187251_1_, World p_187251_2_, ItemStack p_187251_3_, EnumHand p_187251_4_, BlockPos p_187251_5_, EnumFacing p_187251_6_, float p_187251_7_, float p_187251_8_, float p_187251_9_)
    {
        if (this.gameType == WorldSettings.GameType.SPECTATOR)
        {
            TileEntity tileentity = p_187251_2_.getTileEntity(p_187251_5_);

            if (tileentity instanceof ILockableContainer)
            {
                Block block = p_187251_2_.getBlockState(p_187251_5_).getBlock();
                ILockableContainer ilockablecontainer = (ILockableContainer)tileentity;

                if (ilockablecontainer instanceof TileEntityChest && block instanceof BlockChest)
                {
                    ilockablecontainer = ((BlockChest)block).getLockableContainer(p_187251_2_, p_187251_5_);
                }

                if (ilockablecontainer != null)
                {
                    p_187251_1_.displayGUIChest(ilockablecontainer);
                    return EnumActionResult.SUCCESS;
                }
            }
            else if (tileentity instanceof IInventory)
            {
                p_187251_1_.displayGUIChest((IInventory)tileentity);
                return EnumActionResult.SUCCESS;
            }

            return EnumActionResult.PASS;
        }
        else
        {
            if (!p_187251_1_.isSneaking() || p_187251_1_.getHeldItemMainhand() == null && p_187251_1_.getHeldItemOffhand() == null)
            {
                IBlockState iblockstate = p_187251_2_.getBlockState(p_187251_5_);

                if (iblockstate.getBlock().onBlockActivated(p_187251_2_, p_187251_5_, iblockstate, p_187251_1_, p_187251_4_, p_187251_3_, p_187251_6_, p_187251_7_, p_187251_8_, p_187251_9_))
                {
                    return EnumActionResult.SUCCESS;
                }
            }

            if (p_187251_3_ == null)
            {
                return EnumActionResult.PASS;
            }
            else if (p_187251_1_.func_184811_cZ().hasCooldown(p_187251_3_.getItem()))
            {
                return EnumActionResult.PASS;
            }
            else if (p_187251_3_.getItem() instanceof ItemBlock && ((ItemBlock)p_187251_3_.getItem()).getBlock() instanceof BlockCommandBlock && !p_187251_1_.canCommandSenderUseCommand(2, ""))
            {
                return EnumActionResult.FAIL;
            }
            else if (this.isCreative())
            {
                int j = p_187251_3_.getMetadata();
                int i = p_187251_3_.stackSize;
                EnumActionResult enumactionresult = p_187251_3_.onItemUse(p_187251_1_, p_187251_2_, p_187251_5_, p_187251_4_, p_187251_6_, p_187251_7_, p_187251_8_, p_187251_9_);
                p_187251_3_.setItemDamage(j);
                p_187251_3_.stackSize = i;
                return enumactionresult;
            }
            else
            {
                return p_187251_3_.onItemUse(p_187251_1_, p_187251_2_, p_187251_5_, p_187251_4_, p_187251_6_, p_187251_7_, p_187251_8_, p_187251_9_);
            }
        }
    }

    /**
     * Sets the world instance.
     */
    public void setWorld(WorldServer serverWorld)
    {
        this.theWorld = serverWorld;
    }
}
