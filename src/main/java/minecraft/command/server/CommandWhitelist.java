package net.minecraft.command.server;

import com.mojang.authlib.GameProfile;
import java.util.Collections;
import java.util.List;
import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.ICommandSender;
import net.minecraft.command.WrongUsageException;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentTranslation;

public class CommandWhitelist extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "whitelist";
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
        return "commands.whitelist.usage";
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
        if (args.length < 1)
        {
            throw new WrongUsageException("commands.whitelist.usage", new Object[0]);
        }
        else
        {
            if (args[0].equals("on"))
            {
                server.getPlayerList().setWhiteListEnabled(true);
                notifyOperators(sender, this, "commands.whitelist.enabled", new Object[0]);
            }
            else if (args[0].equals("off"))
            {
                server.getPlayerList().setWhiteListEnabled(false);
                notifyOperators(sender, this, "commands.whitelist.disabled", new Object[0]);
            }
            else if (args[0].equals("list"))
            {
                sender.addChatMessage(new TextComponentTranslation("commands.whitelist.list", new Object[] {Integer.valueOf(server.getPlayerList().getWhitelistedPlayerNames().length), Integer.valueOf(server.getPlayerList().getAvailablePlayerDat().length)}));
                String[] astring = server.getPlayerList().getWhitelistedPlayerNames();
                sender.addChatMessage(new TextComponentString(joinNiceString(astring)));
            }
            else if (args[0].equals("add"))
            {
                if (args.length < 2)
                {
                    throw new WrongUsageException("commands.whitelist.add.usage", new Object[0]);
                }

                GameProfile gameprofile = server.getPlayerProfileCache().getGameProfileForUsername(args[1]);

                if (gameprofile == null)
                {
                    throw new CommandException("commands.whitelist.add.failed", new Object[] {args[1]});
                }

                server.getPlayerList().addWhitelistedPlayer(gameprofile);
                notifyOperators(sender, this, "commands.whitelist.add.success", new Object[] {args[1]});
            }
            else if (args[0].equals("remove"))
            {
                if (args.length < 2)
                {
                    throw new WrongUsageException("commands.whitelist.remove.usage", new Object[0]);
                }

                GameProfile gameprofile1 = server.getPlayerList().getWhitelistedPlayers().getBannedProfile(args[1]);

                if (gameprofile1 == null)
                {
                    throw new CommandException("commands.whitelist.remove.failed", new Object[] {args[1]});
                }

                server.getPlayerList().removePlayerFromWhitelist(gameprofile1);
                notifyOperators(sender, this, "commands.whitelist.remove.success", new Object[] {args[1]});
            }
            else if (args[0].equals("reload"))
            {
                server.getPlayerList().reloadWhitelist();
                notifyOperators(sender, this, "commands.whitelist.reloaded", new Object[0]);
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        if (args.length == 1)
        {
            return getListOfStringsMatchingLastWord(args, new String[] {"on", "off", "list", "add", "remove", "reload"});
        }
        else
        {
            if (args.length == 2)
            {
                if (args[0].equals("remove"))
                {
                    return getListOfStringsMatchingLastWord(args, server.getPlayerList().getWhitelistedPlayerNames());
                }

                if (args[0].equals("add"))
                {
                    return getListOfStringsMatchingLastWord(args, server.getPlayerProfileCache().getUsernames());
                }
            }

            return Collections.<String>emptyList();
        }
    }
}
