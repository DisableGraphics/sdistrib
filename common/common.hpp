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
    int n_threads = -1;
    std::string model_path;
    std::string clip_l_path;
    std::string clip_g_path;
    std::string t5xxl_path;
    std::string diffusion_model_path;
    std::string vae_path;
    std::string taesd_path;
    std::string esrgan_path;
    std::string controlnet_path;
    std::string embeddings_path;
    std::string stacked_id_embeddings_path;
    std::string input_id_images_path;
    sd_type_t wtype = SD_TYPE_COUNT;
    std::string lora_model_dir;
    std::string output_path = "output.png";
    std::string input_path;
    std::string mask_path;
    std::string control_image_path;

    std::string prompt;
    std::string negative_prompt;
    float min_cfg     = 1.0f;
    float cfg_scale   = 7.0f;
    float guidance    = 3.5f;
    float eta         = 0.f;
    float style_ratio = 20.f;
    int clip_skip     = -1;  // <= 0 represents unspecified
    int width         = 512;
    int height        = 512;
    int batch_count   = 1;

    int video_frames         = 6;
    int motion_bucket_id     = 127;
    int fps                  = 6;
    float augmentation_level = 0.f;

    sample_method_t sample_method = EULER_A;
    schedule_t schedule           = DEFAULT;
    int sample_steps              = 20;
    float strength                = 0.75f;
    float control_strength        = 0.9f;
    rng_type_t rng_type           = CUDA_RNG;
    int64_t seed                  = 42;
    bool verbose                  = false;
    bool vae_tiling               = false;
    bool control_net_cpu          = false;
    bool normalize_input          = false;
    bool clip_on_cpu              = false;
    bool vae_on_cpu               = false;
    bool diffusion_flash_attn     = false;
    bool canny_preprocess         = false;
    bool color                    = false;
    int upscale_repeats           = 1;

    std::vector<int> skip_layers = {7, 8, 9};
    float slg_scale              = 0.f;
    float skip_layer_start       = 0.01f;
    float skip_layer_end         = 0.2f;
	MSGPACK_DEFINE(
		id,
		n_threads,
		model_path,
		clip_l_path,
		clip_g_path,
		t5xxl_path,
		diffusion_model_path,
		vae_path,
		taesd_path,
		esrgan_path,
		controlnet_path,
		embeddings_path,
		stacked_id_embeddings_path,
		input_id_images_path,
		wtype,
		lora_model_dir,
		output_path,
		input_path,
		mask_path,
		controlnet_path,
		prompt,
		negative_prompt,
		min_cfg,
		cfg_scale,
		guidance,
		eta,
		style_ratio,
		clip_skip,
		width,
		height,
		batch_count,
		video_frames,
		motion_bucket_id,
		fps,
		augmentation_level,
		sample_method,
		schedule,
		sample_steps,
		strength,
		control_strength,
		rng_type,
		seed,
		verbose,
		vae_tiling,
		control_net_cpu,
		normalize_input,
		clip_on_cpu,
		vae_on_cpu,
		diffusion_flash_attn,
		canny_preprocess,
		color,
		upscale_repeats,
		skip_layers,
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