# sdistrib
Distributed Stable Diffusion running on multiple parallel workers

# Overview
A client sends a petition to a manager, which routes the petition to one available worker.
The petition waits if no workers are available.
When workers become (or were beforehand) available, the manager routes the client's request to the free worker.
The worker processes the petition and sends its results back to the manager, which will route them to the client.
If the worker crashes it returns an error via a custom sigsev handler (stable-diffusion.cpp has this thing of trying to malloc() in the GPU, only to forget that the memory wasn't allocated and horribly dies with a segmentation fault).
It the model does not exist in the worker's model directory, returns another error.

# Installing Dependencies
* Ubuntu:
```
sudo apt install libzmq3-dev
```
* Arch linux:
```
sudo pacman -S zeromq
```

# Compilation
* First clone this project
```
git clone https://github.com/DisableGraphics/sdistrib
cd sdistrib
git submodule update --init --recursive
```
* Manager and Client are strightforward:
```
mkdir build
cd build
cmake ..
cmake --build .
```
* Worker:
Not adding any SD_ACCEL-type commands will make the stable diffusion process run on the CPU, which is extremely slow.
Not all SD_* options supported by stable-diffusion.cpp have been added. Contributions are welcome.
```
mkdir build
cd build
cmake .. # -DSD_CUDA -DSD_VULKAN -DSD_SYCL
cmake --build .
```

# How-To
- 1) Compile the manager (Instructions above) & run the manage `./sdistrib-worker localhost <path-to-models-folder>`.
- 2) Compile one or more workers (Instructions above) for one or more computers & run them `./sdistrib-manager`.
- 3) Compile the client (Instructions above).
- 4) Run the client:
```
./sdistrib-client <ip address of manager> -m "768-v-ema.safetensors" -p "a house in the beach" -n "deep fried" --steps 32 -H 768 -W 768 --vae-tiling --clip-on-cpu -o output.png --seed -1 --sampling-method 'dpm2' --guidance 4 --schedule exponential
```
The client supports all txt2img-related options supported by [stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp).

# Notes
- Only txt2img is (officially) supported
- Models folder should have the following structure (popularized by A1111's webui):
```
- models
  | - stable-diffusion
    | - <your models here>
  | - vae
    | - <your vae models here>
  | - clip
    | - <your clip models here>
  | - t5
    | - <your t5 models here>
  | - diffusion
    | - <your diffusion models here>
  | - taesd
    | - <your taesd models here>
  | - esrgan
    | - <your esrgan models here>
  | - controlnet
    | - <your controlnet models here>
  | - embeddings
    | - <your embeddings here>
```