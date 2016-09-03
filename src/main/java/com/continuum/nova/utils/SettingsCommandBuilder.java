package com.continuum.nova.utils;

import com.continuum.nova.NovaNative;
import net.minecraft.client.Minecraft;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.MobEffects;

public class SettingsCommandBuilder
{
    public NovaNative.mc_settings getSettings(Minecraft mc)
    {
        NovaNative.mc_settings settings = new NovaNative.mc_settings();
        settings.view_bobbing = mc.gameSettings.viewBobbing;
        settings.render_distance = mc.gameSettings.renderDistanceChunks;
        settings.fog_color_red = mc.entityRenderer.getFogColorRed();
        settings.fog_color_green = mc.entityRenderer.getFogColorGreen();
        settings.fog_color_blue = mc.entityRenderer.getFogColorBlue();
        settings.anaglyph = mc.gameSettings.anaglyph;
        settings.display_height = mc.displayHeight;
        settings.display_width = mc.displayWidth;

        if (Utils.exists(mc.theWorld))
        {
            settings.should_render_clouds = mc.theWorld.provider.isSurfaceWorld() ? mc.gameSettings.shouldRenderClouds() : 0;
        }

        Entity viewEntity = mc.getRenderViewEntity();

        if (Utils.exists(viewEntity))
        {
            settings.has_blindness = ((EntityLivingBase) viewEntity).isPotionActive(MobEffects.BLINDNESS);
        }

        return settings;
    }
}
