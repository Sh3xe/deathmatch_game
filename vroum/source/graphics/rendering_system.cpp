#include "rendering_system.hpp"
#include <iostream>
#include <glad/glad.h>

using namespace vv;

RenderingSystem::RenderingSystem()
{

}

void RenderingSystem::start_thread()
{
	m_worker_running = true;
	m_gpu_thread = std::thread(&RenderingSystem::worker_loop, this);
}

void RenderingSystem::worker_loop()
{
	m_worker_running = true;

	while(true)
	{
		RenderCmd cmd;

		{
			// Wait for something to do
			std::unique_lock<std::mutex> lock(m_mtx);
			m_cv.wait(lock, [this]() { return !m_command_queue.empty() || !m_worker_running; });
			
			// Shutdown if necessary
			if(!m_worker_running)
			{
				VV_DEBUG("Worker loop shutdown");
				return;
			}
			
			// Get the command at the front
			cmd = m_command_queue.front();
			m_command_queue.pop_front();

		} // we now have the command, we can let other thread send messages again
		
		// Execute the command
		execute_cmd(cmd);
	}
}

void RenderingSystem::execute_cmd(const RenderCmd &cmd)
{
	switch(cmd.type)
	{
	case RenderCmdType::initialize:
		this->init_opengl(std::get< Ref<InitializeCmd> >(cmd.data)->window);
		break;
	case RenderCmdType::shutdown:
		this->shutdown_opengl();
		break;
	default:
		break;
	}
}

void RenderingSystem::send_render_command(const RenderCmd &cmd)
{
	{
		std::lock_guard<std::mutex> lock(m_mtx); 	
		m_command_queue.push_back(cmd);
	}
	m_cv.notify_one();
}

bool RenderingSystem::init( SDL_Window *window )
{
	// start the rendering thread
	start_thread();

	// immediatly send a command to the opengl thread
	// that tells it to initialize opengl on its end
	RenderCmd cmd (RenderCmdType::initialize, std::make_shared<InitializeCmd>(window));
	send_render_command(cmd);

	return true;
}

void RenderingSystem::shutdown()
{
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_command_queue.clear();
	}
	
	send_render_command(RenderCmd(RenderCmdType::shutdown, ShutdownCmd()));

	m_gpu_thread.join();
}

void RenderingSystem::init_opengl( SDL_Window *window )
{
	m_opengl_initialized = false;

	// Set up the SDL side
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	m_context = SDL_GL_CreateContext(window);

	if(m_context == nullptr)
	{
		VV_ERROR("Cannot create context: ", SDL_GetError());
		return;
	}

	SDL_GL_MakeCurrent(window, m_context);

	if( !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) )
	{
		VV_ERROR("Cannot initialize GLAD");
		return;
	}

	int w_width = 0, w_height = 0;
	if (! SDL_GetWindowSizeInPixels(window, &w_width, &w_height) )
	{
		VV_ERROR("SDL_GetWindowSizeInPixels failed: ", SDL_GetError());
		return;
	}

	glViewport(0, 0, w_width, w_height);

	m_opengl_initialized = true;
}

void RenderingSystem::shutdown_opengl()
{
	SDL_GL_DestroyContext(m_context);
	m_worker_running = false;
}
