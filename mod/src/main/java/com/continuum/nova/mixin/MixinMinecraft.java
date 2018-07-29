package com.continuum.nova.mixin;

import com.continuum.nova.NovaConstants;
import com.continuum.nova.NovaForge;
import com.continuum.nova.NovaRenderer;
import com.continuum.nova.input.Keyboard;
import com.continuum.nova.input.Mouse;
import com.continuum.nova.system.NovaNative;
import com.continuum.nova.utils.Utils;
import com.google.common.hash.Hashing;
import com.mojang.authlib.GameProfile;
import com.mojang.authlib.minecraft.MinecraftSessionService;
import net.minecraft.client.ClientBrandRetriever;
import net.minecraft.client.LoadingScreenRenderer;
import net.minecraft.client.Minecraft;
import net.minecraft.client.audio.MusicTicker;
import net.minecraft.client.audio.SoundHandler;
import net.minecraft.client.gui.FontRenderer;
import net.minecraft.client.gui.GuiIngame;
import net.minecraft.client.gui.GuiMainMenu;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.multiplayer.WorldClient;
import net.minecraft.client.particle.ParticleManager;
import net.minecraft.client.renderer.*;
import net.minecraft.client.renderer.block.model.ModelManager;
import net.minecraft.client.renderer.color.BlockColors;
import net.minecraft.client.renderer.color.ItemColors;
import net.minecraft.client.renderer.debug.DebugRenderer;
import net.minecraft.client.renderer.entity.RenderManager;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.resources.*;
import net.minecraft.client.resources.data.MetadataSerializer;
import net.minecraft.client.settings.GameSettings;
import net.minecraft.client.shader.Framebuffer;
import net.minecraft.profiler.Snooper;
import net.minecraft.util.MouseHelper;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.Session;
import net.minecraft.util.Timer;
import net.minecraft.util.datafix.DataFixer;
import net.minecraft.world.chunk.storage.AnvilSaveConverter;
import net.minecraft.world.storage.ISaveFormat;
import net.minecraftforge.fml.common.LoaderExceptionModCrash;
import org.apache.commons.io.Charsets;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.lwjgl.LWJGLException;
import org.lwjgl.opengl.Display;
import org.lwjgl.opengl.OpenGLException;
import org.spongepowered.asm.mixin.Final;
import org.spongepowered.asm.mixin.Mixin;
import org.spongepowered.asm.mixin.Overwrite;
import org.spongepowered.asm.mixin.Shadow;
import org.spongepowered.asm.mixin.injection.At;
import org.spongepowered.asm.mixin.injection.Inject;
import org.spongepowered.asm.mixin.injection.Redirect;
import org.spongepowered.asm.mixin.injection.callback.CallbackInfo;

import javax.annotation.Nullable;

import java.io.File;
import java.io.IOException;
import java.util.List;

@Mixin(Minecraft.class)
public abstract class MixinMinecraft {

    @Shadow
    public boolean inGameHasFocus;

    @Shadow
    public int displayWidth;
    @Shadow
    public int displayHeight;

    @Shadow
    public abstract void resize(int width, int height);

    @Shadow
    @Final
    private Timer timer;
    @Shadow
    public WorldClient world;
    @Shadow
    private BlockColors blockColors;
    @Shadow
    public GameSettings gameSettings;
    @Shadow
    @Final
    public File mcDataDir;
    @Shadow
    @Final
    public DefaultResourcePack mcDefaultResourcePack;
    @Shadow
    @Final
    private List<IResourcePack> defaultResourcePacks;

    @Shadow
    protected abstract void startTimerHackThread();

    @Shadow
    protected abstract void setWindowIcon();

    @Shadow
    protected abstract void setInitialDisplayMode() throws LWJGLException;

    @Shadow
    protected abstract void registerMetadataSerializers();

    @Shadow
    private ResourcePackRepository mcResourcePackRepository;
    @Shadow
    @Final
    private File fileResourcepacks;
    @Shadow
    private IReloadableResourceManager mcResourceManager;
    @Shadow
    @Final
    private MetadataSerializer metadataSerializer;
    @Shadow
    private LanguageManager mcLanguageManager;

