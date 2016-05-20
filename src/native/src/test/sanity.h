/*!
 * \brief Defines a number of functions to perform sanity checks and make sure nothing's stupidly wrong
 * \author David
 * \date 19-May-16.
 */

#ifndef RENDERER_SANITY_H
#define RENDERER_SANITY_H

namespace sanity {
    /*!
     * \brief Checks that we have the right OpenGL version loaded
     */
    static void check_gl_version();

    void run_all();
};

#endif //RENDERER_SANITY_H
