package com.continuum.nova.chunks;

import net.minecraft.block.state.IBlockState;
import net.minecraft.util.BlockRenderLayer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author ddubois
 * @since 27-Jul-17
 */
public interface IGeometryFilter {
    boolean matches(IBlockState blockState);

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
        FULLSCREEN_QUAD,
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
        public boolean matches(IBlockState blockState) {
            return left.matches(blockState) && right.matches(blockState);
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
        public boolean matches(IBlockState blockState) {
            return left.matches(blockState) || right.matches(blockState);
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
        public boolean matches(IBlockState blockState) {
            return blockState.getBlock().getUnlocalizedName().equals(name);
        }

        @Override
        public String toString() {
            return "name::" + name;
        }
    }

    class NamePartGeometryFilter implements IGeometryFilter {
        private String namePart;

        public NamePartGeometryFilter(String namePart) {
            this.namePart = namePart;
        }

        @Override
        public boolean matches(IBlockState blockState) {
            return blockState.getBlock().getUnlocalizedName().contains(namePart);
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
        public boolean matches(IBlockState blockState) {
            return type == GeometryType.BLOCK;
        }

        @Override
        public String toString() {
            return "geometry_type::" + type.name().toLowerCase();
        }
    }

    /**
     * Matches blocks in the translucent render layer
     */
    class TransparentGeometryFilter implements IGeometryFilter {
        boolean shouldBeTransparent;

        private static final Logger LOG = LogManager.getLogger(TransparentGeometryFilter.class);

        public TransparentGeometryFilter(boolean shouldBeTransparent) {
            this.shouldBeTransparent = shouldBeTransparent;
        }

        @Override
        public boolean matches(IBlockState blockState) {
            return (blockState.getBlock().getBlockLayer() == BlockRenderLayer.TRANSLUCENT) == shouldBeTransparent;
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
        public boolean matches(IBlockState blockState) {
            return blockState.getLightValue() > 0 == shouldBeEmissive;
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

    static IGeometryFilter parseFilterString(final String filterString) {
        String[] tokens = filterString.split(" ");

        if(tokens.length % 2 == 0) {
            throw new IllegalArgumentException("Cannot have an even number of tokens in your geometry filter expressions");
        }

        IGeometryFilter filter = makeFilterFromToken(tokens[0]);
        if(tokens.length == 1) {
            return filter;
        }

        return makeFilterExpression(filter, tokens, 1);
    }

    static IGeometryFilter makeFilterExpression(IGeometryFilter previousFilter, String[] tokens, int curToken) {
        IGeometryFilter thisFilter;

        switch(tokens[curToken]) {
            case "AND":
                thisFilter = new AndGeometryFilter(previousFilter, makeFilterFromToken(tokens[curToken + 1]));
                break;

            case "OR":
                thisFilter = new OrGeometryFilter(previousFilter, makeFilterFromToken(tokens[curToken + 1]));
                break;

            default:
                return makeFilterFromToken(tokens[curToken + 1]);
        }

        boolean hasAnotherExpression = curToken + 2 < tokens.length - 1;

        if(hasAnotherExpression) {
            return makeFilterExpression(thisFilter, tokens, curToken + 2);

        } else {
            return thisFilter;
        }
    }

    static IGeometryFilter makeFilterFromToken(final String token) {
        if(token.startsWith("geometry_type::")) {
            String typeName = token.substring(15);
            GeometryType type = GeometryType.valueOf(typeName.toUpperCase());
            return new GeometryTypeGeometryFilter(type);

        } else if(token.startsWith("name::")) {
            String name = token.substring(6);
            return new NameGeometryFilter(name);

        } else if(token.startsWith("name_part::")) {
            String namePart = token.substring(11);
            return new NamePartGeometryFilter(namePart);

        } else if(token.equals("transparent")) {
            return new TransparentGeometryFilter(true);

        } else if(token.equals("not_transparent")) {
            return new TransparentGeometryFilter(false);

        } else if(token.equals("emissive")) {
            return new EmissiveGeometryFilter(true);

        } else if(token.equals("not_emissive")) {
            return new EmissiveGeometryFilter(false);
        }

        throw new IllegalArgumentException("Could not make a filter from token '" + token + "'");
    }
}