    @Shadow
    public abstract void refreshResources();

    @Shadow
    public TextureManager renderEngine;

    @Shadow
    private SkinManager skinManager;
    @Shadow
    private ISaveFormat saveLoader;
    @Shadow
    @Final
    private MinecraftSessionService sessionService;
    @Shadow
    @Final
    private File fileAssets;
    @Shadow
    @Final
    private DataFixer dataFixer;
    @Shadow
    private SoundHandler mcSoundHandler;
    @Shadow
    private MusicTicker mcMusicTicker;
    @Shadow
    public FontRenderer fontRenderer;

    @Shadow
    public abstract boolean isUnicode();

    @Shadow
    public FontRenderer standardGalacticFontRenderer;
    @Shadow
    public MouseHelper mouseHelper;
    @Shadow
    private TextureMap textureMapBlocks;
    @Shadow
    private ModelManager modelManager;
    @Shadow
    private ItemColors itemColors;
    @Shadow
    private RenderItem renderItem;
    @Shadow
    private RenderManager renderManager;
    @Shadow
    private ItemRenderer itemRenderer;
    @Shadow
    public EntityRenderer entityRenderer;
    @Shadow
    private BlockRendererDispatcher blockRenderDispatcher;
    @Shadow
    public RenderGlobal renderGlobal;
    @Shadow
    public ParticleManager effectRenderer;
    @Shadow
    public GuiIngame ingameGUI;
    @Shadow
    private String serverName;

    @Shadow
    public abstract void displayGuiScreen(@Nullable GuiScreen guiScreenIn);

    @Shadow
    private int serverPort;
    @Shadow
    private ResourceLocation mojangLogo;
    @Shadow
    public LoadingScreenRenderer loadingScreen;
    @Shadow
    public DebugRenderer debugRenderer;
    @Shadow
    private boolean fullscreen;

    @Shadow
    public abstract void updateDisplay();

    @Shadow
    @Final
    private String launchedVersion;
    @Shadow
    @Final
    private Session session;
    @Shadow
    @Final
    private static ResourceLocation LOCATION_MOJANG_PNG;
    @Shadow
    @Final
    private static Logger LOGGER;

    @Shadow
    public abstract void draw(int p_draw_1_, int p_draw_2_, int p_draw_3_, int p_draw_4_, int p_draw_5_, int p_draw_6_, int p_draw_7_,
                              int p_draw_8_,
                              int p_draw_9_, int p_draw_10_);

    @Shadow
    protected abstract void checkGLError(String message);

    private Logger novaLogger;

    @Inject(method = "<init>", at = @At("RETURN"))
    private void constructorEnd(CallbackInfo info) {
        novaLogger = LogManager.getLogger("Nova");
        novaLogger.info("VULKAN_SDK is " + System.getenv("VULKAN_SDK"));
        novaLogger.info("PATH is " + System.getenv("PATH"));
        novaLogger.info("LD_LIBRARY_PATH is " + System.getenv("LD_LIBRARY_PATH"));
        novaLogger.info("VK_LAYER_PATH is " + System.getenv("VK_LAYER_PATH"));
        novaLogger.info("Loading nova...");
        NovaRenderer.create();
        NovaRenderer.getInstance().preInit();
        Mouse.create();
        Keyboard.create();
        inGameHasFocus = NovaRenderer.getInstance().getNative().display_is_active();
    }

