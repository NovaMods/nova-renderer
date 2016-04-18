package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.util.math.BlockPos;

public class SPacketBlockBreakAnim implements Packet<INetHandlerPlayClient>
{
    private int breakerId;
    private BlockPos position;
    private int progress;

    public SPacketBlockBreakAnim()
    {
    }

    public SPacketBlockBreakAnim(int p_i46968_1_, BlockPos p_i46968_2_, int p_i46968_3_)
    {
        this.breakerId = p_i46968_1_;
        this.position = p_i46968_2_;
        this.progress = p_i46968_3_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.breakerId = buf.readVarIntFromBuffer();
        this.position = buf.readBlockPos();
        this.progress = buf.readUnsignedByte();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.breakerId);
        buf.writeBlockPos(this.position);
        buf.writeByte(this.progress);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleBlockBreakAnim(this);
    }

    public int getBreakerId()
    {
        return this.breakerId;
    }

    public BlockPos getPosition()
    {
        return this.position;
    }

    public int getProgress()
    {
        return this.progress;
    }
}
