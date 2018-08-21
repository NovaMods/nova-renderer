package com.continuum.nova.utils;

import com.continuum.nova.system.MinecraftSettings;
import net.minecraft.client.Minecraft;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.MobEffects;

import java.awt.*;

public class SettingsCommandBuilder
{
    // TODO: Make it a setting
    public static final Color FOG_COLOR = Color.LIGHT_GRAY;

    public MinecraftSettings getSettings(Minecraft mc)
    {
        MinecraftSettings settings = new MinecraftSettings();
        settings.view_bobbing = mc.gameSettings.viewBobbing;
        settings.render_distance = mc.gameSettings.renderDistanceChunks;
        settings.fog_color_red = /* mc.entityRenderer.getFogColorRed(); */  FOG_COLOR.getRed();
        settings.fog_color_green = /* mc.entityRenderer.getFogColorGreen(); */ FOG_COLOR.getGreen();
        settings.fog_color_blue = /* mc.entityRenderer.getFogColorBlue(); */ FOG_COLOR.getBlue();
        settings.anaglyph = mc.gameSettings.anaglyph;
        settings.display_height = mc.displayHeight;
        settings.display_width = mc.displayWidth;

        if (Utils.exists(mc.world))
        {
            settings.should_render_clouds = mc.world.provider.isSurfaceWorld() ? mc.gameSettings.shouldRenderClouds() : 0;
        }

        Entity viewEntity = mc.getRenderViewEntity();

        if (Utils.exists(viewEntity))
        {
            settings.has_blindness = ((EntityLivingBase) viewEntity).isPotionActive(MobEffects.BLINDNESS);
        }

        return settings;
    }
}
