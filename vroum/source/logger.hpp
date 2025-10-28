#pragma once

#include <string>
#include <mutex>

inline std::string to_string( const std::string &str ) { return str; }

namespace vv
{

enum class LogLevel
{
	trace,
	debug,
	info,
	warn,
	error,
	fatal
};

class Logger
{
public:
	Logger(const Logger&)           = delete;
	Logger operator=(const Logger&) = delete;

	inline static Logger &get()
	{
		static Logger instance;
		return instance;
	}

	template <typename ...types>
	void log(LogLevel log_level, types&& ...args)
	{
		using ::to_string;
		using std::to_string;

		switch(log_level)
		{
			case LogLevel::debug:
				send_to_streams( { "[DEBUG]:", to_string(args)... } );
				break;
			case LogLevel::error:
				send_to_streams( { "[ERROR]:", to_string(args)... } );
				break;
			case LogLevel::fatal:
				send_to_streams( { "[FATAL]:", to_string(args)... } );
				break;
			case LogLevel::info:
				send_to_streams( { "[INFO ]:", to_string(args)... } );
				break;
			case LogLevel::trace:
				send_to_streams( { "[TRACE]:", to_string(args)... } );
				break;
			case LogLevel::warn:
				send_to_streams( { "[WARN ]:", to_string(args)... } );
				break;
			default: break;
		}
	}

	void flush();

private:
	void send_to_streams( std::initializer_list<std::string> init_list );

	Logger(){}

	std::mutex m_mtx;
};

} // namespace vv

#define VV_LOGGER vv::Logger::get()

#if defined( NDEBUG )

	#define VV_DEBUG(...)
	#define VV_TRACE(...)

#else

	#define VV_DEBUG(...) vv::Logger::get().log( LogLevel::debug, __VA_ARGS__ )
	#define VV_TRACE(...) vv::Logger::get().log( LogLevel::trace, __VA_ARGS__ )

#endif

#define VV_INFO(...)  vv::Logger::get().log( LogLevel::info, __VA_ARGS__ )
#define VV_WARN(...)  vv::Logger::get().log( LogLevel::warn, __VA_ARGS__ )
#define VV_ERROR(...) vv::Logger::get().log( LogLevel::error, __VA_ARGS__ )
#define VV_FATAL(...) vv::Logger::get().log( LogLevel::fatal, __VA_ARGS__ )