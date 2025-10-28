#include "graphics_system.hpp"
#include <iostream>
#include <glad/glad.h>

vv::GraphicsSystem::GraphicsSystem()
{

}

void vv::GraphicsSystem::start_thread()
{
	m_worker_running = true;
	m_gpu_thread = std::thread(&vv::GraphicsSystem::worker_loop, this);
}

void vv::GraphicsSystem::worker_loop()
{
	while(true)
	{
		std::unique_lock<std::mutex> lock(m_mtx);
		m_cv.wait(lock, [this]() { return !m_worker_messages.empty() || !m_worker_running; });

		if(!m_worker_running)
		{
			VV_DEBUG("Worker loop shutdown");
			return;
		}

		auto msg = m_worker_messages.front();
		m_worker_messages.pop_front();

		VV_INFO(msg);
	}
}

void vv::GraphicsSystem::send_msg_to_thread(const std::string &hello)
{
	{
		std::lock_guard<std::mutex> lock(m_mtx); 	
		m_worker_messages.push_back(hello);
	}
	m_cv.notify_one();
}

bool vv::GraphicsSystem::init( SDL_Window *window )
{
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	m_context = SDL_GL_CreateContext(window);

	if(m_context == nullptr)
	{
		VV_ERROR("Cannot create context: ", SDL_GetError());
		return false;
	}

	start_thread();

	return true;
}

void vv::GraphicsSystem::shutdown()
{
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_worker_running = false;
	}
	m_cv.notify_all();
	m_gpu_thread.join();
	SDL_GL_DestroyContext(m_context);
}

bool vv::GraphicsSystem::init_opengl( SDL_Window *window )
{

	SDL_GL_MakeCurrent(window, m_context);

	if( !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) )
	{
		VV_ERROR("Cannot initialize GLAD");
		return false;
	}

	int w_width = 0, w_height = 0;
	if (! SDL_GetWindowSizeInPixels(window, &w_width, &w_height) )
	{
		VV_ERROR("SDL_GetWindowSizeInPixels failed: ", SDL_GetError());
		return false;
	}

	glViewport(0, 0, w_width, w_height);

	return true;
}