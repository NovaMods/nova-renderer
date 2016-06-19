package net.minecraft.client.renderer;

import java.nio.ByteBuffer;
import java.util.List;
import net.minecraft.client.renderer.vertex.VertexFormat;
import net.minecraft.client.renderer.vertex.VertexFormatElement;

public class WorldVertexBufferUploader
{
    @SuppressWarnings("incomplete-switch")
    public void draw(VertexBuffer p_181679_1_)
    {
        if (p_181679_1_.getVertexCount() > 0)
        {
            VertexFormat vertexformat = p_181679_1_.getVertexFormat();
            int i = vertexformat.getNextOffset();
            ByteBuffer bytebuffer = p_181679_1_.getByteBuffer();
            List<VertexFormatElement> list = vertexformat.getElements();

            for (int j = 0; j < list.size(); ++j)
            {
                VertexFormatElement vertexformatelement = (VertexFormatElement)list.get(j);
                VertexFormatElement.EnumUsage vertexformatelement$enumusage = vertexformatelement.getUsage();
                int k = vertexformatelement.getType().getGlConstant();
                int l = vertexformatelement.getIndex();
                bytebuffer.position(vertexformat.getOffset(j));

                switch (vertexformatelement$enumusage)
                {
                    case POSITION:
                        GlStateManager.glVertexPointer(vertexformatelement.getElementCount(), k, i, bytebuffer);
                        GlStateManager.glEnableClientState(32884);
                        break;

                    case UV:
                        OpenGlHelper.setClientActiveTexture(OpenGlHelper.defaultTexUnit + l);
                        GlStateManager.glTexCoordPointer(vertexformatelement.getElementCount(), k, i, bytebuffer);
                        GlStateManager.glEnableClientState(32888);
                        OpenGlHelper.setClientActiveTexture(OpenGlHelper.defaultTexUnit);
                        break;

                    case COLOR:
                        GlStateManager.glColorPointer(vertexformatelement.getElementCount(), k, i, bytebuffer);
                        GlStateManager.glEnableClientState(32886);
                        break;

                    case NORMAL:
                        GlStateManager.glNormalPointer(k, i, bytebuffer);
                        GlStateManager.glEnableClientState(32885);
                }
            }

            GlStateManager.glDrawArrays(p_181679_1_.getDrawMode(), 0, p_181679_1_.getVertexCount());
            int i1 = 0;

            for (int j1 = list.size(); i1 < j1; ++i1)
            {
                VertexFormatElement vertexformatelement1 = (VertexFormatElement)list.get(i1);
                VertexFormatElement.EnumUsage vertexformatelement$enumusage1 = vertexformatelement1.getUsage();
                int k1 = vertexformatelement1.getIndex();

                switch (vertexformatelement$enumusage1)
                {
                    case POSITION:
                        GlStateManager.glDisableClientState(32884);
                        break;

                    case UV:
                        OpenGlHelper.setClientActiveTexture(OpenGlHelper.defaultTexUnit + k1);
                        GlStateManager.glDisableClientState(32888);
                        OpenGlHelper.setClientActiveTexture(OpenGlHelper.defaultTexUnit);
                        break;

                    case COLOR:
                        GlStateManager.glDisableClientState(32886);
                        GlStateManager.resetColor();
                        break;

                    case NORMAL:
                        GlStateManager.glDisableClientState(32885);
                }
            }
        }

        p_181679_1_.reset();
    }
}
