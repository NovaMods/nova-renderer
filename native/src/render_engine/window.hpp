/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#ifndef NOVA_RENDERER_WINDOW_HPP
#define NOVA_RENDERER_WINDOW_HPP

namespace nova {
    /*!
     * \brief A platform-independent window interface
     */
    class iwindow {
    public:
        virtual void on_frame_end() = 0;
        virtual bool should_close() const = 0;
    };
}

#endif //NOVA_RENDERER_WINDOW_HPP
