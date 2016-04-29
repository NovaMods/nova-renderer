package com.continuum.nova.replacements;

import net.minecraft.client.Minecraft;
import net.minecraft.client.multiplayer.WorldClient;
import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.init.Blocks;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author David
 */
public class NativeRenderGlobal extends RenderGlobal {
    private static final Logger LOG = LogManager.getLogger(NativeRenderGlobal.class);
    protected Minecraft minecraft;

    public NativeRenderGlobal(Minecraft mcIn) {
        super(mcIn);
        minecraft = mcIn;
    }

    /**
     * Called by ModelLoaderRegistry#registerLoader(ICustomModelLoader)
     *
     * <p>The superclass implementation just calls #updateDestroyBlockIcons, which simply gets the destruction textures
     * from the texture map</p>
     *
     * @param resourceManager
     *
    @Override
    public native void onResourceManagerReload(IResourceManager resourceManager);

    /**
     * Called by Minecraft#startGame. Should definitely keep this one around
     *
    @Override
    public native void makeEntityOutlineShader();

    /**
     * Called by Minecraft#loadWorld(WorldClient, String).
     *
     * <p>Looks like the base class registers itself with WorldClient#addWorldAccess. That seems important</p>
     *
     * <p>Also calls #loadRenderers</p>
     *
     * @param worldClient The WorldClient which (presumably) holds the world
     */
    @Override
    public void setWorldAndLoadRenderers(WorldClient worldClient) {
        loadRenderers();
        //NativeInterface.getInstance().setWorld(worldClient);
    }

    /**
     * Called from Minecraft#runTick and GameSettings#setOptionFloatValue
     *
     * <p>I guess this sets the world as "dirty", saying that we need to re-generate all the geometry. I definitely
     * should keep this around</p>
     *
    @Override
    public native void setDisplayListEntitiesDirty();

    /**
     * Called by Minecraft#runTick. I'll have to make this one work
     *
    @Override
    public native void updateClouds();


    /**
     * Called by EntityRenderer#func_181560_a, which will almost certainly be replaced
     *
    @Override
    public native void renderEntityOutlineFramebuffer();

    /**
     * Called by EntityRenderer#updateShaderGroupSize(int, int). I can probably get rid of it.
     *
     * @param int1
     * @param int2
     *
    @Override
    public native void createBindEntityOutlineFbs(int int1, int int2);

    /**
     * Called by EntityRenderer#renderWorldPass(int, float, long). Probbaly not going to exist in the final design
     *
     * @param renderViewEntity
     * @param camera
     * @param partialTicks
     *
    @Override
    public native void renderEntities(Entity renderViewEntity, ICamera camera, float partialTicks);

    /**
     * Called by EntityRenderer#renderWorldPass(int, float, long). Definitely going to go away.
     * @param viewEntity
     * @param partialTicks
     * @param camera
     * @param frameCount
     * @param playerSpectator
     *
    @Override
    public native void setupTerrain(Entity viewEntity, double partialTicks, ICamera camera, int frameCount, boolean playerSpectator);

    /**
     * Yup, renderWorldPass
     * @param blockLayerIn
     * @param partialTicks
     * @param pass
     * @param entityIn
     * @return
     *
    @Override
    public native int renderBlockLayer(EnumWorldBlockLayer blockLayerIn, double partialTicks, int pass, Entity entityIn);

    /**
     * EntityRenderer#renderWorldPass
     * @param finishTimeNano
     *
    @Override
    public native void updateChunks(long finishTimeNano);

    /**
     * EntityRenderer#renderWorldPass
     * @param entityIn
     * @param partialTicks
     *
    @Override
    public native void renderWorldBorder(Entity entityIn, float partialTicks);

    /**
     * EntityRenderer#renderWorldPass
     * @param tessellatorIn
     * @param worldRendererIn
     * @param entityIn
     * @param partialTicks
     *
    @Override
    public native void drawBlockDamageTexture(Tessellator tessellatorIn, WorldRenderer worldRendererIn, Entity entityIn, float partialTicks);

    /**
     * EntityRenderer#renderWorldPass
     * @param player
     * @param movingObjectPositionIn
     * @param p_72731_3_
     * @param partialTicks
     *
    @Override
    public native void drawSelectionBox(EntityPlayer player, MovingObjectPosition movingObjectPositionIn, int p_72731_3_, float partialTicks);

    /**
     * EntityRenderer#renderWorldPass
     * @param partialTicks
     * @param pass
     *
    @Override
    public native void renderSky(float partialTicks, int pass);

    /**
     * EntityRenderer#renderCloudsCheck. Probably not needed
     * @param partialTicks
     * @param pass
     *
    @Override
    public native void renderClouds(float partialTicks, int pass);


    /**
     * Called by GuiOverlayDialog#call (What kind of method name is that?)
     * @return
     *
    @Override
    public native String getDebugInfoRenders();


    /**
     * World#markBlockForUpdate. Seems pretty useful. I don't really "get" it, though. Is it even a thing?
     *
     * <p>Like, this method just calls another method. Why is there so much indirection? Ugh</p>
     *
     * @param pos
     *
    @Override
    public native void markBlockForUpdate(BlockPos pos);

    /**
     * World#notifyLightSet(BlockPos)
     *
     * Does the same thing as #markBlockForUpdate
     *
     * @param pos
     *
    @Override
    public void notifyLightSet(BlockPos pos) {
        markBlockForUpdate(pos);
    }

    /**
     * World#spawnParticle
     *
     * I'm thinking I could have the implementation for this method add a marker to a list of particles, which I can
     * then instance
     *
     * @param particleId
     * @param ignoreRange
     * @param xCoord
     * @param yCoord
     * @param zCoord
     * @param xOffset
     * @param yOffset
     * @param zOffset
     * @param p_180442_15_
     *
    @Override
    public native void spawnParticle(int particleId, boolean ignoreRange, final double xCoord, final double yCoord, final double zCoord, double xOffset, double yOffset, double zOffset, int... p_180442_15_);

    /**
     * world#sendBlockBreakProgress
     *
     * <p>Looks useful so I know know which block breaking texture to draw over the block</p>
     *
     * @param breakerId
     * @param pos
     * @param progress
     *
    @Override
    public native void sendBlockBreakProgress(int breakerId, BlockPos pos, int progress);


    /**
     * Called all over the place. Definitely not making this a no-op
     *
     * <p>This method does a lot in the base class, a lot that it really shouldn't do.</p>
     */
    @Override
    public void loadRenderers() {
        // Do the stupid stuff that loadRenderers does for no good reason

        // Why does this method set the leaves' graphics levels? Why?
        Blocks.leaves.setGraphicsLevel(minecraft.gameSettings.fancyGraphics);
        Blocks.leaves2.setGraphicsLevel(minecraft.gameSettings.fancyGraphics);

        // Call the thing I actually want to do
        //NativeInterface.startup();
    }

    /**
     * Not sure what this does or why it does it. Will wait for more deobsfucation
     * @param p_181023_1_
     * @param p_181023_2_
     */
    //@Override
    //public native void func_181023_a(Collection<TileEntity> p_181023_1_, Collection<TileEntity> p_181023_2_);
}
