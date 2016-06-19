package net.minecraft.network.play.server;

import java.io.IOException;
import java.util.EnumSet;
import java.util.Set;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;

public class SPacketPlayerPosLook implements Packet<INetHandlerPlayClient>
{
    private double x;
    private double y;
    private double z;
    private float yaw;
    private float pitch;
    private Set<SPacketPlayerPosLook.EnumFlags> flags;
    private int teleportId;

    public SPacketPlayerPosLook()
    {
    }

    public SPacketPlayerPosLook(double xIn, double yIn, double zIn, float yawIn, float pitchIn, Set<SPacketPlayerPosLook.EnumFlags> flagsIn, int teleportIdIn)
    {
        this.x = xIn;
        this.y = yIn;
        this.z = zIn;
        this.yaw = yawIn;
        this.pitch = pitchIn;
        this.flags = flagsIn;
        this.teleportId = teleportIdIn;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.x = buf.readDouble();
        this.y = buf.readDouble();
        this.z = buf.readDouble();
        this.yaw = buf.readFloat();
        this.pitch = buf.readFloat();
        this.flags = SPacketPlayerPosLook.EnumFlags.func_187044_a(buf.readUnsignedByte());
        this.teleportId = buf.readVarIntFromBuffer();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeDouble(this.x);
        buf.writeDouble(this.y);
        buf.writeDouble(this.z);
        buf.writeFloat(this.yaw);
        buf.writeFloat(this.pitch);
        buf.writeByte(SPacketPlayerPosLook.EnumFlags.func_187040_a(this.flags));
        buf.writeVarIntToBuffer(this.teleportId);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handlePlayerPosLook(this);
    }

    public double getX()
    {
        return this.x;
    }

    public double getY()
    {
        return this.y;
    }

    public double getZ()
    {
        return this.z;
    }

    public float getYaw()
    {
        return this.yaw;
    }

    public float getPitch()
    {
        return this.pitch;
    }

    public int getTeleportId()
    {
        return this.teleportId;
    }

    public Set<SPacketPlayerPosLook.EnumFlags> getFlags()
    {
        return this.flags;
    }

    public static enum EnumFlags
    {
        X(0),
        Y(1),
        Z(2),
        Y_ROT(3),
        X_ROT(4);

        private final int field_187050_f;

        private EnumFlags(int p_i46690_3_)
        {
            this.field_187050_f = p_i46690_3_;
        }

        private int func_187042_a()
        {
            return 1 << this.field_187050_f;
        }

        private boolean func_187043_b(int p_187043_1_)
        {
            return (p_187043_1_ & this.func_187042_a()) == this.func_187042_a();
        }

        public static Set<SPacketPlayerPosLook.EnumFlags> func_187044_a(int p_187044_0_)
        {
            Set<SPacketPlayerPosLook.EnumFlags> set = EnumSet.<SPacketPlayerPosLook.EnumFlags>noneOf(SPacketPlayerPosLook.EnumFlags.class);

            for (SPacketPlayerPosLook.EnumFlags spacketplayerposlook$enumflags : values())
            {
                if (spacketplayerposlook$enumflags.func_187043_b(p_187044_0_))
                {
                    set.add(spacketplayerposlook$enumflags);
                }
            }

            return set;
        }

        public static int func_187040_a(Set<SPacketPlayerPosLook.EnumFlags> p_187040_0_)
        {
            int i = 0;

            for (SPacketPlayerPosLook.EnumFlags spacketplayerposlook$enumflags : p_187040_0_)
            {
                i |= spacketplayerposlook$enumflags.func_187042_a();
            }

            return i;
        }
    }
}
