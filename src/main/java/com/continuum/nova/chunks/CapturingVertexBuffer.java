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
    private BlockPos chunkPosition;
    private ArrayList<Integer> intDataStore= new ArrayList<Integer>();
    private Vertex curVertex = new Vertex();

    public CapturingVertexBuffer(BlockPos chunkPosition) {
        super(0);
        this.chunkPosition = chunkPosition;
                this.reset();
        this.vertexFormat=DefaultVertexFormats.BLOCK;
        this.vertexFormatElement = this.vertexFormat.getElement(0);
        //this.begin(7, DefaultVertexFormats.BLOCK);
    }
    public void setChunkPos(BlockPos chunkPosition){
      this.chunkPosition = chunkPosition;
    }

  /*  private void growBuffer(int p_181670_1_)
      {
          if (MathHelper.roundUp(p_181670_1_, 4) / 4 > this.rawIntBuffer.remaining() || this.vertexCount * this.vertexFormat.getNextOffset() + p_181670_1_ > this.byteBuffer.capacity())
          {
              int i = this.byteBuffer.capacity();
              int j = i + MathHelper.roundUp(p_181670_1_, 2097152);
              LOGGER.debug("Needed to grow BufferBuilder buffer: Old size {} bytes, new size {} bytes.", new Object[] {Integer.valueOf(i), Integer.valueOf(j)});
              int k = this.rawIntBuffer.position();
              ByteBuffer bytebuffer = GLAllocation.createDirectByteBuffer(j);
              this.byteBuffer.position(0);
              bytebuffer.put(this.byteBuffer);
              bytebuffer.rewind();
              this.byteBuffer = bytebuffer;
              this.rawFloatBuffer = this.byteBuffer.asFloatBuffer().asReadOnlyBuffer();
              this.rawIntBuffer = this.byteBuffer.asIntBuffer();
              this.rawIntBuffer.position(k);
              this.rawShortBuffer = this.byteBuffer.asShortBuffer();
              this.rawShortBuffer.position(k << 1);
          }
      }*/
    @Override
    public VertexBuffer pos(double x, double y, double z) {
        curVertex.x = (float)x - chunkPosition.getX();
        curVertex.y = (float)y - chunkPosition.getY();
        curVertex.z = (float)z - chunkPosition.getZ();

        return this;
    }
  //  @Override
//    public void putPosition(double x, double y, double z)
//  {
  //  pos(x,y,z);
  //}
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

    //temporarly disable this feature
  //  @Override
  //  public void putBrightness4(int brightness1, int brightness2, int brightness3, int brightness4)
  //  {
      //  int i = (this.vertexCount - 4) * this.vertexFormat.getIntegerSize() + this.vertexFormat.getUvOffsetById(1) / 4;
      //  int j = this.vertexFormat.getNextOffset() >> 2;
        /*this.rawIntBuffer.put(i, brightness1);
        this.rawIntBuffer.put(i + j, brightness2);
        this.rawIntBuffer.put(i + j * 2, brightness3);
        this.rawIntBuffer.put(i + j * 3, brightness4);*/
  //  }


    //temporarly disable this feature
  //  @Override
    //public void putColorMultiplier(float red, float green, float blue, int alpha)
  //  {
    //  color((int)red * 255, (int)green * 255, (int)blue * 255, (int)alpha );
      /*  int colorIndex = this.getColorIndex(p_178978_4_);
        int color = -1;

        if (!this.noColor)
        {
            color = this.rawIntBuffer.get(colorIndex);

            if (ByteOrder.nativeOrder() == ByteOrder.LITTLE_ENDIAN)
            {
                int packedRed = (int)((float)(color & 255) * red);
                int packedGreen = (int)((float)(color >> 8 & 255) * green);
                int packedBlue = (int)((float)(color >> 16 & 255) * blue);
                color = color & -16777216;
                color = color | packedBlue << 16 | packedGreen << 8 | packedRed;
            }
            else
            {
                int packedRed = (int)((float)(color >> 24 & 255) * red);
                int packedGreen = (int)((float)(color >> 16 & 255) * green);
                int packedBlue = (int)((float)(color >> 8 & 255) * blue);
                color = color & 255;    // Don't forget the alpha! I never leave home without it
                color = color | packedRed << 24 | packedGreen << 16 | packedBlue << 8;
            }
        }

        this.rawIntBuffer.put(colorIndex, color);*/
  //  }

    @Override
    public VertexBuffer lightmap(int u, int v) {
        curVertex.lmCoord = (u << 16) + v;

        return this;
    }
  /*  public void addVertexData(int[] vertexData)
    {
      //ArrayList<Integer> ar=new ArrayList<Integer>;
      for(int i :vertexData){
      intDataStore.add(i);
    }
    //   this.growBuffer(vertexData.length * 4);
       this.rawIntBuffer.position(this.getBufferSize());
        this.rawIntBuffer.put(vertexData);
        this.vertexCount += vertexData.length / this.vertexFormat.getIntegerSize();
    }*/
    @Override
    public void endVertex() {
      // ++this.vertexCount;

      /*  boolean shouldAdd = true;
        for(Vertex v : data) {
            if(v.equals(data)) {
                shouldAdd = false;
                break;
            }
        }

        if(shouldAdd) {
            data.add(curVertex);
        }*/

        List<Integer> intsList=curVertex.toInts();
        int[] intArr =new int[intsList.size()];
        int x=0;
        for( Integer i : intsList){
          intArr[x]=i.intValue();
          x++;
        }

        this.addVertexData(intArr);
          //intDataStore.addAll(curVertex.toInts());

        curVertex = new Vertex();
    }

    public List<Integer> getData() {
      List<Integer> finalData = new ArrayList<>();
      //  for(Vertex v : data) {
          //  finalData.addAll(v.toInts());
      //  }

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
    //  List<Integer> finalData = new ArrayList<>();
      //  for(Vertex v : data) {
          //  finalData.addAll(v.toInts());
      //  }

        /*int[] arr = new int[this.rawIntBuffer.limit()];
        int oldPos=this.rawIntBuffer.position();
    ((IntBuffer) this.rawIntBuffer.position(0)).get(arr);
    this.rawIntBuffer.position(oldPos);
        for(int i:arr) {
            finalData.add(new Integer(i));
        }*/

        return this.rawIntBuffer;//finalData;
    }
    public boolean isEmpty(){
      return this.vertexCount<1;
    }
}
