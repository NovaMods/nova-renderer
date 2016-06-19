package com.continuum.nova.transformers;

import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

/**
 * @author David
 */
public abstract class BetterMethodVisitor extends MethodVisitor {
    public BetterMethodVisitor() {
        super(Opcodes.ASM5);
    }

    void setMethodVisitor(MethodVisitor mv) {
        this.mv = mv;
    }
}
