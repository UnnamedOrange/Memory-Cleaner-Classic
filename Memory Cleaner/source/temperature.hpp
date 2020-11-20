#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <chrono>
#include <R430-developer/nvapi.h>

class NvThermal
{
private:
	NvAPI_Status init_status{};
	NvPhysicalGpuHandle hPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS]{};
	NvU32 ngpu{};
	NvS32 temperatures[NVAPI_MAX_PHYSICAL_GPUS]{};

	using seconds = std::chrono::duration<double>;
	static constexpr seconds elapse{ 0.5 };

	std::thread loop_thread;
	std::mutex mtx_exit;
	std::condition_variable cv;
	std::atomic<bool> exit{};
	mutable std::mutex mtx_access;
	void update()
	{
		std::lock_guard lock_guard(mtx_access);
		NvAPI_Status ret = NvAPI_EnumPhysicalGPUs(hPhysicalGpu, &ngpu);
		if (ret != NVAPI_OK)
			ngpu = 0;

		NV_GPU_THERMAL_SETTINGS currentTemp{ NV_GPU_THERMAL_SETTINGS_VER };
		for (NvU32 idx = 0; idx < ngpu; idx++)
		{
			ret = NvAPI_GPU_GetThermalSettings(hPhysicalGpu[idx],
				NVAPI_THERMAL_TARGET_ALL, &currentTemp); // »ñÈ¡ÎÂ¶È
			temperatures[idx] = (ret == NVAPI_OK) ?
				currentTemp.sensor[0].currentTemp :
				0;
		}
	}
	void loop()
	{
		while (true)
		{
			std::unique_lock lock(mtx_exit);
			cv.wait_for(lock, elapse,
				[this]()->bool
				{
					return exit;
				});
			if (exit)
				break;
			update();
		}
	}
public:
	std::vector<int> get_nvgpu_temperatures() const
	{
		std::lock_guard lock_guard(mtx_access);
		return std::vector<int>(temperatures, temperatures + ngpu);
	}

public:
	NvThermal()
	{
		init_status = NvAPI_Initialize();
		if (init_status == NVAPI_OK)
		{
			update();
			loop_thread = std::move(std::thread(&NvThermal::loop, this));
		}
	}
	~NvThermal()
	{
		if (init_status == NVAPI_OK)
		{
			NvAPI_Unload();
			exit = true;
			cv.notify_one();
			loop_thread.join();
		}
	}
};