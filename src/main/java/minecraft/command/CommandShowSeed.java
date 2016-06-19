package net.minecraft.command;

import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.world.World;

public class CommandShowSeed extends CommandBase
{
    /**
     * Check if the given ICommandSender has permission to execute this command
     *  
     * @param server The Minecraft server instance
     * @param sender The command sender who we are checking permission on
     */
    public boolean checkPermission(MinecraftServer server, ICommandSender sender)
    {
        return server.isSinglePlayer() || super.checkPermission(server, sender);
    }

    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "seed";
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
        return "commands.seed.usage";
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
        World world = (World)(sender instanceof EntityPlayer ? ((EntityPlayer)sender).worldObj : server.worldServerForDimension(0));
        sender.addChatMessage(new TextComponentTranslation("commands.seed.success", new Object[] {Long.valueOf(world.getSeed())}));
    }
}
