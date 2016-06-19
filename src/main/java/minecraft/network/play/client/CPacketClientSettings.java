package net.minecraft.network.play.client;

import java.io.IOException;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayServer;
import net.minecraft.util.EnumHandSide;

public class CPacketClientSettings implements Packet<INetHandlerPlayServer>
{
    private String lang;
    private int view;
    private EntityPlayer.EnumChatVisibility chatVisibility;
    private boolean enableColors;
    private int modelPartFlags;
    private EnumHandSide field_186992_f;

    public CPacketClientSettings()
    {
    }

    public CPacketClientSettings(String p_i46885_1_, int p_i46885_2_, EntityPlayer.EnumChatVisibility p_i46885_3_, boolean p_i46885_4_, int p_i46885_5_, EnumHandSide p_i46885_6_)
    {
        this.lang = p_i46885_1_;
        this.view = p_i46885_2_;
        this.chatVisibility = p_i46885_3_;
        this.enableColors = p_i46885_4_;
        this.modelPartFlags = p_i46885_5_;
        this.field_186992_f = p_i46885_6_;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.lang = buf.readStringFromBuffer(7);
        this.view = buf.readByte();
        this.chatVisibility = (EntityPlayer.EnumChatVisibility)buf.readEnumValue(EntityPlayer.EnumChatVisibility.class);
        this.enableColors = buf.readBoolean();
        this.modelPartFlags = buf.readUnsignedByte();
        this.field_186992_f = (EnumHandSide)buf.readEnumValue(EnumHandSide.class);
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeString(this.lang);
        buf.writeByte(this.view);
        buf.writeEnumValue(this.chatVisibility);
        buf.writeBoolean(this.enableColors);
        buf.writeByte(this.modelPartFlags);
        buf.writeEnumValue(this.field_186992_f);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayServer handler)
    {
        handler.processClientSettings(this);
    }

    public String getLang()
    {
        return this.lang;
    }

    public EntityPlayer.EnumChatVisibility getChatVisibility()
    {
        return this.chatVisibility;
    }

    public boolean isColorsEnabled()
    {
        return this.enableColors;
    }

    public int getModelPartFlags()
    {
        return this.modelPartFlags;
    }

    public EnumHandSide func_186991_f()
    {
        return this.field_186992_f;
    }
}
