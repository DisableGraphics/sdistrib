#pragma once
#include "msgtype.hpp"
#include "msgvar.hpp"
#include <msgpack.hpp>

struct Message {
	MessageType type;
	MessageVariant variant;
	MSGPACK_DEFINE(type, variant);
};