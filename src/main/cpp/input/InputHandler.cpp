#include "glad/glad.h"
#include "InputHandler.h"
#include "../render/nova_renderer.h"

namespace nova {


	void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	}
	void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
		nova_renderer::instance->getInputHandler().queueMouseButtonEvent({ button,action,mods ,1 });
	}

	void MousePositionCallback(GLFWwindow *window, double xpos, double ypos) {
		nova_renderer::instance->getInputHandler().queueMousePositionEvent({(int)xpos,(int) nova_renderer::instance->getGameWindow().get_size().y-(int) ypos,1 });
	}


	InputHandler::InputHandler() {};

	InputHandler::~InputHandler() {};

	void InputHandler::queueMouseButtonEvent(struct MouseButtonEvent e) {
		std::lock_guard<std::mutex> lockGuard(lockButton);
		mouseButtonEventQueue.push(e);
	};

	struct MouseButtonEvent InputHandler::dequeueMouseButtonEvent() {
		std::lock_guard<std::mutex> lockGuard(lockButton);
		if (mouseButtonEventQueue.size() != 0) {
			struct MouseButtonEvent e = mouseButtonEventQueue.front();
			mouseButtonEventQueue.pop();
			return e;
		}
		return { 0,0,0,0 };
	};


	void InputHandler::queueMousePositionEvent(struct MousePositionEvent e) {
		std::lock_guard<std::mutex> lockGuard(lockPosition);
		mousePositionEventQueue.push(e);
	};

	struct MousePositionEvent InputHandler::dequeueMousePositionEvent() {
		std::lock_guard<std::mutex> lockGuard(lockPosition);
		if (mousePositionEventQueue.size() != 0) {
			struct MousePositionEvent e = mousePositionEventQueue.front();
			mousePositionEventQueue.pop();
			return e;
		}
		return { 0,0,0 };
	};

}