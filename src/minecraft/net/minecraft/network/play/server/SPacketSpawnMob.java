package net.minecraft.network.play.server;

import java.io.IOException;
import java.util.List;
import java.util.UUID;
import net.minecraft.entity.EntityList;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.network.play.INetHandlerPlayClient;

public class SPacketSpawnMob implements Packet<INetHandlerPlayClient>
{
    private int entityId;
    private UUID uniqueId;
    private int type;
    private double x;
    private double y;
    private double z;
    private int velocityX;
    private int velocityY;
    private int velocityZ;
    private byte yaw;
    private byte pitch;
    private byte headPitch;
    private EntityDataManager dataManager;
    private List < EntityDataManager.DataEntry<? >> watcher;

    public SPacketSpawnMob()
    {
    }

    public SPacketSpawnMob(EntityLivingBase entityIn)
    {
        this.entityId = entityIn.getEntityId();
        this.uniqueId = entityIn.getUniqueID();
        this.type = (byte)EntityList.getEntityID(entityIn);
        this.x = entityIn.posX;
        this.y = entityIn.posY;
        this.z = entityIn.posZ;
        this.yaw = (byte)((int)(entityIn.rotationYaw * 256.0F / 360.0F));
        this.pitch = (byte)((int)(entityIn.rotationPitch * 256.0F / 360.0F));
        this.headPitch = (byte)((int)(entityIn.rotationYawHead * 256.0F / 360.0F));
        double d0 = 3.9D;
        double d1 = entityIn.motionX;
        double d2 = entityIn.motionY;
        double d3 = entityIn.motionZ;

        if (d1 < -d0)
        {
            d1 = -d0;
        }

        if (d2 < -d0)
        {
            d2 = -d0;
        }

        if (d3 < -d0)
        {
            d3 = -d0;
        }

        if (d1 > d0)
        {
            d1 = d0;
        }

        if (d2 > d0)
        {
            d2 = d0;
        }

        if (d3 > d0)
        {
            d3 = d0;
        }

        this.velocityX = (int)(d1 * 8000.0D);
        this.velocityY = (int)(d2 * 8000.0D);
        this.velocityZ = (int)(d3 * 8000.0D);
        this.dataManager = entityIn.getDataManager();
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.entityId = buf.readVarIntFromBuffer();
        this.uniqueId = buf.readUuid();
        this.type = buf.readByte() & 255;
        this.x = buf.readDouble();
        this.y = buf.readDouble();
        this.z = buf.readDouble();
        this.yaw = buf.readByte();
        this.pitch = buf.readByte();
        this.headPitch = buf.readByte();
        this.velocityX = buf.readShort();
        this.velocityY = buf.readShort();
        this.velocityZ = buf.readShort();
        this.watcher = EntityDataManager.readEntries(buf);
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.entityId);
        buf.writeUuid(this.uniqueId);
        buf.writeByte(this.type & 255);
        buf.writeDouble(this.x);
        buf.writeDouble(this.y);
        buf.writeDouble(this.z);
        buf.writeByte(this.yaw);
        buf.writeByte(this.pitch);
        buf.writeByte(this.headPitch);
        buf.writeShort(this.velocityX);
        buf.writeShort(this.velocityY);
        buf.writeShort(this.velocityZ);
        this.dataManager.writeEntries(buf);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleSpawnMob(this);
    }

    public List < EntityDataManager.DataEntry<? >> func_149027_c()
    {
        if (this.watcher == null)
        {
            this.watcher = this.dataManager.getAll();
        }

        return this.watcher;
    }

    public int getEntityID()
    {
        return this.entityId;
    }

    public UUID getUniqueId()
    {
        return this.uniqueId;
    }

    public int getEntityType()
    {
        return this.type;
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

    public int getVelocityX()
    {
        return this.velocityX;
    }

    public int getVelocityY()
    {
        return this.velocityY;
    }

    public int getVelocityZ()
    {
        return this.velocityZ;
    }

    public byte getYaw()
    {
        return this.yaw;
    }

    public byte getPitch()
    {
        return this.pitch;
    }

    public byte getHeadPitch()
    {
        return this.headPitch;
    }
}
