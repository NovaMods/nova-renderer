package net.minecraft.command;

import com.google.gson.JsonParseException;
import java.util.Collections;
import java.util.List;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.network.play.server.SPacketTitle;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class CommandTitle extends CommandBase
{
    private static final Logger LOGGER = LogManager.getLogger();

    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "title";
    }

    /**
     * Return the required permission level for this command.
     */
    public int getRequiredPermissionLevel()
    {
        return 2;
    }

    /**
     * Gets the usage string for the command.
     */
    public String getCommandUsage(ICommandSender sender)
    {
        return "commands.title.usage";
    }

    /**
     * Callback for when the command is executed
     *  
     * @param server The Minecraft server instance
     * @param sender The source of the command invocation
     * @param args The arguments that were passed
     */
    public void execute(MinecraftServer server, ICommandSender sender, String[] args) throws CommandException
    {
        if (args.length < 2)
        {
            throw new WrongUsageException("commands.title.usage", new Object[0]);
        }
        else
        {
            if (args.length < 3)
            {
                if ("title".equals(args[1]) || "subtitle".equals(args[1]))
                {
                    throw new WrongUsageException("commands.title.usage.title", new Object[0]);
                }

                if ("times".equals(args[1]))
                {
                    throw new WrongUsageException("commands.title.usage.times", new Object[0]);
                }
            }

            EntityPlayerMP entityplayermp = getPlayer(server, sender, args[0]);
            SPacketTitle.Type spackettitle$type = SPacketTitle.Type.byName(args[1]);

            if (spackettitle$type != SPacketTitle.Type.CLEAR && spackettitle$type != SPacketTitle.Type.RESET)
            {
                if (spackettitle$type == SPacketTitle.Type.TIMES)
                {
                    if (args.length != 5)
                    {
                        throw new WrongUsageException("commands.title.usage", new Object[0]);
                    }
                    else
                    {
                        int i = parseInt(args[2]);
                        int j = parseInt(args[3]);
                        int k = parseInt(args[4]);
                        SPacketTitle spackettitle2 = new SPacketTitle(i, j, k);
                        entityplayermp.playerNetServerHandler.sendPacket(spackettitle2);
                        notifyOperators(sender, this, "commands.title.success", new Object[0]);
                    }
                }
                else if (args.length < 3)
                {
                    throw new WrongUsageException("commands.title.usage", new Object[0]);
                }
                else
                {
                    String s = buildString(args, 2);
                    ITextComponent itextcomponent;

                    try
                    {
                        itextcomponent = ITextComponent.Serializer.jsonToComponent(s);
                    }
                    catch (JsonParseException jsonparseexception)
                    {
                        throw func_184889_a(jsonparseexception);
                    }

                    SPacketTitle spackettitle1 = new SPacketTitle(spackettitle$type, TextComponentUtils.processComponent(sender, itextcomponent, entityplayermp));
                    entityplayermp.playerNetServerHandler.sendPacket(spackettitle1);
                    notifyOperators(sender, this, "commands.title.success", new Object[0]);
                }
            }
            else if (args.length != 2)
            {
                throw new WrongUsageException("commands.title.usage", new Object[0]);
            }
            else
            {
                SPacketTitle spackettitle = new SPacketTitle(spackettitle$type, (ITextComponent)null);
                entityplayermp.playerNetServerHandler.sendPacket(spackettitle);
                notifyOperators(sender, this, "commands.title.success", new Object[0]);
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : (args.length == 2 ? getListOfStringsMatchingLastWord(args, SPacketTitle.Type.getNames()) : Collections.<String>emptyList());
    }

    /**
     * Return whether the specified command parameter index is a username parameter.
     */
    public boolean isUsernameIndex(String[] args, int index)
    {
        return index == 0;
    }
}
