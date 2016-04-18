package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;

public class CPacketChatMessage implements Packet<INetHandlerPlayServer>
{
    private String message;

    public CPacketChatMessage()
    {
    }

    public CPacketChatMessage(String p_i46887_1_)
    {
        if (p_i46887_1_.length() > 100)
        {
            p_i46887_1_ = p_i46887_1_.substring(0, 100);
        }

        this.message = p_i46887_1_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.message = buf.readStringFromBuffer(100);
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeString(this.message);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processChatMessage(this);
    }

    public String getMessage()
    {
        return this.message;
    }
}
