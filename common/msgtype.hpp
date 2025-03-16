#pragma once
#include <msgpack.hpp>

enum class MessageType { REGISTER_WORKER, JOB_ASSIGN, JOB_RESULT, HEARTBEAT, ACK_JOB, ACK_HEARTBEAT, ACK_REGISTER };

namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
	namespace adaptor {
	
	template <>
	struct convert<MessageType> {
		msgpack::object const& operator()(msgpack::object const& o, MessageType& v) const {
			int temp;
			o.convert(temp);
			v = static_cast<MessageType>(temp);
			return o;
		}
	};
	
	template <>
	struct pack<MessageType> {
		template <typename Stream>
		packer<Stream>& operator()(msgpack::packer<Stream>& o, MessageType const& v) const {
			o.pack(static_cast<int>(v));
			return o;
		}
	};
	
	template <>
	struct object_with_zone<MessageType> {
		void operator()(msgpack::object::with_zone& o, MessageType const& v) const {
			o.type = msgpack::type::POSITIVE_INTEGER;
			o.via.i64 = static_cast<int>(v);
		}
	};
	
}  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE
}  // namespace msgpack