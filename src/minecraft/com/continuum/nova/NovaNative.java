package com.continuum.nova;

import com.sun.jna.*;

/**
 * Provides a direct interface into the native code
 *
 * This class is going to be really big. Sorry.
 *
 * @author David
 */
interface NovaNative extends Library {
    NovaNative INSTANCE = (NovaNative) Native.loadLibrary("nova-renderer", NovaNative.class);

    class mc_atlas_texture extends Structure {
        public int width;
        public int height;
        public int num_components;
        public Pointer texture_data;

        public mc_atlas_texture(int width, int height, int num_components, byte[] texture_data) {
            this.width = width;
            this.height = height;
            this.num_components = num_components;
            this.texture_data = new Memory(width * height * num_components * Native.getNativeSize(Byte.TYPE));
            for(int i = 0; i < width * height * num_components; i++) {
                this.texture_data.setByte(i, texture_data[i]);
            }
        }
    }

    class mc_texture_atlas_location extends Structure {
        public String name;
        public float min_u;
        public float max_u;
        public float min_v;
        public float max_v;

        public mc_texture_atlas_location(String name, float min_u, float min_v, float max_u, float max_v) {
            this.name = name;
            this.min_u = min_u;
            this.max_u = max_u;
            this.min_v = min_v;
            this.max_v = max_v;
        }
    }

    class mc_block extends Structure {
        public boolean is_on_fire;
        public int block_id;
    }

    /**
     * Holds the data in a single chunk
     */
    class mc_chunk extends Structure {
        public long chunk_id;
        public mc_block[] blocks = new mc_block[16 * 16 * 16];
    }

    class mc_button extends Structure {
        public int x_position;
        public int y_position;
        public int width;
        public int height;
        public String text;
    }

    /**
     * Holds everything you need to render a single Minecraft GUI screen
     */
    class mc_gui_screen extends Structure {
        /**
         * Supports up to 22 buttons. This is the maximum that can be reasonably crammed onto the Minecraft GUI. I
         * might add more buttons in the future, depending on how badly mod authors break everything
         */
        public mc_button[] buttons = new mc_button[22];
    }

    /**
     * A class to hold things like what menu we're currently on and whatnot. Currently empty because rendering menus is
     * hard
     */
    class mc_render_menu_params extends Structure {
        public int param1;
    }

    /**
     * A class to hold all the things needed to render the world
     *
     * <p>Hopefully using this class will keep mc_render_command from getting too complicated</p>
     */
    class mc_render_world_params extends Structure {
        /**
         * The position of the camera
         */
        public double camera_x;
        public double camera_y;
        public double camera_z;

        /**
         * The render distance, in chunks
         */
        public int render_distance;

        /**
         * Whether or not the has_blindness status effect is in effect
         */
        public boolean has_blindness;

        public double fog_color_red;
        public double fog_color_green;
        public double fog_color_blue;
        public boolean view_bobbing;
        public int should_render_clouds;

        // Supports up to 12 x 12 chunks
        public mc_chunk[] chunks = new mc_chunk[25 * 25];

        public int num_chunks;
    }

    class mc_render_command extends Structure {
        /**
         * If true, we should render the menu and use the world framebuffer from the last frame as the menu background.
         * If false, we should render the world
         */
        public boolean render_menu;

        /**
         * If true, we should render the chunks and entities of the Real World (TM). If false, don't.
         */
        public boolean render_world;

        /**
         * The system time, in milliseconds
         */
        public long previous_frame_time;

        /**
         * Userd for determining camera rotation/mouse cursor position
         */
        public float mouse_x;
        public float mouse_y;

        /**
         * If true, render the world as red/blue anaglyph. If false, don't do that.
         */
        public boolean anaglyph;

        /**
         * The height and width of the Minecraft screen
         */
        public int display_width;
        public int display_height;

        public mc_render_world_params world_params;
        public mc_render_menu_params menu_params;
    }

    /**
     * AtlasType and TextureType are provided so you don't have to throw a bunch of ints into a gross C soup. You can
     * use strongly-typed carrots and chopped onions and a ham bone.
     */
    enum AtlasType {
        TERRAIN,
        ENTITIES,
        GUI,
        PARTICLES,
        EFFECTS,
        FONT,
        NUM_ATLASES
    }

    enum TextureType {
        ALBEDO,
        NORMAL,
        SPECULAR
    }

    void init_nova();

    void add_texture(mc_atlas_texture texture, int atlas_type, int texture_type);

    void add_texture_location(mc_texture_atlas_location location);

    int get_max_texture_size();

    void reset_texture_manager();

    void send_render_command(mc_render_command cmd);

    void do_test_render();

    boolean should_close();
}