package net.minecraft.network.play.server;

import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.scoreboard.IScoreCriteria;
import net.minecraft.scoreboard.ScoreObjective;

public class SPacketScoreboardObjective implements Packet<INetHandlerPlayClient>
{
    private String objectiveName;
    private String objectiveValue;
    private IScoreCriteria.EnumRenderType type;
    private int action;

    public SPacketScoreboardObjective()
    {
    }

    public SPacketScoreboardObjective(ScoreObjective objective, int actionIn)
    {
        this.objectiveName = objective.getName();
        this.objectiveValue = objective.getDisplayName();
        this.type = objective.getCriteria().getRenderType();
        this.action = actionIn;
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.objectiveName = buf.readStringFromBuffer(16);
        this.action = buf.readByte();

        if (this.action == 0 || this.action == 2)
        {
            this.objectiveValue = buf.readStringFromBuffer(32);
            this.type = IScoreCriteria.EnumRenderType.func_178795_a(buf.readStringFromBuffer(16));
        }
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeString(this.objectiveName);
        buf.writeByte(this.action);

        if (this.action == 0 || this.action == 2)
        {
            buf.writeString(this.objectiveValue);
            buf.writeString(this.type.func_178796_a());
        }
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleScoreboardObjective(this);
    }

    public String getObjectiveName()
    {
        return this.objectiveName;
    }

    public String getObjectiveValue()
    {
        return this.objectiveValue;
    }

    public int getAction()
    {
        return this.action;
    }

    public IScoreCriteria.EnumRenderType getRenderType()
    {
        return this.type;
    }
}
