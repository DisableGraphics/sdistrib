#pragma once
#include <msgpack.hpp>

// Special case for std::monostate (Heartbeat) during serialization
namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		namespace adaptor {
	
			template <>
			struct convert<std::monostate> {
				msgpack::object const& operator()(msgpack::object const& o, std::monostate& v) const {
					return o;
				}
			};
			
			template <>
			struct pack<std::monostate> {
				template <typename Stream>
				packer<Stream>& operator()(msgpack::packer<Stream>& o, std::monostate const& v) const {
					o.pack_nil();
					return o;
				}
			};
			
			template <>
			struct object_with_zone<std::monostate> {
				void operator()(msgpack::object::with_zone& o, std::monostate const& v) const {
					o.type = msgpack::type::NIL;
				}
			};
			
		}  // namespace adaptor
	}
}