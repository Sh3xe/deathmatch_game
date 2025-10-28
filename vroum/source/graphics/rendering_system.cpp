#include "rendering_system.hpp"
#include <iostream>
#include <glad/glad.h>

vv::RenderingSystem::RenderingSystem()
{

}

void vv::RenderingSystem::start_thread()
{
	m_worker_running = true;
	m_gpu_thread = std::thread(&vv::RenderingSystem::worker_loop, this);
}

void vv::RenderingSystem::worker_loop()
{
	while(true)
	{
		RenderCommand cmd;
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

void vv::RenderingSystem::execute_cmd(const vv::RenderCommand &cmd)
{
	switch(cmd.type)
	{
	case vv::RenderCommandTypes::initialize:
		this->init_opengl(std::get< vv::Ref<vv::InitializeCmd> >(cmd.data)->window);
	default:
		break;
	}
}

void vv::RenderingSystem::send_render_command(const vv::RenderCommand &cmd)
{
	{
		std::lock_guard<std::mutex> lock(m_mtx); 	
		m_command_queue.push_back(cmd);
	}
	m_cv.notify_one();
}

bool vv::RenderingSystem::init( SDL_Window *window )
{
	// start the rendering thread
	start_thread();

	// immediatly send a command to the opengl thread
	// that tells it to initialize opengl on its end
	RenderCommand cmd;
	cmd.type = vv::RenderCommandTypes::initialize;
	cmd.data = std::make_shared<vv::InitializeCmd>(window);
	send_render_command(cmd);

	return true;
}

void vv::RenderingSystem::shutdown()
{
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_worker_running = false;
	}
	m_cv.notify_all();
	m_gpu_thread.join();
	SDL_GL_DestroyContext(m_context);
}

void vv::RenderingSystem::init_opengl( SDL_Window *window )
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