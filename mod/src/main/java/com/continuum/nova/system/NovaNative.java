package com.continuum.nova.system;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.nio.IntBuffer;
import java.util.List;

public class NovaNative {
    public static final Logger LOG = LogManager.getLogger(NovaNative.class);

    public enum NovaVertexFormat {
        POS,
        POS_UV,
        POS_UV_LIGHTMAPUV_NORMAL_TANGENT,
        POS_UV_COLOR
    }

    public static class mc_atlas_texture {
        public int width;
        public int height;
        public int num_components;
        public byte[] texture_data;
        public String name;

        public mc_atlas_texture(int width, int height, int num_components, byte[] texture_data) {
            this.width = width;
            this.height = height;
            this.num_components = num_components;

            this.texture_data = texture_data;
        }

        public void setName(String name) {
            this.name = name;
        }

        @Override
        public String toString() {
            return "mc_atlas_texture{" +
                    "width=" + width +
                    ", height=" + height +
                    ", num_components=" + num_components +
                    ", name='" + name + '\'' +
                    '}';
        }
    }

    public static class mc_texture_atlas_location {
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

        @Override
        public String toString() {
            return "mc_texture_atlas_location{" +
                    "name='" + name + '\'' +
                    ", min_u=" + min_u +
                    ", max_u=" + max_u +
                    ", min_v=" + min_v +
                    ", max_v=" + max_v +
                    '}';
        }
    }

    public static class mc_chunk_render_object {
        public int format;
        public float x;
        public float y;
        public float z;
        public int id;
        public int[] vertex_data; // int[]
        public int[] indices;     // int[]
        public int vertex_buffer_size;
        public int index_buffer_size;

        public void setVertex_data(IntBuffer vertexData) {
            vertex_data = vertexData.array();

            vertex_buffer_size = vertex_data.length;
        }

        public void setIndices(List<Integer> indices) {
            this.indices = new int[indices.size()];
            for(int i = 0; i < this.indices.length; i++) {
                this.indices[i] = indices.get(i);
            }

            index_buffer_size = indices.size();
        }
    }

    public static class mc_settings {
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
    }

    public static class mc_gui_buffer  {
        public String texture_name;
        public int index_buffer_size;
        public int vertex_buffer_size;
        public int[] index_buffer; // int[]
        public float[] vertex_buffer; // float[]
        public String atlas_name;
    }

    public class mouse_button_event {
        public int button;
        public int action;
        public int mods;
        public int filled;
    }

    public class mouse_position_event {
        public int xpos;
        public int ypos;
        public int filled;
    }

    public class mouse_scroll_event {
        public double xoffset;
        public double yoffset;
        public int filled;
    }

    public class key_press_event {
        public int key;
        public int scancode;
        public int action;
        public int mods;
        public int filled;
    }

    public class key_char_event {
        public long unicode_char;
        public int filled;
    }

    public class window_size {
        public int height;
        public int width;
    }

    public enum GeometryType {
        BLOCK,
        ENTITY,
        FALLING_BLOCK,
        GUI,
        TEXT,
        GUI_BACKGROUND,
        CLOUD,
        SKY_DECORATION,
        SELECTION_BOX,
        GLINT,
        WEATHER,
        HAND,
        FULLSCREEN_QUAD,
        PARTICLE,
        LIT_PARTICLE,
        EYES
    }

    public enum NativeBoolean {
        FALSE,
        TRUE
    }

    public native void initialize();

    public native void execute_frame();

    public native void send_lightmap_texture(int[] data, int length, int width, int height);

    public native void add_texture(mc_atlas_texture texture);

    public native void add_texture_location(mc_texture_atlas_location location);

    public native int get_max_texture_size();

    public native void reset_texture_manager();

    public native  void add_chunk_geometry_for_filter(String filter_name, mc_chunk_render_object render_object);

    public native void remove_chunk_geometry_for_filter(String filter_name, mc_chunk_render_object render_object);

    public native boolean should_close();

    public native void add_gui_geometry(String type, mc_gui_buffer buffer);

    public native void clear_gui_buffers();

    public native void set_mouse_grabbed(boolean grabbed);

    public native mouse_button_event get_next_mouse_button_event();

    public native mouse_position_event get_next_mouse_position_event();

    public native mouse_scroll_event get_next_mouse_scroll_event();

    public native key_press_event get_next_key_press_event();

    public native key_char_event get_next_key_char_event();

    public native window_size get_window_size();

    public native  void set_fullscreen(int fullscreen);

    public native boolean display_is_active();

    public native void set_string_setting(String setting, String value);

    public native void set_float_setting(String setting_name, float setting_value);

    public native void set_player_camera_transform(double x, double y, double z, float yaw, float pitch);

    public native String get_materials_and_filters();

    public native void destruct();
}
