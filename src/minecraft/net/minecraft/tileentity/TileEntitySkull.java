package net.minecraft.tileentity;

import com.google.common.collect.Iterables;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.minecraft.MinecraftSessionService;
import com.mojang.authlib.properties.Property;
import java.util.UUID;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateTileEntity;
import net.minecraft.server.management.PlayerProfileCache;
import net.minecraft.util.ITickable;
import net.minecraft.util.StringUtils;

public class TileEntitySkull extends TileEntity implements ITickable
{
    private int skullType;
    private int skullRotation;
    private GameProfile playerProfile = null;
    private int field_184296_h;
    private boolean field_184297_i;
    private static PlayerProfileCache profileCache;
    private static MinecraftSessionService sessionService;

    public static void setProfileCache(PlayerProfileCache profileCacheIn)
    {
        profileCache = profileCacheIn;
    }

    public static void setSessionService(MinecraftSessionService sessionServiceIn)
    {
        sessionService = sessionServiceIn;
    }

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        compound.setByte("SkullType", (byte)(this.skullType & 255));
        compound.setByte("Rot", (byte)(this.skullRotation & 255));

        if (this.playerProfile != null)
        {
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            NBTUtil.writeGameProfile(nbttagcompound, this.playerProfile);
            compound.setTag("Owner", nbttagcompound);
        }
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        this.skullType = compound.getByte("SkullType");
        this.skullRotation = compound.getByte("Rot");

        if (this.skullType == 3)
        {
            if (compound.hasKey("Owner", 10))
            {
                this.playerProfile = NBTUtil.readGameProfileFromNBT(compound.getCompoundTag("Owner"));
            }
            else if (compound.hasKey("ExtraType", 8))
            {
                String s = compound.getString("ExtraType");

                if (!StringUtils.isNullOrEmpty(s))
                {
                    this.playerProfile = new GameProfile((UUID)null, s);
                    this.updatePlayerProfile();
                }
            }
        }
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        if (this.skullType == 5)
        {
            if (this.worldObj.isBlockPowered(this.pos))
            {
                this.field_184297_i = true;
                ++this.field_184296_h;
            }
            else
            {
                this.field_184297_i = false;
            }
        }
    }

    public float func_184295_a(float p_184295_1_)
    {
        return this.field_184297_i ? (float)this.field_184296_h + p_184295_1_ : (float)this.field_184296_h;
    }

    public GameProfile getPlayerProfile()
    {
        return this.playerProfile;
    }

    public Packet<?> getDescriptionPacket()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        this.writeToNBT(nbttagcompound);
        return new SPacketUpdateTileEntity(this.pos, 4, nbttagcompound);
    }

    public void setType(int type)
    {
        this.skullType = type;
        this.playerProfile = null;
    }

    public void setPlayerProfile(GameProfile playerProfile)
    {
        this.skullType = 3;
        this.playerProfile = playerProfile;
        this.updatePlayerProfile();
    }

    private void updatePlayerProfile()
    {
        this.playerProfile = updateGameprofile(this.playerProfile);
        this.markDirty();
    }

    public static GameProfile updateGameprofile(GameProfile input)
    {
        if (input != null && !StringUtils.isNullOrEmpty(input.getName()))
        {
            if (input.isComplete() && input.getProperties().containsKey("textures"))
            {
                return input;
            }
            else if (profileCache != null && sessionService != null)
            {
                GameProfile gameprofile = profileCache.getGameProfileForUsername(input.getName());

                if (gameprofile == null)
                {
                    return input;
                }
                else
                {
                    Property property = (Property)Iterables.getFirst(gameprofile.getProperties().get("textures"), null);

                    if (property == null)
                    {
                        gameprofile = sessionService.fillProfileProperties(gameprofile, true);
                    }

                    return gameprofile;
                }
            }
            else
            {
                return input;
            }
        }
        else
        {
            return input;
        }
    }

    public int getSkullType()
    {
        return this.skullType;
    }

    public int getSkullRotation()
    {
        return this.skullRotation;
    }

    public void setSkullRotation(int rotation)
    {
        this.skullRotation = rotation;
    }
}
