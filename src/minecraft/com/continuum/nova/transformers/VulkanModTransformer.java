package com.continuum.nova.transformers;

import com.continuum.nova.transformers.EntityRenderer.RenderWorldPassReplacer;
import com.continuum.nova.transformers.Minecraft.StartGameReplacer;
import com.continuum.nova.transformers.renderglobal.ConstructorReplacer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.Opcodes;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * Checks for the class being currently loaded, and dispatches the currently being loaded class into one of the other
 * transformers
 *
 * <p>Although, right now, all it does is print out the name of the class it's transforming. Useful for investigation,
 * but I'll have to figure something better out for an actual release</p>
 *
 * @author David
 */
public class VulkanModTransformer {
    private static final Logger LOG = LogManager.getLogger(VulkanModTransformer.class);

    private static Map<String, Map<String, BetterMethodVisitor>> classTransformers = new HashMap<>();

    static {
        Map<String, BetterMethodVisitor> entityRendererTransformrs = new HashMap<>();
        entityRendererTransformrs.put("renderWorldPass", new RenderWorldPassReplacer());

        classTransformers.put("net.minecraft.client.renderer.EntityRenderer", entityRendererTransformrs);

        Map<String, BetterMethodVisitor> minecraftTransformers = new HashMap<>();
        minecraftTransformers.put("startGame", new StartGameReplacer());

        classTransformers.put("net.minecraft.client.Minecraft", minecraftTransformers);

        Map<String, BetterMethodVisitor> renderGlobalTransformers = new HashMap<>();
        renderGlobalTransformers.put("<init>", new ConstructorReplacer());

        classTransformers.put("net.minecraft.client.renderer.RenderGlobal", renderGlobalTransformers);
    }

    /**
     * So I'm not really sure what this thing does. Hopefully I'll figure it out maybe.
     *
     * <p>
     * Optifine does a cute thing where it loads its own version of a class that MC is trying to load, hence all the
     * obfuscated MC classes. I might try to do a cuter thing, or I might try to find another core mod to look at
     * </p>
     *
     * @param name The name of the loaded class maybe?
     * @param transformedName The name of the class after I transform it
     * @param basicClass The raw bites of the class I guess
     *
     * @return The transformed class I think
     */
    public byte[] transform(String name, String transformedName, byte[] basicClass) {
        if(classTransformers.containsKey(name)) {
            LOG.info("Transforming class " + name);

            // Dispatch the class code to a transformer
            try {
                return transformClass(basicClass, classTransformers.get(name));
            } catch(IOException e) {
                LOG.trace(e);
            }
        }

        return basicClass;
    }

    /**
     * Transforms a class's bytecode to do something useful to the Nova Renderer. Usually this is as simple as replacing
     * one method with a method that calls native code, however sometimes you'll need to set up extra data
     *
     * @param rawClass The bytes of the original class
     * @param methodReplacers A map from the names of methods to the classes that encapsulate the code to replace those
     * methods
     * @return The transformed bytecode
     */
    private byte[] transformClass(byte[] rawClass, Map<String, BetterMethodVisitor> methodReplacers) throws IOException {
        LOG.info("Replacing methods " + methodReplacers.keySet());
        // Make an InputStream from the class so we can have some fun
        InputStream classStream = new ByteArrayInputStream(rawClass);

        // Initialize some ASM objects
        ClassReader classReader = new ClassReader(classStream);
        ClassWriter classWriter = new ClassWriter(ClassWriter.COMPUTE_MAXS);

        // Make our object to dispatch the method transformers
        MethodDispatchingClassWriter dispatchingClassWriter = new MethodDispatchingClassWriter(methodReplacers, Opcodes.ASM5, classWriter);
        classReader.accept(dispatchingClassWriter, 0);

        return classWriter.toByteArray();
    }
}
