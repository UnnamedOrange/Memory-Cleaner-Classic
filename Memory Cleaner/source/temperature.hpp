#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <R430-developer/nvapi.h>

/**
 * @brief Low level wrapper for getting the temperatures of GPUs.
 * NvAPI_Initialize() should be called before calling the functions of this
 * class.
 */
struct _nv_thermal_ll
{
    NvU32 gpu_count{};
    NvPhysicalGpuHandle physical_gpu_handles[NVAPI_MAX_PHYSICAL_GPUS]{};
    NvS32 temperatures[NVAPI_MAX_PHYSICAL_GPUS]{};

    /**
     * @brief Enumerate physical GPUs.
     * Write to gpu_count and physical_gpu_handles.
     */
    void enumerate_gpus()
    {
        NvAPI_Status ret =
            NvAPI_EnumPhysicalGPUs(physical_gpu_handles, &gpu_count);
        if (ret != NVAPI_OK)
            gpu_count = 0;
    }
    /**
     * @brief Update temperatures of the enumerated GPUs.
     * Temperature is set to 0 if failed.
     *
     * @note Call enumerate_gpus() before calling this function.
     */
    void update_temperatures()
    {
        for (NvU32 idx = 0; idx < gpu_count; idx++)
        {
            NV_GPU_THERMAL_SETTINGS thermal_settings{
                NV_GPU_THERMAL_SETTINGS_VER};
            NvAPI_Status result = NvAPI_GPU_GetThermalSettings(
                physical_gpu_handles[idx], NVAPI_THERMAL_TARGET_ALL,
                &thermal_settings);
            if (result != NVAPI_OK)
                temperatures[idx] = 0;
            else
            {
                temperatures[idx] = thermal_settings.sensor[0].currentTemp;
                break;
            }
        }
    }
};

/**
 * @brief Middle level warpper of _nv_thermal_ll.
 * STL is supported.
 */
struct _nv_thermal_ml : public _nv_thermal_ll
{
    auto temps_to_vector() const
    {
        return std::vector<int>(temperatures, temperatures + gpu_count);
    }
};

class NvThermal
{
private:
    _nv_thermal_ml _impl;

    using milliseconds = std::chrono::milliseconds;
    static constexpr milliseconds interval{500};

    std::jthread background_thread;
    mutable std::mutex mutex_exit;
    mutable std::condition_variable_any cv;

    mutable std::mutex mutex_access;
    void update()
    {
        std::lock_guard lock_guard(mutex_access);
        _impl.enumerate_gpus();
        _impl.update_temperatures();
    }
    void thread_routine(std::stop_token st)
    {
        while (!st.stop_requested())
        {
            std::unique_lock lock(mutex_exit);
            cv.wait_for(lock, st, interval, [] { return false; });
            update();
        }
    }

public:
    auto get_nvgpu_temperatures() const
    {
        std::lock_guard lock_guard(mutex_access);
        return _impl.temps_to_vector();
    }

public:
    NvThermal()
    {
        NvAPI_Status result = NvAPI_Initialize();
        if (result == NVAPI_OK)
        {
            update();
            background_thread =
                std::jthread(std::bind_front(&NvThermal::thread_routine, this));
        }
    }
    ~NvThermal()
    {
        if (background_thread.joinable())
        {
            background_thread.request_stop();
            background_thread.join();
        }
        NvAPI_Unload();
    }
};
