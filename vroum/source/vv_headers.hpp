#pragma once

#include <cstdint>
#include <cassert>
#include <memory>

#include "logger.hpp"
#include "vv_errors.hpp"

namespace vv
{
	template<typename T>
	using Ref = std::shared_ptr<T>;

	using f32 = float;
	using f64 = double;

	using u32 = uint32_t;
	using i32 = int32_t;

	using u64 = uint64_t;
	using i64 = int64_t;

	using u16 = uint16_t;
	using i16 = int16_t;
}