package net.minecraft.client.resources;

import com.google.gson.JsonParseException;
import java.io.IOException;
import net.minecraft.client.gui.GuiScreenResourcePacks;
import net.minecraft.client.renderer.texture.DynamicTexture;
import net.minecraft.client.renderer.texture.TextureUtil;
import net.minecraft.client.resources.data.PackMetadataSection;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.text.TextFormatting;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ResourcePackListEntryServer extends ResourcePackListEntry
{
    private static final Logger logger = LogManager.getLogger();
    private final IResourcePack field_148320_d;
    private final ResourceLocation resourcePackIcon;

    public ResourcePackListEntryServer(GuiScreenResourcePacks p_i46594_1_, IResourcePack p_i46594_2_)
    {
        super(p_i46594_1_);
        this.field_148320_d = p_i46594_2_;
        DynamicTexture dynamictexture;

        try
        {
            dynamictexture = new DynamicTexture(p_i46594_2_.getPackImage());
        }
        catch (IOException var5)
        {
            dynamictexture = TextureUtil.missingTexture;
        }

        this.resourcePackIcon = this.mc.getTextureManager().getDynamicTextureLocation("texturepackicon", dynamictexture);
    }

    protected int func_183019_a()
    {
        return 2;
    }

    protected String func_148311_a()
    {
        try
        {
            PackMetadataSection packmetadatasection = (PackMetadataSection)this.field_148320_d.getPackMetadata(this.mc.getResourcePackRepository().rprMetadataSerializer, "pack");

            if (packmetadatasection != null)
            {
                return packmetadatasection.getPackDescription().getFormattedText();
            }
        }
        catch (JsonParseException jsonparseexception)
        {
            logger.error((String)"Couldn\'t load metadata info", (Throwable)jsonparseexception);
        }
        catch (IOException ioexception)
        {
            logger.error((String)"Couldn\'t load metadata info", (Throwable)ioexception);
        }

        return TextFormatting.RED + "Missing " + "pack.mcmeta" + " :(";
    }

    protected boolean func_148309_e()
    {
        return false;
    }

    protected boolean func_148308_f()
    {
        return false;
    }

    protected boolean func_148314_g()
    {
        return false;
    }

    protected boolean func_148307_h()
    {
        return false;
    }

    protected String func_148312_b()
    {
        return "Server";
    }

    protected void func_148313_c()
    {
        this.mc.getTextureManager().bindTexture(this.resourcePackIcon);
    }

    protected boolean func_148310_d()
    {
        return false;
    }

    public boolean func_186768_j()
    {
        return true;
    }
}
