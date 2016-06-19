package net.minecraft.command;

import java.util.Collections;
import java.util.List;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.world.WorldSettings;

public class CommandGameMode extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "gamemode";
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
        return "commands.gamemode.usage";
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
        if (args.length <= 0)
        {
            throw new WrongUsageException("commands.gamemode.usage", new Object[0]);
        }
        else
        {
            WorldSettings.GameType worldsettings$gametype = this.getGameModeFromCommand(sender, args[0]);
            EntityPlayer entityplayer = args.length >= 2 ? getPlayer(server, sender, args[1]) : getCommandSenderAsPlayer(sender);
            entityplayer.setGameType(worldsettings$gametype);
            ITextComponent itextcomponent = new TextComponentTranslation("gameMode." + worldsettings$gametype.getName(), new Object[0]);

            if (sender.getEntityWorld().getGameRules().getBoolean("sendCommandFeedback"))
            {
                entityplayer.addChatMessage(new TextComponentTranslation("gameMode.changed", new Object[] {itextcomponent}));
            }

            if (entityplayer != sender)
            {
                notifyOperators(sender, this, 1, "commands.gamemode.success.other", new Object[] {entityplayer.getName(), itextcomponent});
            }
            else
            {
                notifyOperators(sender, this, 1, "commands.gamemode.success.self", new Object[] {itextcomponent});
            }
        }
    }

    /**
     * Gets the Game Mode specified in the command.
     */
    protected WorldSettings.GameType getGameModeFromCommand(ICommandSender p_71539_1_, String p_71539_2_) throws CommandException, NumberInvalidException
    {
        WorldSettings.GameType worldsettings$gametype = WorldSettings.GameType.func_185328_a(p_71539_2_, WorldSettings.GameType.NOT_SET);
        return worldsettings$gametype == WorldSettings.GameType.NOT_SET ? WorldSettings.getGameTypeById(parseInt(p_71539_2_, 0, WorldSettings.GameType.values().length - 2)) : worldsettings$gametype;
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, new String[] {"survival", "creative", "adventure", "spectator"}): (args.length == 2 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : Collections.<String>emptyList());
    }

    /**
     * Return whether the specified command parameter index is a username parameter.
     */
    public boolean isUsernameIndex(String[] args, int index)
    {
        return index == 1;
    }
}
