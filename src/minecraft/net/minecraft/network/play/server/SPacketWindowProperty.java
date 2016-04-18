package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;

public class SPacketWindowProperty implements Packet<INetHandlerPlayClient>
{
    private int windowId;
    private int varIndex;
    private int varValue;

    public SPacketWindowProperty()
    {
    }

    public SPacketWindowProperty(int windowIdIn, int p_i46952_2_, int p_i46952_3_)
    {
        this.windowId = windowIdIn;
        this.varIndex = p_i46952_2_;
        this.varValue = p_i46952_3_;
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleWindowProperty(this);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.windowId = buf.readUnsignedByte();
        this.varIndex = buf.readShort();
        this.varValue = buf.readShort();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeByte(this.windowId);
        buf.writeShort(this.varIndex);
        buf.writeShort(this.varValue);
    }

    public int getWindowId()
    {
        return this.windowId;
    }

    public int getVarIndex()
    {
        return this.varIndex;
    }

    public int getVarValue()
    {
        return this.varValue;
    }
}
