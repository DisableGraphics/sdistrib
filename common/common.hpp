#pragma once
#include "zhelpers.hpp"
#include <string>
#include <msgpack.hpp>
#include "../worker/subprojects/stable-diffusion.cpp/stable-diffusion.h"

const std::string jobport = "4133";
const std::string respport = "4134";
#define CHUNK_SIZE 4096 // 4KB chunks

//  Basic request-reply client using REQ socket
//
inline void receive_empty_message(zmq::socket_t& sock)
{
    std::string empty = s_recv(sock);
    assert(empty.size() == 0);
}

template <typename T>
bool s_send_msgp(zmq::socket_t& socket, const T& object, int flags = 0) {
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, object);
    zmq::message_t message(sbuf.size());
    memcpy (message.data(), sbuf.data(), sbuf.size());

    bool rc = socket.send (message, static_cast<zmq::send_flags>(flags)).has_value();
    return (rc);
}

template <typename T>
T s_recv_msgp (zmq::socket_t & socket, int flags = 0) {
    zmq::message_t message;
    auto recv_flags = (flags == 0) ? zmq::recv_flags::none: zmq::recv_flags::dontwait;
    (void)socket.recv(message, recv_flags);

	msgpack::unpacked unpacked;
    msgpack::unpack(unpacked, static_cast<const char*>(message.data()), message.size());
    msgpack::object obj = unpacked.get();
	T object;
	obj.convert(object);

    return object;
}


struct Job {
	int id;
	// Parameters for the sd_model_ctx
	std::string model_path;
	std::string clip_l_path;
	std::string clip_g_path;
	std::string t5xxl_path;
	std::string diffusion_model_path;
	std::string vae_path;
	std::string taesd_path;
	std::string control_net_path_c_str;
	std::string lora_model_dir;
	std::string embed_dir_c_str;
	std::string stacked_id_embed_dir_c_str;
	bool vae_decode_only;
	bool vae_tiling;
	bool free_params_immediately;
	int n_threads;
	enum sd_type_t wtype;
	enum rng_type_t rng_type;
	enum schedule_t s;
	bool keep_clip_on_cpu;
	bool keep_control_net_cpu;
	bool keep_vae_on_cpu;
	bool diffusion_flash_attn;
	// Parameters for txt2img
	std::string prompt;
	std::string negative_prompt;
	int clip_skip;
	float cfg_scale;
	float guidance;
	float eta;
	int width;
	int height;
	enum sample_method_t sample_method;
	int sample_steps;
	int64_t seed;
	int batch_count;
	float control_strength;
	float style_strength;
	bool normalize_input;
	std::string input_id_images_path;
	int skip_layers;
	size_t skip_layers_count;
	float slg_scale;
	float skip_layer_start;
	float skip_layer_end;

	MSGPACK_DEFINE(id,
		model_path,
		clip_l_path,
		clip_g_path,
		t5xxl_path,
		diffusion_model_path,
		vae_path,
		taesd_path,
		control_net_path_c_str,
		lora_model_dir,
		embed_dir_c_str,
		stacked_id_embed_dir_c_str,
		vae_decode_only,
		vae_tiling,
		free_params_immediately,
		n_threads,
		wtype,
		rng_type,
		s,
		keep_clip_on_cpu,
		keep_control_net_cpu,
		keep_vae_on_cpu,
		diffusion_flash_attn,
		prompt,
		negative_prompt,
		clip_skip,
		cfg_scale,
		guidance,
		eta,
		width,
		height,
		sample_method,
		sample_steps,
		seed,
		batch_count,
		control_strength,
		style_strength,
		normalize_input,
		input_id_images_path,
		skip_layers,
		skip_layers_count,
		slg_scale,
		skip_layer_start,
		skip_layer_end
	);
};

struct Image {
	int jobid;
	std::vector<uint8_t> data;
	MSGPACK_DEFINE(jobid, data);
};