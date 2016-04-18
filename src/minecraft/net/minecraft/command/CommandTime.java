package net.minecraft.command;

import java.util.Collections;
import java.util.List;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.WorldServer;

public class CommandTime extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "time";
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
        return "commands.time.usage";
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
        if (args.length > 1)
        {
            if (args[0].equals("set"))
            {
                int i1;

                if (args[1].equals("day"))
                {
                    i1 = 1000;
                }
                else if (args[1].equals("night"))
                {
                    i1 = 13000;
                }
                else
                {
                    i1 = parseInt(args[1], 0);
                }

                this.func_184929_a(server, i1);
                notifyOperators(sender, this, "commands.time.set", new Object[] {Integer.valueOf(i1)});
                return;
            }

            if (args[0].equals("add"))
            {
                int l = parseInt(args[1], 0);
                this.func_184928_b(server, l);
                notifyOperators(sender, this, "commands.time.added", new Object[] {Integer.valueOf(l)});
                return;
            }

            if (args[0].equals("query"))
            {
                if (args[1].equals("daytime"))
                {
                    int k = (int)(sender.getEntityWorld().getWorldTime() % 24000L);
                    sender.setCommandStat(CommandResultStats.Type.QUERY_RESULT, k);
                    notifyOperators(sender, this, "commands.time.query", new Object[] {Integer.valueOf(k)});
                    return;
                }

                if (args[1].equals("day"))
                {
                    int j = (int)(sender.getEntityWorld().getWorldTime() / 24000L % 2147483647L);
                    sender.setCommandStat(CommandResultStats.Type.QUERY_RESULT, j);
                    notifyOperators(sender, this, "commands.time.query", new Object[] {Integer.valueOf(j)});
                    return;
                }

                if (args[1].equals("gametime"))
                {
                    int i = (int)(sender.getEntityWorld().getTotalWorldTime() % 2147483647L);
                    sender.setCommandStat(CommandResultStats.Type.QUERY_RESULT, i);
                    notifyOperators(sender, this, "commands.time.query", new Object[] {Integer.valueOf(i)});
                    return;
                }
            }
        }

        throw new WrongUsageException("commands.time.usage", new Object[0]);
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, new String[] {"set", "add", "query"}): (args.length == 2 && args[0].equals("set") ? getListOfStringsMatchingLastWord(args, new String[] {"day", "night"}): (args.length == 2 && args[0].equals("query") ? getListOfStringsMatchingLastWord(args, new String[] {"daytime", "gametime", "day"}): Collections.<String>emptyList()));
    }

    protected void func_184929_a(MinecraftServer p_184929_1_, int p_184929_2_)
    {
        for (int i = 0; i < p_184929_1_.worldServers.length; ++i)
        {
            p_184929_1_.worldServers[i].setWorldTime((long)p_184929_2_);
        }
    }

    protected void func_184928_b(MinecraftServer p_184928_1_, int p_184928_2_)
    {
        for (int i = 0; i < p_184928_1_.worldServers.length; ++i)
        {
            WorldServer worldserver = p_184928_1_.worldServers[i];
            worldserver.setWorldTime(worldserver.getWorldTime() + (long)p_184928_2_);
        }
    }
}
