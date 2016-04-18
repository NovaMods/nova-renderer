package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.scoreboard.Score;
import net.minecraft.scoreboard.ScoreObjective;

public class SPacketUpdateScore implements Packet<INetHandlerPlayClient>
{
    private String name = "";
    private String objective = "";
    private int value;
    private SPacketUpdateScore.Action action;

    public SPacketUpdateScore()
    {
    }

    public SPacketUpdateScore(Score p_i46904_1_)
    {
        this.name = p_i46904_1_.getPlayerName();
        this.objective = p_i46904_1_.getObjective().getName();
        this.value = p_i46904_1_.getScorePoints();
        this.action = SPacketUpdateScore.Action.CHANGE;
    }

    public SPacketUpdateScore(String p_i46905_1_)
    {
        this.name = p_i46905_1_;
        this.objective = "";
        this.value = 0;
        this.action = SPacketUpdateScore.Action.REMOVE;
    }

    public SPacketUpdateScore(String p_i46906_1_, ScoreObjective p_i46906_2_)
    {
        this.name = p_i46906_1_;
        this.objective = p_i46906_2_.getName();
        this.value = 0;
        this.action = SPacketUpdateScore.Action.REMOVE;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.name = buf.readStringFromBuffer(40);
        this.action = (SPacketUpdateScore.Action)buf.readEnumValue(SPacketUpdateScore.Action.class);
        this.objective = buf.readStringFromBuffer(16);

        if (this.action != SPacketUpdateScore.Action.REMOVE)
        {
            this.value = buf.readVarIntFromBuffer();
        }
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeString(this.name);
        buf.writeEnumValue(this.action);
        buf.writeString(this.objective);

        if (this.action != SPacketUpdateScore.Action.REMOVE)
        {
            buf.writeVarIntToBuffer(this.value);
        }
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleUpdateScore(this);
    }

    public String getPlayerName()
    {
        return this.name;
    }

    public String getObjectiveName()
    {
        return this.objective;
    }

    public int getScoreValue()
    {
        return this.value;
    }

    public SPacketUpdateScore.Action getScoreAction()
    {
        return this.action;
    }

    public static enum Action
    {
        CHANGE,
        REMOVE;
    }
}
