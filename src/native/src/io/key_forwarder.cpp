//
// Created by David on 05-Apr-16.
//

#include <easylogging++.h>
#include "key_forwarder.h"

void key_forwarder::forward_keypress(int key, int action) {
    LOG(INFO) << "Received key " << key << " with action " << action;
}

