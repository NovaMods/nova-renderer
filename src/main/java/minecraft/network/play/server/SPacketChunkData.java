package net.minecraft.network.play.server;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import java.io.IOException;
import net.minecraft.network.Packet;
import net.minecraft.network.PacketBuffer;
import net.minecraft.network.play.INetHandlerPlayClient;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.chunk.storage.ExtendedBlockStorage;

public class SPacketChunkData implements Packet<INetHandlerPlayClient>
{
    private int chunkX;
    private int chunkZ;
    private int field_186948_c;
    private byte[] field_186949_d;
    private boolean field_149279_g;

    public SPacketChunkData()
    {
    }

    public SPacketChunkData(Chunk chunkIn, boolean p_i46941_2_, int p_i46941_3_)
    {
        this.chunkX = chunkIn.xPosition;
        this.chunkZ = chunkIn.zPosition;
        this.field_149279_g = p_i46941_2_;
        boolean flag = !chunkIn.getWorld().provider.getHasNoSky();
        this.field_186949_d = new byte[func_186944_a(chunkIn, p_i46941_2_, flag, p_i46941_3_)];
        this.field_186948_c = func_186947_a(new PacketBuffer(this.func_186945_f()), chunkIn, p_i46941_2_, flag, p_i46941_3_);
    }

    /**
     * Reads the raw packet data from the data stream.
     */
    public void readPacketData(PacketBuffer buf) throws IOException
    {
        this.chunkX = buf.readInt();
        this.chunkZ = buf.readInt();
        this.field_149279_g = buf.readBoolean();
        this.field_186948_c = buf.readVarIntFromBuffer();
        int i = buf.readVarIntFromBuffer();

        if (i > 2097152)
        {
            throw new RuntimeException("Chunk Packet trying to allocate too much memory on read.");
        }
        else
        {
            this.field_186949_d = new byte[i];
            buf.readBytes(this.field_186949_d);
        }
    }

    /**
     * Writes the raw packet data to the data stream.
     */
    public void writePacketData(PacketBuffer buf) throws IOException
    {
        buf.writeInt(this.chunkX);
        buf.writeInt(this.chunkZ);
        buf.writeBoolean(this.field_149279_g);
        buf.writeVarIntToBuffer(this.field_186948_c);
        buf.writeVarIntToBuffer(this.field_186949_d.length);
        buf.writeBytes(this.field_186949_d);
    }

    /**
     * Passes this Packet on to the NetHandler for processing.
     */
    public void processPacket(INetHandlerPlayClient handler)
    {
        handler.handleChunkData(this);
    }

    public PacketBuffer func_186946_a()
    {
        return new PacketBuffer(Unpooled.wrappedBuffer(this.field_186949_d));
    }

    private ByteBuf func_186945_f()
    {
        ByteBuf bytebuf = Unpooled.wrappedBuffer(this.field_186949_d);
        bytebuf.writerIndex(0);
        return bytebuf;
    }

    public static int func_186947_a(PacketBuffer p_186947_0_, Chunk p_186947_1_, boolean p_186947_2_, boolean p_186947_3_, int p_186947_4_)
    {
        int i = 0;
        ExtendedBlockStorage[] aextendedblockstorage = p_186947_1_.getBlockStorageArray();
        int j = 0;

        for (int k = aextendedblockstorage.length; j < k; ++j)
        {
            ExtendedBlockStorage extendedblockstorage = aextendedblockstorage[j];

            if (extendedblockstorage != Chunk.NULL_BLOCK_STORAGE && (!p_186947_2_ || !extendedblockstorage.isEmpty()) && (p_186947_4_ & 1 << j) != 0)
            {
                i |= 1 << j;
                extendedblockstorage.func_186049_g().func_186009_b(p_186947_0_);
                p_186947_0_.writeBytes(extendedblockstorage.getBlocklightArray().getData());

                if (p_186947_3_)
                {
                    p_186947_0_.writeBytes(extendedblockstorage.getSkylightArray().getData());
                }
            }
        }

        if (p_186947_2_)
        {
            p_186947_0_.writeBytes(p_186947_1_.getBiomeArray());
        }

        return i;
    }

    protected static int func_186944_a(Chunk p_186944_0_, boolean p_186944_1_, boolean p_186944_2_, int p_186944_3_)
    {
        int i = 0;
        ExtendedBlockStorage[] aextendedblockstorage = p_186944_0_.getBlockStorageArray();
        int j = 0;

        for (int k = aextendedblockstorage.length; j < k; ++j)
        {
            ExtendedBlockStorage extendedblockstorage = aextendedblockstorage[j];

            if (extendedblockstorage != Chunk.NULL_BLOCK_STORAGE && (!p_186944_1_ || !extendedblockstorage.isEmpty()) && (p_186944_3_ & 1 << j) != 0)
            {
                i = i + extendedblockstorage.func_186049_g().func_186018_a();
                i = i + extendedblockstorage.getBlocklightArray().getData().length;

                if (p_186944_2_)
                {
                    i += extendedblockstorage.getSkylightArray().getData().length;
                }
            }
        }

        if (p_186944_1_)
        {
            i += p_186944_0_.getBiomeArray().length;
        }

        return i;
    }

    public int getChunkX()
    {
        return this.chunkX;
    }

    public int getChunkZ()
    {
        return this.chunkZ;
    }

    public int getExtractedSize()
    {
        return this.field_186948_c;
    }

    public boolean func_149274_i()
    {
        return this.field_149279_g;
    }
}
