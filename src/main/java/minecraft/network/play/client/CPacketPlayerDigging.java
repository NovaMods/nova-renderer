package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.BlockPos;

public class CPacketPlayerDigging implements Packet<INetHandlerPlayServer>
{
    private BlockPos position;
    private EnumFacing facing;

    /** Status of the digging (started, ongoing, broken). */
    private CPacketPlayerDigging.Action status;

    public CPacketPlayerDigging()
    {
    }

    public CPacketPlayerDigging(CPacketPlayerDigging.Action actionIn, BlockPos posIn, EnumFacing facingIn)
    {
        this.status = actionIn;
        this.position = posIn;
        this.facing = facingIn;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.status = (CPacketPlayerDigging.Action)buf.readEnumValue(CPacketPlayerDigging.Action.class);
        this.position = buf.readBlockPos();
        this.facing = EnumFacing.getFront(buf.readUnsignedByte());
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeEnumValue(this.status);
        buf.writeBlockPos(this.position);
        buf.writeByte(this.facing.getIndex());
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processPlayerDigging(this);
    }

    public BlockPos getPosition()
    {
        return this.position;
    }

    public EnumFacing getFacing()
    {
        return this.facing;
    }

    public CPacketPlayerDigging.Action getStatus()
    {
        return this.status;
    }

    public static enum Action
    {
        START_DESTROY_BLOCK,
        ABORT_DESTROY_BLOCK,
        STOP_DESTROY_BLOCK,
        DROP_ALL_ITEMS,
        DROP_ITEM,
        RELEASE_USE_ITEM,
        SWAP_HELD_ITEMS;
    }
}
