package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.util.math.BlockPos;

public class SPacketEffect implements Packet<INetHandlerPlayClient>
{
    private int soundType;
    private BlockPos soundPos;

    /** can be a block/item id or other depending on the soundtype */
    private int soundData;

    /** If true the sound is played across the server */
    private boolean serverWide;

    public SPacketEffect()
    {
    }

    public SPacketEffect(int p_i46940_1_, BlockPos p_i46940_2_, int p_i46940_3_, boolean p_i46940_4_)
    {
        this.soundType = p_i46940_1_;
        this.soundPos = p_i46940_2_;
        this.soundData = p_i46940_3_;
        this.serverWide = p_i46940_4_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.soundType = buf.readInt();
        this.soundPos = buf.readBlockPos();
        this.soundData = buf.readInt();
        this.serverWide = buf.readBoolean();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeInt(this.soundType);
        buf.writeBlockPos(this.soundPos);
        buf.writeInt(this.soundData);
        buf.writeBoolean(this.serverWide);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleEffect(this);
    }

    public boolean isSoundServerwide()
    {
        return this.serverWide;
    }

    public int getSoundType()
    {
        return this.soundType;
    }

    public int getSoundData()
    {
        return this.soundData;
    }

    public BlockPos getSoundPos()
    {
        return this.soundPos;
    }
}
