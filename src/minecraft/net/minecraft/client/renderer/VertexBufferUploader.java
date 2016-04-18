package net.minecraft.client.renderer;

public class VertexBufferUploader extends WorldVertexBufferUploader
{
    private net.minecraft.client.renderer.vertex.VertexBuffer vertexBuffer = null;

    public void draw(VertexBuffer p_181679_1_)
    {
        p_181679_1_.reset();
        this.vertexBuffer.bufferData(p_181679_1_.getByteBuffer());
    }

    public void setVertexBuffer(net.minecraft.client.renderer.vertex.VertexBuffer vertexBufferIn)
    {
        this.vertexBuffer = vertexBufferIn;
    }
}
