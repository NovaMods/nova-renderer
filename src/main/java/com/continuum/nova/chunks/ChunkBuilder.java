package com.continuum.nova.chunks;

import java.util.Map;

/**
 * Splits chunks up into meshes with one mesh for each shader
 *
 * @author ddubois
 * @since 27-Jul-17
 */
public class ChunkBuilder {
    private Map<String, IGeometryFilter> filters;

    public ChunkBuilder(Map<String, IGeometryFilter> filters) {
        this.filters = filters;
    }

    public void createMeshesForChunk(ChunkUpdateListener.BlockUpdateRange range) {

    }
}
