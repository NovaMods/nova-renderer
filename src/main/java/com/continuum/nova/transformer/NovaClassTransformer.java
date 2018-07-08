package com.continuum.nova.transformer;

import net.minecraft.launchwrapper.IClassTransformer;
import net.minecraftforge.fml.common.asm.transformers.deobf.FMLRemappingAdapter;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.commons.RemappingClassAdapter;

public class NovaClassTransformer implements IClassTransformer {

    @Override public byte[] transform(String name, String transformedName, byte[] basicClass) {
        if (transformedName == null) {
            return basicClass;
        }
        if (transformedName.contains("ForgeHooksClient")) {
            ClassReader classReader = new ClassReader(basicClass);
            ClassWriter classWriter = new ClassWriter(0);
            ForgeHooksVisitor visitor = new ForgeHooksVisitor(classWriter);
            classReader.accept(visitor, 0);
            return classWriter.toByteArray();
        }
        return basicClass;
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
            // todo: add srg names
            final String STATE_MANAGER = "net/minecraft/client/renderer/GlStateManager",
                    ROTATE = "rotate", MULT_MATRIX = "multMatrix", VERTEX_POINTER = "glVertexPointer",
                    ENABLE_CLIENT_STATE = "glEnableClientState", NORMAL_POINTER = "glNormalPointer",
            COLOR_PTR = "glColorPointer", TEX_COORD_PTR = "glTexCoordPointer", DISABLE_CLIENT_STATE = "glDisableClientState";

            return new MethodVisitor(Opcodes.ASM5, cv.visitMethod(access, name, desc, signature, exceptions)) {
                @Override public void visitMethodInsn(int opcode, String owner, String name, String desc, boolean itf) {
                    if (owner.startsWith("org/lwjgl/opengl")) {
                        switch (name) {
                            case "glRotatef":
                                super.visitMethodInsn(opcode, STATE_MANAGER, ROTATE, desc, itf);
                                return;
                            case "glMultMatrix":
                                super.visitMethodInsn(opcode, STATE_MANAGER, MULT_MATRIX, desc, itf);
                                return;
                            case "glVertexPointer":
                                super.visitMethodInsn(opcode, STATE_MANAGER, VERTEX_POINTER, desc, itf);
                                return;
                            case "glEnableClientState":
                                super.visitMethodInsn(opcode, STATE_MANAGER, ENABLE_CLIENT_STATE, desc, itf);
                                return;
                            case "glNormalPointer":
                                super.visitMethodInsn(opcode, STATE_MANAGER, NORMAL_POINTER, desc, itf);
                                return;
                            case "glColorPointer":
                                super.visitMethodInsn(opcode, STATE_MANAGER, COLOR_PTR, desc, itf);
                                return;
                            case "glTexCoordPointer":
                                super.visitMethodInsn(opcode, STATE_MANAGER, TEX_COORD_PTR, desc, itf);
                                return;

                            case "glEnableVertexAttribArray":
                                // this is used only for GENERIC VertexFormatElement.EnumUsage, which is not used by vanilla
                                super.visitMethodInsn(opcode, owner, name, desc, itf);
                                return;
                            case "glVertexAttribPointer":
                                // this is used only for GENERIC VertexFormatElement.EnumUsage, which is not used by vanilla
                                super.visitMethodInsn(opcode, owner, name, desc, itf);
                                return;
                            case "glDisableClientState":
                                super.visitMethodInsn(opcode, STATE_MANAGER, DISABLE_CLIENT_STATE, desc, itf);
                                return;
                            case "glDisableVertexAttribArray":
                                // this is used only for GENERIC VertexFormatElement.EnumUsage, which is not used by vanilla
                                super.visitMethodInsn(opcode, owner, name, desc, itf);
                                return;
                        }
                        int i = 0;
                    }
                    super.visitMethodInsn(opcode, owner, name, desc, itf);
                }
            };
        }
    }
}
