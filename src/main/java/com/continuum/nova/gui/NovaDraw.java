package com.continuum.nova.gui;

import com.continuum.nova.NovaNative;
import com.continuum.nova.NovaRenderer;
import com.continuum.nova.input.Mouse;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.gui.ScaledResolution;
import net.minecraft.util.ResourceLocation;
import org.apache.commons.lang3.ArrayUtils;

import java.awt.*;
import java.util.*;
import java.util.List;

public class NovaDraw {

    private static int mouseX, mouseY;

    static HashMap<ResourceLocation, Buffers> buffers = new HashMap<>();

    /**
     * private constructor cause this class only has static things
     */
    private NovaDraw() {}

    private static void clearBuffers() {
        buffers.clear();
        NovaNative.INSTANCE.clear_gui_buffers();
    }

    public static int getMouseX() {
        return mouseX;
    }

    public static int getMouseY() {
        return mouseY;
    }

    /**
     * Add data to the indexBuffer and vertexBuffer which is associated with the specified texture.
     *
     * @param texture      the texture by which the index buffers and vertex buffers are grouped
     * @param indexBuffer  index buffer data
     * @param vertexbuffer vertex buffer data
     */
    public static void draw(ResourceLocation texture, Integer[] indexBuffer, Float[] vertexbuffer) {
        if(buffers.containsKey(texture)) {
            buffers.get(texture).add(indexBuffer, vertexbuffer);
        } else {
            buffers.put(texture, new Buffers().add(indexBuffer, vertexbuffer));
        }
    }

    /**
     * Add data to the indexBuffer and vertexBuffer which is associated with the specified texture.
     *
     * @param texture     the texture by which the index buffers and vertex buffers are grouped
     * @param indexBuffer index buffer data
     * @param vertices    the vertices as Vertex objects
     */
    public static void draw(ResourceLocation texture, Integer[] indexBuffer, Vertex[] vertices) {
        Float[] vertexbuffer = new Float[vertices.length * 8];
        for (int v = 0; v < vertices.length; v++) {
            vertexbuffer[v * 8] = vertices[v].x;
            vertexbuffer[v * 8 + 1] = vertices[v].y;
            vertexbuffer[v * 8 + 2] = vertices[v].z;
            vertexbuffer[v * 8 + 3] = vertices[v].u;
            vertexbuffer[v * 8 + 4] = vertices[v].v;
            vertexbuffer[v * 8 + 5] = vertices[v].r;
            vertexbuffer[v * 8 + 6] = vertices[v].g;
            vertexbuffer[v * 8 + 7] = vertices[v].b;
        }

        if(buffers.containsKey(texture)) {
            buffers.get(texture).add(indexBuffer, vertexbuffer);
        } else {
            buffers.put(texture, new Buffers().add(indexBuffer, vertexbuffer));
        }
    }

    /**
     * Add the vertices and indices for a rectangle.
     * <p>
     * Minecrafts GUI classes frequently call "drawTexturedModalRect" to draw textured rectangles.
     * These calls can be easily converted to calls to this function.
     * The only real difference is that minecraft uses texture coordinates in pixels and we use uv coordinates,
     * but they can be converted by dividing them by 256.
     *
     * @param texture   the texture
     * @param x         screen coordinates in pixel
     * @param y         screen coordinates in pixel
     * @param width     screen coordinates in pixel
     * @param height    screen coordinates in pixel
     * @param texX      texture / UV coordinates, relative to the original minecraft textures (not the texture atlas)
     * @param texY      texture / UV coordinates, relative to the original minecraft textures (not the texture atlas)
     * @param texWidth  texture / UV coordinates, relative to the original minecraft textures (not the texture atlas)
     * @param texHeight texture / UV coordinates, relative to the original minecraft textures (not the texture atlas)
     */
    public static void drawRectangle(ResourceLocation texture, int x, int y, float z, int width, int height, float texX, float texY, float texWidth, float texHeight) {
        Integer[] indexBuffer = new Integer[]{0, 1, 2, 2, 1, 3};
        Vertex[] vertices = new Vertex[]{
                new Vertex(
                        x, y, z,
                        texX, texY
                ),
                new Vertex(
                        x + width, y, z,
                        texX + texWidth, texY
                ),
                new Vertex(
                        x, y + height, z,
                        texX, texY + texHeight
                ),
                new Vertex(
                        x + width, y + height, z,
                        texX + texWidth, texY + texHeight
                )
        };
        draw(texture, indexBuffer, vertices);
    }

    public static void drawRectangle(ResourceLocation texture, int x, int y, int width, int height, float texX, float texY, float texWidth, float texHeight) {
        drawRectangle(texture, x, y, 0.5f, width, height, texX, texY, texWidth, texHeight);
    }


