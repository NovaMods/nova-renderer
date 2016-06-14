//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_IGRAPHICS_CONTEXT_H
#define RENDERER_IGRAPHICS_CONTEXT_H

// TODO: Document this one next

/*!
 * \brief Defines the interface for a graphics context
 *
 * Handles context creation, destruction, and attribute setting of any graphics context
 */
class igraphics_context {
    virtual int init() = 0;
    virtual void destroy() = 0;
};


#endif //RENDERER_IGRAPHICS_CONTEXT_H
