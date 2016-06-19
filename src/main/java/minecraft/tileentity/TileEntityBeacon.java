package net.minecraft.tileentity;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.BlockStainedGlass;
import net.minecraft.block.BlockStainedGlassPane;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.passive.EntitySheep;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.ContainerBeacon;
import net.minecraft.inventory.ISidedInventory;
import net.minecraft.item.EnumDyeColor;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateTileEntity;
import net.minecraft.potion.Potion;
import net.minecraft.potion.PotionEffect;
import net.minecraft.stats.AchievementList;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ITickable;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;

public class TileEntityBeacon extends TileEntityLockable implements ITickable, ISidedInventory
{
    /** List of effects that Beacon can apply */
    public static final Potion[][] effectsList = new Potion[][] {{MobEffects.moveSpeed, MobEffects.digSpeed}, {MobEffects.resistance, MobEffects.jump}, {MobEffects.damageBoost}, {MobEffects.regeneration}};
    private static final Set<Potion> field_184280_f = Sets.<Potion>newHashSet();
    private final List<TileEntityBeacon.BeamSegment> beamSegments = Lists.<TileEntityBeacon.BeamSegment>newArrayList();
    private long beamRenderCounter;
    private float field_146014_j;
    private boolean isComplete;

    /** Level of this beacon's pyramid. */
    private int levels = -1;

    /** Primary potion effect given by this beacon. */
    private Potion primaryEffect;

    /** Secondary potion effect given by this beacon. */
    private Potion secondaryEffect;

