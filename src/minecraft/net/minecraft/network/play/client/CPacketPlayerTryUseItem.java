package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;

public class CPacketPlayerTryUseItem implements Packet<INetHandlerPlayServer>
{
    private BlockPos position;
    private EnumFacing placedBlockDirection;
    private EnumHand hand;
    private float facingX;
    private float facingY;
    private float facingZ;

    public CPacketPlayerTryUseItem()
    {
    }

    public CPacketPlayerTryUseItem(BlockPos p_i46858_1_, EnumFacing p_i46858_2_, EnumHand handIn, float p_i46858_4_, float p_i46858_5_, float p_i46858_6_)
    {
        this.position = p_i46858_1_;
        this.placedBlockDirection = p_i46858_2_;
        this.hand = handIn;
        this.facingX = p_i46858_4_;
        this.facingY = p_i46858_5_;
        this.facingZ = p_i46858_6_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.position = buf.readBlockPos();
        this.placedBlockDirection = (EnumFacing)buf.readEnumValue(EnumFacing.class);
        this.hand = (EnumHand)buf.readEnumValue(EnumHand.class);
        this.facingX = (float)buf.readUnsignedByte() / 16.0F;
        this.facingY = (float)buf.readUnsignedByte() / 16.0F;
        this.facingZ = (float)buf.readUnsignedByte() / 16.0F;
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeBlockPos(this.position);
        buf.writeEnumValue(this.placedBlockDirection);
        buf.writeEnumValue(this.hand);
        buf.writeByte((int)(this.facingX * 16.0F));
        buf.writeByte((int)(this.facingY * 16.0F));
        buf.writeByte((int)(this.facingZ * 16.0F));
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.func_184337_a(this);
    }

    public BlockPos func_187023_a()
    {
        return this.position;
    }

    public EnumFacing func_187024_b()
    {
        return this.placedBlockDirection;
    }

    public EnumHand getHand()
    {
        return this.hand;
    }

    public float func_187026_d()
    {
        return this.facingX;
    }

    public float func_187025_e()
    {
        return this.facingY;
    }

    public float func_187020_f()
    {
        return this.facingZ;
    }
}
