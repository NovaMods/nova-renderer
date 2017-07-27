package com.continuum.nova.chunks;

import net.minecraft.block.Block;

/**
 * @author ddubois
 * @since 27-Jul-17
 */
public interface IGeometryFilter {
    boolean matches(Block block);

    enum GeometryType {
        BLOCK,
        ENTITY,
        FALLING_BLOCK,
        GUI,
        CLOUD,
        SKY_DECORATION,
        SELECTION_BOX,
        GLINT,
        WEATHER,
        HAND,
        FULSCREEN_QUAD,
        PARTICLE,
        LIT_PARTICLE,
        EYES
    }

    class AndGeometryFilter implements IGeometryFilter {
        private IGeometryFilter left;
        private IGeometryFilter right;

        public AndGeometryFilter(IGeometryFilter left, IGeometryFilter right) {
            this.left = left;
            this.right = right;
        }

        @Override
        public boolean matches(Block block) {
            return left.matches(block) && right.matches(block);
        }

        @Override
        public String toString() {
            return "(" + left.toString() + " AND " + right.toString() + ")";
        }
    }

    class OrGeometryFilter implements IGeometryFilter {
        private IGeometryFilter left;
        private IGeometryFilter right;

        public OrGeometryFilter(IGeometryFilter left, IGeometryFilter right) {
            this.left = left;
            this.right = right;
        }

        @Override
        public boolean matches(Block block) {
            return left.matches(block) || right.matches(block);
        }

        @Override
        public String toString() {
            return "(" + left.toString() + " OR " + right.toString() + ")";
        }
    }

    class NameGeometryFilter implements IGeometryFilter {
        private String name;

        public NameGeometryFilter(String name) {
            this.name = name;
        }

        @Override
        public boolean matches(Block block) {
            return block.getUnlocalizedName().equals(name);
        }

        @Override
        public String toString() {
            return "name::" + name;
        }
    }

    class NamePartGeometryFilte implements IGeometryFilter {
        private String namePart;

        public NamePartGeometryFilte(String namePart) {
            this.namePart = namePart;
        }

        @Override
        public boolean matches(Block block) {
            return block.getUnlocalizedName().contains(namePart);
        }

        @Override
        public String toString() {
            return "name_part::" + namePart;
        }
    }

    class GeometryTypeGeometryFilter implements IGeometryFilter {
        private GeometryType type;

        public GeometryTypeGeometryFilter(GeometryType type) {
            this.type = type;
        }

        @Override
        public boolean matches(Block block) {
            return type == GeometryType.BLOCK;
        }

        @Override
        public String toString() {
            return "geometry_type::" + type.name().toLowerCase();
        }
    }

    class TransparentGeometryFilter implements IGeometryFilter {
        boolean shouldBeTransparent;

        public TransparentGeometryFilter(boolean shouldBeTransparent) {
            this.shouldBeTransparent = shouldBeTransparent;
        }

        @Override
        public boolean matches(Block block) {
            return !block.isVisuallyOpaque() == shouldBeTransparent;
        }

        @Override
        public String toString() {
            if(shouldBeTransparent) {
                return "transparent";
            } else {
                return "not_transparent";
            }
        }
    }

    class EmissiveGeometryFilter implements IGeometryFilter {
        boolean shouldBeEmissive;

        public EmissiveGeometryFilter(boolean shouldBeEmissive) {
            this.shouldBeEmissive = shouldBeEmissive;
        }

        @Override
        public boolean matches(Block block) {
            return block.getBlockState().getBaseState().getLightValue() > 0 == shouldBeEmissive;
        }

        @Override
        public String toString() {
            if(shouldBeEmissive) {
                return "emissive";
            } else {
                return "not_emissive";
            }
        }
    }
}
