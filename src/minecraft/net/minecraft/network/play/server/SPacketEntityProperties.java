package net.minecraft.network.play.server;

import com.google.common.collect.Lists;
import java.io.IOException;
import java.util.Collection;
import java.util.List;
import java.util.UUID;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;

public class SPacketEntityProperties implements Packet<INetHandlerPlayClient>
{
    private int entityId;
    private final List<SPacketEntityProperties.Snapshot> field_149444_b = Lists.<SPacketEntityProperties.Snapshot>newArrayList();

    public SPacketEntityProperties()
    {
    }

    public SPacketEntityProperties(int p_i46892_1_, Collection<IAttributeInstance> p_i46892_2_)
    {
        this.entityId = p_i46892_1_;

        for (IAttributeInstance iattributeinstance : p_i46892_2_)
        {
            this.field_149444_b.add(new SPacketEntityProperties.Snapshot(iattributeinstance.getAttribute().getAttributeUnlocalizedName(), iattributeinstance.getBaseValue(), iattributeinstance.func_111122_c()));
        }
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.entityId = buf.readVarIntFromBuffer();
        int i = buf.readInt();

        for (int j = 0; j < i; ++j)
        {
            String s = buf.readStringFromBuffer(64);
            double d0 = buf.readDouble();
            List<AttributeModifier> list = Lists.<AttributeModifier>newArrayList();
            int k = buf.readVarIntFromBuffer();

            for (int l = 0; l < k; ++l)
            {
                UUID uuid = buf.readUuid();
                list.add(new AttributeModifier(uuid, "Unknown synced attribute modifier", buf.readDouble(), buf.readByte()));
            }

            this.field_149444_b.add(new SPacketEntityProperties.Snapshot(s, d0, list));
        }
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeVarIntToBuffer(this.entityId);
        buf.writeInt(this.field_149444_b.size());

        for (SPacketEntityProperties.Snapshot spacketentityproperties$snapshot : this.field_149444_b)
        {
            buf.writeString(spacketentityproperties$snapshot.func_151409_a());
            buf.writeDouble(spacketentityproperties$snapshot.func_151410_b());
            buf.writeVarIntToBuffer(spacketentityproperties$snapshot.func_151408_c().size());

            for (AttributeModifier attributemodifier : spacketentityproperties$snapshot.func_151408_c())
            {
                buf.writeUuid(attributemodifier.getID());
                buf.writeDouble(attributemodifier.getAmount());
                buf.writeByte(attributemodifier.getOperation());
            }
        }
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleEntityProperties(this);
    }

    public int getEntityId()
    {
        return this.entityId;
    }

    public List<SPacketEntityProperties.Snapshot> func_149441_d()
    {
        return this.field_149444_b;
    }

    public class Snapshot
    {
        private final String field_151412_b;
        private final double field_151413_c;
        private final Collection<AttributeModifier> field_151411_d;

        public Snapshot(String p_i47075_2_, double p_i47075_3_, Collection<AttributeModifier> p_i47075_5_)
        {
            this.field_151412_b = p_i47075_2_;
            this.field_151413_c = p_i47075_3_;
            this.field_151411_d = p_i47075_5_;
        }

        public String func_151409_a()
        {
            return this.field_151412_b;
        }

        public double func_151410_b()
        {
            return this.field_151413_c;
        }

        public Collection<AttributeModifier> func_151408_c()
        {
            return this.field_151411_d;
        }
    }
}
