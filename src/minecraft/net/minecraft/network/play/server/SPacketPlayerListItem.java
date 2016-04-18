package net.minecraft.network.play.server;

import com.google.common.base.Objects;
import com.google.common.collect.Lists;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.properties.Property;
import java.io.IOException;
import java.util.List;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.world.WorldSettings;

public class SPacketPlayerListItem implements Packet<INetHandlerPlayClient>
{
    private SPacketPlayerListItem.Action action;
    private final List<SPacketPlayerListItem.AddPlayerData> players = Lists.<SPacketPlayerListItem.AddPlayerData>newArrayList();

    public SPacketPlayerListItem()
    {
    }

    public SPacketPlayerListItem(SPacketPlayerListItem.Action actionIn, EntityPlayerMP... playersIn)
    {
        this.action = actionIn;

        for (EntityPlayerMP entityplayermp : playersIn)
        {
            this.players.add(new SPacketPlayerListItem.AddPlayerData(entityplayermp.getGameProfile(), entityplayermp.ping, entityplayermp.theItemInWorldManager.getGameType(), entityplayermp.getTabListDisplayName()));
        }
    }

    public SPacketPlayerListItem(SPacketPlayerListItem.Action actionIn, Iterable<EntityPlayerMP> playersIn)
    {
        this.action = actionIn;

        for (EntityPlayerMP entityplayermp : playersIn)
        {
            this.players.add(new SPacketPlayerListItem.AddPlayerData(entityplayermp.getGameProfile(), entityplayermp.ping, entityplayermp.theItemInWorldManager.getGameType(), entityplayermp.getTabListDisplayName()));
        }
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.action = (SPacketPlayerListItem.Action)buf.readEnumValue(SPacketPlayerListItem.Action.class);
        int i = buf.readVarIntFromBuffer();

        for (int j = 0; j < i; ++j)
        {
            GameProfile gameprofile = null;
            int k = 0;
            WorldSettings.GameType worldsettings$gametype = null;
            ITextComponent itextcomponent = null;

            switch (this.action)
            {
                case ADD_PLAYER:
                    gameprofile = new GameProfile(buf.readUuid(), buf.readStringFromBuffer(16));
                    int l = buf.readVarIntFromBuffer();
                    int i1 = 0;

                    for (; i1 < l; ++i1)
                    {
                        String s = buf.readStringFromBuffer(32767);
                        String s1 = buf.readStringFromBuffer(32767);

                        if (buf.readBoolean())
                        {
                            gameprofile.getProperties().put(s, new Property(s, s1, buf.readStringFromBuffer(32767)));
                        }
                        else
                        {
                            gameprofile.getProperties().put(s, new Property(s, s1));
                        }
                    }

                    worldsettings$gametype = WorldSettings.GameType.getByID(buf.readVarIntFromBuffer());
                    k = buf.readVarIntFromBuffer();

                    if (buf.readBoolean())
                    {
                        itextcomponent = buf.readChatComponent();
                    }

                    break;

                case UPDATE_GAME_MODE:
                    gameprofile = new GameProfile(buf.readUuid(), (String)null);
                    worldsettings$gametype = WorldSettings.GameType.getByID(buf.readVarIntFromBuffer());
                    break;

                case UPDATE_LATENCY:
                    gameprofile = new GameProfile(buf.readUuid(), (String)null);
                    k = buf.readVarIntFromBuffer();
                    break;

                case UPDATE_DISPLAY_NAME:
                    gameprofile = new GameProfile(buf.readUuid(), (String)null);

                    if (buf.readBoolean())
                    {
                        itextcomponent = buf.readChatComponent();
                    }

                    break;

                case REMOVE_PLAYER:
                    gameprofile = new GameProfile(buf.readUuid(), (String)null);
            }

            this.players.add(new SPacketPlayerListItem.AddPlayerData(gameprofile, k, worldsettings$gametype, itextcomponent));
        }
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeEnumValue(this.action);
        buf.writeVarIntToBuffer(this.players.size());

        for (SPacketPlayerListItem.AddPlayerData spacketplayerlistitem$addplayerdata : this.players)
        {
            switch (this.action)
            {
                case ADD_PLAYER:
                    buf.writeUuid(spacketplayerlistitem$addplayerdata.getProfile().getId());
                    buf.writeString(spacketplayerlistitem$addplayerdata.getProfile().getName());
                    buf.writeVarIntToBuffer(spacketplayerlistitem$addplayerdata.getProfile().getProperties().size());

                    for (Property property : spacketplayerlistitem$addplayerdata.getProfile().getProperties().values())
                    {
                        buf.writeString(property.getName());
                        buf.writeString(property.getValue());

                        if (property.hasSignature())
                        {
                            buf.writeBoolean(true);
                            buf.writeString(property.getSignature());
                        }
                        else
                        {
                            buf.writeBoolean(false);
                        }
                    }

                    buf.writeVarIntToBuffer(spacketplayerlistitem$addplayerdata.getGameMode().getID());
                    buf.writeVarIntToBuffer(spacketplayerlistitem$addplayerdata.getPing());

                    if (spacketplayerlistitem$addplayerdata.getDisplayName() == null)
                    {
                        buf.writeBoolean(false);
                    }
                    else
                    {
                        buf.writeBoolean(true);
                        buf.writeChatComponent(spacketplayerlistitem$addplayerdata.getDisplayName());
                    }

                    break;

                case UPDATE_GAME_MODE:
                    buf.writeUuid(spacketplayerlistitem$addplayerdata.getProfile().getId());
                    buf.writeVarIntToBuffer(spacketplayerlistitem$addplayerdata.getGameMode().getID());
                    break;

                case UPDATE_LATENCY:
                    buf.writeUuid(spacketplayerlistitem$addplayerdata.getProfile().getId());
                    buf.writeVarIntToBuffer(spacketplayerlistitem$addplayerdata.getPing());
                    break;

                case UPDATE_DISPLAY_NAME:
                    buf.writeUuid(spacketplayerlistitem$addplayerdata.getProfile().getId());

                    if (spacketplayerlistitem$addplayerdata.getDisplayName() == null)
                    {
                        buf.writeBoolean(false);
                    }
                    else
                    {
                        buf.writeBoolean(true);
                        buf.writeChatComponent(spacketplayerlistitem$addplayerdata.getDisplayName());
                    }

                    break;

                case REMOVE_PLAYER:
                    buf.writeUuid(spacketplayerlistitem$addplayerdata.getProfile().getId());
            }
        }
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handlePlayerListItem(this);
    }

