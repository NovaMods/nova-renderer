package net.minecraft.tileentity;

import net.minecraft.command.CommandException;
import net.minecraft.command.CommandResultStats;
import net.minecraft.command.ICommandSender;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateSign;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.Style;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentUtils;
import net.minecraft.util.text.event.ClickEvent;
import net.minecraft.world.World;

public class TileEntitySign extends TileEntity
{
    public final ITextComponent[] signText = new ITextComponent[] {new TextComponentString(""), new TextComponentString(""), new TextComponentString(""), new TextComponentString("")};

    /**
     * The index of the line currently being edited. Only used on client side, but defined on both. Note this is only
     * really used when the > < are going to be visible.
     */
    public int lineBeingEdited = -1;
    private boolean isEditable = true;
    private EntityPlayer player;
    private final CommandResultStats stats = new CommandResultStats();

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);

        for (int i = 0; i < 4; ++i)
        {
            String s = ITextComponent.Serializer.componentToJson(this.signText[i]);
            compound.setString("Text" + (i + 1), s);
        }

        this.stats.writeStatsToNBT(compound);
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        this.isEditable = false;
        super.readFromNBT(compound);
        ICommandSender icommandsender = new ICommandSender()
        {
            public String getName()
            {
                return "Sign";
            }
            public ITextComponent getDisplayName()
            {
                return new TextComponentString(this.getName());
            }
            public void addChatMessage(ITextComponent component)
            {
            }
            public boolean canCommandSenderUseCommand(int permLevel, String commandName)
            {
                return true;
            }
            public BlockPos getPosition()
            {
                return TileEntitySign.this.pos;
            }
            public Vec3d getPositionVector()
            {
                return new Vec3d((double)TileEntitySign.this.pos.getX() + 0.5D, (double)TileEntitySign.this.pos.getY() + 0.5D, (double)TileEntitySign.this.pos.getZ() + 0.5D);
            }
            public World getEntityWorld()
            {
                return TileEntitySign.this.worldObj;
            }
            public Entity getCommandSenderEntity()
            {
                return null;
            }
            public boolean sendCommandFeedback()
            {
                return false;
            }
            public void setCommandStat(CommandResultStats.Type type, int amount)
            {
            }
            public MinecraftServer getServer()
            {
                return TileEntitySign.this.worldObj.getMinecraftServer();
            }
        };

        for (int i = 0; i < 4; ++i)
        {
            String s = compound.getString("Text" + (i + 1));
            ITextComponent itextcomponent = ITextComponent.Serializer.jsonToComponent(s);

            try
            {
                this.signText[i] = TextComponentUtils.processComponent(icommandsender, itextcomponent, (Entity)null);
            }
            catch (CommandException var7)
            {
                this.signText[i] = itextcomponent;
            }
        }

        this.stats.readStatsFromNBT(compound);
    }

    public Packet<?> getDescriptionPacket()
    {
        ITextComponent[] aitextcomponent = new ITextComponent[4];
        System.arraycopy(this.signText, 0, aitextcomponent, 0, 4);
        return new SPacketUpdateSign(this.worldObj, this.pos, aitextcomponent);
    }

    public boolean func_183000_F()
    {
        return true;
    }

    public boolean getIsEditable()
    {
        return this.isEditable;
    }

    /**
     * Sets the sign's isEditable flag to the specified parameter.
     */
    public void setEditable(boolean isEditableIn)
    {
        this.isEditable = isEditableIn;

        if (!isEditableIn)
        {
            this.player = null;
        }
    }

    public void setPlayer(EntityPlayer playerIn)
    {
        this.player = playerIn;
    }

    public EntityPlayer getPlayer()
    {
        return this.player;
    }

    public boolean executeCommand(final EntityPlayer playerIn)
    {
        ICommandSender icommandsender = new ICommandSender()
        {
            public String getName()
            {
                return playerIn.getName();
            }
            public ITextComponent getDisplayName()
            {
                return playerIn.getDisplayName();
            }
            public void addChatMessage(ITextComponent component)
            {
            }
            public boolean canCommandSenderUseCommand(int permLevel, String commandName)
            {
                return permLevel <= 2;
            }
            public BlockPos getPosition()
            {
                return TileEntitySign.this.pos;
            }
            public Vec3d getPositionVector()
            {
                return new Vec3d((double)TileEntitySign.this.pos.getX() + 0.5D, (double)TileEntitySign.this.pos.getY() + 0.5D, (double)TileEntitySign.this.pos.getZ() + 0.5D);
            }
            public World getEntityWorld()
            {
                return playerIn.getEntityWorld();
            }
            public Entity getCommandSenderEntity()
            {
                return playerIn;
            }
            public boolean sendCommandFeedback()
            {
                return false;
            }
            public void setCommandStat(CommandResultStats.Type type, int amount)
            {
                if (TileEntitySign.this.worldObj != null && !TileEntitySign.this.worldObj.isRemote)
                {
                    TileEntitySign.this.stats.func_184932_a(TileEntitySign.this.worldObj.getMinecraftServer(), this, type, amount);
                }
            }
            public MinecraftServer getServer()
            {
                return playerIn.getServer();
            }
        };

        for (int i = 0; i < this.signText.length; ++i)
        {
            Style style = this.signText[i] == null ? null : this.signText[i].getChatStyle();

            if (style != null && style.getChatClickEvent() != null)
            {
                ClickEvent clickevent = style.getChatClickEvent();

                if (clickevent.getAction() == ClickEvent.Action.RUN_COMMAND)
                {
                    playerIn.getServer().getCommandManager().executeCommand(icommandsender, clickevent.getValue());
                }
            }
        }

        return true;
    }

    public CommandResultStats getStats()
    {
        return this.stats;
    }
}
