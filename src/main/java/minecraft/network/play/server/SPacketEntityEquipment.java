package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;

public class SPacketEntityEquipment implements Packet<INetHandlerPlayClient>
{
    private int entityID;
    private EntityEquipmentSlot equipmentSlot;
    private ItemStack itemStack;

    public SPacketEntityEquipment()
    {
    }

    public SPacketEntityEquipment(int p_i46913_1_, EntityEquipmentSlot p_i46913_2_, ItemStack p_i46913_3_)
    {
        this.entityID = p_i46913_1_;
        this.equipmentSlot = p_i46913_2_;
        this.itemStack = p_i46913_3_ == null ? null : p_i46913_3_.copy();
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.entityID = buf.readVarIntFromBuffer();
        this.equipmentSlot = (EntityEquipmentSlot)buf.readEnumValue(EntityEquipmentSlot.class);
        this.itemStack = buf.readItemStackFromBuffer();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.entityID);
        buf.writeEnumValue(this.equipmentSlot);
        buf.writeItemStackToBuffer(this.itemStack);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleEntityEquipment(this);
    }

    public ItemStack getItemStack()
    {
        return this.itemStack;
    }

    public int getEntityID()
    {
        return this.entityID;
    }

    public EntityEquipmentSlot func_186969_c()
    {
        return this.equipmentSlot;
    }
}
