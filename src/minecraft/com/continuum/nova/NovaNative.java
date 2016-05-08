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

    /**
     * AtlasType and TextureType are provided so you don't have to throw a bunch of ints into a gross C soup. You can
     * use nicely peeled carrots and chopped onions and a ham bone.
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
}