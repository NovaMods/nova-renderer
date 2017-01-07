//
// Created by David on 25-Dec-15.
//

#ifndef RENDERER_WINDOW_H
#define RENDERER_WINDOW_H

#include <glm/glm.hpp>
#include "data_loading/settings.h"

/*
 * !\brief Represents a window that can be drawn to.
 *
 * This class simply defines an interface. I expect I'll need a separate subclass for each rendering API
 */
class iwindow : public nova::iconfig_listener {
public:
    /*!
     * \brief Initializes the window, creating it and making it visible.
     *
     * In this code, the window controls the graphics context which draws to it. This method should also initialize the
     * graphics context
     */
    virtual int init() = 0;

    /*!
     * \brief de-initializes the window, shuts down the grpahics context
     */
    virtual void destroy() = 0;

    /*!
     *
     *\brief Ends the current frame, swapping buffers and whatnot
     */
    virtual void end_frame() = 0;

    /*!
     * \brief Toggles whether the window is fullscreen or not
     *
     * \param fullscreen If true, the window will be made fullscreen. If false, the window will be made windowed
     */
    virtual void set_fullscreen(bool fullscreen) = 0;

    /*!
     * \brief Gets the current size, in pixels, of the window
     */
    virtual glm::vec2 get_size() = 0;

    /*!
     * \brief Tells you if the window should close or not
     */
    virtual bool should_close() = 0;
};

#endif //RENDERER_WINDOW_H
