package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.entity.Entity;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.world.World;

public class SPacketEntity implements Packet<INetHandlerPlayClient>
{
    protected int entityId;
    protected int posX;
    protected int posY;
    protected int posZ;
    protected byte yaw;
    protected byte pitch;
    protected boolean onGround;
    protected boolean field_149069_g;

    public SPacketEntity()
    {
    }

    public SPacketEntity(int p_i46936_1_)
    {
        this.entityId = p_i46936_1_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.entityId = buf.readVarIntFromBuffer();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.entityId);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleEntityMovement(this);
    }

    public String toString()
    {
        return "Entity_" + super.toString();
    }

    public Entity getEntity(World worldIn)
    {
        return worldIn.getEntityByID(this.entityId);
    }

    public int func_186952_a()
    {
        return this.posX;
    }

    public int func_186953_b()
    {
        return this.posY;
    }

    public int func_186951_c()
    {
        return this.posZ;
    }

    public byte func_149066_f()
    {
        return this.yaw;
    }

    public byte func_149063_g()
    {
        return this.pitch;
    }

    public boolean func_149060_h()
    {
        return this.field_149069_g;
    }

    public boolean getOnGround()
    {
        return this.onGround;
    }

    public static class S15PacketEntityRelMove extends SPacketEntity
    {
        public S15PacketEntityRelMove()
        {
        }

        public S15PacketEntityRelMove(int p_i47083_1_, long p_i47083_2_, long p_i47083_4_, long p_i47083_6_, boolean p_i47083_8_)
        {
            super(p_i47083_1_);
            this.posX = (int)p_i47083_2_;
            this.posY = (int)p_i47083_4_;
            this.posZ = (int)p_i47083_6_;
            this.onGround = p_i47083_8_;
        }

        public void readPacketData(PacketBuffer buf) throws IOException
        {
            super.readPacketData(buf);
            this.posX = buf.readShort();
            this.posY = buf.readShort();
            this.posZ = buf.readShort();
            this.onGround = buf.readBoolean();
        }

        public void writePacketData(PacketBuffer buf) throws IOException
        {
            super.writePacketData(buf);
            buf.writeShort(this.posX);
            buf.writeShort(this.posY);
            buf.writeShort(this.posZ);
            buf.writeBoolean(this.onGround);
        }
    }

    public static class S16PacketEntityLook extends SPacketEntity
    {
        public S16PacketEntityLook()
        {
            this.field_149069_g = true;
        }

        public S16PacketEntityLook(int p_i47081_1_, byte p_i47081_2_, byte p_i47081_3_, boolean p_i47081_4_)
        {
            super(p_i47081_1_);
            this.yaw = p_i47081_2_;
            this.pitch = p_i47081_3_;
            this.field_149069_g = true;
            this.onGround = p_i47081_4_;
        }

        public void readPacketData(PacketBuffer buf) throws IOException
        {
            super.readPacketData(buf);
            this.yaw = buf.readByte();
            this.pitch = buf.readByte();
            this.onGround = buf.readBoolean();
        }

        public void writePacketData(PacketBuffer buf) throws IOException
        {
            super.writePacketData(buf);
            buf.writeByte(this.yaw);
            buf.writeByte(this.pitch);
            buf.writeBoolean(this.onGround);
        }
    }

    public static class S17PacketEntityLookMove extends SPacketEntity
    {
        public S17PacketEntityLookMove()
        {
            this.field_149069_g = true;
        }

        public S17PacketEntityLookMove(int p_i47082_1_, long p_i47082_2_, long p_i47082_4_, long p_i47082_6_, byte p_i47082_8_, byte p_i47082_9_, boolean p_i47082_10_)
        {
            super(p_i47082_1_);
            this.posX = (int)p_i47082_2_;
            this.posY = (int)p_i47082_4_;
            this.posZ = (int)p_i47082_6_;
            this.yaw = p_i47082_8_;
            this.pitch = p_i47082_9_;
            this.onGround = p_i47082_10_;
            this.field_149069_g = true;
        }

        public void readPacketData(PacketBuffer buf) throws IOException
        {
            super.readPacketData(buf);
            this.posX = buf.readShort();
            this.posY = buf.readShort();
            this.posZ = buf.readShort();
            this.yaw = buf.readByte();
            this.pitch = buf.readByte();
            this.onGround = buf.readBoolean();
        }

        public void writePacketData(PacketBuffer buf) throws IOException
        {
            super.writePacketData(buf);
            buf.writeShort(this.posX);
            buf.writeShort(this.posY);
            buf.writeShort(this.posZ);
            buf.writeByte(this.yaw);
            buf.writeByte(this.pitch);
            buf.writeBoolean(this.onGround);
        }
    }
}
