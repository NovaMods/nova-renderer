package net.minecraft.command;

import net.minecraft.server.MinecraftServer;
import net.minecraft.world.storage.WorldInfo;

public class CommandToggleDownfall extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "toggledownfall";
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
        return "commands.downfall.usage";
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
        this.func_184930_a(server);
        notifyOperators(sender, this, "commands.downfall.success", new Object[0]);
    }

    protected void func_184930_a(MinecraftServer p_184930_1_)
    {
        WorldInfo worldinfo = p_184930_1_.worldServers[0].getWorldInfo();
        worldinfo.setRaining(!worldinfo.isRaining());
    }
}
