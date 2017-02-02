package com.continuum.nova.gui;

import com.continuum.nova.NovaNative;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;

import java.util.List;

/**
 * Builds the set of quads which render a given string
 *
 * @author ddubois
 */
public class RenderObject {
    private String texutre;
    private List<Double> vboData;
    private DefaultVertexFormats vertexFormat;
    private int id;
    private NovaNative.GeometryType geometryType;

    /**
     * Creates a new RenderObejct with the specified texture and vertex format
     *
     * @param id A unique identifier for this RenderObject. This is mostly useful for the GUI so that you can send new
     *           geometry for a GUI element to support i.e. button UVs changing when you hover over a button
     * @param texture The texture to use with this RenderObject
     * @param vertexFormat The format of the vertices
     * @param geometryType The type of geometry contained in this RenderObject
     */
    RenderObject(int id, String texture, DefaultVertexFormats vertexFormat, NovaNative.GeometryType geometryType) {
        this.id = id;
        this.texutre = texture;
        this.vertexFormat = vertexFormat;
        this.geometryType = geometryType;
    }

    /**
     * Adds the specified position to the current vertex buffer
     *
     * @param x The x position of the vertex
     * @param y The y position of the vertex
     * @param z The z position of the vertex
     * @return This RenderObject
     */
    RenderObject pos(double x, double y, double z) {
        vboData.add(x);
        vboData.add(y);
        vboData.add(z);

        return this;
    }

    /**
     * Adds the specifies texture coordinate to the current bertex buffer
     *
     * @param u The u part of the texture coordinate
     * @param v The v part of the texture coordinate
     * @return Thie RenderObject
     */
    RenderObject tex(double u, double v) {
        vboData.add(u);
        vboData.add(v);

        return this;
    }
}
