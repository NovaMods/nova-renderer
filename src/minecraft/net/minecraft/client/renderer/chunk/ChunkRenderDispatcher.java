package net.minecraft.client.renderer.chunk;

import com.google.common.collect.Lists;
import com.google.common.collect.Queues;
import com.google.common.primitives.Doubles;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.ListenableFutureTask;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.PriorityBlockingQueue;
import java.util.concurrent.ThreadFactory;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.OpenGlHelper;
import net.minecraft.client.renderer.RegionRenderCacheBuilder;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.VertexBufferUploader;
import net.minecraft.client.renderer.WorldVertexBufferUploader;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.math.MathHelper;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ChunkRenderDispatcher
{
    private static final Logger logger = LogManager.getLogger();
    private static final ThreadFactory threadFactory = (new ThreadFactoryBuilder()).setNameFormat("Chunk Batcher %d").setDaemon(true).build();
    private final int field_188249_c;
    private final List<Thread> field_188250_d = Lists.<Thread>newArrayList();
    private final List<ChunkRenderWorker> listThreadedWorkers = Lists.<ChunkRenderWorker>newArrayList();
    private final PriorityBlockingQueue<ChunkCompileTaskGenerator> queueChunkUpdates = Queues.<ChunkCompileTaskGenerator>newPriorityBlockingQueue();
    private final BlockingQueue<RegionRenderCacheBuilder> queueFreeRenderBuilders;
    private final WorldVertexBufferUploader worldVertexUploader = new WorldVertexBufferUploader();
    private final VertexBufferUploader vertexUploader = new VertexBufferUploader();
    private final Queue<ChunkRenderDispatcher.PendingUpload> queueChunkUploads = Queues.<ChunkRenderDispatcher.PendingUpload>newPriorityQueue();
    private final ChunkRenderWorker renderWorker;

    public ChunkRenderDispatcher()
    {
        int i = Math.max(1, (int)((double)Runtime.getRuntime().maxMemory() * 0.3D) / 10485760);
        int j = Math.max(1, MathHelper.clamp_int(Runtime.getRuntime().availableProcessors(), 1, i / 5));
        this.field_188249_c = MathHelper.clamp_int(j * 10, 1, i);

        if (j > 1)
        {
            for (int k = 0; k < j; ++k)
            {
                ChunkRenderWorker chunkrenderworker = new ChunkRenderWorker(this);
                Thread thread = threadFactory.newThread(chunkrenderworker);
                thread.start();
                this.listThreadedWorkers.add(chunkrenderworker);
                this.field_188250_d.add(thread);
            }
        }

        this.queueFreeRenderBuilders = Queues.<RegionRenderCacheBuilder>newArrayBlockingQueue(this.field_188249_c);

        for (int l = 0; l < this.field_188249_c; ++l)
        {
            this.queueFreeRenderBuilders.add(new RegionRenderCacheBuilder());
        }

        this.renderWorker = new ChunkRenderWorker(this, new RegionRenderCacheBuilder());
    }

    public String getDebugInfo()
    {
        return this.field_188250_d.isEmpty() ? String.format("pC: %03d, single-threaded", new Object[] {Integer.valueOf(this.queueChunkUpdates.size())}): String.format("pC: %03d, pU: %1d, aB: %1d", new Object[] {Integer.valueOf(this.queueChunkUpdates.size()), Integer.valueOf(this.queueChunkUploads.size()), Integer.valueOf(this.queueFreeRenderBuilders.size())});
    }

    public boolean runChunkUploads(long p_178516_1_)
    {
        boolean flag = false;

        while (true)
        {
            boolean flag1 = false;

            if (this.field_188250_d.isEmpty())
            {
                ChunkCompileTaskGenerator chunkcompiletaskgenerator = (ChunkCompileTaskGenerator)this.queueChunkUpdates.poll();

                if (chunkcompiletaskgenerator != null)
                {
                    try
                    {
                        this.renderWorker.processTask(chunkcompiletaskgenerator);
                        flag1 = true;
                    }
                    catch (InterruptedException var8)
                    {
                        logger.warn("Skipped task due to interrupt");
                    }
                }
            }

            synchronized (this.queueChunkUploads)
            {
                if (!this.queueChunkUploads.isEmpty())
                {
                    ((ChunkRenderDispatcher.PendingUpload)this.queueChunkUploads.poll()).field_188241_b.run();
                    flag1 = true;
                    flag = true;
                }
            }

            if (p_178516_1_ == 0L || !flag1 || p_178516_1_ < System.nanoTime())
            {
                break;
            }
        }

        return flag;
    }

    public boolean updateChunkLater(RenderChunk chunkRenderer)
    {
        chunkRenderer.getLockCompileTask().lock();
        boolean flag1;

        try
        {
            final ChunkCompileTaskGenerator chunkcompiletaskgenerator = chunkRenderer.makeCompileTaskChunk();
            chunkcompiletaskgenerator.addFinishRunnable(new Runnable()
            {
                public void run()
                {
                    ChunkRenderDispatcher.this.queueChunkUpdates.remove(chunkcompiletaskgenerator);
                }
            });
            boolean flag = this.queueChunkUpdates.offer(chunkcompiletaskgenerator);

            if (!flag)
            {
                chunkcompiletaskgenerator.finish();
            }

            flag1 = flag;
        }
        finally
        {
            chunkRenderer.getLockCompileTask().unlock();
        }

        return flag1;
    }

    public boolean updateChunkNow(RenderChunk chunkRenderer)
    {
        chunkRenderer.getLockCompileTask().lock();
        boolean flag;

        try
        {
            ChunkCompileTaskGenerator chunkcompiletaskgenerator = chunkRenderer.makeCompileTaskChunk();

            try
            {
                this.renderWorker.processTask(chunkcompiletaskgenerator);
            }
            catch (InterruptedException var7)
            {
                ;
            }

            flag = true;
        }
        finally
        {
            chunkRenderer.getLockCompileTask().unlock();
        }

        return flag;
    }

    public void stopChunkUpdates()
    {
        this.clearChunkUpdates();
        List<RegionRenderCacheBuilder> list = Lists.<RegionRenderCacheBuilder>newArrayList();

        while (((List)list).size() != this.field_188249_c)
        {
            this.runChunkUploads(Long.MAX_VALUE);

            try
            {
                list.add(this.allocateRenderBuilder());
            }
            catch (InterruptedException var3)
            {
                ;
            }
        }

        this.queueFreeRenderBuilders.addAll(list);
    }

    public void freeRenderBuilder(RegionRenderCacheBuilder p_178512_1_)
    {
        this.queueFreeRenderBuilders.add(p_178512_1_);
    }

    public RegionRenderCacheBuilder allocateRenderBuilder() throws InterruptedException
    {
        return (RegionRenderCacheBuilder)this.queueFreeRenderBuilders.take();
    }

    public ChunkCompileTaskGenerator getNextChunkUpdate() throws InterruptedException
    {
        return (ChunkCompileTaskGenerator)this.queueChunkUpdates.take();
    }

    public boolean updateTransparencyLater(RenderChunk chunkRenderer)
    {
        chunkRenderer.getLockCompileTask().lock();
        boolean flag;

        try
        {
            final ChunkCompileTaskGenerator chunkcompiletaskgenerator = chunkRenderer.makeCompileTaskTransparency();

            if (chunkcompiletaskgenerator == null)
            {
                flag = true;
                return flag;
            }

            chunkcompiletaskgenerator.addFinishRunnable(new Runnable()
            {
                public void run()
                {
                    ChunkRenderDispatcher.this.queueChunkUpdates.remove(chunkcompiletaskgenerator);
                }
            });
            flag = this.queueChunkUpdates.offer(chunkcompiletaskgenerator);
        }
        finally
        {
            chunkRenderer.getLockCompileTask().unlock();
        }

        return flag;
    }

    public ListenableFuture<Object> func_188245_a(final BlockRenderLayer p_188245_1_, final VertexBuffer p_188245_2_, final RenderChunk p_188245_3_, final CompiledChunk p_188245_4_, final double p_188245_5_)
    {
        if (Minecraft.getMinecraft().isCallingFromMinecraftThread())
        {
            if (OpenGlHelper.useVbo())
            {
                this.uploadVertexBuffer(p_188245_2_, p_188245_3_.getVertexBufferByLayer(p_188245_1_.ordinal()));
            }
            else
            {
                this.uploadDisplayList(p_188245_2_, ((ListedRenderChunk)p_188245_3_).getDisplayList(p_188245_1_, p_188245_4_), p_188245_3_);
            }

            p_188245_2_.setTranslation(0.0D, 0.0D, 0.0D);
            return Futures.<Object>immediateFuture((Object)null);
        }
        else
        {
            ListenableFutureTask<Object> listenablefuturetask = ListenableFutureTask.<Object>create(new Runnable()
            {
                public void run()
                {
                    ChunkRenderDispatcher.this.func_188245_a(p_188245_1_, p_188245_2_, p_188245_3_, p_188245_4_, p_188245_5_);
                }
            }, (Object)null);

            synchronized (this.queueChunkUploads)
            {
                this.queueChunkUploads.add(new ChunkRenderDispatcher.PendingUpload(listenablefuturetask, p_188245_5_));
                return listenablefuturetask;
            }
        }
    }

    private void uploadDisplayList(VertexBuffer p_178510_1_, int p_178510_2_, RenderChunk chunkRenderer)
    {
        GlStateManager.glNewList(p_178510_2_, 4864);
        GlStateManager.pushMatrix();
        chunkRenderer.multModelviewMatrix();
        this.worldVertexUploader.draw(p_178510_1_);
        GlStateManager.popMatrix();
        GlStateManager.glEndList();
    }

    private void uploadVertexBuffer(VertexBuffer p_178506_1_, net.minecraft.client.renderer.vertex.VertexBuffer vertexBufferIn)
    {
        this.vertexUploader.setVertexBuffer(vertexBufferIn);
        this.vertexUploader.draw(p_178506_1_);
    }

    public void clearChunkUpdates()
    {
        while (!this.queueChunkUpdates.isEmpty())
        {
            ChunkCompileTaskGenerator chunkcompiletaskgenerator = (ChunkCompileTaskGenerator)this.queueChunkUpdates.poll();

            if (chunkcompiletaskgenerator != null)
            {
                chunkcompiletaskgenerator.finish();
            }
        }
    }

    public boolean func_188247_f()
    {
        return this.queueChunkUpdates.isEmpty() && this.queueChunkUploads.isEmpty();
    }

    public void func_188244_g()
    {
        this.clearChunkUpdates();

        for (ChunkRenderWorker chunkrenderworker : this.listThreadedWorkers)
        {
            chunkrenderworker.func_188264_a();
        }

        for (Thread thread : this.field_188250_d)
        {
            try
            {
                thread.interrupt();
                thread.join();
            }
            catch (InterruptedException interruptedexception)
            {
                logger.warn((String)"Interrupted whilst waiting for worker to die", (Throwable)interruptedexception);
            }
        }

        this.queueFreeRenderBuilders.clear();
    }

    public boolean func_188248_h()
    {
        return this.queueFreeRenderBuilders.size() == 0;
    }

    class PendingUpload implements Comparable<ChunkRenderDispatcher.PendingUpload>
    {
        private final ListenableFutureTask<Object> field_188241_b;
        private final double field_188242_c;

        public PendingUpload(ListenableFutureTask<Object> p_i46994_2_, double p_i46994_3_)
        {
            this.field_188241_b = p_i46994_2_;
            this.field_188242_c = p_i46994_3_;
        }

        public int compareTo(ChunkRenderDispatcher.PendingUpload p_compareTo_1_)
        {
            return Doubles.compare(this.field_188242_c, p_compareTo_1_.field_188242_c);
        }
    }
}
