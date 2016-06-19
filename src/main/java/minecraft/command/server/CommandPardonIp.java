package net.minecraft.command.server;

import java.util.Collections;
import java.util.List;
import java.util.regex.Matcher;
import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.ICommandSender;
import net.minecraft.command.SyntaxErrorException;
import net.minecraft.command.WrongUsageException;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;

public class CommandPardonIp extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "pardon-ip";
    }

    /**
     * Return the required permission level for this command.
     */
    public int getRequiredPermissionLevel()
    {
        return 3;
    }

    /**
     * Check if the given ICommandSender has permission to execute this command
     *  
     * @param server The Minecraft server instance
     * @param sender The command sender who we are checking permission on
     */
    public boolean checkPermission(MinecraftServer server, ICommandSender sender)
    {
        return server.getPlayerList().getBannedIPs().isLanServer() && super.checkPermission(server, sender);
    }

    /**
     * Gets the usage string for the command.
     */
    public String getCommandUsage(ICommandSender sender)
    {
        return "commands.unbanip.usage";
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
        if (args.length == 1 && args[0].length() > 1)
        {
            Matcher matcher = CommandBanIp.field_147211_a.matcher(args[0]);

            if (matcher.matches())
            {
                server.getPlayerList().getBannedIPs().removeEntry(args[0]);
                notifyOperators(sender, this, "commands.unbanip.success", new Object[] {args[0]});
            }
            else
            {
                throw new SyntaxErrorException("commands.unbanip.invalid", new Object[0]);
            }
        }
        else
        {
            throw new WrongUsageException("commands.unbanip.usage", new Object[0]);
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, server.getPlayerList().getBannedIPs().getKeys()) : Collections.<String>emptyList();
    }
}
