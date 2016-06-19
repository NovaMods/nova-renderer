package net.minecraft.command.server;

import java.util.Collections;
import java.util.List;
import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.ICommandSender;
import net.minecraft.command.WrongUsageException;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentTranslation;

public class CommandBroadcast extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "say";
    }

    /**
     * Return the required permission level for this command.
     */
    public int getRequiredPermissionLevel()
    {
        return 1;
    }

    /**
     * Gets the usage string for the command.
     */
    public String getCommandUsage(ICommandSender sender)
    {
        return "commands.say.usage";
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
        if (args.length > 0 && args[0].length() > 0)
        {
            ITextComponent itextcomponent = getChatComponentFromNthArg(sender, args, 0, true);
            server.getPlayerList().sendChatMsg(new TextComponentTranslation("chat.type.announcement", new Object[] {sender.getDisplayName(), itextcomponent}));
        }
        else
        {
            throw new WrongUsageException("commands.say.usage", new Object[0]);
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length >= 1 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : Collections.<String>emptyList();
    }
}