    /**
     * @author Janrupf
     * @reason Overwritten because of to many changes
     * @inheritDoc
     */
    @Overwrite
    private void init() throws LWJGLException {
        this.gameSettings = new GameSettings(Minecraft.getMinecraft(), this.mcDataDir);
        this.defaultResourcePacks.add(this.mcDefaultResourcePack);
        this.startTimerHackThread();

        if (this.gameSettings.overrideHeight > 0 && this.gameSettings.overrideWidth > 0) {
            this.displayWidth = this.gameSettings.overrideWidth;
            this.displayHeight = this.gameSettings.overrideHeight;
        }

        this.setWindowIcon();
        this.setInitialDisplayMode();

        this.registerMetadataSerializers();
        this.mcResourcePackRepository = new ResourcePackRepository(this.fileResourcepacks, new File(this.mcDataDir, "server-resource-packs"), this.mcDefaultResourcePack, this.metadataSerializer, this.gameSettings);
        this.mcResourceManager = new SimpleReloadableResourceManager(this.metadataSerializer);
        this.mcLanguageManager = new LanguageManager(this.metadataSerializer, this.gameSettings.language);
        this.mcResourceManager.registerReloadListener(this.mcLanguageManager);
        net.minecraftforge.fml.client.FMLClientHandler.instance().beginMinecraftLoading((Minecraft) (Object) this, this.defaultResourcePacks, this.mcResourceManager, this.metadataSerializer);
        this.renderEngine = new TextureManager(this.mcResourceManager);
        this.mcResourceManager.registerReloadListener(this.renderEngine);
        net.minecraftforge.fml.client.SplashProgress.drawVanillaScreen(this.renderEngine);
        this.skinManager = new SkinManager(this.renderEngine, new File(this.fileAssets, "skins"), this.sessionService);
        this.saveLoader = new AnvilSaveConverter(new File(this.mcDataDir, "saves"), this.dataFixer);
        this.mcSoundHandler = new SoundHandler(this.mcResourceManager, this.gameSettings);
        this.mcResourceManager.registerReloadListener(this.mcSoundHandler);
        this.mcMusicTicker = new MusicTicker(Minecraft.getMinecraft());
        this.fontRenderer = new FontRenderer(this.gameSettings, new ResourceLocation("font/ascii"), this.renderEngine, false);

        if (this.gameSettings.language != null) {
            this.fontRenderer.setUnicodeFlag(this.isUnicode());
            this.fontRenderer.setBidiFlag(this.mcLanguageManager.isCurrentLanguageBidirectional());
        }

        this.standardGalacticFontRenderer = new FontRenderer(this.gameSettings, new ResourceLocation("font/ascii_sga"), this.renderEngine, false);
        this.mcResourceManager.registerReloadListener(this.fontRenderer);
        this.mcResourceManager.registerReloadListener(this.standardGalacticFontRenderer);
        this.mcResourceManager.registerReloadListener(new GrassColorReloadListener());
        this.mcResourceManager.registerReloadListener(new FoliageColorReloadListener());
        mcResourceManager.registerReloadListener(NovaRenderer.getInstance());
        this.mouseHelper = new MouseHelper();
        net.minecraftforge.fml.common.ProgressManager.ProgressBar bar = net.minecraftforge.fml.common.ProgressManager.push("Rendering Setup", 5, true);
        bar.step("GL Setup");


        this.checkGLError("Pre startup");
        GlStateManager.enableTexture2D();
        GlStateManager.shadeModel(7425);
        GlStateManager.clearDepth(1.0D);
        GlStateManager.enableDepth();
        GlStateManager.depthFunc(515);
        GlStateManager.enableAlpha();
        GlStateManager.alphaFunc(516, 0.1F);
        GlStateManager.cullFace(GlStateManager.CullFace.BACK);
        GlStateManager.matrixMode(5889);
        GlStateManager.loadIdentity();
        GlStateManager.matrixMode(5888);
        this.checkGLError("Startup");
        bar.step("Loading Texture Map");

        this.textureMapBlocks = new TextureMap("textures");
        this.textureMapBlocks.setMipmapLevels(this.gameSettings.mipmapLevels);

        this.renderEngine.loadTickableTexture(TextureMap.LOCATION_BLOCKS_TEXTURE, this.textureMapBlocks);
        this.renderEngine.bindTexture(TextureMap.LOCATION_BLOCKS_TEXTURE);
        this.textureMapBlocks.setBlurMipmapDirect(false, this.gameSettings.mipmapLevels > 0);
        bar.step("Loading Model Manager");
        this.modelManager = new ModelManager(this.textureMapBlocks);
        this.blockColors = BlockColors.init();
        try {
            NovaRenderer.getInstance().loadShaderpack(Utils.getShaderpackNameFromConfig(), this.blockColors);
        } catch (IOException e) {
            novaLogger.error("Failed to load shaderpack name  from config! Loading default...", e);
            NovaRenderer.getInstance().loadShaderpack("DefaultShaderpack", this.blockColors);
        }
        this.mcResourceManager.registerReloadListener(this.modelManager);
        this.itemColors = ItemColors.init(this.blockColors);
        bar.step("Loading Item Renderer");
        this.renderItem = new RenderItem(this.renderEngine, this.modelManager, this.itemColors);
        this.renderManager = new RenderManager(this.renderEngine, this.renderItem);
        this.itemRenderer = new ItemRenderer(Minecraft.getMinecraft());
        this.mcResourceManager.registerReloadListener(this.renderItem);
        bar.step("Loading Entity Renderer");
        net.minecraftforge.fml.client.SplashProgress.pause();
        this.entityRenderer = new EntityRenderer(Minecraft.getMinecraft(), this.mcResourceManager);
        this.mcResourceManager.registerReloadListener(this.entityRenderer);
        this.blockRenderDispatcher = new BlockRendererDispatcher(this.modelManager.getBlockModelShapes(), this.blockColors);
        this.mcResourceManager.registerReloadListener(this.blockRenderDispatcher);
        this.renderGlobal = new RenderGlobal(Minecraft.getMinecraft());
        this.mcResourceManager.registerReloadListener(this.renderGlobal);
        GlStateManager.viewport(0, 0, this.displayWidth, this.displayHeight);
        this.effectRenderer = new ParticleManager(this.world, this.renderEngine);
        net.minecraftforge.fml.client.SplashProgress.resume();
        net.minecraftforge.fml.common.ProgressManager.pop(bar);
        net.minecraftforge.fml.client.FMLClientHandler.instance().finishMinecraftLoading();
        this.checkGLError("Post startup");
        this.ingameGUI = new GuiIngame(Minecraft.getMinecraft());

        if (this.serverName != null) {
            net.minecraftforge.fml.client.FMLClientHandler.instance().connectToServerAtStartup(this.serverName, this.serverPort);
        } else {
            this.displayGuiScreen(new GuiMainMenu());
        }

        net.minecraftforge.fml.client.SplashProgress.clearVanillaResources(renderEngine, mojangLogo);
        this.mojangLogo = null;
        this.loadingScreen = new LoadingScreenRenderer(Minecraft.getMinecraft());
        this.debugRenderer = new DebugRenderer(Minecraft.getMinecraft());

        net.minecraftforge.fml.client.FMLClientHandler.instance().onInitializationComplete();
        if (this.gameSettings.fullScreen && !this.fullscreen) {
            this.toggleFullscreen();
        }

        try {
            Display.setVSyncEnabled(this.gameSettings.enableVsync);
        } catch (OpenGLException var2) {
            this.gameSettings.enableVsync = false;
            this.gameSettings.saveOptions();
        }

        this.renderGlobal.makeEntityOutlineShader();
    }

