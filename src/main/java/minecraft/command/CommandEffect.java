package net.minecraft.command;

import java.util.Collections;
import java.util.List;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.potion.Potion;
import net.minecraft.potion.PotionEffect;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.TextComponentTranslation;

public class CommandEffect extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "effect";
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
        return "commands.effect.usage";
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
        if (args.length < 2)
        {
            throw new WrongUsageException("commands.effect.usage", new Object[0]);
        }
        else
        {
            EntityLivingBase entitylivingbase = (EntityLivingBase)func_184884_a(server, sender, args[0], EntityLivingBase.class);

            if (args[1].equals("clear"))
            {
                if (entitylivingbase.getActivePotionEffects().isEmpty())
                {
                    throw new CommandException("commands.effect.failure.notActive.all", new Object[] {entitylivingbase.getName()});
                }
                else
                {
                    entitylivingbase.clearActivePotions();
                    notifyOperators(sender, this, "commands.effect.success.removed.all", new Object[] {entitylivingbase.getName()});
                }
            }
            else
            {
                Potion potion;

                try
                {
                    potion = Potion.getPotionById(parseInt(args[1], 1));
                }
                catch (NumberInvalidException var11)
                {
                    potion = Potion.getPotionFromResourceLocation(args[1]);
                }

                if (potion == null)
                {
                    throw new NumberInvalidException("commands.effect.notFound", new Object[] {args[1]});
                }
                else
                {
                    int i = 600;
                    int j = 30;
                    int k = 0;

                    if (args.length >= 3)
                    {
                        j = parseInt(args[2], 0, 1000000);

                        if (potion.isInstant())
                        {
                            i = j;
                        }
                        else
                        {
                            i = j * 20;
                        }
                    }
                    else if (potion.isInstant())
                    {
                        i = 1;
                    }

                    if (args.length >= 4)
                    {
                        k = parseInt(args[3], 0, 255);
                    }

                    boolean flag = true;

                    if (args.length >= 5 && "true".equalsIgnoreCase(args[4]))
                    {
                        flag = false;
                    }

                    if (j > 0)
                    {
                        PotionEffect potioneffect = new PotionEffect(potion, i, k, false, flag);
                        entitylivingbase.addPotionEffect(potioneffect);
                        notifyOperators(sender, this, "commands.effect.success", new Object[] {new TextComponentTranslation(potioneffect.getEffectName(), new Object[0]), Integer.valueOf(Potion.getIdFromPotion(potion)), Integer.valueOf(k), entitylivingbase.getName(), Integer.valueOf(j)});
                    }
                    else if (entitylivingbase.isPotionActive(potion))
                    {
                        entitylivingbase.func_184589_d(potion);
                        notifyOperators(sender, this, "commands.effect.success.removed", new Object[] {new TextComponentTranslation(potion.getName(), new Object[0]), entitylivingbase.getName()});
                    }
                    else
                    {
                        throw new CommandException("commands.effect.failure.notActive", new Object[] {new TextComponentTranslation(potion.getName(), new Object[0]), entitylivingbase.getName()});
                    }
                }
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : (args.length == 2 ? getListOfStringsMatchingLastWord(args, Potion.potionRegistry.getKeys()) : (args.length == 5 ? getListOfStringsMatchingLastWord(args, new String[] {"true", "false"}): Collections.<String>emptyList()));
    }

    /**
     * Return whether the specified command parameter index is a username parameter.
     */
    public boolean isUsernameIndex(String[] args, int index)
    {
        return index == 0;
    }
}
