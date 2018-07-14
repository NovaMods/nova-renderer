package com.continuum.nova.transformer;

import com.sun.org.apache.bcel.internal.generic.INVOKESTATIC;
import net.minecraft.launchwrapper.IClassTransformer;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

import java.util.HashSet;
import java.util.Set;

public class NovaClassTransformer implements IClassTransformer {

    private static final Set<String> blacklistPrefixes = new HashSet<>();

    static {
        blacklistPrefixes.add("org.lwjgl.");
        blacklistPrefixes.add("net.minecraft.client.renderer.GlStateManage");
        blacklistPrefixes.add("net.minecraft.client.renderer.OpenGlHelper");
        blacklistPrefixes.add("com.continuum.nova.transformer.");
    }
    @Override public byte[] transform(String name, String transformedName, byte[] basicClass) {
        if (transformedName == null) {
            return basicClass;
        }
        for (String p : blacklistPrefixes) {
            if (transformedName.startsWith(p)) {
                return basicClass;
            }
        }

        ClassReader classReader = new ClassReader(basicClass);
        ClassWriter classWriter = new ClassWriter(0);
        ForgeHooksVisitor visitor = new ForgeHooksVisitor(classWriter);
        classReader.accept(visitor, 0);
        return classWriter.toByteArray();
    }

    private static class ForgeHooksVisitor extends ClassVisitor {

        public ForgeHooksVisitor() {
            super(Opcodes.ASM5);
        }

        public ForgeHooksVisitor(ClassVisitor cv) {
            super(Opcodes.ASM5, cv);
        }

        @Override
        public MethodVisitor visitMethod(int access, String name, String desc,
                String signature, String[] exceptions) {
            return new MethodVisitor(Opcodes.ASM5, cv.visitMethod(access, name, desc, signature, exceptions)) {
                @Override public void visitMethodInsn(int opcode, String owner, String name, String desc, boolean itf) {
                    if (owner.equals("org/lwjgl/opengl/GL11")
                            || owner.equals("org/lwjgl/opengl/GL12")
                            || owner.equals("org/lwjgl/opengl/GL13")
                            || owner.equals("org/lwjgl/opengl/GL14")
                            || owner.equals("org/lwjgl/opengl/GL15")
                            || owner.equals("org/lwjgl/opengl/GL20")
                            || owner.equals("org/lwjgl/opengl/GL21")
                            || owner.equals("org/lwjgl/opengl/GL30")
                            || owner.equals("org/lwjgl/opengl/GL31")
                            || owner.equals("org/lwjgl/opengl/GL32")
                            || owner.equals("org/lwjgl/opengl/GL33")
                            || owner.equals("org/lwjgl/opengl/GL40")
                            || owner.equals("org/lwjgl/opengl/GL41")
                            || owner.equals("org/lwjgl/opengl/GL42")
                            || owner.equals("org/lwjgl/opengl/GL43")
                            || owner.equals("org/lwjgl/opengl/GL44")
                            || owner.equals("org/lwjgl/opengl/GL45")
                            || owner.equals("org/lwjgl/opengl/GLContext")
                            || owner.equals("org/lwjgl/opengl/Display")) {
                        if (removeCalls(opcode, owner, name, desc, itf)) {
                            return;
                        }
                    }
                    super.visitMethodInsn(opcode, owner, name, desc, itf);
                }

                private boolean removeCalls(int opcode, String owner, String name, String desc, boolean itf) {
                    if (owner.equals("org/lwjgl/opengl/Display") &&
                            (name.equals("create") || name.equals("getDisplayMode") || name.equals("getWidth") || name.equals("getHeight")
                                    || name.equals("isActive"))) {
                        return false;
                    }
                    System.err.println("Found usage of method " + owner + "." + name + ";" + desc + ", removing");

                    for (Type t : Type.getArgumentTypes(desc)) {
                        switch (t.getSize()) {
                            case 2:
                                super.visitInsn(Opcodes.POP2);
                                System.err.println("\tPOP2");
                                break;
                            case 1:
                                super.visitInsn(Opcodes.POP);
                                System.err.println("\tPOP");
                                break;
                        }
                    }
                    if (opcode != Opcodes.INVOKESTATIC) {
                        super.visitInsn(Opcodes.POP);
                        System.err.println("\tPOP (object)");
                    }
                    switch (Type.getReturnType(desc).getSort()) {
                        case Type.OBJECT:
                        case Type.ARRAY:
                            System.err.println("\tACONST_NULL");
                            super.visitInsn(Opcodes.ACONST_NULL);
                            break;
                        case Type.DOUBLE:
                            System.err.println("\tDCONST_0");
                            super.visitInsn(Opcodes.DCONST_0);
                            break;
                        case Type.LONG:
                            System.err.println("\tLCONST_0");
                            super.visitInsn(Opcodes.LCONST_0);
                            break;
                        case Type.FLOAT:
                            System.err.println("\tFCONST_0");
                            super.visitInsn(Opcodes.FCONST_0);
                            break;
                        case Type.INT:
                        case Type.SHORT:
                        case Type.BYTE:
                        case Type.CHAR:
                        case Type.BOOLEAN:
                            System.err.println("\tICONST_0");
                            super.visitInsn(Opcodes.ICONST_0);
                            break;
                    }
                    return true;
                }
            };
        }
    }
}
