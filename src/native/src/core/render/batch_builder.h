/*!
 * \author David
 * \date 17-May-16.
 */

#ifndef RENDERER_BATCH_BUILDER_H
#define RENDERER_BATCH_BUILDER_H

/*!
 * \brief A class to build up batches for glMultiDrawIndirect calls to process
 *
 * Eventually this should be moved to the GPU, so I just upload chunks and I can execute a compute shader to cull
 * chunks for visibility for the shadow or main pass
 */
class batch_builder {

};


#endif //RENDERER_BATCH_BUILDER_H
