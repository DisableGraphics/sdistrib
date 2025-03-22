# sdistrib
Distributed Stable Diffusion running on multiple parallel workers

# Overview
A client sends a petition to a manager, which routes the petition to one available worker.
The petition waits if no workers are available.
When workers become (or were beforehand) available, the manager routes the client's request to the free worker.
The worker processes the petition and sends its results back to the manager, which will route them to the client.
If the worker crashes it returns an error via a custom sigsev handler (stable-diffusion.cpp has this thing of trying to malloc() in the GPU, only to forget that the memory wasn't allocated and horribly dies with a segmentation fault).
It the model does not exist in the worker's model directory, returns another error.

# Compilation
* Manager and Client are strightforward:
```
mkdir build
cd build
cmake ..
cmake --build .
```
* Worker:
Note that for SYCL to work you need to install the sycl toolkit. More info [here](https://github.com/leejet/stable-diffusion.cpp)
Not adding any SD_ACCEL-type commands will make the stable diffusion process run on the CPU, which is extremely slow.
Not all SD_* options supported by stable-diffusion.cpp have been added. Contributions are welcome.
```
mkdir build
cd build
cmake .. # -DSD_CUDA -DSD_VULKAN -DSD_SYCL
cmake --build .
```