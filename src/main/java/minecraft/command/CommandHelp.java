package net.minecraft.command;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.server.MinecraftServer;
import net.minecraft.tileentity.CommandBlockBaseLogic;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.util.text.event.ClickEvent;

public class CommandHelp extends CommandBase
{
    private static String[] seargeSays = new String[] {"Yolo", "/achievement take achievement.understandCommands @p", "Ask for help on twitter", "/deop @p", "Scoreboard deleted, commands blocked", "Contact helpdesk for help", "/testfornoob @p", "/trigger warning", "Oh my god, it\'s full of stats", "/kill @p[name=!Searge]", "Have you tried turning it off and on again?", "Sorry, no help today"};
    private Random rand = new Random();

    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "help";
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
        return "commands.help.usage";
    }

    public List<String> getCommandAliases()
    {
        return Arrays.<String>asList(new String[] {"?"});
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
        if (sender instanceof CommandBlockBaseLogic)
        {
            sender.addChatMessage((new TextComponentString("Searge says: ")).appendText(seargeSays[this.rand.nextInt(seargeSays.length) % seargeSays.length]));
        }
        else
        {
            List<ICommand> list = this.func_184900_a(sender, server);
            int i = 7;
            int j = (list.size() - 1) / 7;
            int k = 0;

            try
            {
                k = args.length == 0 ? 0 : parseInt(args[0], 1, j + 1) - 1;
            }
            catch (NumberInvalidException numberinvalidexception)
            {
                Map<String, ICommand> map = this.func_184899_a(server);
                ICommand icommand = (ICommand)map.get(args[0]);

                if (icommand != null)
                {
                    throw new WrongUsageException(icommand.getCommandUsage(sender), new Object[0]);
                }

                if (MathHelper.parseIntWithDefault(args[0], -1) != -1)
                {
                    throw numberinvalidexception;
                }

                throw new CommandNotFoundException();
            }

            int l = Math.min((k + 1) * 7, list.size());
            TextComponentTranslation textcomponenttranslation1 = new TextComponentTranslation("commands.help.header", new Object[] {Integer.valueOf(k + 1), Integer.valueOf(j + 1)});
            textcomponenttranslation1.getChatStyle().setColor(TextFormatting.DARK_GREEN);
            sender.addChatMessage(textcomponenttranslation1);

            for (int i1 = k * 7; i1 < l; ++i1)
            {
                ICommand icommand1 = (ICommand)list.get(i1);
                TextComponentTranslation textcomponenttranslation = new TextComponentTranslation(icommand1.getCommandUsage(sender), new Object[0]);
                textcomponenttranslation.getChatStyle().setChatClickEvent(new ClickEvent(ClickEvent.Action.SUGGEST_COMMAND, "/" + icommand1.getCommandName() + " "));
                sender.addChatMessage(textcomponenttranslation);
            }

            if (k == 0 && sender instanceof EntityPlayer)
            {
                TextComponentTranslation textcomponenttranslation2 = new TextComponentTranslation("commands.help.footer", new Object[0]);
                textcomponenttranslation2.getChatStyle().setColor(TextFormatting.GREEN);
                sender.addChatMessage(textcomponenttranslation2);
            }
        }
    }

    protected List<ICommand> func_184900_a(ICommandSender p_184900_1_, MinecraftServer p_184900_2_)
    {
        List<ICommand> list = p_184900_2_.getCommandManager().getPossibleCommands(p_184900_1_);
        Collections.sort(list);
        return list;
    }

    protected Map<String, ICommand> func_184899_a(MinecraftServer p_184899_1_)
    {
        return p_184899_1_.getCommandManager().getCommands();
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        if (args.length == 1)
        {
            Set<String> set = this.func_184899_a(server).keySet();
            return getListOfStringsMatchingLastWord(args, (String[])set.toArray(new String[set.size()]));
        }
        else
        {
            return Collections.<String>emptyList();
        }
    }
}
