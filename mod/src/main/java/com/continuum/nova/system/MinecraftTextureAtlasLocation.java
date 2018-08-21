package com.continuum.nova.system;

public class MinecraftTextureAtlasLocation {
    public String name;
    public float min_u;
    public float max_u;
    public float min_v;
    public float max_v;

    public MinecraftTextureAtlasLocation(String name, float min_u, float min_v, float max_u, float max_v) {
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
