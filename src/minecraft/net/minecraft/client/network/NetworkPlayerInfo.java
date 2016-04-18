package net.minecraft.client.network;

import com.google.common.base.Objects;
import com.google.common.collect.Maps;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.minecraft.MinecraftProfileTexture;
import com.mojang.authlib.minecraft.MinecraftProfileTexture.Type;
import java.util.Map;
import net.minecraft.client.Minecraft;
import net.minecraft.client.resources.DefaultPlayerSkin;
import net.minecraft.client.resources.SkinManager;
import net.minecraft.network.play.server.SPacketPlayerListItem;
import net.minecraft.scoreboard.ScorePlayerTeam;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.world.WorldSettings;

public class NetworkPlayerInfo
{
    /**
     * The GameProfile for the player represented by this NetworkPlayerInfo instance
     */
    private final GameProfile gameProfile;
    Map<Type, ResourceLocation> playerTextures = Maps.newEnumMap(Type.class);
    private WorldSettings.GameType gameType;

    /** Player response time to server in milliseconds */
    private int responseTime;
    private boolean playerTexturesLoaded;
    private String skinType;

    /**
     * When this is non-null, it is displayed instead of the player's real name
     */
    private ITextComponent displayName;
    private int field_178873_i;
    private int field_178870_j;
    private long field_178871_k;
    private long field_178868_l;
    private long field_178869_m;

    public NetworkPlayerInfo(GameProfile profile)
    {
        this.gameProfile = profile;
    }

    public NetworkPlayerInfo(SPacketPlayerListItem.AddPlayerData entry)
    {
        this.gameProfile = entry.getProfile();
        this.gameType = entry.getGameMode();
        this.responseTime = entry.getPing();
        this.displayName = entry.getDisplayName();
    }

    /**
     * Returns the GameProfile for the player represented by this NetworkPlayerInfo instance
     */
    public GameProfile getGameProfile()
    {
        return this.gameProfile;
    }

    public WorldSettings.GameType getGameType()
    {
        return this.gameType;
    }

    protected void setGameType(WorldSettings.GameType gameMode)
    {
        this.gameType = gameMode;
    }

    public int getResponseTime()
    {
        return this.responseTime;
    }

    protected void setResponseTime(int latency)
    {
        this.responseTime = latency;
    }

    public boolean hasLocationSkin()
    {
        return this.getLocationSkin() != null;
    }

    public String getSkinType()
    {
        return this.skinType == null ? DefaultPlayerSkin.getSkinType(this.gameProfile.getId()) : this.skinType;
    }

    public ResourceLocation getLocationSkin()
    {
        this.loadPlayerTextures();
        return (ResourceLocation)Objects.firstNonNull(this.playerTextures.get(Type.SKIN), DefaultPlayerSkin.getDefaultSkin(this.gameProfile.getId()));
    }

    public ResourceLocation getLocationCape()
    {
        this.loadPlayerTextures();
        return (ResourceLocation)this.playerTextures.get(Type.CAPE);
    }

    /**
     * Gets the special Elytra texture for the player.
     */
    public ResourceLocation getLocationElytra()
    {
        this.loadPlayerTextures();
        return (ResourceLocation)this.playerTextures.get(Type.ELYTRA);
    }

    public ScorePlayerTeam getPlayerTeam()
    {
        return Minecraft.getMinecraft().theWorld.getScoreboard().getPlayersTeam(this.getGameProfile().getName());
    }

    protected void loadPlayerTextures()
    {
        synchronized (this)
        {
            if (!this.playerTexturesLoaded)
            {
                this.playerTexturesLoaded = true;
                Minecraft.getMinecraft().getSkinManager().loadProfileTextures(this.gameProfile, new SkinManager.SkinAvailableCallback()
                {
                    public void skinAvailable(Type typeIn, ResourceLocation location, MinecraftProfileTexture profileTexture)
                    {
                        switch (typeIn)
                        {
                            case SKIN:
                                NetworkPlayerInfo.this.playerTextures.put(Type.SKIN, location);
                                NetworkPlayerInfo.this.skinType = profileTexture.getMetadata("model");

                                if (NetworkPlayerInfo.this.skinType == null)
                                {
                                    NetworkPlayerInfo.this.skinType = "default";
                                }

                                break;

                            case CAPE:
                                NetworkPlayerInfo.this.playerTextures.put(Type.CAPE, location);
                                break;

                            case ELYTRA:
                                NetworkPlayerInfo.this.playerTextures.put(Type.ELYTRA, location);
                        }
                    }
                }, true);
            }
        }
    }

    public void setDisplayName(ITextComponent displayNameIn)
    {
        this.displayName = displayNameIn;
    }

    public ITextComponent getDisplayName()
    {
        return this.displayName;
    }

    public int func_178835_l()
    {
        return this.field_178873_i;
    }

    public void func_178836_b(int p_178836_1_)
    {
        this.field_178873_i = p_178836_1_;
    }

    public int func_178860_m()
    {
        return this.field_178870_j;
    }

    public void func_178857_c(int p_178857_1_)
    {
        this.field_178870_j = p_178857_1_;
    }

    public long func_178847_n()
    {
        return this.field_178871_k;
    }

    public void func_178846_a(long p_178846_1_)
    {
        this.field_178871_k = p_178846_1_;
    }

    public long func_178858_o()
    {
        return this.field_178868_l;
    }

    public void func_178844_b(long p_178844_1_)
    {
        this.field_178868_l = p_178844_1_;
    }

    public long func_178855_p()
    {
        return this.field_178869_m;
    }

    public void func_178843_c(long p_178843_1_)
    {
        this.field_178869_m = p_178843_1_;
    }
}
