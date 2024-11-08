#pragma once
#include "vulkan/vulkan.h"
#include "NonCopyable.h"
#include "utility/window.h"
#include "renderer/engine.h"
#include <thread>
#include <chrono>

const uint32_t APPLICATION_WIDTH = 1920;
const uint32_t APPLICATION_HEIGHT = 1080;

class Engine;

// @brief The main program
class Application : NonCopyable {
public:
	// @brief Constructor for a new application
	Application();

	// @brief The main running loop of the application
	void run() {
		main_loop();
	};

private:
	// @brief The main window to display the application
	Window window;
	// @brief The graphics engine that manages drawing and rendering tasks
	Engine engine;
	// @brief Debug logger
	Logger* logger;

	void main_loop();
};