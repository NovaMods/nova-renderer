package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.MathHelper;
import org.apache.commons.lang3.Validate;

public class SPacketCustomSound implements Packet<INetHandlerPlayClient>
{
    private String soundName;
    private SoundCategory category;
    private int field_186934_c;
    private int field_186935_d = Integer.MAX_VALUE;
    private int field_186936_e;
    private float field_186937_f;
    private int field_186938_g;

    public SPacketCustomSound()
    {
    }

    public SPacketCustomSound(String soundNameIn, SoundCategory categoryIn, double p_i46948_3_, double p_i46948_5_, double p_i46948_7_, float p_i46948_9_, float p_i46948_10_)
    {
        Validate.notNull(soundNameIn, "name", new Object[0]);
        this.soundName = soundNameIn;
        this.category = categoryIn;
        this.field_186934_c = (int)(p_i46948_3_ * 8.0D);
        this.field_186935_d = (int)(p_i46948_5_ * 8.0D);
        this.field_186936_e = (int)(p_i46948_7_ * 8.0D);
        this.field_186937_f = p_i46948_9_;
        this.field_186938_g = (int)(p_i46948_10_ * 63.0F);
        p_i46948_10_ = MathHelper.clamp_float(p_i46948_10_, 0.0F, 255.0F);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.soundName = buf.readStringFromBuffer(256);
        this.category = (SoundCategory)buf.readEnumValue(SoundCategory.class);
        this.field_186934_c = buf.readInt();
        this.field_186935_d = buf.readInt();
        this.field_186936_e = buf.readInt();
        this.field_186937_f = buf.readFloat();
        this.field_186938_g = buf.readUnsignedByte();
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeString(this.soundName);
        buf.writeEnumValue(this.category);
        buf.writeInt(this.field_186934_c);
        buf.writeInt(this.field_186935_d);
        buf.writeInt(this.field_186936_e);
        buf.writeFloat(this.field_186937_f);
        buf.writeByte(this.field_186938_g);
    }

    public String getSoundName()
    {
        return this.soundName;
    }

    public SoundCategory getCategory()
    {
        return this.category;
    }

    public double func_186932_c()
    {
        return (double)((float)this.field_186934_c / 8.0F);
    }

    public double func_186926_d()
    {
        return (double)((float)this.field_186935_d / 8.0F);
    }

    public double func_186925_e()
    {
        return (double)((float)this.field_186936_e / 8.0F);
    }

    public float func_186927_f()
    {
        return this.field_186937_f;
    }

    public float func_186928_g()
    {
        return (float)this.field_186938_g / 63.0F;
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleCustomSound(this);
    }
}
