#pragma once

#include "vv_headers.hpp"
#include "core/shader.hpp"
#include "scene_2d.hpp"

#include <SDL3/SDL.h>

#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>

namespace vv
{
	
class GraphicsSystem
{
public:
	GraphicsSystem();

	GraphicsSystem(const GraphicsSystem &) = delete;
	GraphicsSystem &operator=(const GraphicsSystem &) = delete;

	bool init( SDL_Window *window );

	void shutdown();

	void send_msg_to_thread(const std::string &hello);
	
private:
	
	void start_thread();

	void worker_loop();

	bool init_opengl(SDL_Window *window);

	std::mutex m_mtx;
	std::condition_variable m_cv;
	std::deque<std::string> m_worker_messages;
	std::thread m_gpu_thread;
	bool m_worker_running = true;

	SDL_GLContext m_context;
};

} // namespace vv