    /** Item given to this beacon as payment. */
    private ItemStack payment;
    private String customName;

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        if (this.worldObj.getTotalWorldTime() % 80L == 0L)
        {
            this.updateBeacon();
        }
    }

    public void updateBeacon()
    {
        if (this.worldObj != null)
        {
            this.updateSegmentColors();
            this.addEffectsToPlayers();
        }
    }

    private void addEffectsToPlayers()
    {
        if (this.isComplete && this.levels > 0 && !this.worldObj.isRemote && this.primaryEffect != null)
        {
            double d0 = (double)(this.levels * 10 + 10);
            int i = 0;

            if (this.levels >= 4 && this.primaryEffect == this.secondaryEffect)
            {
                i = 1;
            }

            int j = (9 + this.levels * 2) * 20;
            int k = this.pos.getX();
            int l = this.pos.getY();
            int i1 = this.pos.getZ();
            AxisAlignedBB axisalignedbb = (new AxisAlignedBB((double)k, (double)l, (double)i1, (double)(k + 1), (double)(l + 1), (double)(i1 + 1))).func_186662_g(d0).addCoord(0.0D, (double)this.worldObj.getHeight(), 0.0D);
            List<EntityPlayer> list = this.worldObj.<EntityPlayer>getEntitiesWithinAABB(EntityPlayer.class, axisalignedbb);

            for (EntityPlayer entityplayer : list)
            {
                entityplayer.addPotionEffect(new PotionEffect(this.primaryEffect, j, i, true, true));
            }

            if (this.levels >= 4 && this.primaryEffect != this.secondaryEffect && this.secondaryEffect != null)
            {
                for (EntityPlayer entityplayer1 : list)
                {
                    entityplayer1.addPotionEffect(new PotionEffect(this.secondaryEffect, j, 0, true, true));
                }
            }
        }
    }

    private void updateSegmentColors()
    {
        int i = this.levels;
        int j = this.pos.getX();
        int k = this.pos.getY();
        int l = this.pos.getZ();
        this.levels = 0;
        this.beamSegments.clear();
        this.isComplete = true;
        TileEntityBeacon.BeamSegment tileentitybeacon$beamsegment = new TileEntityBeacon.BeamSegment(EntitySheep.getDyeRgb(EnumDyeColor.WHITE));
        this.beamSegments.add(tileentitybeacon$beamsegment);
        boolean flag = true;
        BlockPos.MutableBlockPos blockpos$mutableblockpos = new BlockPos.MutableBlockPos();

        for (int i1 = k + 1; i1 < 256; ++i1)
        {
            IBlockState iblockstate = this.worldObj.getBlockState(blockpos$mutableblockpos.set(j, i1, l));
            float[] afloat;

            if (iblockstate.getBlock() == Blocks.stained_glass)
            {
                afloat = EntitySheep.getDyeRgb((EnumDyeColor)iblockstate.getValue(BlockStainedGlass.COLOR));
            }
            else
            {
                if (iblockstate.getBlock() != Blocks.stained_glass_pane)
                {
                    if (iblockstate.getLightOpacity() >= 15 && iblockstate.getBlock() != Blocks.bedrock)
                    {
                        this.isComplete = false;
                        this.beamSegments.clear();
                        break;
                    }

                    tileentitybeacon$beamsegment.incrementHeight();
                    continue;
                }

                afloat = EntitySheep.getDyeRgb((EnumDyeColor)iblockstate.getValue(BlockStainedGlassPane.COLOR));
            }

            if (!flag)
            {
                afloat = new float[] {(tileentitybeacon$beamsegment.getColors()[0] + afloat[0]) / 2.0F, (tileentitybeacon$beamsegment.getColors()[1] + afloat[1]) / 2.0F, (tileentitybeacon$beamsegment.getColors()[2] + afloat[2]) / 2.0F};
            }

            if (Arrays.equals(afloat, tileentitybeacon$beamsegment.getColors()))
            {
                tileentitybeacon$beamsegment.incrementHeight();
            }
            else
            {
                tileentitybeacon$beamsegment = new TileEntityBeacon.BeamSegment(afloat);
                this.beamSegments.add(tileentitybeacon$beamsegment);
            }

            flag = false;
        }

        if (this.isComplete)
        {
            for (int l1 = 1; l1 <= 4; this.levels = l1++)
            {
                int i2 = k - l1;

                if (i2 < 0)
                {
                    break;
                }

                boolean flag1 = true;

                for (int j1 = j - l1; j1 <= j + l1 && flag1; ++j1)
                {
                    for (int k1 = l - l1; k1 <= l + l1; ++k1)
                    {
                        Block block = this.worldObj.getBlockState(new BlockPos(j1, i2, k1)).getBlock();

                        if (block != Blocks.emerald_block && block != Blocks.gold_block && block != Blocks.diamond_block && block != Blocks.iron_block)
                        {
                            flag1 = false;
                            break;
                        }
                    }
                }

                if (!flag1)
                {
                    break;
                }
            }

            if (this.levels == 0)
            {
                this.isComplete = false;
            }
        }

        if (!this.worldObj.isRemote && this.levels == 4 && i < this.levels)
        {
            for (EntityPlayer entityplayer : this.worldObj.getEntitiesWithinAABB(EntityPlayer.class, (new AxisAlignedBB((double)j, (double)k, (double)l, (double)j, (double)(k - 4), (double)l)).expand(10.0D, 5.0D, 10.0D)))
            {
                entityplayer.triggerAchievement(AchievementList.field_187978_K);
            }
        }
    }

    public List<TileEntityBeacon.BeamSegment> getBeamSegments()
    {
        return this.beamSegments;
    }

    public float shouldBeamRender()
    {
        if (!this.isComplete)
        {
            return 0.0F;
        }
        else
        {
            int i = (int)(this.worldObj.getTotalWorldTime() - this.beamRenderCounter);
            this.beamRenderCounter = this.worldObj.getTotalWorldTime();

            if (i > 1)
            {
                this.field_146014_j -= (float)i / 40.0F;

                if (this.field_146014_j < 0.0F)
                {
                    this.field_146014_j = 0.0F;
                }
            }

            this.field_146014_j += 0.025F;

            if (this.field_146014_j > 1.0F)
            {
                this.field_146014_j = 1.0F;
            }

            return this.field_146014_j;
        }
    }

    public Packet<?> getDescriptionPacket()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        this.writeToNBT(nbttagcompound);
        return new SPacketUpdateTileEntity(this.pos, 3, nbttagcompound);
    }

    public double getMaxRenderDistanceSquared()
    {
        return 65536.0D;
    }

    private static Potion func_184279_f(int p_184279_0_)
    {
        Potion potion = Potion.getPotionById(p_184279_0_);
        return field_184280_f.contains(potion) ? potion : null;
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        this.primaryEffect = func_184279_f(compound.getInteger("Primary"));
        this.secondaryEffect = func_184279_f(compound.getInteger("Secondary"));
        this.levels = compound.getInteger("Levels");
    }

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        compound.setInteger("Primary", Potion.getIdFromPotion(this.primaryEffect));
        compound.setInteger("Secondary", Potion.getIdFromPotion(this.secondaryEffect));
        compound.setInteger("Levels", this.levels);
    }

    /**
     * Returns the number of slots in the inventory.
     */
    public int getSizeInventory()
    {
        return 1;
    }

    /**
     * Returns the stack in the given slot.
     */
    public ItemStack getStackInSlot(int index)
    {
        return index == 0 ? this.payment : null;
    }

    /**
     * Removes up to a specified number of items from an inventory slot and returns them in a new stack.
     */
    public ItemStack decrStackSize(int index, int count)
    {
        if (index == 0 && this.payment != null)
        {
            if (count >= this.payment.stackSize)
            {
                ItemStack itemstack = this.payment;
                this.payment = null;
                return itemstack;
            }
            else
            {
                this.payment.stackSize -= count;
                return new ItemStack(this.payment.getItem(), count, this.payment.getMetadata());
            }
        }
        else
        {
            return null;
        }
    }

    /**
     * Removes a stack from the given slot and returns it.
     */
    public ItemStack removeStackFromSlot(int index)
    {
        if (index == 0)
        {
            ItemStack itemstack = this.payment;
            this.payment = null;
            return itemstack;
        }
        else
        {
            return null;
        }
    }

    /**
     * Sets the given item stack to the specified slot in the inventory (can be crafting or armor sections).
     */
    public void setInventorySlotContents(int index, ItemStack stack)
    {
        if (index == 0)
        {
            this.payment = stack;
        }
    }

    /**
     * Get the name of this object. For players this returns their username
     */
    public String getName()
    {
        return this.hasCustomName() ? this.customName : "container.beacon";
    }

    /**
     * Returns true if this thing is named
     */
    public boolean hasCustomName()
    {
        return this.customName != null && !this.customName.isEmpty();
    }

    public void setName(String name)
    {
        this.customName = name;
    }

    /**
     * Returns the maximum stack size for a inventory slot. Seems to always be 64, possibly will be extended.
     */
    public int getInventoryStackLimit()
    {
        return 1;
    }

    /**
     * Do not make give this method the name canInteractWith because it clashes with Container
     */
    public boolean isUseableByPlayer(EntityPlayer player)
    {
        return this.worldObj.getTileEntity(this.pos) != this ? false : player.getDistanceSq((double)this.pos.getX() + 0.5D, (double)this.pos.getY() + 0.5D, (double)this.pos.getZ() + 0.5D) <= 64.0D;
    }

    public void openInventory(EntityPlayer player)
    {
    }

    public void closeInventory(EntityPlayer player)
    {
    }

    /**
     * Returns true if automation is allowed to insert the given stack (ignoring stack size) into the given slot.
     */
    public boolean isItemValidForSlot(int index, ItemStack stack)
    {
        return stack.getItem() == Items.emerald || stack.getItem() == Items.diamond || stack.getItem() == Items.gold_ingot || stack.getItem() == Items.iron_ingot;
    }

    public String getGuiID()
    {
        return "minecraft:beacon";
    }

    public Container createContainer(InventoryPlayer playerInventory, EntityPlayer playerIn)
    {
        return new ContainerBeacon(playerInventory, this);
    }

    public int getField(int id)
    {
        switch (id)
        {
            case 0:
                return this.levels;

            case 1:
                return Potion.getIdFromPotion(this.primaryEffect);

            case 2:
                return Potion.getIdFromPotion(this.secondaryEffect);

            default:
                return 0;
        }
    }

    public void setField(int id, int value)
    {
        switch (id)
        {
            case 0:
                this.levels = value;
                break;

            case 1:
                this.primaryEffect = func_184279_f(value);
                break;

            case 2:
                this.secondaryEffect = func_184279_f(value);
        }
    }

    public int getFieldCount()
    {
        return 3;
    }

    public void clear()
    {
        this.payment = null;
    }

    public boolean receiveClientEvent(int id, int type)
    {
        if (id == 1)
        {
            this.updateBeacon();
            return true;
        }
        else
        {
            return super.receiveClientEvent(id, type);
        }
    }

    public int[] getSlotsForFace(EnumFacing side)
    {
        return new int[0];
    }

    /**
     * Returns true if automation can insert the given item in the given slot from the given side. Args: slot, item,
     * side
     */
    public boolean canInsertItem(int index, ItemStack itemStackIn, EnumFacing direction)
    {
        return false;
    }

    /**
     * Returns true if automation can extract the given item in the given slot from the given side. Args: slot, item,
     * side
     */
    public boolean canExtractItem(int index, ItemStack stack, EnumFacing direction)
    {
        return false;
    }

    static
    {
        for (Potion[] apotion : effectsList)
        {
            Collections.addAll(field_184280_f, apotion);
        }
    }

    public static class BeamSegment
    {
        private final float[] colors;
        private int height;

        public BeamSegment(float[] p_i45669_1_)
        {
            this.colors = p_i45669_1_;
            this.height = 1;
        }

        protected void incrementHeight()
        {
            ++this.height;
        }

        public float[] getColors()
        {
            return this.colors;
        }

        public int getHeight()
        {
            return this.height;
        }
    }
}
