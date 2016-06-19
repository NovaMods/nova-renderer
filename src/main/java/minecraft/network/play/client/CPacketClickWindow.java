package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.inventory.ClickType;
import net.minecraft.item.ItemStack;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;

public class CPacketClickWindow implements Packet<INetHandlerPlayServer>
{
    /** The id of the window which was clicked. 0 for player inventory. */
    private int windowId;

    /** Id of the clicked slot */
    private int slotId;

    /** Button used */
    private int usedButton;

    /** A unique number for the action, used for transaction handling */
    private short actionNumber;

    /** The item stack present in the slot */
    private ItemStack clickedItem;

    /** Inventory operation mode */
    private ClickType mode;

    public CPacketClickWindow()
    {
    }

    public CPacketClickWindow(int p_i46882_1_, int p_i46882_2_, int p_i46882_3_, ClickType p_i46882_4_, ItemStack p_i46882_5_, short p_i46882_6_)
    {
        this.windowId = p_i46882_1_;
        this.slotId = p_i46882_2_;
        this.usedButton = p_i46882_3_;
        this.clickedItem = p_i46882_5_ != null ? p_i46882_5_.copy() : null;
        this.actionNumber = p_i46882_6_;
        this.mode = p_i46882_4_;
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processClickWindow(this);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.windowId = buf.readByte();
        this.slotId = buf.readShort();
        this.usedButton = buf.readByte();
        this.actionNumber = buf.readShort();
        this.mode = (ClickType)buf.readEnumValue(ClickType.class);
        this.clickedItem = buf.readItemStackFromBuffer();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeByte(this.windowId);
        buf.writeShort(this.slotId);
        buf.writeByte(this.usedButton);
        buf.writeShort(this.actionNumber);
        buf.writeEnumValue(this.mode);
        buf.writeItemStackToBuffer(this.clickedItem);
    }

    public int getWindowId()
    {
        return this.windowId;
    }

    public int getSlotId()
    {
        return this.slotId;
    }

    public int getUsedButton()
    {
        return this.usedButton;
    }

    public short getActionNumber()
    {
        return this.actionNumber;
    }

    public ItemStack getClickedItem()
    {
        return this.clickedItem;
    }

    public ClickType getClickType()
    {
        return this.mode;
    }
}
