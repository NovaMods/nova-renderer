package net.minecraft.client.resources;

import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiScreenResourcePacks;

public class ResourcePackListEntryDefault extends ResourcePackListEntryServer
{
    public ResourcePackListEntryDefault(GuiScreenResourcePacks resourcePacksGUIIn)
    {
        super(resourcePacksGUIIn, Minecraft.getMinecraft().getResourcePackRepository().rprDefaultResourcePack);
    }

    protected String func_148312_b()
    {
        return "Default";
    }

    public boolean func_186768_j()
    {
        return false;
    }
}
