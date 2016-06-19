package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;

public class CPacketPlayer implements Packet<INetHandlerPlayServer>
{
    protected double x;
    protected double y;
    protected double z;
    protected float yaw;
    protected float pitch;
    protected boolean onGround;
    protected boolean moving;
    protected boolean rotating;

    public CPacketPlayer()
    {
    }

    public CPacketPlayer(boolean onGroundIn)
    {
        this.onGround = onGroundIn;
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processPlayer(this);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.onGround = buf.readUnsignedByte() != 0;
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeByte(this.onGround ? 1 : 0);
    }

    public double func_186997_a(double p_186997_1_)
    {
        return this.moving ? this.x : p_186997_1_;
    }

    public double func_186996_b(double p_186996_1_)
    {
        return this.moving ? this.y : p_186996_1_;
    }

    public double func_187000_c(double p_187000_1_)
    {
        return this.moving ? this.z : p_187000_1_;
    }

    public float func_186999_a(float p_186999_1_)
    {
        return this.rotating ? this.yaw : p_186999_1_;
    }

    public float func_186998_b(float p_186998_1_)
    {
        return this.rotating ? this.pitch : p_186998_1_;
    }

    public boolean isOnGround()
    {
        return this.onGround;
    }

    public static class C04PacketPlayerPosition extends CPacketPlayer
    {
        public C04PacketPlayerPosition()
        {
            this.moving = true;
        }

        public C04PacketPlayerPosition(double xIn, double yIn, double zIn, boolean onGroundIn)
        {
            this.x = xIn;
            this.y = yIn;
            this.z = zIn;
            this.onGround = onGroundIn;
            this.moving = true;
        }

        public void readPacketData(PacketBuffer buf) throws IOException
        {
            this.x = buf.readDouble();
            this.y = buf.readDouble();
            this.z = buf.readDouble();
            super.readPacketData(buf);
        }

        public void writePacketData(PacketBuffer buf) throws IOException
        {
            buf.writeDouble(this.x);
            buf.writeDouble(this.y);
            buf.writeDouble(this.z);
            super.writePacketData(buf);
        }
    }

    public static class C05PacketPlayerLook extends CPacketPlayer
    {
        public C05PacketPlayerLook()
        {
            this.rotating = true;
        }

        public C05PacketPlayerLook(float yawIn, float pitchIn, boolean onGroundIn)
        {
            this.yaw = yawIn;
            this.pitch = pitchIn;
            this.onGround = onGroundIn;
            this.rotating = true;
        }

        public void readPacketData(PacketBuffer buf) throws IOException
        {
            this.yaw = buf.readFloat();
            this.pitch = buf.readFloat();
            super.readPacketData(buf);
        }

        public void writePacketData(PacketBuffer buf) throws IOException
        {
            buf.writeFloat(this.yaw);
            buf.writeFloat(this.pitch);
            super.writePacketData(buf);
        }
    }

    public static class C06PacketPlayerPosLook extends CPacketPlayer
    {
        public C06PacketPlayerPosLook()
        {
            this.moving = true;
            this.rotating = true;
        }

        public C06PacketPlayerPosLook(double xIn, double yIn, double zIn, float yawIn, float pitchIn, boolean onGroundIn)
        {
            this.x = xIn;
            this.y = yIn;
            this.z = zIn;
            this.yaw = yawIn;
            this.pitch = pitchIn;
            this.onGround = onGroundIn;
            this.rotating = true;
            this.moving = true;
        }

        public void readPacketData(PacketBuffer buf) throws IOException
        {
            this.x = buf.readDouble();
            this.y = buf.readDouble();
            this.z = buf.readDouble();
            this.yaw = buf.readFloat();
            this.pitch = buf.readFloat();
            super.readPacketData(buf);
        }

        public void writePacketData(PacketBuffer buf) throws IOException
        {
            buf.writeDouble(this.x);
            buf.writeDouble(this.y);
            buf.writeDouble(this.z);
            buf.writeFloat(this.yaw);
            buf.writeFloat(this.pitch);
            super.writePacketData(buf);
        }
    }
}
