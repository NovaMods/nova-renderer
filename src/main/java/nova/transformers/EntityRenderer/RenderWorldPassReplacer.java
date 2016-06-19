package com.continuum.nova.transformers.EntityRenderer;

import com.continuum.nova.transformers.BetterMethodVisitor;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.objectweb.asm.Label;

import static org.objectweb.asm.Opcodes.*;

/**
 * Replaces the #renderWorldPass(int) method with one that will come along and forward render calls to Vulkan
 *
 * @author David
 */
public class RenderWorldPassReplacer extends BetterMethodVisitor {
    private final Logger LOG = LogManager.getLogger(RenderWorldPassReplacer.class);

    public RenderWorldPassReplacer() {
        super();
    }

    @Override
    public void visitCode() {
        // We want to do a few things here
        // First, check if the native thread is still alive. If it isn't report it and try to start it up again
        // Once we know that the native thread is running fine, check if it's ready for another render. If it is, dispatch the render. If not, return and move on with our lives.
        // If the thread is ready for another render, gather everything we need to dispatch a rendering command. First I'll just render chunks, so I'll have to keep track of when new chunks come into existence.

        LOG.info("Visiting the code");
        Label l0 = new Label();
        mv.visitLabel(l0);
        mv.visitLineNumber(29, l0);
        mv.visitFieldInsn(GETSTATIC, "java/lang/System", "out", "Ljava/io/PrintStream;");
        mv.visitLdcInsn("Making a render call");
        mv.visitMethodInsn(INVOKEVIRTUAL, "java/io/PrintStream", "println", "(Ljava/lang/String;)V", false);
        Label l1 = new Label();
        mv.visitLabel(l1);
        mv.visitLineNumber(30, l1);
        mv.visitInsn(RETURN);
        Label l2 = new Label();
        mv.visitLabel(l2);
        mv.visitLocalVariable("this", "Lcom/continuum/nova/transformers/EntityRendererTransformer$RenderWorldPassReplacer;", null, l0, l2, 0);
        mv.visitMaxs(2, 1);
    }
}
