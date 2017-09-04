package com.continuum.nova.chunks;

import net.minecraft.client.renderer.VertexBuffer;

import java.util.ArrayList;
import java.util.List;

/**
 * @author ddubois
 * @since 01-Sep-17
 */
public class FluidVertexBuffer extends VertexBuffer {
    private List<Integer> data = new ArrayList<>();

    public FluidVertexBuffer() {
        super(0);
    }

    @Override
    public VertexBuffer pos(double x, double y, double z) {
        float xFloat = (float)x;
        float yFloat = (float)y;
        float zFloat = (float)z;

        data.add(Float.floatToIntBits(xFloat));
        data.add(Float.floatToIntBits(yFloat));
        data.add(Float.floatToIntBits(zFloat));

        return this;
    }

    @Override
    public VertexBuffer color(float red, float green, float blue, float alpha) {
        return color((int)red * 255, (int)green * 255, (int)blue * 255, (int)alpha * 255);
    }

    @Override
    public VertexBuffer color(int red, int green, int blue, int alpha) {
        int data = 0;
        data |= (red & 255) << 24;
        data |= (green & 255) << 16;
        data |= (blue & 255) << 8;
        data |= alpha & 255;

        this.data.add(data);

        return this;
    }

    @Override
    public VertexBuffer tex(double u, double v) {
        float uFloat = (float)u;
        float tFloat = (float)v;

        data.add(Float.floatToIntBits(uFloat));
        data.add(Float.floatToIntBits(tFloat));

        return this;
    }

    @Override
    public VertexBuffer lightmap(int u, int v) {
        data.add((u << 16) + v);

        return this;
    }

    @Override
    public void endVertex() {
        ++this.vertexCount;
    }

    public List<Integer> getData() {
        return data;
    }
}
