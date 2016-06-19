package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;

public class CPacketEnchantItem implements Packet<INetHandlerPlayServer>
{
    private int windowId;
    private int button;

    public CPacketEnchantItem()
    {
    }

    public CPacketEnchantItem(int p_i46883_1_, int p_i46883_2_)
    {
        this.windowId = p_i46883_1_;
        this.button = p_i46883_2_;
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processEnchantItem(this);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.windowId = buf.readByte();
        this.button = buf.readByte();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeByte(this.windowId);
        buf.writeByte(this.button);
    }

    public int getWindowId()
    {
        return this.windowId;
    }

    public int getButton()
    {
        return this.button;
    }
}
