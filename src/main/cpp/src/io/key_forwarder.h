//
// Created by David on 05-Apr-16.
//

#ifndef RENDERER_KEY_FORWARDER_H
#define RENDERER_KEY_FORWARDER_H

/*!
 * \brief Handles sending input to MC
 *
 * Not sure how I'll do this yet. It's very much a TODO
 */
class key_forwarder {
public:
    static void forward_keypress(int key, int action);
};


#endif //RENDERER_KEY_FORWARDER_H
