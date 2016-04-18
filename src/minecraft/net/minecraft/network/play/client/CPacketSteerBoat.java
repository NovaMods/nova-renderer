package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;

public class CPacketSteerBoat implements Packet<INetHandlerPlayServer>
{
    private boolean field_187015_a;
    private boolean field_187016_b;

    public CPacketSteerBoat()
    {
    }

    public CPacketSteerBoat(boolean p_i46873_1_, boolean p_i46873_2_)
    {
        this.field_187015_a = p_i46873_1_;
        this.field_187016_b = p_i46873_2_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.field_187015_a = buf.readBoolean();
        this.field_187016_b = buf.readBoolean();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeBoolean(this.field_187015_a);
        buf.writeBoolean(this.field_187016_b);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.func_184340_a(this);
    }

    public boolean func_187012_a()
    {
        return this.field_187015_a;
    }

    public boolean func_187014_b()
    {
        return this.field_187016_b;
    }
}
