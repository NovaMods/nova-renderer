package net.minecraft.util.datafix.walkers;

import com.google.common.collect.Maps;
import java.util.Map;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.datafix.FixTypes;
import net.minecraft.util.datafix.IDataFixer;
import net.minecraft.util.datafix.IDataWalker;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class BlockEntityTag implements IDataWalker
{
    private static final Logger field_188268_a = LogManager.getLogger();
    private static final Map<String, String> field_188269_b = Maps.<String, String>newHashMap();

    private static String func_188267_a(String p_188267_0_)
    {
        return (String)field_188269_b.get((new ResourceLocation(p_188267_0_)).toString());
    }

    public NBTTagCompound func_188266_a(IDataFixer p_188266_1_, NBTTagCompound p_188266_2_, int p_188266_3_)
    {
        if (!p_188266_2_.hasKey("tag", 10))
        {
            return p_188266_2_;
        }
        else
        {
            NBTTagCompound nbttagcompound = p_188266_2_.getCompoundTag("tag");

            if (nbttagcompound.hasKey("BlockEntityTag", 10))
            {
                NBTTagCompound nbttagcompound1 = nbttagcompound.getCompoundTag("BlockEntityTag");
                String s = p_188266_2_.getString("id");
                String s1 = func_188267_a(s);
                boolean flag;

                if (s1 == null)
                {
                    field_188268_a.warn("Unable to resolve BlockEntity for ItemInstance: {}", new Object[] {s});
                    flag = false;
                }
                else
                {
                    flag = !nbttagcompound1.hasKey("id");
                    nbttagcompound1.setString("id", s1);
                }

                p_188266_1_.func_188251_a(FixTypes.BLOCK_ENTITY, nbttagcompound1, p_188266_3_);

                if (flag)
                {
                    nbttagcompound1.removeTag("id");
                }
            }

            return p_188266_2_;
        }
    }

    static
    {
        Map<String, String> map = field_188269_b;
        map.put("minecraft:furnace", "Furnace");
        map.put("minecraft:lit_furnace", "Furnace");
        map.put("minecraft:chest", "Chest");
        map.put("minecraft:trapped_chest", "Chest");
        map.put("minecraft:ender_chest", "EnderChest");
        map.put("minecraft:jukebox", "RecordPlayer");
        map.put("minecraft:dispenser", "Trap");
        map.put("minecraft:dropper", "Dropper");
        map.put("minecraft:sign", "Sign");
        map.put("minecraft:mob_spawner", "MobSpawner");
        map.put("minecraft:noteblock", "Music");
        map.put("minecraft:brewing_stand", "Cauldron");
        map.put("minecraft:enhanting_table", "EnchantTable");
        map.put("minecraft:command_block", "CommandBlock");
        map.put("minecraft:beacon", "Beacon");
        map.put("minecraft:skull", "Skull");
        map.put("minecraft:daylight_detector", "DLDetector");
        map.put("minecraft:hopper", "Hopper");
        map.put("minecraft:banner", "Banner");
        map.put("minecraft:flower_pot", "FlowerPot");
        map.put("minecraft:repeating_command_block", "CommandBlock");
        map.put("minecraft:chain_command_block", "CommandBlock");
        map.put("minecraft:standing_sign", "Sign");
        map.put("minecraft:wall_sign", "Sign");
        map.put("minecraft:piston_head", "Piston");
        map.put("minecraft:daylight_detector_inverted", "DLDetector");
        map.put("minecraft:unpowered_comparator", "Comparator");
        map.put("minecraft:powered_comparator", "Comparator");
        map.put("minecraft:wall_banner", "Banner");
        map.put("minecraft:standing_banner", "Banner");
        map.put("minecraft:structure_block", "Structure");
        map.put("minecraft:end_portal", "Airportal");
        map.put("minecraft:end_gateway", "EndGateway");
        map.put("minecraft:shield", "Shield");
    }
}
