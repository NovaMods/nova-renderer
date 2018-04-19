package com.continuum.nova.chunks;

import java.util.ArrayList;

/**
 * @author ddubois
 * @since 02-Aug-17
 */
class IndexList extends ArrayList<Integer> {
    /**
     * Adds the appropriate values for a quad to the underlying array
     *
     * @param faceOffset The offset of the current face
     * @param blockOffset The offset of the current block
     */
    void addIndicesForFace(int faceOffset, int blockOffset) {
        add(0 + faceOffset + blockOffset);
        add(1 + faceOffset + blockOffset);
        add(2 + faceOffset + blockOffset);
        add(0 + faceOffset + blockOffset);
        add(2 + faceOffset + blockOffset);
        add(3 + faceOffset + blockOffset);
    }
}
