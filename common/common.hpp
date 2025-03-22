#pragma once
#include "zhelpers.hpp"
#include <string>
#include <msgpack.hpp>
#include "../worker/stable-diffusion.cpp/stable-diffusion.h"

// Port to send jobs from client to manager
const std::string jobport = "4133";
// Port to send results from worker to manager
const std::string respport = "4134";

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

	bool equals_for_ctx(const Job& other) {
		return model_path == other.model_path &&
			clip_l_path == other.clip_l_path &&
			clip_g_path == other.clip_g_path &&
			t5xxl_path == other.t5xxl_path &&
			diffusion_model_path == other.diffusion_model_path &&
			vae_path == other.vae_path &&
			taesd_path == other.taesd_path &&
			esrgan_path == other.esrgan_path &&
			controlnet_path == other.controlnet_path &&
			embeddings_path == other.embeddings_path && 
			stacked_id_embeddings_path == other.stacked_id_embeddings_path && 
			input_id_images_path == other.input_id_images_path && 
			wtype == other.wtype && 
			lora_model_dir == other.lora_model_dir && 
			input_path == other.input_path &&
			mask_path == other.mask_path &&
			control_image_path == other.control_image_path &&
			vae_tiling == other.vae_tiling &&
			n_threads == other.n_threads &&
			rng_type == other.rng_type &&
			schedule == other.schedule &&
			clip_on_cpu == other.clip_on_cpu &&
			control_net_cpu == other.control_net_cpu &&
			vae_on_cpu == other.vae_on_cpu &&
			diffusion_flash_attn == other.diffusion_flash_attn;
	}

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
enum IMG_ERROR {
	OK,
	OUT_OF_MEMORY,
	MODEL_DOES_NOT_EXIST
};

MSGPACK_ADD_ENUM(sd_type_t);
MSGPACK_ADD_ENUM(sample_method_t);
MSGPACK_ADD_ENUM(schedule_t);
MSGPACK_ADD_ENUM(rng_type_t);
MSGPACK_ADD_ENUM(IMG_ERROR);

inline std::string IMG_ERROR2str(IMG_ERROR err) {
	switch(err) {
		case OK:
			return "OK";
		case OUT_OF_MEMORY:
			return "Out of memory";
		case MODEL_DOES_NOT_EXIST:
			return "Model not found on worker. You may need to copy the model to the worker's model folder.";
	}
	return "UNKNOWN";
}

struct Image {
	int jobid;
	IMG_ERROR error = OK;
	uint32_t width, height;
	std::vector<char> data;
	MSGPACK_DEFINE(jobid, error, width, height, data);
};