    @Redirect(
            method = "runGameLoop",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/shader/Framebuffer;bindFramebuffer(Z)V")
    )
    private void proxyBindFramebuffer(Framebuffer instance, boolean bind) {
    }

    @Redirect(
            method = "runGameLoop",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/shader/Framebuffer;unbindFramebuffer()V")
    )
    private void proxyUnbindFrameBuffer(Framebuffer instance) {
    }

    @Redirect(
            method = "runGameLoop",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/shader/Framebuffer;framebufferRender(II)V")
    )
    private void proxyFramebufferRender(Framebuffer instance, int width, int height) {
    }

    @Redirect(
            method = "runGameLoop",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/EntityRenderer;renderStreamIndicator(F)V")
    )
    private void proxyRenderStreamIndicator(EntityRenderer instance, float partialTicks) {
    }

    @Redirect(
            method = "setIngameFocus",
            at = @At(value = "INVOKE", target = "Lorg/lwjgl/opengl/Display;isActive()Z", remap = false)
    )
    private boolean proxyIsActive() {
        return NovaRenderer.getInstance().getNative().display_is_active();
    }

    /**
     * @author Janrupf
     * @reason Overwritten because this has to be handled by nova now
     * @inheritDoc
     */
    @Overwrite
    protected void checkWindowResize() {
        if (NovaRenderer.getInstance().wasResized()) {
            this.displayWidth = NovaRenderer.getInstance().getWidth();
            this.displayHeight = NovaRenderer.getInstance().getHeight();

            novaLogger.info("Window was resized to: " + displayWidth + "x" + displayHeight);

            this.resize(this.displayWidth, this.displayHeight);
        }
    }

