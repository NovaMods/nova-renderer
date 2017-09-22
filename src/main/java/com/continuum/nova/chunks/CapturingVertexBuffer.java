package com.continuum.nova.chunks;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.util.math.BlockPos;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.ArrayList;
import java.util.List;

/**
 * @author ddubois
 * @since 01-Sep-17
 */
public class CapturingVertexBuffer extends VertexBuffer {
    private List<Integer> data = new ArrayList<>();
    private final BlockPos chunkPosition;

    public CapturingVertexBuffer(BlockPos chunkPosition) {
        super(0);
        this.chunkPosition = chunkPosition;
    }

    @Override
    public VertexBuffer pos(double x, double y, double z) {
        float xFloat = (float)x - chunkPosition.getX();
        float yFloat = (float)y - chunkPosition.getY();
        float zFloat = (float)z - chunkPosition.getZ();

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
        float vFloat = (float)v;

        data.add(Float.floatToIntBits(uFloat));
        data.add(Float.floatToIntBits(vFloat));

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
