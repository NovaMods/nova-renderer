package net.minecraft.command;

import java.io.File;
import java.io.FileWriter;
import java.text.SimpleDateFormat;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import net.minecraft.profiler.Profiler;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class CommandDebug extends CommandBase
{
    private static final Logger logger = LogManager.getLogger();

    /** The time (in milliseconds) that profiling was started */
    private long profileStartTime;

    /** The tick number that profiling was started on */
    private int profileStartTick;

    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "debug";
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
        return "commands.debug.usage";
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
            throw new WrongUsageException("commands.debug.usage", new Object[0]);
        }
        else
        {
            if (args[0].equals("start"))
            {
                if (args.length != 1)
                {
                    throw new WrongUsageException("commands.debug.usage", new Object[0]);
                }

                notifyOperators(sender, this, "commands.debug.start", new Object[0]);
                server.enableProfiling();
                this.profileStartTime = MinecraftServer.getCurrentTimeMillis();
                this.profileStartTick = server.getTickCounter();
            }
            else
            {
                if (!args[0].equals("stop"))
                {
                    throw new WrongUsageException("commands.debug.usage", new Object[0]);
                }

                if (args.length != 1)
                {
                    throw new WrongUsageException("commands.debug.usage", new Object[0]);
                }

                if (!server.theProfiler.profilingEnabled)
                {
                    throw new CommandException("commands.debug.notStarted", new Object[0]);
                }

                long i = MinecraftServer.getCurrentTimeMillis();
                int j = server.getTickCounter();
                long k = i - this.profileStartTime;
                int l = j - this.profileStartTick;
                this.func_184894_a(k, l, server);
                server.theProfiler.profilingEnabled = false;
                notifyOperators(sender, this, "commands.debug.stop", new Object[] {Float.valueOf((float)k / 1000.0F), Integer.valueOf(l)});
            }
        }
    }

    private void func_184894_a(long p_184894_1_, int p_184894_3_, MinecraftServer p_184894_4_)
    {
        File file1 = new File(p_184894_4_.getFile("debug"), "profile-results-" + (new SimpleDateFormat("yyyy-MM-dd_HH.mm.ss")).format(new Date()) + ".txt");
        file1.getParentFile().mkdirs();

        try
        {
            FileWriter filewriter = new FileWriter(file1);
            filewriter.write(this.func_184893_b(p_184894_1_, p_184894_3_, p_184894_4_));
            filewriter.close();
        }
        catch (Throwable throwable)
        {
            logger.error("Could not save profiler results to " + file1, throwable);
        }
    }

    private String func_184893_b(long p_184893_1_, int p_184893_3_, MinecraftServer p_184893_4_)
    {
        StringBuilder stringbuilder = new StringBuilder();
        stringbuilder.append("---- Minecraft Profiler Results ----\n");
        stringbuilder.append("// ");
        stringbuilder.append(getWittyComment());
        stringbuilder.append("\n\n");
        stringbuilder.append("Time span: ").append(p_184893_1_).append(" ms\n");
        stringbuilder.append("Tick span: ").append(p_184893_3_).append(" ticks\n");
        stringbuilder.append("// This is approximately ").append(String.format("%.2f", new Object[] {Float.valueOf((float)p_184893_3_ / ((float)p_184893_1_ / 1000.0F))})).append(" ticks per second. It should be ").append((int)20).append(" ticks per second\n\n");
        stringbuilder.append("--- BEGIN PROFILE DUMP ---\n\n");
        this.func_184895_a(0, "root", stringbuilder, p_184893_4_);
        stringbuilder.append("--- END PROFILE DUMP ---\n\n");
        return stringbuilder.toString();
    }

    private void func_184895_a(int p_184895_1_, String p_184895_2_, StringBuilder p_184895_3_, MinecraftServer p_184895_4_)
    {
        List<Profiler.Result> list = p_184895_4_.theProfiler.getProfilingData(p_184895_2_);

        if (list != null && list.size() >= 3)
        {
            for (int i = 1; i < list.size(); ++i)
            {
                Profiler.Result profiler$result = (Profiler.Result)list.get(i);
                p_184895_3_.append(String.format("[%02d] ", new Object[] {Integer.valueOf(p_184895_1_)}));

                for (int j = 0; j < p_184895_1_; ++j)
                {
                    p_184895_3_.append("|   ");
                }

                p_184895_3_.append(profiler$result.field_76331_c).append(" - ").append(String.format("%.2f", new Object[] {Double.valueOf(profiler$result.field_76332_a)})).append("%/").append(String.format("%.2f", new Object[] {Double.valueOf(profiler$result.field_76330_b)})).append("%\n");

                if (!profiler$result.field_76331_c.equals("unspecified"))
                {
                    try
                    {
                        this.func_184895_a(p_184895_1_ + 1, p_184895_2_ + "." + profiler$result.field_76331_c, p_184895_3_, p_184895_4_);
                    }
                    catch (Exception exception)
                    {
                        p_184895_3_.append("[[ EXCEPTION ").append((Object)exception).append(" ]]");
                    }
                }
            }
        }
    }

    /**
     * Get a random witty comment
     */
    private static String getWittyComment()
    {
        String[] astring = new String[] {"Shiny numbers!", "Am I not running fast enough? :(", "I\'m working as hard as I can!", "Will I ever be good enough for you? :(", "Speedy. Zoooooom!", "Hello world", "40% better than a crash report.", "Now with extra numbers", "Now with less numbers", "Now with the same numbers", "You should add flames to things, it makes them go faster!", "Do you feel the need for... optimization?", "*cracks redstone whip*", "Maybe if you treated it better then it\'ll have more motivation to work faster! Poor server."};

        try
        {
            return astring[(int)(System.nanoTime() % (long)astring.length)];
        }
        catch (Throwable var2)
        {
            return "Witty comment unavailable :(";
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, new String[] {"start", "stop"}): Collections.<String>emptyList();
    }
}
