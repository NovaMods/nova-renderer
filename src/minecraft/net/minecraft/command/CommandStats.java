package net.minecraft.command;

import com.google.common.collect.Lists;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import net.minecraft.entity.Entity;
import net.minecraft.scoreboard.ScoreObjective;
import net.minecraft.server.MinecraftServer;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityCommandBlock;
import net.minecraft.tileentity.TileEntitySign;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class CommandStats extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "stats";
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
        return "commands.stats.usage";
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
            throw new WrongUsageException("commands.stats.usage", new Object[0]);
        }
        else
        {
            boolean flag;

            if (args[0].equals("entity"))
            {
                flag = false;
            }
            else
            {
                if (!args[0].equals("block"))
                {
                    throw new WrongUsageException("commands.stats.usage", new Object[0]);
                }

                flag = true;
            }

            int i;

            if (flag)
            {
                if (args.length < 5)
                {
                    throw new WrongUsageException("commands.stats.block.usage", new Object[0]);
                }

                i = 4;
            }
            else
            {
                if (args.length < 3)
                {
                    throw new WrongUsageException("commands.stats.entity.usage", new Object[0]);
                }

                i = 2;
            }

            String s = args[i++];

            if ("set".equals(s))
            {
                if (args.length < i + 3)
                {
                    if (i == 5)
                    {
                        throw new WrongUsageException("commands.stats.block.set.usage", new Object[0]);
                    }

                    throw new WrongUsageException("commands.stats.entity.set.usage", new Object[0]);
                }
            }
            else
            {
                if (!"clear".equals(s))
                {
                    throw new WrongUsageException("commands.stats.usage", new Object[0]);
                }

                if (args.length < i + 1)
                {
                    if (i == 5)
                    {
                        throw new WrongUsageException("commands.stats.block.clear.usage", new Object[0]);
                    }

                    throw new WrongUsageException("commands.stats.entity.clear.usage", new Object[0]);
                }
            }

            CommandResultStats.Type commandresultstats$type = CommandResultStats.Type.getTypeByName(args[i++]);

            if (commandresultstats$type == null)
            {
                throw new CommandException("commands.stats.failed", new Object[0]);
            }
            else
            {
                World world = sender.getEntityWorld();
                CommandResultStats commandresultstats;

                if (flag)
                {
                    BlockPos blockpos = parseBlockPos(sender, args, 1, false);
                    TileEntity tileentity = world.getTileEntity(blockpos);

                    if (tileentity == null)
                    {
                        throw new CommandException("commands.stats.noCompatibleBlock", new Object[] {Integer.valueOf(blockpos.getX()), Integer.valueOf(blockpos.getY()), Integer.valueOf(blockpos.getZ())});
                    }

                    if (tileentity instanceof TileEntityCommandBlock)
                    {
                        commandresultstats = ((TileEntityCommandBlock)tileentity).getCommandResultStats();
                    }
                    else
                    {
                        if (!(tileentity instanceof TileEntitySign))
                        {
                            throw new CommandException("commands.stats.noCompatibleBlock", new Object[] {Integer.valueOf(blockpos.getX()), Integer.valueOf(blockpos.getY()), Integer.valueOf(blockpos.getZ())});
                        }

                        commandresultstats = ((TileEntitySign)tileentity).getStats();
                    }
                }
                else
                {
                    Entity entity = func_184885_b(server, sender, args[1]);
                    commandresultstats = entity.getCommandStats();
                }

                if ("set".equals(s))
                {
                    String s1 = args[i++];
                    String s2 = args[i];

                    if (s1.isEmpty() || s2.isEmpty())
                    {
                        throw new CommandException("commands.stats.failed", new Object[0]);
                    }

                    CommandResultStats.setScoreBoardStat(commandresultstats, commandresultstats$type, s1, s2);
                    notifyOperators(sender, this, "commands.stats.success", new Object[] {commandresultstats$type.getTypeName(), s2, s1});
                }
                else if ("clear".equals(s))
                {
                    CommandResultStats.setScoreBoardStat(commandresultstats, commandresultstats$type, (String)null, (String)null);
                    notifyOperators(sender, this, "commands.stats.cleared", new Object[] {commandresultstats$type.getTypeName()});
                }

                if (flag)
                {
                    BlockPos blockpos1 = parseBlockPos(sender, args, 1, false);
                    TileEntity tileentity1 = world.getTileEntity(blockpos1);
                    tileentity1.markDirty();
                }
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, new String[] {"entity", "block"}): (args.length == 2 && args[0].equals("entity") ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : (args.length >= 2 && args.length <= 4 && args[0].equals("block") ? func_175771_a(args, 1, pos) : ((args.length != 3 || !args[0].equals("entity")) && (args.length != 5 || !args[0].equals("block")) ? ((args.length != 4 || !args[0].equals("entity")) && (args.length != 6 || !args[0].equals("block")) ? ((args.length != 6 || !args[0].equals("entity")) && (args.length != 8 || !args[0].equals("block")) ? Collections.<String>emptyList() : getListOfStringsMatchingLastWord(args, this.func_184927_a(server))) : getListOfStringsMatchingLastWord(args, CommandResultStats.Type.getTypeNames())) : getListOfStringsMatchingLastWord(args, new String[] {"set", "clear"}))));
    }

    protected List<String> func_184927_a(MinecraftServer p_184927_1_)
    {
        Collection<ScoreObjective> collection = p_184927_1_.worldServerForDimension(0).getScoreboard().getScoreObjectives();
        List<String> list = Lists.<String>newArrayList();

        for (ScoreObjective scoreobjective : collection)
        {
            if (!scoreobjective.getCriteria().isReadOnly())
            {
                list.add(scoreobjective.getName());
            }
        }

        return list;
    }

    /**
     * Return whether the specified command parameter index is a username parameter.
     */
    public boolean isUsernameIndex(String[] args, int index)
    {
        return args.length > 0 && args[0].equals("entity") && index == 1;
    }
}
