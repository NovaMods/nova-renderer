package com.continuum.nova.chunks;

import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.util.math.BlockPos;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.*;
import java.util.stream.Collectors;

/**
 * @author ddubois
 * @since 01-Sep-17
 */
public class CapturingVertexBuffer extends VertexBuffer {
    private static class Vertex {
        float x;
        float y;
        float z;
        int color;
        float u;
        float v;
        int lmCoord;

        @Override
        public boolean equals(Object o) {
            if(this == o) return true;
            if(!(o instanceof Vertex)) return false;

            Vertex vertex = (Vertex) o;

            if(Float.compare(vertex.x, x) != 0) return false;
            if(Float.compare(vertex.y, y) != 0) return false;
            if(Float.compare(vertex.z, z) != 0) return false;
            // if(color != vertex.color) return false;
            if(Float.compare(vertex.u, u) != 0) return false;
            return Float.compare(vertex.v, v) != 0;
            // return lmCoord == vertex.lmCoord;
        }

        @Override
        public int hashCode() {
            int result = (x != +0.0f ? Float.floatToIntBits(x) : 0);
            result = 31 * result + (y != +0.0f ? Float.floatToIntBits(y) : 0);
            result = 31 * result + (z != +0.0f ? Float.floatToIntBits(z) : 0);
            result = 31 * result + color;
            result = 31 * result + (u != +0.0f ? Float.floatToIntBits(u) : 0);
            result = 31 * result + (v != +0.0f ? Float.floatToIntBits(v) : 0);
            result = 31 * result + lmCoord;
            return result;
        }

        List<Integer> toInts() {
            List<Integer> ints = new ArrayList<>();

            ints.add(Float.floatToIntBits(x));
            ints.add(Float.floatToIntBits(y));
            ints.add(Float.floatToIntBits(z));

            ints.add(color);

            ints.add(Float.floatToIntBits(u));
            ints.add(Float.floatToIntBits(v));

            ints.add(lmCoord);

            return ints;
        }
    }

    private List<Vertex> data = new ArrayList<>();
    private final BlockPos chunkPosition;

    private Vertex curVertex = new Vertex();

    public CapturingVertexBuffer(BlockPos chunkPosition) {
        super(0);
        this.chunkPosition = chunkPosition;
    }

    @Override
    public VertexBuffer pos(double x, double y, double z) {
        curVertex.x = (float)x - chunkPosition.getX();
        curVertex.y = (float)y - chunkPosition.getY();
        curVertex.z = (float)z - chunkPosition.getZ();

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

        curVertex.color = data;

        return this;
    }

    @Override
    public VertexBuffer tex(double u, double v) {
        curVertex.u = (float)u;
        curVertex.v = (float)v;

        return this;
    }

    @Override
    public VertexBuffer lightmap(int u, int v) {
        curVertex.lmCoord = (u << 16) + v;

        return this;
    }

    @Override
    public void endVertex() {
        ++this.vertexCount;

        boolean shouldAdd = true;
        for(Vertex v : data) {
            if(v.equals(data)) {
                shouldAdd = false;
                break;
            }
        }

        if(shouldAdd) {
            data.add(curVertex);
        }

        curVertex = new Vertex();
    }

    public List<Integer> getData() {
        List<Integer> finalData = new ArrayList<>();

        for(Vertex v : data) {
            finalData.addAll(v.toInts());
        }

        return finalData;
    }
}
