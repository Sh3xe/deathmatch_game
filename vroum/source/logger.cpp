#include "logger.hpp"

#include <sstream>
#include <iostream>

void vv::Logger::send_to_streams( std::initializer_list<std::string> init_list )
{
	m_mtx.lock();
	std::stringstream ss;

	for(auto &el: init_list)
		ss << el << ' ';
	
	ss << '\n';
	std::cout << ss.str();
	m_mtx.unlock();
}

void vv::Logger::flush()
{
	std::cout << std::flush;
}