#include <common.hpp>
#include <csetjmp>
#include <stable-diffusion.h>
#include <thirdparty/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include <thirdparty/stb_image_write.h>
#include <thirdparty/stb_image_resize.h>
#include <filesystem>

typedef struct {
    int last_pos;
    void *context;
} custom_stbi_mem_context;

// custom write function for writing the raw image into an image with a format in memory
static void custom_stbi_write_mem(void *context, void *data, int size) {
   custom_stbi_mem_context *c = (custom_stbi_mem_context*)context; 
   //char *dst = (char *)c->context;
   std::vector<char> *dst = (std::vector<char> *)c->context;
   char *src = (char *)data;
   int cur_pos = c->last_pos;
   for (int i = 0; i < size; i++, cur_pos++) {
       dst->emplace_back(src[i]);
   }
   c->last_pos = cur_pos;
}
// Get a context from a job request received from the socket
sd_ctx_t* context_from_job(const Job& job) {
	sd_ctx_t* newctx =
		new_sd_ctx(job.model_path.c_str(), 
		job.clip_l_path.c_str(), 
		job.clip_g_path.c_str(), 
		job.t5xxl_path.c_str(), 
		job.diffusion_model_path.c_str(), 
		job.vae_path.c_str(), 
		job.taesd_path.c_str(),
		job.controlnet_path.c_str(),
		job.lora_model_dir.c_str(), 
		job.embeddings_path.c_str(), 
		job.stacked_id_embeddings_path.c_str(), 
		true, 
		job.vae_tiling,  
		false, 
		job.n_threads, 
		job.wtype, 
		job.rng_type, 
		job.schedule, 
		job.clip_on_cpu, 
		job.control_net_cpu, 
		job.vae_on_cpu, 
		job.diffusion_flash_attn);
	return newctx;
}
// Do the image generation work
sd_image_t* work(sd_ctx_t* context, const Job& job) {
	// Generate the image
	return txt2img(context, 
		job.prompt.c_str(), 
		job.negative_prompt.c_str(), 
		job.clip_skip, 
		job.cfg_scale, 
		job.guidance, 
		job.eta, 
		job.width, 
		job.height, 
		job.sample_method, 
		job.sample_steps, 
		job.seed, 
		job.batch_count, 
		nullptr, 
		job.control_strength, 
		job.style_ratio, 
		job.normalize_input, 
		job.input_id_images_path.c_str(), 
		const_cast<int*>(job.skip_layers.data()), 
		job.skip_layers.size(), 
		job.slg_scale, 
		job.skip_layer_start, 
		job.skip_layer_end);
}

// Segfault handler since stable-diffusion.cpp has this bad habit
// of trying to allocate mem in the GPU, getting an out of memory error, 
// ignoring that completely and horribly dying with a segfault.
std::string address;
IMG_ERROR err;
sigjmp_buf jmpbuf;
void sigsev_handler(int sig, siginfo_t *info, void *ucontext) {
	std::cout << "Worker " << address << ": Out of memory" << std::endl;
	siglongjmp(jmpbuf, 1);
}

void postfix_models_path(Job& job, const std::string& model_path) {
	if(!job.model_path.empty())
		job.model_path = model_path + "/stable-diffusion/" + job.model_path;
	if(!job.clip_l_path.empty())
		job.clip_l_path = model_path + "/clip/" + job.clip_l_path;
	if(!job.clip_g_path.empty())	
		job.clip_g_path = model_path + "/clip/" + job.clip_g_path;
	if(!job.t5xxl_path.empty())
		job.t5xxl_path = model_path + "/t5/" + job.t5xxl_path;
	if(!job.diffusion_model_path.empty())
		job.diffusion_model_path = model_path + "/diffusion/" + job.diffusion_model_path;
	if(!job.vae_path.empty())
		job.vae_path = model_path + "/vae/" + job.vae_path;
	if(!job.taesd_path.empty())
		job.taesd_path = model_path + "/taesd/" + job.taesd_path;
	if(!job.esrgan_path.empty())
		job.esrgan_path = model_path + "/esrgan/" + job.esrgan_path;
	if(!job.controlnet_path.empty())	
		job.controlnet_path = model_path + "/controlnet/" + job.controlnet_path;
	if(!job.embeddings_path.empty())
		job.embeddings_path = model_path + "/embeddings/" + job.embeddings_path;
	if(!job.stacked_id_embeddings_path.empty())
		job.stacked_id_embeddings_path = model_path + "/embeddings/" + job.stacked_id_embeddings_path;
	if(!job.input_id_images_path.empty())
		job.input_id_images_path = model_path + "/input_id_images/" + job.input_id_images_path;
}

