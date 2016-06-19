package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;

public class CPacketConfirmTransaction implements Packet<INetHandlerPlayServer>
{
    private int windowId;
    private short uid;
    private boolean accepted;

    public CPacketConfirmTransaction()
    {
    }

    public CPacketConfirmTransaction(int p_i46884_1_, short p_i46884_2_, boolean p_i46884_3_)
    {
        this.windowId = p_i46884_1_;
        this.uid = p_i46884_2_;
        this.accepted = p_i46884_3_;
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processConfirmTransaction(this);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.windowId = buf.readByte();
        this.uid = buf.readShort();
        this.accepted = buf.readByte() != 0;
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeByte(this.windowId);
        buf.writeShort(this.uid);
        buf.writeByte(this.accepted ? 1 : 0);
    }

    public int getWindowId()
    {
        return this.windowId;
    }

    public short getUid()
    {
        return this.uid;
    }
}
