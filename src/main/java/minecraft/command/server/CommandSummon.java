package net.minecraft.command.server;

import java.util.Collections;
import java.util.List;
import net.minecraft.command.CommandBase;
import net.minecraft.command.CommandException;
import net.minecraft.command.ICommandSender;
import net.minecraft.command.WrongUsageException;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityList;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.effect.EntityLightningBolt;
import net.minecraft.nbt.JsonToNBT;
import net.minecraft.nbt.NBTException;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.world.World;
import net.minecraft.world.chunk.storage.AnvilChunkLoader;

public class CommandSummon extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "summon";
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
        return "commands.summon.usage";
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
            throw new WrongUsageException("commands.summon.usage", new Object[0]);
        }
        else
        {
            String s = args[0];
            BlockPos blockpos = sender.getPosition();
            Vec3d vec3d = sender.getPositionVector();
            double d0 = vec3d.xCoord;
            double d1 = vec3d.yCoord;
            double d2 = vec3d.zCoord;

            if (args.length >= 4)
            {
                d0 = parseDouble(d0, args[1], true);
                d1 = parseDouble(d1, args[2], false);
                d2 = parseDouble(d2, args[3], true);
                blockpos = new BlockPos(d0, d1, d2);
            }

            World world = sender.getEntityWorld();

            if (!world.isBlockLoaded(blockpos))
            {
                throw new CommandException("commands.summon.outOfWorld", new Object[0]);
            }
            else if ("LightningBolt".equals(s))
            {
                world.addWeatherEffect(new EntityLightningBolt(world, d0, d1, d2, false));
                notifyOperators(sender, this, "commands.summon.success", new Object[0]);
            }
            else
            {
                NBTTagCompound nbttagcompound = new NBTTagCompound();
                boolean flag = false;

                if (args.length >= 5)
                {
                    ITextComponent itextcomponent = getChatComponentFromNthArg(sender, args, 4);

                    try
                    {
                        nbttagcompound = JsonToNBT.getTagFromJson(itextcomponent.getUnformattedText());
                        flag = true;
                    }
                    catch (NBTException nbtexception)
                    {
                        throw new CommandException("commands.summon.tagError", new Object[] {nbtexception.getMessage()});
                    }
                }

                nbttagcompound.setString("id", s);
                Entity entity = AnvilChunkLoader.func_186054_a(nbttagcompound, world, d0, d1, d2, true);

                if (entity == null)
                {
                    throw new CommandException("commands.summon.failed", new Object[0]);
                }
                else
                {
                    entity.setLocationAndAngles(d0, d1, d2, entity.rotationYaw, entity.rotationPitch);

                    if (!flag && entity instanceof EntityLiving)
                    {
                        ((EntityLiving)entity).onInitialSpawn(world.getDifficultyForLocation(new BlockPos(entity)), (IEntityLivingData)null);
                    }

                    notifyOperators(sender, this, "commands.summon.success", new Object[0]);
                }
            }
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, EntityList.getEntityNameList()) : (args.length > 1 && args.length <= 4 ? func_175771_a(args, 1, pos) : Collections.<String>emptyList());
    }
}
