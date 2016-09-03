package com.continuum.nova;

import com.sun.jna.*;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

public interface NovaNative extends Library
{
    NovaNative INSTANCE = (NovaNative) Native.loadLibrary("nova-renderer", NovaNative.class);

    class mc_atlas_texture extends Structure
    {
        public int width;
        public int height;
        public int num_components;
        public Pointer texture_data;

        public mc_atlas_texture(int width, int height, int num_components, byte[] texture_data)
        {
            this.width = width;
            this.height = height;
            this.num_components = num_components;
            this.texture_data = new Memory(width * height * num_components * Native.getNativeSize(Byte.TYPE));

            for (int i = 0; i < width * height * num_components; i++)
            {
                this.texture_data.setByte(i, texture_data[i]);
            }
        }

        @Override
        public List<String> getFieldOrder()
        {
            return Arrays.asList("width", "height", "num_components", "texture_data");
        }
    }

    class mc_texture_atlas_location extends Structure
    {
        public String name;
        public float min_u;
        public float max_u;
        public float min_v;
        public float max_v;

        public mc_texture_atlas_location(String name, float min_u, float min_v, float max_u, float max_v)
        {
            this.name = name;
            this.min_u = min_u;
            this.max_u = max_u;
            this.min_v = min_v;
            this.max_v = max_v;
        }

        @Override
        public List<String> getFieldOrder()
        {
            return Arrays.asList("name", "min_u", "max_u", "min_v", "max_v");
        }
    }

    class mc_block extends Structure
    {
        public boolean is_on_fire;
        public int block_id;

        @Override
        public List<String> getFieldOrder()
        {
            return Arrays.asList("is_on_fire", "block_id");
        }
    }

    class mc_chunk extends Structure
    {
        public long chunk_id;

        public boolean is_dirty;
        public mc_block[] blocks = new mc_block[16 * 16 * 16];

        @Override
        public List<String> getFieldOrder()
        {
            return Arrays.asList("chunk_id", "is_dirty", "blocks");
        }
    }

    class mc_gui_button extends Structure
    {
        public int x_position;
        public int y_position;
        public int width;
        public int height;
        public String text;
        public boolean is_pressed;

        @Override
        protected List<String> getFieldOrder()
        {
            return Arrays.asList("x_position", "y_position", "width", "height", "is_pressed");
        }
    }

    class mc_gui_screen extends Structure
    {
        public mc_gui_button[] buttons = new mc_gui_button[22];
        public int num_buttons;

        @Override
        public List<String> getFieldOrder()
        {
            return Arrays.asList("buttons", "num_buttons");
        }
    }

    class mc_render_menu_params extends Structure
    {
        public mc_gui_screen cur_screen;

        @Override
        protected List<String> getFieldOrder()
        {
            return Collections.singletonList("cur_screen");
        }
    }

    class mc_render_world_params extends Structure
    {
        public double camera_x;
        public double camera_y;
        public double camera_z;

        @Override
        protected List<String> getFieldOrder()
        {
            return Arrays.asList(
                       "camera_x", "camera_y", "camera_z"
                   );
        }
    }

    class mc_render_command extends Structure
    {
        public long previous_frame_time;

        public float mouse_x;
        public float mouse_y;

        public mc_render_world_params world_params;
        public mc_render_menu_params menu_params;

        @Override
        protected List<String> getFieldOrder()
        {
            return Arrays.asList("previous_frame_time", "mouse_x", "mouse_y", "world_params", "menu_params");
        }
    }

    class mc_settings extends Structure
    {
        public boolean render_menu;

        public boolean anaglyph;

        public double fog_color_red;
        public double fog_color_green;
        public double fog_color_blue;

        public int display_width;
        public int display_height;

        public boolean view_bobbing;
        public int should_render_clouds;

        public int render_distance;

        public boolean has_blindness;

        @Override
        protected List<String> getFieldOrder()
        {
            return Arrays.asList(
                       "anaglyph", "fog_color_red", "fog_color_green", "fog_color_blue", "display_width",
                       "display_height", "view_bobbing", "should_render_clouds", "render_distance", "render_menu",
                       "has_blindness"
                   );
        }
    }

    class mc_add_chunk_command extends Structure
    {
        public mc_chunk new_chunk;

        public float chunk_x;
        public float chunk_y;
        public float chunk_z;

        @Override
        protected List<String> getFieldOrder()
        {
            return Arrays.asList("new_chunk", "chunk_x", "chunk_y", "chunk_x");
        }
    }

    class mc_set_gui_screen_command extends Structure
    {
        public mc_gui_screen screen;

        @Override
        protected List<String> getFieldOrder()
        {
            return Collections.singletonList("screen");
        }
    }

    enum AtlasType
    {
        TERRAIN,
        ENTITIES,
        GUI,
        PARTICLES,
        EFFECTS,
        FONT,
        NUM_ATLASES
    }

    enum TextureType
    {
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

    void send_change_gui_screen_command(mc_set_gui_screen_command set_gui_screen);
}