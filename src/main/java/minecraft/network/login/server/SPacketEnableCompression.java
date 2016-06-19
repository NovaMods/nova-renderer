package net.minecraft.network.login.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.login.INetHandlerLoginClient;

public class SPacketEnableCompression implements Packet<INetHandlerLoginClient>
{
    private int compressionTreshold;

    public SPacketEnableCompression()
    {
    }

    public SPacketEnableCompression(int thresholdIn)
    {
        this.compressionTreshold = thresholdIn;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.compressionTreshold = buf.readVarIntFromBuffer();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.compressionTreshold);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerLoginClient handler)
    {
        handler.handleEnableCompression(this);
    }

    public int getCompressionTreshold()
    {
        return this.compressionTreshold;
    }
}
