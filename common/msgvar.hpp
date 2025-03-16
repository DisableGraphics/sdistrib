#pragma once
#include "wkinfo.hpp"
#include "job.hpp"
#include "image.hpp"
#include "monoser.hpp"
#include <variant>

using MessageVariant = std::variant<
	WorkerInfo,
	Job,
	Image,
	std::monostate
>;
