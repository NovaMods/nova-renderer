package com.continuum.nova;

import com.continuum.nova.input.Mouse;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.gui.ScaledResolution;
import net.minecraft.util.ResourceLocation;
import org.apache.commons.lang3.ArrayUtils;

import java.util.*;

public class NovaDraw {

    private static int mouseX, mouseY;

    public static int getMouseX() {
        return mouseX;
    }

    public static int getMouseY() {
        return mouseY;
    }

    static HashMap<ResourceLocation, Buffers> buffers = new HashMap<>();

    static void clearBuffers() {
        buffers.clear();
        NovaNative.INSTANCE.clear_gui_buffers();
    }

    public static void draw(ResourceLocation texture, Integer[] indexBuffer, Float[] vertexbuffer) {
        if (buffers.containsKey(texture)) {
            buffers.get(texture).add(indexBuffer, vertexbuffer);
        } else {
            buffers.put(texture, new Buffers().add(indexBuffer, vertexbuffer));
        }
    }

    public static void draw(ResourceLocation texture, Integer[] indexBuffer, Vertex[] vertices) {
        Float[] vertexbuffer = new Float[vertices.length * 5];
        for (int v = 0; v < vertices.length; v++) {
            vertexbuffer[v * 5] = vertices[v].x;
            vertexbuffer[v * 5 + 1] = vertices[v].y;
            vertexbuffer[v * 5 + 2] = vertices[v].z;
            vertexbuffer[v * 5 + 3] = vertices[v].u;
            vertexbuffer[v * 5 + 4] = vertices[v].v;
        }

        if (buffers.containsKey(texture)) {
            buffers.get(texture).add(indexBuffer, vertexbuffer);
        } else {
            buffers.put(texture, new Buffers().add(indexBuffer, vertexbuffer));
        }
    }

    public static void drawRectangle(ResourceLocation texture, int x, int y, int width, int height, float texX, float texY, float texWidth, float texHeight) {
        Integer[] indexBuffer = new Integer[]{0, 1, 2, 2, 1, 3};
        Vertex[] vertices = new Vertex[]{
                new Vertex(
                        x, y,
                        texX, texY
                ),
                new Vertex(
                        x + width, y,
                        texX + texWidth, texY
                ),
                new Vertex(
                        x, y + height,
                        texX, texY + texHeight
                ),
                new Vertex(
                        x + width, y + height,
                        texX + texWidth, texY + texHeight
                )
        };
        draw(texture, indexBuffer, vertices);
    }

    static void computeCorrectMousePosition() {
        Minecraft mc = Minecraft.getMinecraft();

        // compute mouse position (from EntityRenderer.java)
        final ScaledResolution scaledresolution = new ScaledResolution(mc);
        int i1 = scaledresolution.getScaledWidth();
        int j1 = scaledresolution.getScaledHeight();
        final int k1 = Mouse.getX() * i1 / mc.displayWidth;
        final int l1 = j1 - Mouse.getY() * j1 / mc.displayHeight - 1;
        // set mouse position
        mouseX = k1;
        mouseY = l1;
    }

    public static void novaDrawScreen(GuiScreen screen) {
        computeCorrectMousePosition();

        if (screen.checkStateChanged()) {
            clearBuffers();
            screen.drawNova();

            for (ResourceLocation texture : buffers.keySet()) {
                Buffers b = buffers.get(texture);
                NovaNative.INSTANCE.send_gui_buffer_command(b.toNativeCommand(texture));
            }
        }
    }

    public static class Vertex {
        public float x;
        public float y;
        public float z;
        public float u;
        public float v;

        public Vertex(int x, int y, float u, float v) {
            this.x = x;
            this.y = y;
            this.z = 0;
            this.u = u;
            this.v = v;
        }

        public Vertex(int x, int y, float z, float u, float v) {
            this.x = x;
            this.y = y;
            this.z = z;
            this.u = u;
            this.v = v;
        }
    }

    static class Buffers {
        public List<Integer> indexBuffer = new ArrayList<>();
        public List<Float> vertexBuffer = new ArrayList<>();

        public Buffers add(Integer[] indexBuffer, Float[] vertexBuffer) {
            //System.out.println("write index: " + Arrays.toString(indexBuffer));
            //System.out.println("write vertex: " + Arrays.toString(vertexBuffer));

            // add index buffer
            int indexbuffer_size = this.vertexBuffer.size() / 5;
            for (int index : indexBuffer) {
                this.indexBuffer.add(index + indexbuffer_size);
            }

            // add vertex buffer
            Collections.addAll(this.vertexBuffer, vertexBuffer);

            return this;
        }

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

            return command;
        }
    }

}
