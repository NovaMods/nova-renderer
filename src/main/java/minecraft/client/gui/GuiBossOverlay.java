package net.minecraft.client.gui;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.UUID;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.network.play.server.SPacketUpdateEntityNBT;
import net.minecraft.util.ResourceLocation;
import net.minecraft.world.BossInfo;
import net.minecraft.world.BossInfoLerping;

public class GuiBossOverlay extends Gui
{
    private static final ResourceLocation field_184058_a = new ResourceLocation("textures/gui/bars.png");
    private final Minecraft field_184059_f;
    private final Map<UUID, BossInfoLerping> field_184060_g = Maps.<UUID, BossInfoLerping>newLinkedHashMap();

    public GuiBossOverlay(Minecraft p_i46606_1_)
    {
        this.field_184059_f = p_i46606_1_;
    }

    public void func_184051_a()
    {
        if (!this.field_184060_g.isEmpty())
        {
            ScaledResolution scaledresolution = new ScaledResolution(this.field_184059_f);
            int i = scaledresolution.getScaledWidth();
            int j = 12;

            for (BossInfoLerping bossinfolerping : this.field_184060_g.values())
            {
                int k = i / 2 - 91;
                GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
                this.field_184059_f.getTextureManager().bindTexture(field_184058_a);
                this.func_184052_a(k, j, bossinfolerping);
                String s = bossinfolerping.getName().getFormattedText();
                this.field_184059_f.fontRendererObj.drawStringWithShadow(s, (float)(i / 2 - this.field_184059_f.fontRendererObj.getStringWidth(s) / 2), (float)(j - 9), 16777215);
                j += 10 + this.field_184059_f.fontRendererObj.FONT_HEIGHT;

                if (j >= scaledresolution.getScaledHeight() / 3)
                {
                    break;
                }
            }
        }
    }

    private void func_184052_a(int p_184052_1_, int p_184052_2_, BossInfo p_184052_3_)
    {
        this.drawTexturedModalRect(p_184052_1_, p_184052_2_, 0, p_184052_3_.getColor().ordinal() * 5 * 2, 182, 5);

        if (p_184052_3_.getOverlay() != BossInfo.Overlay.PROGRESS)
        {
            this.drawTexturedModalRect(p_184052_1_, p_184052_2_, 0, 80 + (p_184052_3_.getOverlay().ordinal() - 1) * 5 * 2, 182, 5);
        }

        int i = (int)(p_184052_3_.getPercent() * 183.0F);

        if (i > 0)
        {
            this.drawTexturedModalRect(p_184052_1_, p_184052_2_, 0, p_184052_3_.getColor().ordinal() * 5 * 2 + 5, i, 5);

            if (p_184052_3_.getOverlay() != BossInfo.Overlay.PROGRESS)
            {
                this.drawTexturedModalRect(p_184052_1_, p_184052_2_, 0, 80 + (p_184052_3_.getOverlay().ordinal() - 1) * 5 * 2 + 5, i, 5);
            }
        }
    }

    public void func_184055_a(SPacketUpdateEntityNBT p_184055_1_)
    {
        if (p_184055_1_.getOperation() == SPacketUpdateEntityNBT.Operation.ADD)
        {
            this.field_184060_g.put(p_184055_1_.getUniqueId(), new BossInfoLerping(p_184055_1_));
        }
        else if (p_184055_1_.getOperation() == SPacketUpdateEntityNBT.Operation.REMOVE)
        {
            this.field_184060_g.remove(p_184055_1_.getUniqueId());
        }
        else
        {
            ((BossInfoLerping)this.field_184060_g.get(p_184055_1_.getUniqueId())).updateFromPacket(p_184055_1_);
        }
    }

    public void func_184057_b()
    {
        this.field_184060_g.clear();
    }

    public boolean shouldPlayEndBossMusic()
    {
        if (!this.field_184060_g.isEmpty())
        {
            for (BossInfo bossinfo : this.field_184060_g.values())
            {
                if (bossinfo.shouldPlayEndBossMusic())
                {
                    return true;
                }
            }
        }

        return false;
    }

    public boolean shouldDarkenSky()
    {
        if (!this.field_184060_g.isEmpty())
        {
            for (BossInfo bossinfo : this.field_184060_g.values())
            {
                if (bossinfo.shouldDarkenSky())
                {
                    return true;
                }
            }
        }

        return false;
    }

    public boolean shouldCreateFog()
    {
        if (!this.field_184060_g.isEmpty())
        {
            for (BossInfo bossinfo : this.field_184060_g.values())
            {
                if (bossinfo.shouldCreateFog())
                {
                    return true;
                }
            }
        }

        return false;
    }
}
