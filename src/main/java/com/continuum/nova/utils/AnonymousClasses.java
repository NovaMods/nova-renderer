package com.continuum.nova.utils;

import net.minecraft.client.Minecraft;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.stats.IStatStringFormat;

public class AnonymousClasses {

    public static class MinecraftStatStringFormatReplacement implements IStatStringFormat {
        @Override
        public String formatString(String str) {
            try {
                return String.format(str, GameSettings.getKeyDisplayString(Minecraft.getMinecraft().gameSettings.keyBindInventory.getKeyCode()));
            } catch (Exception exception) {
                return "Error: " + exception.getLocalizedMessage();
            }
        }
    }

}
