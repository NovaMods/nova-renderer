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
    public static final String BLOCK_COLOR_ATLAS_NAME   = "NovaColorAtlas";
    public static final String FONT_ATLAS_NAME          = "font";

    public static final String MODEL_FILENAME           = "all_models.json";
    public static final String CHUNK_FILENAME           = "chunk.json";

    public static final String MOD_NAME                 = "Nova Renderer";
    public static final String MOD_ID                   = "nova-renderer";
    public static final String VERSION                  = "0.0.4";

    public static final String CORE_MOD_NAME            = "Nova Renderer Core";
    public static final String CORE_MOD_ID              = "nova-renderer-core";
    public static final String CORE_MOD_VERSION         = "0.0.1";

    public static final ResourceLocation WHITE_TEXTURE_GUI_LOCATION = new ResourceLocation("white_gui");
    public static final ResourceLocation LIGHTMAP_LOCATION = new ResourceLocation("");

    // TODO: What is this?!
    public static final float[] STANDARD_BUTTON_LOCATIONS = {
            0.0f, 0.1796862f,
            0.78125f, 0.3359375f
    };
    // TODO: What is this?!
    public static final float WIDGETS_TEXTURE_FACTOR = 0.00390625F;
}
