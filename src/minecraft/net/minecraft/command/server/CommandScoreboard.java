package net.minecraft.command.server;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;
import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.CommandResultStats;
import net.minecraft.command.ICommandSender;
import net.minecraft.command.SyntaxErrorException;
import net.minecraft.command.WrongUsageException;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.nbt.JsonToNBT;
import net.minecraft.nbt.NBTException;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.scoreboard.IScoreCriteria;
import net.minecraft.scoreboard.Score;
import net.minecraft.scoreboard.ScoreObjective;
import net.minecraft.scoreboard.ScorePlayerTeam;
import net.minecraft.scoreboard.Scoreboard;
import net.minecraft.scoreboard.Team;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.util.text.TextFormatting;

public class CommandScoreboard extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "scoreboard";
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
        return "commands.scoreboard.usage";
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
        if (!this.func_184909_b(server, sender, args))
        {
            if (args.length < 1)
            {
                throw new WrongUsageException("commands.scoreboard.usage", new Object[0]);
            }
            else
            {
                if (args[0].equalsIgnoreCase("objectives"))
                {
                    if (args.length == 1)
                    {
                        throw new WrongUsageException("commands.scoreboard.objectives.usage", new Object[0]);
                    }

                    if (args[1].equalsIgnoreCase("list"))
                    {
                        this.func_184925_a(sender, server);
                    }
                    else if (args[1].equalsIgnoreCase("add"))
                    {
                        if (args.length < 4)
                        {
                            throw new WrongUsageException("commands.scoreboard.objectives.add.usage", new Object[0]);
                        }

                        this.func_184908_a(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("remove"))
                    {
                        if (args.length != 3)
                        {
                            throw new WrongUsageException("commands.scoreboard.objectives.remove.usage", new Object[0]);
                        }

                        this.func_184905_a(sender, args[2], server);
                    }
                    else
                    {
                        if (!args[1].equalsIgnoreCase("setdisplay"))
                        {
                            throw new WrongUsageException("commands.scoreboard.objectives.usage", new Object[0]);
                        }

                        if (args.length != 3 && args.length != 4)
                        {
                            throw new WrongUsageException("commands.scoreboard.objectives.setdisplay.usage", new Object[0]);
                        }

                        this.func_184919_i(sender, args, 2, server);
                    }
                }
                else if (args[0].equalsIgnoreCase("players"))
                {
                    if (args.length == 1)
                    {
                        throw new WrongUsageException("commands.scoreboard.players.usage", new Object[0]);
                    }

                    if (args[1].equalsIgnoreCase("list"))
                    {
                        if (args.length > 3)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.list.usage", new Object[0]);
                        }

                        this.func_184920_j(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("add"))
                    {
                        if (args.length < 5)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.add.usage", new Object[0]);
                        }

                        this.func_184918_k(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("remove"))
                    {
                        if (args.length < 5)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.remove.usage", new Object[0]);
                        }

                        this.func_184918_k(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("set"))
                    {
                        if (args.length < 5)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.set.usage", new Object[0]);
                        }

                        this.func_184918_k(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("reset"))
                    {
                        if (args.length != 3 && args.length != 4)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.reset.usage", new Object[0]);
                        }

                        this.func_184912_l(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("enable"))
                    {
                        if (args.length != 4)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.enable.usage", new Object[0]);
                        }

                        this.func_184914_m(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("test"))
                    {
                        if (args.length != 5 && args.length != 6)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.test.usage", new Object[0]);
                        }

                        this.func_184907_n(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("operation"))
                    {
                        if (args.length != 7)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.operation.usage", new Object[0]);
                        }

                        this.func_184906_o(sender, args, 2, server);
                    }
                    else
                    {
                        if (!args[1].equalsIgnoreCase("tag"))
                        {
                            throw new WrongUsageException("commands.scoreboard.players.usage", new Object[0]);
                        }

                        if (args.length < 4)
                        {
                            throw new WrongUsageException("commands.scoreboard.players.tag.usage", new Object[0]);
                        }

                        this.func_184924_a(server, sender, args, 2);
                    }
                }
                else
                {
                    if (!args[0].equalsIgnoreCase("teams"))
                    {
                        throw new WrongUsageException("commands.scoreboard.usage", new Object[0]);
                    }

                    if (args.length == 1)
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.usage", new Object[0]);
                    }

                    if (args[1].equalsIgnoreCase("list"))
                    {
                        if (args.length > 3)
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.list.usage", new Object[0]);
                        }

                        this.func_184922_e(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("add"))
                    {
                        if (args.length < 3)
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.add.usage", new Object[0]);
                        }

                        this.func_184910_b(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("remove"))
                    {
                        if (args.length != 3)
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.remove.usage", new Object[0]);
                        }

                        this.func_184921_d(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("empty"))
                    {
                        if (args.length != 3)
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.empty.usage", new Object[0]);
                        }

                        this.func_184917_h(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("join"))
                    {
                        if (args.length < 4 && (args.length != 3 || !(sender instanceof EntityPlayer)))
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.join.usage", new Object[0]);
                        }

                        this.func_184916_f(sender, args, 2, server);
                    }
                    else if (args[1].equalsIgnoreCase("leave"))
                    {
                        if (args.length < 3 && !(sender instanceof EntityPlayer))
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.leave.usage", new Object[0]);
                        }

                        this.func_184911_g(sender, args, 2, server);
                    }
                    else
                    {
                        if (!args[1].equalsIgnoreCase("option"))
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.usage", new Object[0]);
                        }

                        if (args.length != 4 && args.length != 5)
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.option.usage", new Object[0]);
                        }

                        this.func_184923_c(sender, args, 2, server);
                    }
                }
            }
        }
    }

    private boolean func_184909_b(MinecraftServer p_184909_1_, ICommandSender p_184909_2_, String[] p_184909_3_) throws CommandException
    {
        int i = -1;

        for (int j = 0; j < p_184909_3_.length; ++j)
        {
            if (this.isUsernameIndex(p_184909_3_, j) && "*".equals(p_184909_3_[j]))
            {
                if (i >= 0)
                {
                    throw new CommandException("commands.scoreboard.noMultiWildcard", new Object[0]);
                }

                i = j;
            }
        }

        if (i < 0)
        {
            return false;
        }
        else
        {
            List<String> list1 = Lists.newArrayList(this.func_184913_a(p_184909_1_).getObjectiveNames());
            String s = p_184909_3_[i];
            List<String> list = Lists.<String>newArrayList();

            for (String s1 : list1)
            {
                p_184909_3_[i] = s1;

                try
                {
                    this.execute(p_184909_1_, p_184909_2_, p_184909_3_);
                    list.add(s1);
                }
                catch (CommandException commandexception)
                {
                    TextComponentTranslation textcomponenttranslation = new TextComponentTranslation(commandexception.getMessage(), commandexception.getErrorObjects());
                    textcomponenttranslation.getChatStyle().setColor(TextFormatting.RED);
                    p_184909_2_.addChatMessage(textcomponenttranslation);
                }
            }

            p_184909_3_[i] = s;
            p_184909_2_.setCommandStat(CommandResultStats.Type.AFFECTED_ENTITIES, list.size());

            if (list.isEmpty())
            {
                throw new WrongUsageException("commands.scoreboard.allMatchesFailed", new Object[0]);
            }
            else
            {
                return true;
            }
        }
    }

    protected Scoreboard func_184913_a(MinecraftServer p_184913_1_)
    {
        return p_184913_1_.worldServerForDimension(0).getScoreboard();
    }

    protected ScoreObjective func_184903_a(String p_184903_1_, boolean p_184903_2_, MinecraftServer p_184903_3_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184903_3_);
        ScoreObjective scoreobjective = scoreboard.getObjective(p_184903_1_);

        if (scoreobjective == null)
        {
            throw new CommandException("commands.scoreboard.objectiveNotFound", new Object[] {p_184903_1_});
        }
        else if (p_184903_2_ && scoreobjective.getCriteria().isReadOnly())
        {
            throw new CommandException("commands.scoreboard.objectiveReadOnly", new Object[] {p_184903_1_});
        }
        else
        {
            return scoreobjective;
        }
    }

    protected ScorePlayerTeam func_184915_a(String p_184915_1_, MinecraftServer p_184915_2_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184915_2_);
        ScorePlayerTeam scoreplayerteam = scoreboard.getTeam(p_184915_1_);

        if (scoreplayerteam == null)
        {
            throw new CommandException("commands.scoreboard.teamNotFound", new Object[] {p_184915_1_});
        }
        else
        {
            return scoreplayerteam;
        }
    }

    protected void func_184908_a(ICommandSender p_184908_1_, String[] p_184908_2_, int p_184908_3_, MinecraftServer p_184908_4_) throws CommandException
    {
        String s = p_184908_2_[p_184908_3_++];
        String s1 = p_184908_2_[p_184908_3_++];
        Scoreboard scoreboard = this.func_184913_a(p_184908_4_);
        IScoreCriteria iscorecriteria = (IScoreCriteria)IScoreCriteria.INSTANCES.get(s1);

        if (iscorecriteria == null)
        {
            throw new WrongUsageException("commands.scoreboard.objectives.add.wrongType", new Object[] {s1});
        }
        else if (scoreboard.getObjective(s) != null)
        {
            throw new CommandException("commands.scoreboard.objectives.add.alreadyExists", new Object[] {s});
        }
        else if (s.length() > 16)
        {
            throw new SyntaxErrorException("commands.scoreboard.objectives.add.tooLong", new Object[] {s, Integer.valueOf(16)});
        }
        else if (s.isEmpty())
        {
            throw new WrongUsageException("commands.scoreboard.objectives.add.usage", new Object[0]);
        }
        else
        {
            if (p_184908_2_.length > p_184908_3_)
            {
                String s2 = getChatComponentFromNthArg(p_184908_1_, p_184908_2_, p_184908_3_).getUnformattedText();

                if (s2.length() > 32)
                {
                    throw new SyntaxErrorException("commands.scoreboard.objectives.add.displayTooLong", new Object[] {s2, Integer.valueOf(32)});
                }

                if (!s2.isEmpty())
                {
                    scoreboard.addScoreObjective(s, iscorecriteria).setDisplayName(s2);
                }
                else
                {
                    scoreboard.addScoreObjective(s, iscorecriteria);
                }
            }
            else
            {
                scoreboard.addScoreObjective(s, iscorecriteria);
            }

            notifyOperators(p_184908_1_, this, "commands.scoreboard.objectives.add.success", new Object[] {s});
        }
    }

    protected void func_184910_b(ICommandSender p_184910_1_, String[] p_184910_2_, int p_184910_3_, MinecraftServer p_184910_4_) throws CommandException
    {
        String s = p_184910_2_[p_184910_3_++];
        Scoreboard scoreboard = this.func_184913_a(p_184910_4_);

        if (scoreboard.getTeam(s) != null)
        {
            throw new CommandException("commands.scoreboard.teams.add.alreadyExists", new Object[] {s});
        }
        else if (s.length() > 16)
        {
            throw new SyntaxErrorException("commands.scoreboard.teams.add.tooLong", new Object[] {s, Integer.valueOf(16)});
        }
        else if (s.isEmpty())
        {
            throw new WrongUsageException("commands.scoreboard.teams.add.usage", new Object[0]);
        }
        else
        {
            if (p_184910_2_.length > p_184910_3_)
            {
                String s1 = getChatComponentFromNthArg(p_184910_1_, p_184910_2_, p_184910_3_).getUnformattedText();

                if (s1.length() > 32)
                {
                    throw new SyntaxErrorException("commands.scoreboard.teams.add.displayTooLong", new Object[] {s1, Integer.valueOf(32)});
                }

                if (!s1.isEmpty())
                {
                    scoreboard.createTeam(s).setTeamName(s1);
                }
                else
                {
                    scoreboard.createTeam(s);
                }
            }
            else
            {
                scoreboard.createTeam(s);
            }

            notifyOperators(p_184910_1_, this, "commands.scoreboard.teams.add.success", new Object[] {s});
        }
    }

    protected void func_184923_c(ICommandSender p_184923_1_, String[] p_184923_2_, int p_184923_3_, MinecraftServer p_184923_4_) throws CommandException
    {
        ScorePlayerTeam scoreplayerteam = this.func_184915_a(p_184923_2_[p_184923_3_++], p_184923_4_);

        if (scoreplayerteam != null)
        {
            String s = p_184923_2_[p_184923_3_++].toLowerCase();

            if (!s.equalsIgnoreCase("color") && !s.equalsIgnoreCase("friendlyfire") && !s.equalsIgnoreCase("seeFriendlyInvisibles") && !s.equalsIgnoreCase("nametagVisibility") && !s.equalsIgnoreCase("deathMessageVisibility") && !s.equalsIgnoreCase("collisionRule"))
            {
                throw new WrongUsageException("commands.scoreboard.teams.option.usage", new Object[0]);
            }
            else if (p_184923_2_.length == 4)
            {
                if (s.equalsIgnoreCase("color"))
                {
                    throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceStringFromCollection(TextFormatting.getValidValues(true, false))});
                }
                else if (!s.equalsIgnoreCase("friendlyfire") && !s.equalsIgnoreCase("seeFriendlyInvisibles"))
                {
                    if (!s.equalsIgnoreCase("nametagVisibility") && !s.equalsIgnoreCase("deathMessageVisibility"))
                    {
                        if (s.equalsIgnoreCase("collisionRule"))
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceString(Team.CollisionRule.getNames())});
                        }
                        else
                        {
                            throw new WrongUsageException("commands.scoreboard.teams.option.usage", new Object[0]);
                        }
                    }
                    else
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceString(Team.EnumVisible.getNames())});
                    }
                }
                else
                {
                    throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceStringFromCollection(Arrays.asList(new String[]{"true", "false"}))});
                }
            }
            else
            {
                String s1 = p_184923_2_[p_184923_3_];

                if (s.equalsIgnoreCase("color"))
                {
                    TextFormatting textformatting = TextFormatting.getValueByName(s1);

                    if (textformatting == null || textformatting.isFancyStyling())
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceStringFromCollection(TextFormatting.getValidValues(true, false))});
                    }

                    scoreplayerteam.setChatFormat(textformatting);
                    scoreplayerteam.setNamePrefix(textformatting.toString());
                    scoreplayerteam.setNameSuffix(TextFormatting.RESET.toString());
                }
                else if (s.equalsIgnoreCase("friendlyfire"))
                {
                    if (!s1.equalsIgnoreCase("true") && !s1.equalsIgnoreCase("false"))
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceStringFromCollection(Arrays.asList(new String[]{"true", "false"}))});
                    }

                    scoreplayerteam.setAllowFriendlyFire(s1.equalsIgnoreCase("true"));
                }
                else if (s.equalsIgnoreCase("seeFriendlyInvisibles"))
                {
                    if (!s1.equalsIgnoreCase("true") && !s1.equalsIgnoreCase("false"))
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceStringFromCollection(Arrays.asList(new String[]{"true", "false"}))});
                    }

                    scoreplayerteam.setSeeFriendlyInvisiblesEnabled(s1.equalsIgnoreCase("true"));
                }
                else if (s.equalsIgnoreCase("nametagVisibility"))
                {
                    Team.EnumVisible team$enumvisible = Team.EnumVisible.getByName(s1);

                    if (team$enumvisible == null)
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceString(Team.EnumVisible.getNames())});
                    }

                    scoreplayerteam.setNameTagVisibility(team$enumvisible);
                }
                else if (s.equalsIgnoreCase("deathMessageVisibility"))
                {
                    Team.EnumVisible team$enumvisible1 = Team.EnumVisible.getByName(s1);

                    if (team$enumvisible1 == null)
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceString(Team.EnumVisible.getNames())});
                    }

                    scoreplayerteam.setDeathMessageVisibility(team$enumvisible1);
                }
                else if (s.equalsIgnoreCase("collisionRule"))
                {
                    Team.CollisionRule team$collisionrule = Team.CollisionRule.getByName(s1);

                    if (team$collisionrule == null)
                    {
                        throw new WrongUsageException("commands.scoreboard.teams.option.noValue", new Object[] {s, joinNiceString(Team.CollisionRule.getNames())});
                    }

                    scoreplayerteam.setCollisionRule(team$collisionrule);
                }

                notifyOperators(p_184923_1_, this, "commands.scoreboard.teams.option.success", new Object[] {s, scoreplayerteam.getRegisteredName(), s1});
            }
        }
    }

    protected void func_184921_d(ICommandSender p_184921_1_, String[] p_184921_2_, int p_184921_3_, MinecraftServer p_184921_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184921_4_);
        ScorePlayerTeam scoreplayerteam = this.func_184915_a(p_184921_2_[p_184921_3_], p_184921_4_);

        if (scoreplayerteam != null)
        {
            scoreboard.removeTeam(scoreplayerteam);
            notifyOperators(p_184921_1_, this, "commands.scoreboard.teams.remove.success", new Object[] {scoreplayerteam.getRegisteredName()});
        }
    }

    protected void func_184922_e(ICommandSender p_184922_1_, String[] p_184922_2_, int p_184922_3_, MinecraftServer p_184922_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184922_4_);

        if (p_184922_2_.length > p_184922_3_)
        {
            ScorePlayerTeam scoreplayerteam = this.func_184915_a(p_184922_2_[p_184922_3_], p_184922_4_);

            if (scoreplayerteam == null)
            {
                return;
            }

            Collection<String> collection = scoreplayerteam.getMembershipCollection();
            p_184922_1_.setCommandStat(CommandResultStats.Type.QUERY_RESULT, collection.size());

            if (collection.isEmpty())
            {
                throw new CommandException("commands.scoreboard.teams.list.player.empty", new Object[] {scoreplayerteam.getRegisteredName()});
            }

            TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("commands.scoreboard.teams.list.player.count", new Object[] {Integer.valueOf(collection.size()), scoreplayerteam.getRegisteredName()});
            textcomponenttranslation.getChatStyle().setColor(TextFormatting.DARK_GREEN);
            p_184922_1_.addChatMessage(textcomponenttranslation);
            p_184922_1_.addChatMessage(new TextComponentString(joinNiceString(collection.toArray())));
        }
        else
        {
            Collection<ScorePlayerTeam> collection1 = scoreboard.getTeams();
            p_184922_1_.setCommandStat(CommandResultStats.Type.QUERY_RESULT, collection1.size());

            if (collection1.isEmpty())
            {
                throw new CommandException("commands.scoreboard.teams.list.empty", new Object[0]);
            }

            TextComponentTranslation textcomponenttranslation1 = new TextComponentTranslation("commands.scoreboard.teams.list.count", new Object[] {Integer.valueOf(collection1.size())});
            textcomponenttranslation1.getChatStyle().setColor(TextFormatting.DARK_GREEN);
            p_184922_1_.addChatMessage(textcomponenttranslation1);

            for (ScorePlayerTeam scoreplayerteam1 : collection1)
            {
                p_184922_1_.addChatMessage(new TextComponentTranslation("commands.scoreboard.teams.list.entry", new Object[] {scoreplayerteam1.getRegisteredName(), scoreplayerteam1.getTeamName(), Integer.valueOf(scoreplayerteam1.getMembershipCollection().size())}));
            }
        }
    }

    protected void func_184916_f(ICommandSender p_184916_1_, String[] p_184916_2_, int p_184916_3_, MinecraftServer p_184916_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184916_4_);
        String s = p_184916_2_[p_184916_3_++];
        Set<String> set = Sets.<String>newHashSet();
        Set<String> set1 = Sets.<String>newHashSet();

        if (p_184916_1_ instanceof EntityPlayer && p_184916_3_ == p_184916_2_.length)
        {
            String s4 = getCommandSenderAsPlayer(p_184916_1_).getName();

            if (scoreboard.addPlayerToTeam(s4, s))
            {
                set.add(s4);
            }
            else
            {
                set1.add(s4);
            }
        }
        else
        {
            while (p_184916_3_ < p_184916_2_.length)
            {
                String s1 = p_184916_2_[p_184916_3_++];

                if (s1.startsWith("@"))
                {
                    for (Entity entity : func_184890_c(p_184916_4_, p_184916_1_, s1))
                    {
                        String s3 = func_184891_e(p_184916_4_, p_184916_1_, entity.getUniqueID().toString());

                        if (scoreboard.addPlayerToTeam(s3, s))
                        {
                            set.add(s3);
                        }
                        else
                        {
                            set1.add(s3);
                        }
                    }
                }
                else
                {
                    String s2 = func_184891_e(p_184916_4_, p_184916_1_, s1);

                    if (scoreboard.addPlayerToTeam(s2, s))
                    {
                        set.add(s2);
                    }
                    else
                    {
                        set1.add(s2);
                    }
                }
            }
        }

        if (!set.isEmpty())
        {
            p_184916_1_.setCommandStat(CommandResultStats.Type.AFFECTED_ENTITIES, set.size());
            notifyOperators(p_184916_1_, this, "commands.scoreboard.teams.join.success", new Object[] {Integer.valueOf(set.size()), s, joinNiceString(set.toArray(new String[set.size()]))});
        }

        if (!set1.isEmpty())
        {
            throw new CommandException("commands.scoreboard.teams.join.failure", new Object[] {Integer.valueOf(set1.size()), s, joinNiceString(set1.toArray(new String[set1.size()]))});
        }
    }

    protected void func_184911_g(ICommandSender p_184911_1_, String[] p_184911_2_, int p_184911_3_, MinecraftServer p_184911_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184911_4_);
        Set<String> set = Sets.<String>newHashSet();
        Set<String> set1 = Sets.<String>newHashSet();

        if (p_184911_1_ instanceof EntityPlayer && p_184911_3_ == p_184911_2_.length)
        {
            String s3 = getCommandSenderAsPlayer(p_184911_1_).getName();

            if (scoreboard.removePlayerFromTeams(s3))
            {
                set.add(s3);
            }
            else
            {
                set1.add(s3);
            }
        }
        else
        {
            while (p_184911_3_ < p_184911_2_.length)
            {
                String s = p_184911_2_[p_184911_3_++];

                if (s.startsWith("@"))
                {
                    for (Entity entity : func_184890_c(p_184911_4_, p_184911_1_, s))
                    {
                        String s2 = func_184891_e(p_184911_4_, p_184911_1_, entity.getUniqueID().toString());

                        if (scoreboard.removePlayerFromTeams(s2))
                        {
                            set.add(s2);
                        }
                        else
                        {
                            set1.add(s2);
                        }
                    }
                }
                else
                {
                    String s1 = func_184891_e(p_184911_4_, p_184911_1_, s);

                    if (scoreboard.removePlayerFromTeams(s1))
                    {
                        set.add(s1);
                    }
                    else
                    {
                        set1.add(s1);
                    }
                }
            }
        }

        if (!set.isEmpty())
        {
            p_184911_1_.setCommandStat(CommandResultStats.Type.AFFECTED_ENTITIES, set.size());
            notifyOperators(p_184911_1_, this, "commands.scoreboard.teams.leave.success", new Object[] {Integer.valueOf(set.size()), joinNiceString(set.toArray(new String[set.size()]))});
        }

        if (!set1.isEmpty())
        {
            throw new CommandException("commands.scoreboard.teams.leave.failure", new Object[] {Integer.valueOf(set1.size()), joinNiceString(set1.toArray(new String[set1.size()]))});
        }
    }

    protected void func_184917_h(ICommandSender p_184917_1_, String[] p_184917_2_, int p_184917_3_, MinecraftServer p_184917_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184917_4_);
        ScorePlayerTeam scoreplayerteam = this.func_184915_a(p_184917_2_[p_184917_3_], p_184917_4_);

        if (scoreplayerteam != null)
        {
            Collection<String> collection = Lists.newArrayList(scoreplayerteam.getMembershipCollection());
            p_184917_1_.setCommandStat(CommandResultStats.Type.AFFECTED_ENTITIES, collection.size());

            if (collection.isEmpty())
            {
                throw new CommandException("commands.scoreboard.teams.empty.alreadyEmpty", new Object[] {scoreplayerteam.getRegisteredName()});
            }
            else
            {
                for (String s : collection)
                {
                    scoreboard.removePlayerFromTeam(s, scoreplayerteam);
                }

                notifyOperators(p_184917_1_, this, "commands.scoreboard.teams.empty.success", new Object[] {Integer.valueOf(collection.size()), scoreplayerteam.getRegisteredName()});
            }
        }
    }

    protected void func_184905_a(ICommandSender p_184905_1_, String p_184905_2_, MinecraftServer p_184905_3_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184905_3_);
        ScoreObjective scoreobjective = this.func_184903_a(p_184905_2_, false, p_184905_3_);
        scoreboard.removeObjective(scoreobjective);
        notifyOperators(p_184905_1_, this, "commands.scoreboard.objectives.remove.success", new Object[] {p_184905_2_});
    }

    protected void func_184925_a(ICommandSender p_184925_1_, MinecraftServer p_184925_2_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184925_2_);
        Collection<ScoreObjective> collection = scoreboard.getScoreObjectives();

        if (collection.isEmpty())
        {
            throw new CommandException("commands.scoreboard.objectives.list.empty", new Object[0]);
        }
        else
        {
            TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("commands.scoreboard.objectives.list.count", new Object[] {Integer.valueOf(collection.size())});
            textcomponenttranslation.getChatStyle().setColor(TextFormatting.DARK_GREEN);
            p_184925_1_.addChatMessage(textcomponenttranslation);

            for (ScoreObjective scoreobjective : collection)
            {
                p_184925_1_.addChatMessage(new TextComponentTranslation("commands.scoreboard.objectives.list.entry", new Object[] {scoreobjective.getName(), scoreobjective.getDisplayName(), scoreobjective.getCriteria().getName()}));
            }
        }
    }

    protected void func_184919_i(ICommandSender p_184919_1_, String[] p_184919_2_, int p_184919_3_, MinecraftServer p_184919_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184919_4_);
        String s = p_184919_2_[p_184919_3_++];
        int i = Scoreboard.getObjectiveDisplaySlotNumber(s);
        ScoreObjective scoreobjective = null;

        if (p_184919_2_.length == 4)
        {
            scoreobjective = this.func_184903_a(p_184919_2_[p_184919_3_], false, p_184919_4_);
        }

        if (i < 0)
        {
            throw new CommandException("commands.scoreboard.objectives.setdisplay.invalidSlot", new Object[] {s});
        }
        else
        {
            scoreboard.setObjectiveInDisplaySlot(i, scoreobjective);

            if (scoreobjective != null)
            {
                notifyOperators(p_184919_1_, this, "commands.scoreboard.objectives.setdisplay.successSet", new Object[] {Scoreboard.getObjectiveDisplaySlot(i), scoreobjective.getName()});
            }
            else
            {
                notifyOperators(p_184919_1_, this, "commands.scoreboard.objectives.setdisplay.successCleared", new Object[] {Scoreboard.getObjectiveDisplaySlot(i)});
            }
        }
    }

    protected void func_184920_j(ICommandSender p_184920_1_, String[] p_184920_2_, int p_184920_3_, MinecraftServer p_184920_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184920_4_);

        if (p_184920_2_.length > p_184920_3_)
        {
            String s = func_184891_e(p_184920_4_, p_184920_1_, p_184920_2_[p_184920_3_]);
            Map<ScoreObjective, Score> map = scoreboard.getObjectivesForEntity(s);
            p_184920_1_.setCommandStat(CommandResultStats.Type.QUERY_RESULT, map.size());

            if (map.isEmpty())
            {
                throw new CommandException("commands.scoreboard.players.list.player.empty", new Object[] {s});
            }

            TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("commands.scoreboard.players.list.player.count", new Object[] {Integer.valueOf(map.size()), s});
            textcomponenttranslation.getChatStyle().setColor(TextFormatting.DARK_GREEN);
            p_184920_1_.addChatMessage(textcomponenttranslation);

            for (Score score : map.values())
            {
                p_184920_1_.addChatMessage(new TextComponentTranslation("commands.scoreboard.players.list.player.entry", new Object[] {Integer.valueOf(score.getScorePoints()), score.getObjective().getDisplayName(), score.getObjective().getName()}));
            }
        }
        else
        {
            Collection<String> collection = scoreboard.getObjectiveNames();
            p_184920_1_.setCommandStat(CommandResultStats.Type.QUERY_RESULT, collection.size());

            if (collection.isEmpty())
            {
                throw new CommandException("commands.scoreboard.players.list.empty", new Object[0]);
            }

            TextComponentTranslation textcomponenttranslation1 = new TextComponentTranslation("commands.scoreboard.players.list.count", new Object[] {Integer.valueOf(collection.size())});
            textcomponenttranslation1.getChatStyle().setColor(TextFormatting.DARK_GREEN);
            p_184920_1_.addChatMessage(textcomponenttranslation1);
            p_184920_1_.addChatMessage(new TextComponentString(joinNiceString(collection.toArray())));
        }
    }

    protected void func_184918_k(ICommandSender p_184918_1_, String[] p_184918_2_, int p_184918_3_, MinecraftServer p_184918_4_) throws CommandException
    {
        String s = p_184918_2_[p_184918_3_ - 1];
        int i = p_184918_3_;
        String s1 = func_184891_e(p_184918_4_, p_184918_1_, p_184918_2_[p_184918_3_++]);

        if (s1.length() > 40)
        {
            throw new SyntaxErrorException("commands.scoreboard.players.name.tooLong", new Object[] {s1, Integer.valueOf(40)});
        }
        else
        {
            ScoreObjective scoreobjective = this.func_184903_a(p_184918_2_[p_184918_3_++], true, p_184918_4_);
            int j = s.equalsIgnoreCase("set") ? parseInt(p_184918_2_[p_184918_3_++]) : parseInt(p_184918_2_[p_184918_3_++], 0);

            if (p_184918_2_.length > p_184918_3_)
            {
                Entity entity = func_184885_b(p_184918_4_, p_184918_1_, p_184918_2_[i]);

                try
                {
                    NBTTagCompound nbttagcompound = JsonToNBT.getTagFromJson(buildString(p_184918_2_, p_184918_3_));
                    NBTTagCompound nbttagcompound1 = func_184887_a(entity);

                    if (!NBTUtil.func_181123_a(nbttagcompound, nbttagcompound1, true))
                    {
                        throw new CommandException("commands.scoreboard.players.set.tagMismatch", new Object[] {s1});
                    }
                }
                catch (NBTException nbtexception)
                {
                    throw new CommandException("commands.scoreboard.players.set.tagError", new Object[] {nbtexception.getMessage()});
                }
            }

            Scoreboard scoreboard = this.func_184913_a(p_184918_4_);
            Score score = scoreboard.getValueFromObjective(s1, scoreobjective);

            if (s.equalsIgnoreCase("set"))
            {
                score.setScorePoints(j);
            }
            else if (s.equalsIgnoreCase("add"))
            {
                score.increseScore(j);
            }
            else
            {
                score.decreaseScore(j);
            }

            notifyOperators(p_184918_1_, this, "commands.scoreboard.players.set.success", new Object[] {scoreobjective.getName(), s1, Integer.valueOf(score.getScorePoints())});
        }
    }

    protected void func_184912_l(ICommandSender p_184912_1_, String[] p_184912_2_, int p_184912_3_, MinecraftServer p_184912_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184912_4_);
        String s = func_184891_e(p_184912_4_, p_184912_1_, p_184912_2_[p_184912_3_++]);

        if (p_184912_2_.length > p_184912_3_)
        {
            ScoreObjective scoreobjective = this.func_184903_a(p_184912_2_[p_184912_3_++], false, p_184912_4_);
            scoreboard.removeObjectiveFromEntity(s, scoreobjective);
            notifyOperators(p_184912_1_, this, "commands.scoreboard.players.resetscore.success", new Object[] {scoreobjective.getName(), s});
        }
        else
        {
            scoreboard.removeObjectiveFromEntity(s, (ScoreObjective)null);
            notifyOperators(p_184912_1_, this, "commands.scoreboard.players.reset.success", new Object[] {s});
        }
    }

    protected void func_184914_m(ICommandSender p_184914_1_, String[] p_184914_2_, int p_184914_3_, MinecraftServer p_184914_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184914_4_);
        String s = func_184886_d(p_184914_4_, p_184914_1_, p_184914_2_[p_184914_3_++]);

        if (s.length() > 40)
        {
            throw new SyntaxErrorException("commands.scoreboard.players.name.tooLong", new Object[] {s, Integer.valueOf(40)});
        }
        else
        {
            ScoreObjective scoreobjective = this.func_184903_a(p_184914_2_[p_184914_3_], false, p_184914_4_);

            if (scoreobjective.getCriteria() != IScoreCriteria.TRIGGER)
            {
                throw new CommandException("commands.scoreboard.players.enable.noTrigger", new Object[] {scoreobjective.getName()});
            }
            else
            {
                Score score = scoreboard.getValueFromObjective(s, scoreobjective);
                score.setLocked(false);
                notifyOperators(p_184914_1_, this, "commands.scoreboard.players.enable.success", new Object[] {scoreobjective.getName(), s});
            }
        }
    }

    protected void func_184907_n(ICommandSender p_184907_1_, String[] p_184907_2_, int p_184907_3_, MinecraftServer p_184907_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184907_4_);
        String s = func_184891_e(p_184907_4_, p_184907_1_, p_184907_2_[p_184907_3_++]);

        if (s.length() > 40)
        {
            throw new SyntaxErrorException("commands.scoreboard.players.name.tooLong", new Object[] {s, Integer.valueOf(40)});
        }
        else
        {
            ScoreObjective scoreobjective = this.func_184903_a(p_184907_2_[p_184907_3_++], false, p_184907_4_);

            if (!scoreboard.entityHasObjective(s, scoreobjective))
            {
                throw new CommandException("commands.scoreboard.players.test.notFound", new Object[] {scoreobjective.getName(), s});
            }
            else
            {
                int i = p_184907_2_[p_184907_3_].equals("*") ? Integer.MIN_VALUE : parseInt(p_184907_2_[p_184907_3_]);
                ++p_184907_3_;
                int j = p_184907_3_ < p_184907_2_.length && !p_184907_2_[p_184907_3_].equals("*") ? parseInt(p_184907_2_[p_184907_3_], i) : Integer.MAX_VALUE;
                Score score = scoreboard.getValueFromObjective(s, scoreobjective);

                if (score.getScorePoints() >= i && score.getScorePoints() <= j)
                {
                    notifyOperators(p_184907_1_, this, "commands.scoreboard.players.test.success", new Object[] {Integer.valueOf(score.getScorePoints()), Integer.valueOf(i), Integer.valueOf(j)});
                }
                else
                {
                    throw new CommandException("commands.scoreboard.players.test.failed", new Object[] {Integer.valueOf(score.getScorePoints()), Integer.valueOf(i), Integer.valueOf(j)});
                }
            }
        }
    }

    protected void func_184906_o(ICommandSender p_184906_1_, String[] p_184906_2_, int p_184906_3_, MinecraftServer p_184906_4_) throws CommandException
    {
        Scoreboard scoreboard = this.func_184913_a(p_184906_4_);
        String s = func_184891_e(p_184906_4_, p_184906_1_, p_184906_2_[p_184906_3_++]);
        ScoreObjective scoreobjective = this.func_184903_a(p_184906_2_[p_184906_3_++], true, p_184906_4_);
        String s1 = p_184906_2_[p_184906_3_++];
        String s2 = func_184891_e(p_184906_4_, p_184906_1_, p_184906_2_[p_184906_3_++]);
        ScoreObjective scoreobjective1 = this.func_184903_a(p_184906_2_[p_184906_3_], false, p_184906_4_);

        if (s.length() > 40)
        {
            throw new SyntaxErrorException("commands.scoreboard.players.name.tooLong", new Object[] {s, Integer.valueOf(40)});
        }
        else if (s2.length() > 40)
        {
            throw new SyntaxErrorException("commands.scoreboard.players.name.tooLong", new Object[] {s2, Integer.valueOf(40)});
        }
        else
        {
            Score score = scoreboard.getValueFromObjective(s, scoreobjective);

            if (!scoreboard.entityHasObjective(s2, scoreobjective1))
            {
                throw new CommandException("commands.scoreboard.players.operation.notFound", new Object[] {scoreobjective1.getName(), s2});
            }
            else
            {
                Score score1 = scoreboard.getValueFromObjective(s2, scoreobjective1);

                if (s1.equals("+="))
                {
                    score.setScorePoints(score.getScorePoints() + score1.getScorePoints());
                }
                else if (s1.equals("-="))
                {
                    score.setScorePoints(score.getScorePoints() - score1.getScorePoints());
                }
                else if (s1.equals("*="))
                {
                    score.setScorePoints(score.getScorePoints() * score1.getScorePoints());
                }
                else if (s1.equals("/="))
                {
                    if (score1.getScorePoints() != 0)
                    {
                        score.setScorePoints(score.getScorePoints() / score1.getScorePoints());
                    }
                }
                else if (s1.equals("%="))
                {
                    if (score1.getScorePoints() != 0)
                    {
                        score.setScorePoints(score.getScorePoints() % score1.getScorePoints());
                    }
                }
                else if (s1.equals("="))
                {
                    score.setScorePoints(score1.getScorePoints());
                }
                else if (s1.equals("<"))
                {
                    score.setScorePoints(Math.min(score.getScorePoints(), score1.getScorePoints()));
                }
                else if (s1.equals(">"))
                {
                    score.setScorePoints(Math.max(score.getScorePoints(), score1.getScorePoints()));
                }
                else
                {
                    if (!s1.equals("><"))
                    {
                        throw new CommandException("commands.scoreboard.players.operation.invalidOperation", new Object[] {s1});
                    }

                    int i = score.getScorePoints();
                    score.setScorePoints(score1.getScorePoints());
                    score1.setScorePoints(i);
                }

                notifyOperators(p_184906_1_, this, "commands.scoreboard.players.operation.success", new Object[0]);
            }
        }
    }

    protected void func_184924_a(MinecraftServer p_184924_1_, ICommandSender p_184924_2_, String[] p_184924_3_, int p_184924_4_) throws CommandException
    {
        String s = func_184891_e(p_184924_1_, p_184924_2_, p_184924_3_[p_184924_4_]);
        Entity entity = func_184885_b(p_184924_1_, p_184924_2_, p_184924_3_[p_184924_4_++]);
        String s1 = p_184924_3_[p_184924_4_++];
        Set<String> set = entity.getTags();

        if ("list".equals(s1))
        {
            if (!set.isEmpty())
            {
                TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("commands.scoreboard.players.tag.list", new Object[] {s});
                textcomponenttranslation.getChatStyle().setColor(TextFormatting.DARK_GREEN);
                p_184924_2_.addChatMessage(textcomponenttranslation);
                p_184924_2_.addChatMessage(new TextComponentString(joinNiceString(set.toArray())));
            }

            p_184924_2_.setCommandStat(CommandResultStats.Type.QUERY_RESULT, set.size());
        }
        else if (p_184924_3_.length < 5)
        {
            throw new WrongUsageException("commands.scoreboard.players.tag.usage", new Object[0]);
        }
        else
        {
            String s2 = p_184924_3_[p_184924_4_++];

            if (p_184924_3_.length > p_184924_4_)
            {
                try
                {
                    NBTTagCompound nbttagcompound = JsonToNBT.getTagFromJson(buildString(p_184924_3_, p_184924_4_));
                    NBTTagCompound nbttagcompound1 = func_184887_a(entity);

                    if (!NBTUtil.func_181123_a(nbttagcompound, nbttagcompound1, true))
                    {
                        throw new CommandException("commands.scoreboard.players.tag.tagMismatch", new Object[] {s});
                    }
                }
                catch (NBTException nbtexception)
                {
                    throw new CommandException("commands.scoreboard.players.tag.tagError", new Object[] {nbtexception.getMessage()});
                }
            }

            if ("add".equals(s1))
            {
                if (!entity.addTag(s2))
                {
                    throw new CommandException("commands.scoreboard.players.tag.tooMany", new Object[] {Integer.valueOf(1024)});
                }

                notifyOperators(p_184924_2_, this, "commands.scoreboard.players.tag.success.add", new Object[] {s2});
            }
            else
            {
                if (!"remove".equals(s1))
                {
                    throw new WrongUsageException("commands.scoreboard.players.tag.usage", new Object[0]);
                }

                if (!entity.removeTag(s2))
                {
                    throw new CommandException("commands.scoreboard.players.tag.notFound", new Object[] {s2});
                }

                notifyOperators(p_184924_2_, this, "commands.scoreboard.players.tag.success.remove", new Object[] {s2});
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        if (args.length == 1)
        {
            return getListOfStringsMatchingLastWord(args, new String[] {"objectives", "players", "teams"});
        }
        else
        {
            if (args[0].equalsIgnoreCase("objectives"))
            {
                if (args.length == 2)
                {
                    return getListOfStringsMatchingLastWord(args, new String[] {"list", "add", "remove", "setdisplay"});
                }

                if (args[1].equalsIgnoreCase("add"))
                {
                    if (args.length == 4)
                    {
                        Set<String> set = IScoreCriteria.INSTANCES.keySet();
                        return getListOfStringsMatchingLastWord(args, set);
                    }
                }
                else if (args[1].equalsIgnoreCase("remove"))
                {
                    if (args.length == 3)
                    {
                        return getListOfStringsMatchingLastWord(args, this.func_184926_a(false, server));
                    }
                }
                else if (args[1].equalsIgnoreCase("setdisplay"))
                {
                    if (args.length == 3)
                    {
                        return getListOfStringsMatchingLastWord(args, Scoreboard.getDisplaySlotStrings());
                    }

                    if (args.length == 4)
                    {
                        return getListOfStringsMatchingLastWord(args, this.func_184926_a(false, server));
                    }
                }
            }
            else if (args[0].equalsIgnoreCase("players"))
            {
                if (args.length == 2)
                {
                    return getListOfStringsMatchingLastWord(args, new String[] {"set", "add", "remove", "reset", "list", "enable", "test", "operation", "tag"});
                }

                if (!args[1].equalsIgnoreCase("set") && !args[1].equalsIgnoreCase("add") && !args[1].equalsIgnoreCase("remove") && !args[1].equalsIgnoreCase("reset"))
                {
                    if (args[1].equalsIgnoreCase("enable"))
                    {
                        if (args.length == 3)
                        {
                            return getListOfStringsMatchingLastWord(args, server.getAllUsernames());
                        }

                        if (args.length == 4)
                        {
                            return getListOfStringsMatchingLastWord(args, this.func_184904_b(server));
                        }
                    }
                    else if (!args[1].equalsIgnoreCase("list") && !args[1].equalsIgnoreCase("test"))
                    {
                        if (args[1].equalsIgnoreCase("operation"))
                        {
                            if (args.length == 3)
                            {
                                return getListOfStringsMatchingLastWord(args, this.func_184913_a(server).getObjectiveNames());
                            }

                            if (args.length == 4)
                            {
                                return getListOfStringsMatchingLastWord(args, this.func_184926_a(true, server));
                            }

                            if (args.length == 5)
                            {
                                return getListOfStringsMatchingLastWord(args, new String[] {"+=", "-=", "*=", "/=", "%=", "=", "<", ">", "><"});
                            }

                            if (args.length == 6)
                            {
                                return getListOfStringsMatchingLastWord(args, server.getAllUsernames());
                            }

                            if (args.length == 7)
                            {
                                return getListOfStringsMatchingLastWord(args, this.func_184926_a(false, server));
                            }
                        }
                        else if (args[1].equalsIgnoreCase("tag"))
                        {
                            if (args.length == 3)
                            {
                                return getListOfStringsMatchingLastWord(args, this.func_184913_a(server).getObjectiveNames());
                            }

                            if (args.length == 4)
                            {
                                return getListOfStringsMatchingLastWord(args, new String[] {"add", "remove", "list"});
                            }
                        }
                    }
                    else
                    {
                        if (args.length == 3)
                        {
                            return getListOfStringsMatchingLastWord(args, this.func_184913_a(server).getObjectiveNames());
                        }

                        if (args.length == 4 && args[1].equalsIgnoreCase("test"))
                        {
                            return getListOfStringsMatchingLastWord(args, this.func_184926_a(false, server));
                        }
                    }
                }
                else
                {
                    if (args.length == 3)
                    {
                        return getListOfStringsMatchingLastWord(args, server.getAllUsernames());
                    }

                    if (args.length == 4)
                    {
                        return getListOfStringsMatchingLastWord(args, this.func_184926_a(true, server));
                    }
                }
            }
            else if (args[0].equalsIgnoreCase("teams"))
            {
                if (args.length == 2)
                {
                    return getListOfStringsMatchingLastWord(args, new String[] {"add", "remove", "join", "leave", "empty", "list", "option"});
                }

                if (args[1].equalsIgnoreCase("join"))
                {
                    if (args.length == 3)
                    {
                        return getListOfStringsMatchingLastWord(args, this.func_184913_a(server).getTeamNames());
                    }

                    if (args.length >= 4)
                    {
                        return getListOfStringsMatchingLastWord(args, server.getAllUsernames());
                    }
                }
                else
                {
                    if (args[1].equalsIgnoreCase("leave"))
                    {
                        return getListOfStringsMatchingLastWord(args, server.getAllUsernames());
                    }

                    if (!args[1].equalsIgnoreCase("empty") && !args[1].equalsIgnoreCase("list") && !args[1].equalsIgnoreCase("remove"))
                    {
                        if (args[1].equalsIgnoreCase("option"))
                        {
                            if (args.length == 3)
                            {
                                return getListOfStringsMatchingLastWord(args, this.func_184913_a(server).getTeamNames());
                            }

                            if (args.length == 4)
                            {
                                return getListOfStringsMatchingLastWord(args, new String[] {"color", "friendlyfire", "seeFriendlyInvisibles", "nametagVisibility", "deathMessageVisibility", "collisionRule"});
                            }

                            if (args.length == 5)
                            {
                                if (args[3].equalsIgnoreCase("color"))
                                {
                                    return getListOfStringsMatchingLastWord(args, TextFormatting.getValidValues(true, false));
                                }

                                if (args[3].equalsIgnoreCase("nametagVisibility") || args[3].equalsIgnoreCase("deathMessageVisibility"))
                                {
                                    return getListOfStringsMatchingLastWord(args, Team.EnumVisible.getNames());
                                }

                                if (args[3].equalsIgnoreCase("collisionRule"))
                                {
                                    return getListOfStringsMatchingLastWord(args, Team.CollisionRule.getNames());
                                }

                                if (args[3].equalsIgnoreCase("friendlyfire") || args[3].equalsIgnoreCase("seeFriendlyInvisibles"))
                                {
                                    return getListOfStringsMatchingLastWord(args, new String[] {"true", "false"});
                                }
                            }
                        }
                    }
                    else if (args.length == 3)
                    {
                        return getListOfStringsMatchingLastWord(args, this.func_184913_a(server).getTeamNames());
                    }
                }
            }

            return Collections.<String>emptyList();
        }
    }

    protected List<String> func_184926_a(boolean p_184926_1_, MinecraftServer p_184926_2_)
    {
        Collection<ScoreObjective> collection = this.func_184913_a(p_184926_2_).getScoreObjectives();
        List<String> list = Lists.<String>newArrayList();

        for (ScoreObjective scoreobjective : collection)
        {
            if (!p_184926_1_ || !scoreobjective.getCriteria().isReadOnly())
            {
                list.add(scoreobjective.getName());
            }
        }

        return list;
    }

    protected List<String> func_184904_b(MinecraftServer p_184904_1_)
    {
        Collection<ScoreObjective> collection = this.func_184913_a(p_184904_1_).getScoreObjectives();
        List<String> list = Lists.<String>newArrayList();

        for (ScoreObjective scoreobjective : collection)
        {
            if (scoreobjective.getCriteria() == IScoreCriteria.TRIGGER)
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
        return !args[0].equalsIgnoreCase("players") ? (args[0].equalsIgnoreCase("teams") ? index == 2 : false) : (args.length > 1 && args[1].equalsIgnoreCase("operation") ? index == 2 || index == 5 : index == 2);
    }
}