    public List<SPacketPlayerListItem.AddPlayerData> getEntries()
    {
        return this.players;
    }

    public SPacketPlayerListItem.Action getAction()
    {
        return this.action;
    }

    public String toString()
    {
        return Objects.toStringHelper(this).add("action", this.action).add("entries", this.players).toString();
    }

    public static enum Action
    {
        ADD_PLAYER,
        UPDATE_GAME_MODE,
        UPDATE_LATENCY,
        UPDATE_DISPLAY_NAME,
        REMOVE_PLAYER;
    }

    public class AddPlayerData
    {
        private final int ping;
        private final WorldSettings.GameType gamemode;
        private final GameProfile profile;
        private final ITextComponent displayName;

        public AddPlayerData(GameProfile profileIn, int latencyIn, WorldSettings.GameType gameModeIn, ITextComponent displayNameIn)
        {
            this.profile = profileIn;
            this.ping = latencyIn;
            this.gamemode = gameModeIn;
            this.displayName = displayNameIn;
        }

        public GameProfile getProfile()
        {
            return this.profile;
        }

        public int getPing()
        {
            return this.ping;
        }

        public WorldSettings.GameType getGameMode()
        {
            return this.gamemode;
        }

        public ITextComponent getDisplayName()
        {
            return this.displayName;
        }

        public String toString()
        {
            return Objects.toStringHelper(this).add("latency", this.ping).add("gameMode", this.gamemode).add("profile", this.profile).add("displayName", this.displayName == null ? null : ITextComponent.Serializer.componentToJson(this.displayName)).toString();
        }
    }
}
