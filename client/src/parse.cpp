#include "parse.hpp"
#include <regex>
#include <thread>

const char* rng_type_to_str[] = {
    "std_default",
    "cuda",
};

// Names of the sampler method, same order as enum sample_method in stable-diffusion.h
const char* sample_method_str[] = {
    "euler_a",
    "euler",
    "heun",
    "dpm2",
    "dpm++2s_a",
    "dpm++2m",
    "dpm++2mv2",
    "ipndm",
    "ipndm_v",
    "lcm",
    "ddim_trailing",
    "tcd",
};

// Names of the sigma schedule overrides, same order as sample_schedule in stable-diffusion.h
const char* schedule_str[] = {
    "default",
    "discrete",
    "karras",
    "exponential",
    "ays",
    "gits",
};

void parse_args(int argc, const char** argv, Job& params) {
    bool invalid_arg = false;
    std::string arg;
    for (int i = 1; i < argc; i++) {
        arg = argv[i];

        if (arg == "-t" || arg == "--threads") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.n_threads = std::stoi(argv[i]);
        } else if (arg == "-m" || arg == "--model") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.model_path = argv[i];
        } else if (arg == "--clip_l") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.clip_l_path = argv[i];
        } else if (arg == "--clip_g") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.clip_g_path = argv[i];
        } else if (arg == "--t5xxl") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.t5xxl_path = argv[i];
        } else if (arg == "--diffusion-model") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.diffusion_model_path = argv[i];
        } else if (arg == "--vae") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.vae_path = argv[i];
        } else if (arg == "--taesd") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.taesd_path = argv[i];
        } else if (arg == "--control-net") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.controlnet_path = argv[i];
        } else if (arg == "--upscale-model") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.esrgan_path = argv[i];
        } else if (arg == "--embd-dir") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.embeddings_path = argv[i];
        } else if (arg == "--stacked-id-embd-dir") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.stacked_id_embeddings_path = argv[i];
        } else if (arg == "--input-id-images-dir") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.input_id_images_path = argv[i];
        } else if (arg == "--lora-model-dir") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.lora_model_dir = argv[i];
        } else if (arg == "-i" || arg == "--init-img") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.input_path = argv[i];
        } else if (arg == "--mask") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.mask_path = argv[i];
        } else if (arg == "--control-image") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.control_image_path = argv[i];
        } else if (arg == "-o" || arg == "--output") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.output_path = argv[i];
        } else if (arg == "-p" || arg == "--prompt") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.prompt = argv[i];
        } else if (arg == "--upscale-repeats") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.upscale_repeats = std::stoi(argv[i]);
            if (params.upscale_repeats < 1) {
                fprintf(stderr, "error: upscale multiplier must be at least 1\n");
                exit(1);
            }
        } else if (arg == "-n" || arg == "--negative-prompt") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.negative_prompt = argv[i];
        } else if (arg == "--cfg-scale") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.cfg_scale = std::stof(argv[i]);
        } else if (arg == "--guidance") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.guidance = std::stof(argv[i]);
        } else if (arg == "--eta") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.eta = std::stof(argv[i]);
        } else if (arg == "--strength") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.strength = std::stof(argv[i]);
        } else if (arg == "--style-ratio") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.style_ratio = std::stof(argv[i]);
        } else if (arg == "--control-strength") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.control_strength = std::stof(argv[i]);
        } else if (arg == "-H" || arg == "--height") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.height = std::stoi(argv[i]);
        } else if (arg == "-W" || arg == "--width") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.width = std::stoi(argv[i]);
        } else if (arg == "--steps") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.sample_steps = std::stoi(argv[i]);
        } else if (arg == "--clip-skip") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.clip_skip = std::stoi(argv[i]);
        } else if (arg == "--vae-tiling") {
            params.vae_tiling = true;
        } else if (arg == "--control-net-cpu") {
            params.control_net_cpu = true;
        } else if (arg == "--normalize-input") {
            params.normalize_input = true;
        } else if (arg == "--clip-on-cpu") {
            params.clip_on_cpu = true;  // will slow down get_learned_condiotion but necessary for low MEM GPUs
        } else if (arg == "--vae-on-cpu") {
            params.vae_on_cpu = true;  // will slow down latent decoding but necessary for low MEM GPUs
        } else if (arg == "--diffusion-fa") {
            params.diffusion_flash_attn = true;  // can reduce MEM significantly
        } else if (arg == "--canny") {
            params.canny_preprocess = true;
        } else if (arg == "-b" || arg == "--batch-count") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.batch_count = std::stoi(argv[i]);
        } else if (arg == "--rng") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            std::string rng_type_str = argv[i];
            if (rng_type_str == "std_default") {
                params.rng_type = STD_DEFAULT_RNG;
            } else if (rng_type_str == "cuda") {
                params.rng_type = CUDA_RNG;
            } else {
                invalid_arg = true;
                break;
            }
        } else if (arg == "--schedule") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            const char* schedule_selected = argv[i];
            int schedule_found            = -1;
            for (int d = 0; d < N_SCHEDULES; d++) {
                if (!strcmp(schedule_selected, schedule_str[d])) {
                    schedule_found = d;
                }
            }
            if (schedule_found == -1) {
                invalid_arg = true;
                break;
            }
            params.schedule = (schedule_t)schedule_found;
        } else if (arg == "-s" || arg == "--seed") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.seed = std::stoll(argv[i]);
        } else if (arg == "--sampling-method") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            const char* sample_method_selected = argv[i];
            int sample_method_found            = -1;
            for (int m = 0; m < N_SAMPLE_METHODS; m++) {
                if (!strcmp(sample_method_selected, sample_method_str[m])) {
                    sample_method_found = m;
                }
            }
            if (sample_method_found == -1) {
                invalid_arg = true;
                break;
            }
            params.sample_method = (sample_method_t)sample_method_found;
        } else if (arg == "-h" || arg == "--help") {
            exit(0);
        } else if (arg == "-v" || arg == "--verbose") {
            params.verbose = true;
        } else if (arg == "--color") {
            params.color = true;
        } else if (arg == "--slg-scale") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.slg_scale = std::stof(argv[i]);
        } else if (arg == "--skip-layers") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            if (argv[i][0] != '[') {
                invalid_arg = true;
                break;
            }
            std::string layers_str = argv[i];
            while (layers_str.back() != ']') {
                if (++i >= argc) {
                    invalid_arg = true;
                    break;
                }
                layers_str += " " + std::string(argv[i]);
            }
            layers_str = layers_str.substr(1, layers_str.size() - 2);

            std::regex regex("[, ]+");
            std::sregex_token_iterator iter(layers_str.begin(), layers_str.end(), regex, -1);
            std::sregex_token_iterator end;
            std::vector<std::string> tokens(iter, end);
            std::vector<int> layers;
            for (const auto& token : tokens) {
                try {
                    layers.push_back(std::stoi(token));
                } catch (const std::invalid_argument& e) {
                    invalid_arg = true;
                    break;
                }
            }
            params.skip_layers = layers;

            if (invalid_arg) {
                break;
            }
        } else if (arg == "--skip-layer-start") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.skip_layer_start = std::stof(argv[i]);
        } else if (arg == "--skip-layer-end") {
            if (++i >= argc) {
                invalid_arg = true;
                break;
            }
            params.skip_layer_end = std::stof(argv[i]);
        } else {
            fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
            exit(1);
        }
    }
    if (invalid_arg) {
        fprintf(stderr, "error: invalid parameter for argument: %s\n", arg.c_str());
        exit(1);
    }
    if (params.n_threads <= 0) {
        params.n_threads = std::thread::hardware_concurrency();
    }

    if (params.model_path.length() == 0 && params.diffusion_model_path.length() == 0) {
        fprintf(stderr, "error: the following arguments are required: model_path/diffusion_model\n");
        exit(1);
    }

    if (params.output_path.length() == 0) {
        fprintf(stderr, "error: the following arguments are required: output_path\n");
        exit(1);
    }

    if (params.width <= 0 || params.width % 64 != 0) {
        fprintf(stderr, "error: the width must be a multiple of 64\n");
        exit(1);
    }

    if (params.height <= 0 || params.height % 64 != 0) {
        fprintf(stderr, "error: the height must be a multiple of 64\n");
        exit(1);
    }

    if (params.sample_steps <= 0) {
        fprintf(stderr, "error: the sample_steps must be greater than 0\n");
        exit(1);
    }

    if (params.strength < 0.f || params.strength > 1.f) {
        fprintf(stderr, "error: can only work with strength in [0.0, 1.0]\n");
        exit(1);
    }

    if (params.seed < 0) {
        srand((int)time(NULL));
        params.seed = rand();
    }
}