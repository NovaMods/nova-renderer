package com.continuum.nova.system;

public class MinecraftAtlasTexture {
    public int width;
    public int height;
    public int num_components;
    public byte[] texture_data;
    public String name;

    public MinecraftAtlasTexture(int width, int height, int num_components, byte[] texture_data) {
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
