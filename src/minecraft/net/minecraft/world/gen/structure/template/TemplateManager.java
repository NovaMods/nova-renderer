package net.minecraft.world.gen.structure.template;

import com.google.common.collect.Maps;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Map;
import net.minecraft.nbt.CompressedStreamTools;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.ResourceLocation;
import org.apache.commons.io.IOUtils;

public class TemplateManager
{
    private final Map<String, Template> templates;
    private final String field_186241_b;

    public TemplateManager()
    {
        this("structures");
    }

    public TemplateManager(String p_i46661_1_)
    {
        this.templates = Maps.<String, Template>newHashMap();
        this.field_186241_b = p_i46661_1_;
    }

    public Template getTemplate(MinecraftServer server, ResourceLocation id)
    {
        String s = id.getResourcePath();

        if (this.templates.containsKey(s))
        {
            return (Template)this.templates.get(s);
        }
        else
        {
            if (server != null)
            {
                this.func_186235_b(server, id);
            }
            else
            {
                this.func_186236_a(id);
            }

            if (this.templates.containsKey(s))
            {
                return (Template)this.templates.get(s);
            }
            else
            {
                Template template = new Template();
                this.templates.put(s, template);
                return template;
            }
        }
    }

    public boolean func_186235_b(MinecraftServer server, ResourceLocation id)
    {
        String s = id.getResourcePath();
        File file1 = server.getFile(this.field_186241_b);
        File file2 = new File(file1, s + ".nbt");

        if (!file2.exists())
        {
            return this.func_186236_a(id);
        }
        else
        {
            InputStream inputstream = null;
            boolean flag;

            try
            {
                inputstream = new FileInputStream(file2);
                this.func_186239_a(s, inputstream);
                return true;
            }
            catch (Throwable var12)
            {
                flag = false;
            }
            finally
            {
                IOUtils.closeQuietly(inputstream);
            }

            return flag;
        }
    }

    private boolean func_186236_a(ResourceLocation id)
    {
        String s = id.getResourceDomain();
        String s1 = id.getResourcePath();
        InputStream inputstream = null;
        boolean flag;

        try
        {
            inputstream = MinecraftServer.class.getResourceAsStream("/assets/" + s + "/structures/" + s1 + ".nbt");
            this.func_186239_a(s1, inputstream);
            return true;
        }
        catch (Throwable var10)
        {
            flag = false;
        }
        finally
        {
            IOUtils.closeQuietly(inputstream);
        }

        return flag;
    }

    private void func_186239_a(String p_186239_1_, InputStream p_186239_2_) throws IOException
    {
        NBTTagCompound nbttagcompound = CompressedStreamTools.readCompressed(p_186239_2_);
        Template template = new Template();
        template.read(nbttagcompound);
        this.templates.put(p_186239_1_, template);
    }

    public boolean func_186238_c(MinecraftServer server, ResourceLocation id)
    {
        String s = id.getResourcePath();

        if (!this.templates.containsKey(s))
        {
            return false;
        }
        else
        {
            File file1 = server.getFile(this.field_186241_b);

            if (!file1.exists())
            {
                if (!file1.mkdirs())
                {
                    return false;
                }
            }
            else if (!file1.isDirectory())
            {
                return false;
            }

            File file2 = new File(file1, s + ".nbt");
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            Template template = (Template)this.templates.get(s);
            OutputStream outputstream = null;
            boolean flag;

            try
            {
                template.write(nbttagcompound);
                outputstream = new FileOutputStream(file2);
                CompressedStreamTools.writeCompressed(nbttagcompound, outputstream);
                return true;
            }
            catch (Throwable var14)
            {
                flag = false;
            }
            finally
            {
                IOUtils.closeQuietly(outputstream);
            }

            return flag;
        }
    }
}
