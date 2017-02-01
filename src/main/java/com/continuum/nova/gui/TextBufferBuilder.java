package com.continuum.nova.gui;

import java.util.ArrayList;
import java.util.List;

/**
 * Singleton class which can build a buffer for text to render with
 *
 * Something import to note: This class expects that vertices will be specified as XYZUV. Any other format will cause
 * strange behavior
 *
 * @author ddubois
 */
public class TextBufferBuilder {
    private static TextBufferBuilder instance = new TextBufferBuilder();

    private List<Integer> indices = new ArrayList<>();
    private List<Float> vertexBuffer = new ArrayList<>();

    private TextBufferBuilder() {}

    public static TextBufferBuilder getInstance() {
        return instance;
    }

    public void reset() {
        indices.clear();
        vertexBuffer.clear();
    }

    public void addVertex(float x, float y, float z) {
        vertexBuffer.add(x);
        vertexBuffer.add(y);
        vertexBuffer.add(z);
    }

    public void addTexCoord(float u, float v) {
        vertexBuffer.add(u);
        vertexBuffer.add(v);
    }

    public void addIndex(int index) {
        indices.add(index);
    }
}
