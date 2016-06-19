package net.minecraft.command;

import java.util.Collections;
import java.util.List;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.network.play.server.SPacketEntityStatus;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.world.GameRules;

public class CommandGameRule extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "gamerule";
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
        return "commands.gamerule.usage";
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
        GameRules gamerules = this.func_184897_a(server);
        String s = args.length > 0 ? args[0] : "";
        String s1 = args.length > 1 ? buildString(args, 1) : "";

        switch (args.length)
        {
            case 0:
                sender.addChatMessage(new TextComponentString(joinNiceString(gamerules.getRules())));
                break;

            case 1:
                if (!gamerules.hasRule(s))
                {
                    throw new CommandException("commands.gamerule.norule", new Object[] {s});
                }

                String s2 = gamerules.getString(s);
                sender.addChatMessage((new TextComponentString(s)).appendText(" = ").appendText(s2));
                sender.setCommandStat(CommandResultStats.Type.QUERY_RESULT, gamerules.getInt(s));
                break;

            default:
                if (gamerules.areSameType(s, GameRules.ValueType.BOOLEAN_VALUE) && !"true".equals(s1) && !"false".equals(s1))
                {
                    throw new CommandException("commands.generic.boolean.invalid", new Object[] {s1});
                }

                gamerules.setOrCreateGameRule(s, s1);
                func_184898_a(gamerules, s, server);
                notifyOperators(sender, this, "commands.gamerule.success", new Object[] {s, s1});
        }
    }

    public static void func_184898_a(GameRules p_184898_0_, String p_184898_1_, MinecraftServer p_184898_2_)
    {
        if ("reducedDebugInfo".equals(p_184898_1_))
        {
            byte b0 = (byte)(p_184898_0_.getBoolean(p_184898_1_) ? 22 : 23);

            for (EntityPlayerMP entityplayermp : p_184898_2_.getPlayerList().getPlayerList())
            {
                entityplayermp.playerNetServerHandler.sendPacket(new SPacketEntityStatus(entityplayermp, b0));
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        if (args.length == 1)
        {
            return getListOfStringsMatchingLastWord(args, this.func_184897_a(server).getRules());
        }
        else
        {
            if (args.length == 2)
            {
                GameRules gamerules = this.func_184897_a(server);

                if (gamerules.areSameType(args[0], GameRules.ValueType.BOOLEAN_VALUE))
                {
                    return getListOfStringsMatchingLastWord(args, new String[] {"true", "false"});
                }
            }

            return Collections.<String>emptyList();
        }
    }

    private GameRules func_184897_a(MinecraftServer p_184897_1_)
    {
        return p_184897_1_.worldServerForDimension(0).getGameRules();
    }
}
