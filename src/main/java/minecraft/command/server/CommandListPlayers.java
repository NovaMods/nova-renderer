package net.minecraft.command.server;

import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.CommandResultStats;
import net.minecraft.command.ICommandSender;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentTranslation;

public class CommandListPlayers extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "list";
    }

    /**
     * Return the required permission level for this command.
     */
    public int getRequiredPermissionLevel()
    {
        return 0;
    }

    /**
     * Gets the usage string for the command.
     */
    public String getCommandUsage(ICommandSender sender)
    {
        return "commands.players.usage";
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
        int i = server.getCurrentPlayerCount();
        sender.addChatMessage(new TextComponentTranslation("commands.players.list", new Object[] {Integer.valueOf(i), Integer.valueOf(server.getMaxPlayers())}));
        sender.addChatMessage(new TextComponentString(server.getPlayerList().func_181058_b(args.length > 0 && "uuids".equalsIgnoreCase(args[0]))));
        sender.setCommandStat(CommandResultStats.Type.QUERY_RESULT, i);
    }
}
