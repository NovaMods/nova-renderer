package com.continuum.nova.chunks;

import net.minecraft.client.renderer.VertexBuffer;

import net.minecraft.util.math.BlockPos;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.*;
import java.util.stream.Collectors;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import java.nio.IntBuffer;

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
            if(Float.compare(vertex.u, u) != 0) return false;
            return Float.compare(vertex.v, v) != 0;
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
    private BlockPos chunkPosition;
    private ArrayList<Integer> intDataStore= new ArrayList<Integer>();
    private Vertex curVertex = new Vertex();

    public CapturingVertexBuffer(BlockPos chunkPosition) {
        super(0);
        this.chunkPosition = chunkPosition;
        this.reset();
        this.vertexFormat=DefaultVertexFormats.BLOCK;
        this.vertexFormatElement = this.vertexFormat.getElement(0);
    }
    public void setChunkPos(BlockPos chunkPosition){
      this.chunkPosition = chunkPosition;
    }

    @Override
    public VertexBuffer pos(double x, double y, double z) {
        curVertex.x = (float)x - chunkPosition.getX();
        curVertex.y = (float)y - chunkPosition.getY();
        curVertex.z = (float)z - chunkPosition.getZ();

        return this;
    }

    private int getBufferSize()
    {
        return this.vertexCount * this.vertexFormat.getIntegerSize();
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
        List<Integer> intsList=curVertex.toInts();
        int[] intArr =new int[intsList.size()];
        int x=0;
        for( Integer i : intsList){
          intArr[x]=i.intValue();
          x++;
        }
        this.addVertexData(intArr);
        curVertex = new Vertex();
    }

    public List<Integer> getData() {
        List<Integer> finalData = new ArrayList<>();
        int[] arr = new int[this.rawIntBuffer.limit()];
        int oldPos=this.rawIntBuffer.position();
        ((IntBuffer) this.rawIntBuffer.position(0)).get(arr);
        this.rawIntBuffer.position(oldPos);
        for(int i:arr) {
            finalData.add(new Integer(i));
        }
        return finalData;
    }

    public IntBuffer getRawData() {
        return this.rawIntBuffer;
    }

    public boolean isEmpty(){
      return this.vertexCount<1;
    }
}
