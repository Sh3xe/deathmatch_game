#pragma once

#include "vv_headers.hpp"
#include "core/shader.hpp"
#include "render_cmd.hpp"

#include <SDL3/SDL.h>

#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>

namespace vv
{
	
class RenderingSystem
{
public:
	RenderingSystem();

	RenderingSystem(const RenderingSystem &) = delete;
	RenderingSystem &operator=(const RenderingSystem &) = delete;

	bool init( SDL_Window *window );

	void shutdown();

	void send_render_command(const RenderCommand &hello);
	
private:
	
	void start_thread();

	void worker_loop();

	void init_opengl(SDL_Window *window);

	void execute_cmd(const RenderCommand &cmd);

	std::mutex m_mtx;
	std::condition_variable m_cv;
	std::deque<RenderCommand> m_command_queue;
	std::thread m_gpu_thread;
	bool m_worker_running = true;

	bool m_opengl_initialized = false;
	SDL_GLContext m_context;
};

} // namespace vv