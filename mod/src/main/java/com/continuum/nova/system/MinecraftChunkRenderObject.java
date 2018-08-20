package com.continuum.nova.system;

import java.nio.IntBuffer;
import java.util.List;

public class MinecraftChunkRenderObject {
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