bool models_are_available(const Job& job) {
	if(!job.model_path.empty())
		if(!std::filesystem::exists(job.model_path))
			return false;
	if(!job.clip_l_path.empty())
		if(!std::filesystem::exists(job.clip_l_path))
			return false;
	if(!job.clip_g_path.empty())
		if(!std::filesystem::exists(job.clip_g_path))
			return false;
	if(!job.t5xxl_path.empty())
		if(!std::filesystem::exists(job.t5xxl_path))
			return false;
	if(!job.diffusion_model_path.empty())
		if(!std::filesystem::exists(job.diffusion_model_path))
			return false;
	if(!job.vae_path.empty())	
		if(!std::filesystem::exists(job.vae_path))
			return false;
	if(!job.taesd_path.empty())
		if(!std::filesystem::exists(job.taesd_path))
			return false;
	if(!job.esrgan_path.empty())
		if(!std::filesystem::exists(job.esrgan_path))
			return false;
	if(!job.controlnet_path.empty())
		if(!std::filesystem::exists(job.controlnet_path))
			return false;
	if(!job.embeddings_path.empty())
		if(!std::filesystem::exists(job.embeddings_path))
			return false;
	if(!job.stacked_id_embeddings_path.empty())
		if(!std::filesystem::exists(job.stacked_id_embeddings_path))
			return false;
	if(!job.input_id_images_path.empty())
		if(!std::filesystem::exists(job.input_id_images_path))
			return false;
	return true;
}

//  Worker using REQ socket to do LRU routing
//
void worker_fn(int id, const std::string& addr, const std::string& model_path) {
    zmq::context_t context(1);
    zmq::socket_t worker(context, ZMQ_REQ);
	// Set identity
    s_set_id(worker);
    worker.connect(addr);

	std::cout << "Worker " << id << " created" << std::endl;

    //  Tell backend we're ready for work
    s_send(worker, std::string("READY"));
	srand(time(NULL));
	sd_ctx_t* sdctx = nullptr;
    while (1) {
        //  Read and save all frames until we get an empty frame
        address = s_recv(worker);
		receive_empty_message(worker);
        // Get job request
        Job request = s_recv_msgp<Job>(worker);
		postfix_models_path(request, model_path);
		if(!models_are_available(request)) {
			std::cout << "Worker " << id << ": " << "Models are not available" << std::endl;
			Image response{request.id, MODEL_DOES_NOT_EXIST};
			s_sendmore(worker, address);
			s_sendmore(worker, std::string(""));
			s_send_msgp(worker, response);
			continue;
		}
		// Create new context.
		// For some reason I can't reuse the context.
		// It segfaults if I do.
		if(sdctx) free_sd_ctx(sdctx);
		sdctx = context_from_job(request);
		
        std::cout << "Worker " << id << ": " << request.id << std::endl;
		// If the worker segfaults, we catch it and send an error message
		if(sigsetjmp(jmpbuf, 1) == 1) {
			Image response{request.id, OUT_OF_MEMORY};
			s_sendmore(worker, address);
			s_sendmore(worker, std::string(""));
			s_send_msgp(worker, response);
			continue;
		}
		// This is where it *may* segfault
		// Usually with an OOM error
		sd_image_t* img = work(sdctx, request);
		std::cout << "Worker " << id << " finished" << std::endl;

		// If the worker doesn't segfault, we send the image back
		custom_stbi_mem_context context;
		context.last_pos = 0;
		std::vector<char> data;
		context.context = (void*)&data;
		// Convert to the format of choice
		if(request.output_path.ends_with("jpg") || request.output_path.ends_with("jpeg")) {
			int result = stbi_write_jpg_to_func(custom_stbi_write_mem, 
				&context, 
				img->width, 
				img->height, 
				img->channel, 
				img->data, 
				100);
		} else if(request.output_path.ends_with("png")) {
			int result = stbi_write_png_to_func(custom_stbi_write_mem, 
				&context, 
				img->width, 
				img->height, 
				img->channel, 
				img->data, 
				img->width * img->channel);
		}
		// Send it back
		Image response{request.id, OK, img->width, img->height, std::move(data)};
		free(img);
        s_sendmore(worker, address);
        s_sendmore(worker, std::string(""));
        s_send_msgp(worker, response);
    }
}

int main(int argc, char** argv) {
	struct sigaction sa;
	if(argc < 3) { 
		std::cerr << "Usage: " << argv[0] << " <server address> <model_path>" << std::endl;
		return 1;
	}
	std::string address = "tcp://" + std::string(argv[1]) + ":" + respport;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsev_handler;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
		perror("sigaction");

	srand(time(nullptr));
	worker_fn(rand(), address, argv[2]);
	return 0;
}