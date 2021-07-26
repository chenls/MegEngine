./scripts/cmake-build/host_build.sh
export LD_LIBRARY_PATH=build_dir/host/MGE_WITH_CUDA_OFF/MGE_INFERENCE_ONLY_ON/Debug/install/lib
build_dir/host/MGE_WITH_CUDA_OFF/MGE_INFERENCE_ONLY_ON/Debug/install/bin/xor_deploy xornet_deploy.mge 1 1
file build_dir/host/MGE_WITH_CUDA_OFF/MGE_INFERENCE_ONLY_ON/Debug/install/bin/xor_deploy