    @Inject(
            method = "runGameLoop",
            at = @At(value = "INVOKE", target = "Lnet/minecraft/client/renderer/EntityRenderer;updateCameraAndRender(FJ)V", shift = At.Shift.AFTER)
    )
    private void hookUpdateCameraAndWindow(CallbackInfo callbackInfo) {
        NovaRenderer.getInstance().updateCameraAndRender(timer.renderPartialTicks, Minecraft.getMinecraft());
    }

    @Inject(
            method = "displayGuiScreen",
            at = @At(value = "HEAD")
    )
    private void hookDisplayGuiScreen(GuiScreen guiScreen, CallbackInfo callbackInfo) {
        if (guiScreen == null) {
            NovaRenderer.getInstance().getNative().clear_gui_buffers();
        }
    }

    /**
     * @author Janrupf
     * @reason Overwritten because this has to be handled by nova now
     */
    @Overwrite
    public void toggleFullscreen() {
        try {
            fullscreen = !fullscreen;
            gameSettings.fullScreen = fullscreen;

            if (fullscreen) {
                NovaRenderer.getInstance().getNative().set_fullscreen(NovaNative.NativeBoolean.TRUE.ordinal());
            } else {
                NovaRenderer.getInstance().getNative().set_fullscreen(NovaNative.NativeBoolean.FALSE.ordinal());
            }

            updateDisplay();
        } catch (Exception e) {
            novaLogger.error("Failed to toggle fullscreen!", e);
        }
    }

    @Inject(
            method = "resize",
            at = @At(value = "NEW", target = "net/minecraft/client/LoadingScreenRenderer", shift = At.Shift.BEFORE),
            cancellable = true
    )
    private void hookAndReturnResize(CallbackInfo callbackInfo) {
        callbackInfo.cancel();
    }

    /**
     * @author Janrupf
     * @reason Overwritten because this has to be handled by nova now
     */
    @Overwrite
    public void addServerTypeToSnooper(Snooper playerSnooper) {
        // TODO: Add vulkan stats
        playerSnooper.addStatToSnooper("client_brand", ClientBrandRetriever.getClientModName());
        playerSnooper.addStatToSnooper("launched_version", this.launchedVersion);
        playerSnooper.addStatToSnooper("vulkan_max_texture_size", NovaRenderer.getInstance().getNative().get_max_texture_size());
        playerSnooper.addStatToSnooper("nova_renderer_version", NovaConstants.VERSION);

        GameProfile gameprofile = this.session.getProfile();

        if (gameprofile.getId() != null) {
            playerSnooper.addStatToSnooper("uuid", Hashing.sha1().hashBytes(gameprofile.getId().toString().getBytes(Charsets.ISO_8859_1)).toString());
        }
    }

    /**
     * @author Janrupf
     * @reason Overwritten because this has to be handled by nova now
     * NOTE: This might not be required but is here for being correct
     */
    @Overwrite
    public static int getGLMaximumTextureSize() {
        return NovaRenderer.getInstance().getNative().get_max_texture_size();
    }

    @Inject(
            method = "shutdownMinecraftApplet",
            at = @At(value = "HEAD")
    )
    private void destructNova(CallbackInfo info) {
        novaLogger.info("Destroying nova");
        NovaRenderer.getInstance().getNative().destruct();
    }
}
