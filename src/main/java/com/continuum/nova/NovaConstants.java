package com.continuum.nova;

import net.minecraft.util.ResourceLocation;

/**
 * Holds constants that any Nova code can access
 *
 * @author ddubois
 * @since 11-Jul-17
 */
public class NovaConstants {
    public static final int CHUNK_WIDTH                 = 16;
    public static final int CHUNK_HEIGHT                = 256;
    public static final int CHUNK_DEPTH                 = 16;

    public static final String GUI_ATLAS_NAME           = "gui";
    public static final String BLOCK_COLOR_ATLAS_NAME   = "block_color";
    public static final String FONT_ATLAS_NAME          = "font";

    public static final String MODEL_FILENAME           = "all_models.json";
    public static final String CHUNK_FILENAME           = "chunk.json";

    public static final String MODID                    = "Nova Renderer";
    public static final String VERSION                  = "0.0.4";

    public static final ResourceLocation WHITE_TEXTURE_GUI_LOCATION = new ResourceLocation("white_gui");
    public static final ResourceLocation LIGHTMAP_LOCATION = new ResourceLocation("");
}
