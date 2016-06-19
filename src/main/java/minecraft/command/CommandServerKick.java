package net.minecraft.command;

import java.util.Collections;
import java.util.List;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;

public class CommandServerKick extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "kick";
    }

    /**
     * Return the required permission level for this command.
     */
    public int getRequiredPermissionLevel()
    {
        return 3;
    }

    /**
     * Gets the usage string for the command.
     */
    public String getCommandUsage(ICommandSender sender)
    {
        return "commands.kick.usage";
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
        if (args.length > 0 && args[0].length() > 1)
        {
            EntityPlayerMP entityplayermp = server.getPlayerList().getPlayerByUsername(args[0]);
            String s = "Kicked by an operator.";
            boolean flag = false;

            if (entityplayermp == null)
            {
                throw new PlayerNotFoundException();
            }
            else
            {
                if (args.length >= 2)
                {
                    s = getChatComponentFromNthArg(sender, args, 1).getUnformattedText();
                    flag = true;
                }

                entityplayermp.playerNetServerHandler.kickPlayerFromServer(s);

                if (flag)
                {
                    notifyOperators(sender, this, "commands.kick.success.reason", new Object[] {entityplayermp.getName(), s});
                }
                else
                {
                    notifyOperators(sender, this, "commands.kick.success", new Object[] {entityplayermp.getName()});
                }
            }
        }
        else
        {
            throw new WrongUsageException("commands.kick.usage", new Object[0]);
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length >= 1 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : Collections.<String>emptyList();
    }
}
