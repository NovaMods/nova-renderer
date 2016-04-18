package net.minecraft.command.server;

import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.ICommandSender;
import net.minecraft.server.MinecraftServer;

public class CommandStop extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "stop";
    }

    /**
     * Gets the usage string for the command.
     */
    public String getCommandUsage(ICommandSender sender)
    {
        return "commands.stop.usage";
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
        if (server.worldServers != null)
        {
            notifyOperators(sender, this, "commands.stop.start", new Object[0]);
        }

        server.initiateShutdown();
    }
}
