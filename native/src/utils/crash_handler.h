//
// Created by jannis on 19.07.18.
//

#ifndef NOVA_RENDERER_CRASH_HANDLER_H
#define NOVA_RENDERER_CRASH_HANDLER_H

namespace nova {
    class crash_handler {
    private:
        static void handle_terminate();
        static void print_stacktrace();
    public:
        static void install();
    };
}


#endif //NOVA_RENDERER_CRASH_HANDLER_H
