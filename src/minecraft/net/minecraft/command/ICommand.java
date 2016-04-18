package net.minecraft.command;

import java.util.List;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;

public interface ICommand extends Comparable<ICommand>
{
    /**
     * Gets the name of the command
     */
    String getCommandName();

    /**
     * Gets the usage string for the command.
     */
    String getCommandUsage(ICommandSender sender);

    List<String> getCommandAliases();

    /**
     * Callback for when the command is executed
     *  
     * @param server The Minecraft server instance
     * @param sender The source of the command invocation
     * @param args The arguments that were passed
     */
    void execute(MinecraftServer server, ICommandSender sender, String[] args) throws CommandException;

    /**
     * Check if the given ICommandSender has permission to execute this command
     *  
     * @param server The Minecraft server instance
     * @param sender The command sender who we are checking permission on
     */
    boolean checkPermission(MinecraftServer server, ICommandSender sender);

    List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos);

    /**
     * Return whether the specified command parameter index is a username parameter.
     */
    boolean isUsernameIndex(String[] args, int index);
}
