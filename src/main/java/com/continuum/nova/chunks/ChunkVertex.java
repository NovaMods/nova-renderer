package com.continuum.nova.chunks;

import glm.vec._2.Vec2;
import glm.vec._3.Vec3;

/**
 * @author ddubois
 * @since 30-Jul-17
 */
public class ChunkVertex {
    private Vec3 position;

    private byte red;
    private byte green;
    private byte blue;
    private byte alpha;

    private Vec2 uv = new Vec2();
    private Vec2 lightmapUv = new Vec2;

    private Vec3 normal = new Vec3();
    private Vec3 tangent = new Vec3();

    public ChunkVertex(int[] rawData, int startPos) {
        if(rawData.length % 7 != 0) {
            throw new IllegalArgumentException("Number of element in the vertex data must be divisible by seven");
        }

        float x = Float.intBitsToFloat(rawData[startPos]);
        float y = Float.intBitsToFloat(rawData[startPos + 1]);
        float z = Float.intBitsToFloat(rawData[startPos + 2]);

        position = new Vec3(x, y, z);
    }
}