    /**
     * This code is from the EntityRenderer class.
     * <p>
     * We can't use Mouse.getX/Y() because for example, the GuiButton's hovered state depends on the comparison
     * of the button's yPosition and the mouseY value.
     * This mouseY value is expected to be upside down.
     */
    private static void computeCorrectMousePosition() {
        Minecraft mc = Minecraft.getMinecraft();

        // compute mouse position (from EntityRenderer.java)
        final ScaledResolution scaledresolution = new ScaledResolution(mc);
        int i1 = scaledresolution.getScaledWidth();
        int j1 = scaledresolution.getScaledHeight();
        final int k1 = Mouse.getEventX() * i1 / mc.displayWidth;
        final int l1 = j1 - Mouse.getEventY() * j1 / mc.displayHeight - 1;
        // set mouse position
        mouseX = k1;
        mouseY = l1;
    }

    /**
     * This build the index and vertex buffers of the specified GUI screen, grouped by the original minecraft textures,
     * if any state changes occured to any GUI element (hovered, visibility, completely different screen).
     *
     * @param screen the gui screen
     */
    public static void novaDrawScreen(GuiScreen screen) {
        computeCorrectMousePosition();

        if(screen.checkStateChanged()) {
            clearBuffers();
            screen.drawNova();

            for (Map.Entry<ResourceLocation, Buffers> entry : buffers.entrySet()) {
                Buffers b = entry.getValue();
                ResourceLocation texture = entry.getKey();
                NovaNative.INSTANCE.send_gui_buffer_command(b.toNativeCommand(texture));
            }
        }
    }

    public static class Vertex {
        // Position
        public float x;
        public float y;
        public float z;

        // Texture coordinate
        public float u;
        public float v;

        // Vertex color
        public float r;
        public float g;
        public float b;

        public Vertex(int x, int y, float u, float v) {
            this(x, y, 0.5f, u, v);
        }

        private Vertex(float x, float y, float z, float u, float v) {
            this(x, y, z, u, v, new Color(1.0f, 1.0f, 1.0f));
        }

        public Vertex(float x, float y, float z, float u, float v, Color color) {
            if(color == null) {
                color = new Color(255, 255, 255);
            }

            this.x = x;
            this.y = y;
            this.z = z;
            this.u = u;
            this.v = v;
            this.r = (float)color.getRed() / 255.f;
            this.g = (float)color.getGreen() / 255.f;
            this.b = (float)color.getBlue() / 255.f;
        }
    }

    /**
     * This class aggregates the index and vertex buffers of all GUI elements which share the same texture.
     * <p>
     * It is only used internally in NovaDraw.
     */
    static class Buffers {
        public List<Integer> indexBuffer = new ArrayList<>();
        public List<Float> vertexBuffer = new ArrayList<>();

        public Buffers add(Integer[] indexBuffer, Float[] vertexBuffer) {
            // add index buffer
            int indexbuffer_size = this.vertexBuffer.size() / 8;    // 8 is the number of floats per vertex
            for (int index : indexBuffer) {
                this.indexBuffer.add(index + indexbuffer_size);
            }

            // add vertex buffer
            Collections.addAll(this.vertexBuffer, vertexBuffer);

            return this;
        }

        /**
         * Generate a native struct which can be sent to c++.
         * <p>
         * Native memory is allocated here so that the index and vertex buffers can be dynamically sized
         * (this is the only way to achieve that, otherwise we would have to specify a fixed array size
         * with a maximum number of vertices.)
         *
         * @param texture the texture
         * @return the native struct
         */
        public NovaNative.mc_gui_send_buffer_command toNativeCommand(ResourceLocation texture) {


            // create a new struct
            NovaNative.mc_gui_send_buffer_command command = new NovaNative.mc_gui_send_buffer_command();
            command.texture_name = texture.getResourcePath();

            // assign the index buffer
            command.index_buffer_size = this.indexBuffer.size();
            command.index_buffer = new Memory(command.index_buffer_size * Native.getNativeSize(Integer.TYPE));
            for (int i = 0; i < command.index_buffer_size; i++) {
                Integer index = this.indexBuffer.get(i);
                command.index_buffer.setInt(i * Native.getNativeSize(Integer.TYPE), (int) (index != null ? index : 0));
            }

            // assign the vertex buffer
            command.vertex_buffer_size = this.vertexBuffer.size();
            command.vertex_buffer = new Memory(command.vertex_buffer_size * Native.getNativeSize(Float.TYPE));
            for (int i = 0; i < command.vertex_buffer_size; i++) {
                Float vertex = this.vertexBuffer.get(i);
                command.vertex_buffer.setFloat(i * Native.getNativeSize(Float.TYPE), (float) (vertex != null ? vertex : 0));
            }

            NovaNative.TextureType atlasType = NovaRenderer.atlasTextureOfSprite(texture);
            command.texture_atlas = atlasType.ordinal();

            return command;
        }
    }

}
