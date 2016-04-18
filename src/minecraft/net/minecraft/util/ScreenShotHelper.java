package net.minecraft.util;

import java.awt.image.BufferedImage;
import java.io.File;
import java.nio.IntBuffer;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import javax.imageio.ImageIO;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.OpenGlHelper;
import net.minecraft.client.renderer.texture.TextureUtil;
import net.minecraft.client.shader.Framebuffer;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.util.text.event.ClickEvent;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.lwjgl.BufferUtils;

public class ScreenShotHelper
{
    private static final Logger logger = LogManager.getLogger();
    private static final DateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd_HH.mm.ss");

    /** A buffer to hold pixel values returned by OpenGL. */
    private static IntBuffer pixelBuffer;

    /**
     * The built-up array that contains all the pixel values returned by OpenGL.
     */
    private static int[] pixelValues;

    /**
     * Saves a screenshot in the game directory with a time-stamped filename.  Args: gameDirectory,
     * requestedWidthInPixels, requestedHeightInPixels, frameBuffer
     */
    public static ITextComponent saveScreenshot(File gameDirectory, int width, int height, Framebuffer buffer)
    {
        return saveScreenshot(gameDirectory, (String)null, width, height, buffer);
    }

    /**
     * Saves a screenshot in the game directory with the given file name (or null to generate a time-stamped name).
     * Args: gameDirectory, fileName, requestedWidthInPixels, requestedHeightInPixels, frameBuffer
     */
    public static ITextComponent saveScreenshot(File gameDirectory, String screenshotName, int width, int height, Framebuffer buffer)
    {
        try
        {
            File file1 = new File(gameDirectory, "screenshots");
            file1.mkdir();
            BufferedImage bufferedimage = func_186719_a(width, height, buffer);
            File file2;

            if (screenshotName == null)
            {
                file2 = getTimestampedPNGFileForDirectory(file1);
            }
            else
            {
                file2 = new File(file1, screenshotName);
            }

            ImageIO.write(bufferedimage, "png", (File)file2);
            ITextComponent itextcomponent = new TextComponentString(file2.getName());
            itextcomponent.getChatStyle().setChatClickEvent(new ClickEvent(ClickEvent.Action.OPEN_FILE, file2.getAbsolutePath()));
            itextcomponent.getChatStyle().setUnderlined(Boolean.valueOf(true));
            return new TextComponentTranslation("screenshot.success", new Object[] {itextcomponent});
        }
        catch (Exception exception)
        {
            logger.warn((String)"Couldn\'t save screenshot", (Throwable)exception);
            return new TextComponentTranslation("screenshot.failure", new Object[] {exception.getMessage()});
        }
    }

    public static BufferedImage func_186719_a(int p_186719_0_, int p_186719_1_, Framebuffer p_186719_2_)
    {
        if (OpenGlHelper.isFramebufferEnabled())
        {
            p_186719_0_ = p_186719_2_.framebufferTextureWidth;
            p_186719_1_ = p_186719_2_.framebufferTextureHeight;
        }

        int i = p_186719_0_ * p_186719_1_;

        if (pixelBuffer == null || pixelBuffer.capacity() < i)
        {
            pixelBuffer = BufferUtils.createIntBuffer(i);
            pixelValues = new int[i];
        }

        GlStateManager.glPixelStorei(3333, 1);
        GlStateManager.glPixelStorei(3317, 1);
        pixelBuffer.clear();

        if (OpenGlHelper.isFramebufferEnabled())
        {
            GlStateManager.bindTexture(p_186719_2_.framebufferTexture);
            GlStateManager.glGetTexImage(3553, 0, 32993, 33639, pixelBuffer);
        }
        else
        {
            GlStateManager.glReadPixels(0, 0, p_186719_0_, p_186719_1_, 32993, 33639, pixelBuffer);
        }

        pixelBuffer.get(pixelValues);
        TextureUtil.processPixelValues(pixelValues, p_186719_0_, p_186719_1_);
        BufferedImage bufferedimage = null;

        if (OpenGlHelper.isFramebufferEnabled())
        {
            bufferedimage = new BufferedImage(p_186719_2_.framebufferWidth, p_186719_2_.framebufferHeight, 1);
            int j = p_186719_2_.framebufferTextureHeight - p_186719_2_.framebufferHeight;

            for (int k = j; k < p_186719_2_.framebufferTextureHeight; ++k)
            {
                for (int l = 0; l < p_186719_2_.framebufferWidth; ++l)
                {
                    bufferedimage.setRGB(l, k - j, pixelValues[k * p_186719_2_.framebufferTextureWidth + l]);
                }
            }
        }
        else
        {
            bufferedimage = new BufferedImage(p_186719_0_, p_186719_1_, 1);
            bufferedimage.setRGB(0, 0, p_186719_0_, p_186719_1_, pixelValues, 0, p_186719_0_);
        }

        return bufferedimage;
    }

    /**
     * Creates a unique PNG file in the given directory named by a timestamp.  Handles cases where the timestamp alone
     * is not enough to create a uniquely named file, though it still might suffer from an unlikely race condition where
     * the filename was unique when this method was called, but another process or thread created a file at the same
     * path immediately after this method returned.
     */
    private static File getTimestampedPNGFileForDirectory(File gameDirectory)
    {
        String s = dateFormat.format(new Date()).toString();
        int i = 1;

        while (true)
        {
            File file1 = new File(gameDirectory, s + (i == 1 ? "" : "_" + i) + ".png");

            if (!file1.exists())
            {
                return file1;
            }

            ++i;
        }
    }
}
