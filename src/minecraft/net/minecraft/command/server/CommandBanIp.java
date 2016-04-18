package net.minecraft.command.server;

import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.ICommandSender;
import net.minecraft.command.PlayerNotFoundException;
import net.minecraft.command.WrongUsageException;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.server.MinecraftServer;
import net.minecraft.server.management.UserListIPBansEntry;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;

public class CommandBanIp extends CommandBase
{
    public static final Pattern field_147211_a = Pattern.compile("^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\.([01]?\\d\\d?|2[0-4]\\d|25[0-5])$");

    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "ban-ip";
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
        return "commands.banip.usage";
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
        if (args.length >= 1 && args[0].length() > 1)
        {
            ITextComponent itextcomponent = args.length >= 2 ? getChatComponentFromNthArg(sender, args, 1) : null;
            Matcher matcher = field_147211_a.matcher(args[0]);

            if (matcher.matches())
            {
                this.func_184892_a(server, sender, args[0], itextcomponent == null ? null : itextcomponent.getUnformattedText());
            }
            else
            {
                EntityPlayerMP entityplayermp = server.getPlayerList().getPlayerByUsername(args[0]);

                if (entityplayermp == null)
                {
                    throw new PlayerNotFoundException("commands.banip.invalid", new Object[0]);
                }

                this.func_184892_a(server, sender, entityplayermp.getPlayerIP(), itextcomponent == null ? null : itextcomponent.getUnformattedText());
            }
        }
        else
        {
            throw new WrongUsageException("commands.banip.usage", new Object[0]);
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : Collections.<String>emptyList();
    }

    protected void func_184892_a(MinecraftServer p_184892_1_, ICommandSender p_184892_2_, String p_184892_3_, String p_184892_4_)
    {
        UserListIPBansEntry userlistipbansentry = new UserListIPBansEntry(p_184892_3_, (Date)null, p_184892_2_.getName(), (Date)null, p_184892_4_);
        p_184892_1_.getPlayerList().getBannedIPs().addEntry(userlistipbansentry);
        List<EntityPlayerMP> list = p_184892_1_.getPlayerList().getPlayersMatchingAddress(p_184892_3_);
        String[] astring = new String[list.size()];
        int i = 0;

        for (EntityPlayerMP entityplayermp : list)
        {
            entityplayermp.playerNetServerHandler.kickPlayerFromServer("You have been IP banned.");
            astring[i++] = entityplayermp.getName();
        }

        if (list.isEmpty())
        {
            notifyOperators(p_184892_2_, this, "commands.banip.success", new Object[] {p_184892_3_});
        }
        else
        {
            notifyOperators(p_184892_2_, this, "commands.banip.success.players", new Object[] {p_184892_3_, joinNiceString(astring)});
        }
    }
}
