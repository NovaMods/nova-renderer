package net.minecraft.network.play.server;

import java.io.IOException;
import java.util.Collection;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.util.math.Vec4b;
import net.minecraft.world.storage.MapData;

public class SPacketMaps implements Packet<INetHandlerPlayClient>
{
    private int mapId;
    private byte mapScale;
    private boolean field_186950_c;
    private Vec4b[] mapVisiblePlayersVec4b;
    private int mapMinX;
    private int mapMinY;
    private int mapMaxX;
    private int mapMaxY;
    private byte[] mapDataBytes;

    public SPacketMaps()
    {
    }

    public SPacketMaps(int p_i46937_1_, byte p_i46937_2_, boolean p_i46937_3_, Collection<Vec4b> p_i46937_4_, byte[] p_i46937_5_, int p_i46937_6_, int p_i46937_7_, int p_i46937_8_, int p_i46937_9_)
    {
        this.mapId = p_i46937_1_;
        this.mapScale = p_i46937_2_;
        this.field_186950_c = p_i46937_3_;
        this.mapVisiblePlayersVec4b = (Vec4b[])p_i46937_4_.toArray(new Vec4b[p_i46937_4_.size()]);
        this.mapMinX = p_i46937_6_;
        this.mapMinY = p_i46937_7_;
        this.mapMaxX = p_i46937_8_;
        this.mapMaxY = p_i46937_9_;
        this.mapDataBytes = new byte[p_i46937_8_ * p_i46937_9_];

        for (int i = 0; i < p_i46937_8_; ++i)
        {
            for (int j = 0; j < p_i46937_9_; ++j)
            {
                this.mapDataBytes[i + j * p_i46937_8_] = p_i46937_5_[p_i46937_6_ + i + (p_i46937_7_ + j) * 128];
            }
        }
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.mapId = buf.readVarIntFromBuffer();
        this.mapScale = buf.readByte();
        this.field_186950_c = buf.readBoolean();
        this.mapVisiblePlayersVec4b = new Vec4b[buf.readVarIntFromBuffer()];

        for (int i = 0; i < this.mapVisiblePlayersVec4b.length; ++i)
        {
            short short1 = (short)buf.readByte();
            this.mapVisiblePlayersVec4b[i] = new Vec4b((byte)(short1 >> 4 & 15), buf.readByte(), buf.readByte(), (byte)(short1 & 15));
        }

        this.mapMaxX = buf.readUnsignedByte();

        if (this.mapMaxX > 0)
        {
            this.mapMaxY = buf.readUnsignedByte();
            this.mapMinX = buf.readUnsignedByte();
            this.mapMinY = buf.readUnsignedByte();
            this.mapDataBytes = buf.readByteArray();
        }
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.mapId);
        buf.writeByte(this.mapScale);
        buf.writeBoolean(this.field_186950_c);
        buf.writeVarIntToBuffer(this.mapVisiblePlayersVec4b.length);

        for (Vec4b vec4b : this.mapVisiblePlayersVec4b)
        {
            buf.writeByte((vec4b.getX() & 15) << 4 | vec4b.getW() & 15);
            buf.writeByte(vec4b.getY());
            buf.writeByte(vec4b.getZ());
        }

        buf.writeByte(this.mapMaxX);

        if (this.mapMaxX > 0)
        {
            buf.writeByte(this.mapMaxY);
            buf.writeByte(this.mapMinX);
            buf.writeByte(this.mapMinY);
            buf.writeByteArray(this.mapDataBytes);
        }
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleMaps(this);
    }

    public int getMapId()
    {
        return this.mapId;
    }

    /**
     * Sets new MapData from the packet to given MapData param
     */
    public void setMapdataTo(MapData mapdataIn)
    {
        mapdataIn.scale = this.mapScale;
        mapdataIn.trackingPosition = this.field_186950_c;
        mapdataIn.mapDecorations.clear();

        for (int i = 0; i < this.mapVisiblePlayersVec4b.length; ++i)
        {
            Vec4b vec4b = this.mapVisiblePlayersVec4b[i];
            mapdataIn.mapDecorations.put("icon-" + i, vec4b);
        }

        for (int j = 0; j < this.mapMaxX; ++j)
        {
            for (int k = 0; k < this.mapMaxY; ++k)
            {
                mapdataIn.colors[this.mapMinX + j + (this.mapMinY + k) * 128] = this.mapDataBytes[j + k * this.mapMaxX];
            }
        }
    }
}
