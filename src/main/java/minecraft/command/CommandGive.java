package net.minecraft.command;

import java.util.Collections;
import java.util.List;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.JsonToNBT;
import net.minecraft.nbt.NBTException;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;

public class CommandGive extends CommandBase
{
    /**
     * Gets the name of the command
     */
    public String getCommandName()
    {
        return "give";
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
        return "commands.give.usage";
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
            throw new WrongUsageException("commands.give.usage", new Object[0]);
        }
        else
        {
            EntityPlayer entityplayer = getPlayer(server, sender, args[0]);
            Item item = getItemByText(sender, args[1]);
            int i = args.length >= 3 ? parseInt(args[2], 1, 64) : 1;
            int j = args.length >= 4 ? parseInt(args[3]) : 0;
            ItemStack itemstack = new ItemStack(item, i, j);

            if (args.length >= 5)
            {
                String s = getChatComponentFromNthArg(sender, args, 4).getUnformattedText();

                try
                {
                    itemstack.setTagCompound(JsonToNBT.getTagFromJson(s));
                }
                catch (NBTException nbtexception)
                {
                    throw new CommandException("commands.give.tagError", new Object[] {nbtexception.getMessage()});
                }
            }

            boolean flag = entityplayer.inventory.addItemStackToInventory(itemstack);

            if (flag)
            {
                entityplayer.worldObj.func_184148_a((EntityPlayer)null, entityplayer.posX, entityplayer.posY, entityplayer.posZ, SoundEvents.entity_item_pickup, SoundCategory.PLAYERS, 0.2F, ((entityplayer.getRNG().nextFloat() - entityplayer.getRNG().nextFloat()) * 0.7F + 1.0F) * 2.0F);
                entityplayer.inventoryContainer.detectAndSendChanges();
            }

            if (flag && itemstack.stackSize <= 0)
            {
                itemstack.stackSize = 1;
                sender.setCommandStat(CommandResultStats.Type.AFFECTED_ITEMS, i);
                EntityItem entityitem1 = entityplayer.dropPlayerItemWithRandomChoice(itemstack, false);

                if (entityitem1 != null)
                {
                    entityitem1.func_174870_v();
                }
            }
            else
            {
                sender.setCommandStat(CommandResultStats.Type.AFFECTED_ITEMS, i - itemstack.stackSize);
                EntityItem entityitem = entityplayer.dropPlayerItemWithRandomChoice(itemstack, false);

                if (entityitem != null)
                {
                    entityitem.setNoPickupDelay();
                    entityitem.setOwner(entityplayer.getName());
                }
            }

            notifyOperators(sender, this, "commands.give.success", new Object[] {itemstack.getChatComponent(), Integer.valueOf(i), entityplayer.getName()});
        }
    }

    public List<String> getTabCompletionOptions(MinecraftServer server, ICommandSender sender, String[] args, BlockPos pos)
    {
        return args.length == 1 ? getListOfStringsMatchingLastWord(args, server.getAllUsernames()) : (args.length == 2 ? getListOfStringsMatchingLastWord(args, Item.itemRegistry.getKeys()) : Collections.<String>emptyList());
    }

    /**
     * Return whether the specified command parameter index is a username parameter.
     */
    public boolean isUsernameIndex(String[] args, int index)
    {
        return index == 0;
    }
}
