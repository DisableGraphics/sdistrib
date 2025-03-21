#include <common.hpp>
#include <stable-diffusion.h>
#include <thirdparty/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include <thirdparty/stb_image_write.h>
#include <thirdparty/stb_image_resize.h>

typedef struct {
    int last_pos;
    void *context;
} custom_stbi_mem_context;

// custom write function
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
		true, 
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

sd_image_t* work(sd_ctx_t* context, const Job& job) {
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

//  Worker using REQ socket to do LRU routing
//
void worker_thread(int id, const std::string& model_path) {
    zmq::context_t context(1);
    zmq::socket_t worker(context, ZMQ_REQ);

    s_set_id(worker);
    worker.connect("tcp://localhost:" + respport);

	std::cout << "Worker " << id << " created" << std::endl;

    //  Tell backend we're ready for work
    s_send(worker, std::string("READY"));
	srand(time(NULL));
	sd_ctx_t* sdctx = nullptr;
	Job prevjob;
    while (1) {
        //  Read and save all frames until we get an empty frame
        //  In this example there is only 1 but it could be more
        std::string address = s_recv(worker);
		receive_empty_message(worker);
        //  Get request, send reply
        Job request = s_recv_msgp<Job>(worker);

		if(!sdctx || !request.equals_for_ctx(prevjob)) {
			if(sdctx) free_sd_ctx(sdctx);
			sdctx = context_from_job(request);
		}
		prevjob = request;
		
        std::cout << "Worker " << id << ": " << request.id << std::endl;
		sd_image_t* img = work(sdctx, request);
		std::cout << "Worker " << id << " finished" << std::endl;

		custom_stbi_mem_context context;
		context.last_pos = 0;
		std::vector<char> data;
		context.context = (void*)&data;

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
		
		Image response{request.id, img->width, img->height, std::move(data)};
		free(img);
        s_sendmore(worker, address);
        s_sendmore(worker, std::string(""));
        s_send_msgp(worker, response);
    }
}

int main(int argc, char** argv) {
	srand(time(nullptr));
	worker_thread(rand(), "");
	return 0;
}