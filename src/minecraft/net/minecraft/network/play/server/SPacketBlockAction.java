package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.block.Block;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.util.math.BlockPos;

public class SPacketBlockAction implements Packet<INetHandlerPlayClient>
{
    private BlockPos blockPosition;
    private int instrument;
    private int pitch;
    private Block block;

    public SPacketBlockAction()
    {
    }

    public SPacketBlockAction(BlockPos p_i46966_1_, Block p_i46966_2_, int p_i46966_3_, int p_i46966_4_)
    {
        this.blockPosition = p_i46966_1_;
        this.instrument = p_i46966_3_;
        this.pitch = p_i46966_4_;
        this.block = p_i46966_2_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.blockPosition = buf.readBlockPos();
        this.instrument = buf.readUnsignedByte();
        this.pitch = buf.readUnsignedByte();
        this.block = Block.getBlockById(buf.readVarIntFromBuffer() & 4095);
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeBlockPos(this.blockPosition);
        buf.writeByte(this.instrument);
        buf.writeByte(this.pitch);
        buf.writeVarIntToBuffer(Block.getIdFromBlock(this.block) & 4095);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleBlockAction(this);
    }

    public BlockPos getBlockPosition()
    {
        return this.blockPosition;
    }

    /**
     * instrument data for noteblocks
     */
    public int getData1()
    {
        return this.instrument;
    }

    /**
     * pitch data for noteblocks
     */
    public int getData2()
    {
        return this.pitch;
    }

    public Block getBlockType()
    {
        return this.block;
    }
}
