package com.continuum.nova.transformers;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.MethodVisitor;

import java.util.Map;

/**
 * @author David
 */
class MethodDispatchingClassWriter extends ClassVisitor {
    private static final Logger LOG = LogManager.getLogger(MethodDispatchingClassWriter.class);

    private Map<String, BetterMethodVisitor> methodReplacers;

    MethodDispatchingClassWriter(Map<String, BetterMethodVisitor> methodReplacers, int api, ClassVisitor cv) {
        super(api, cv);

        this.methodReplacers = methodReplacers;
    }

    @Override
    public MethodVisitor visitMethod(int access, String name, String desc, String signature, String[] exceptions) {
        LOG.info("Visiting method " + name);
        MethodVisitor mv = super.visitMethod(access, name, desc, signature, exceptions);

        if(methodReplacers.containsKey(name)) {
            BetterMethodVisitor replacer = methodReplacers.get(name);
            replacer.setMethodVisitor(mv);
            return replacer;
        }

        return mv;
    }
}
