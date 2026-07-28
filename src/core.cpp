#include <liara/abi_version.h>
#include <liara/core/core.h>
#include <liara/core/LiaraCore.h>
#include <liara/modules.h>
#include <liara/result.h>
#include <liara/version.h>

#include <chrono>
#include <cstdint>
#include <thread>

#include <config.h>

struct liara_core_t
{
    mutable bool m_Valid = false;
    LiaraCore m_Core;
};

static constexpr liara_module_info_t LIARA_CORE_MODULE_INFO = {
    .struct_version = LIARA_MODULE_INFO_VERSION,
    .abi_version = LIARA_ABI_VERSION,
    .module_version = LIARA_CORE_VERSION,
    .module_name = "Core",
    .abi_version_str = LIARA_ABI_VERSION_STR,
    .module_version_str = LIARA_CORE_VERSION_STRING,
};

const liara_module_info_t* liara_core_info(void) { return &LIARA_CORE_MODULE_INFO; }

uint32_t liara_core_abi_version(void) { return LIARA_CORE_MODULE_INFO.abi_version; }

uint32_t liara_core_version() {
    return LIARA_MAKE_VERSION_UNSAFE(LIARA_CORE_MAJOR_VERSION, LIARA_CORE_MINOR_VERSION, LIARA_CORE_PATCH_VERSION);
}

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
// NOLINTBEGIN(readability-identifier-naming)
liara_result liara_core_create(liara_renderer_handle_t* renderer_handle, liara_core_handle_t** out_core) {
    // NOLINTEND(readability-identifier-naming)
    if (out_core == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    if (renderer_handle == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    auto* core = new liara_core_handle_t();
    core->m_Valid = true;
    core->m_Core.SetCore(core);
    core->m_Core.SetRenderer(renderer_handle);
    *out_core = core;
    return LIARA_RESULT_SUCCESS;
}  // NOLINTEND(cppcoreguidelines-owning-memory)

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
// NOLINTBEGIN(readability-identifier-naming)
liara_result liara_core_destroy(const liara_core_handle_t* core_handle) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    if (!core_handle->m_Valid) { return LIARA_RESULT_INVALID_STATE; }
    delete core_handle;
    return LIARA_RESULT_SUCCESS;
}  // NOLINTEND(cppcoreguidelines-owning-memory)

// NOLINTBEGIN(readability-identifier-naming)
void liara_core_set_run_mode(liara_core_handle_t* core_handle,
                             enum liara_core_run_mode run_mode,
                             float fixed_time_step) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || !core_handle->m_Valid) { return; }
    core_handle->m_Core.SetRunMode(run_mode, fixed_time_step);
}

// NOLINTBEGIN(readability-identifier-naming)
void liara_core_run(liara_core_handle_t* core_handle) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || !core_handle->m_Valid
        || core_handle->m_Core.GetRunMode() == LIARA_CORE_RUN_MODE_MANUAL) {
        return;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!core_handle->m_Core.IsStopRequested()) {
        const auto NEW_TIME = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> DELTA_TIME = NEW_TIME - currentTime;
        currentTime = NEW_TIME;

        core_handle->m_Core.Update(DELTA_TIME.count());

        auto frameDuration = std::chrono::high_resolution_clock::now() - currentTime;
        if (core_handle->m_Core.GetRunMode() == LIARA_CORE_RUN_MODE_FIXED) {
            if (auto sleepDuration =
                    std::chrono::duration<float>(core_handle->m_Core.GetFixedTimeStep()) - frameDuration;
                sleepDuration.count() > 0) {
                std::this_thread::sleep_for(sleepDuration);
            }
        }
    }
}

// NOLINTBEGIN(readability-identifier-naming)
void liara_core_stop(liara_core_handle_t* core_handle) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || !core_handle->m_Valid
        || core_handle->m_Core.GetRunMode() == LIARA_CORE_RUN_MODE_MANUAL) {
        return;
    }
    core_handle->m_Core.StopRequested();
}

// NOLINTBEGIN(readability-identifier-naming)
void liara_core_update(liara_core_handle_t* core_handle, float delta_time) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || !core_handle->m_Valid
        || core_handle->m_Core.GetRunMode() != LIARA_CORE_RUN_MODE_MANUAL) {
        return;
    }
    core_handle->m_Core.Update(delta_time);
}

// NOLINTBEGIN(readability-identifier-naming)
void liara_core_set_late_update_callback(liara_core_handle_t* core_handle,
                                         void (*callback)(liara_core_handle_t* core, float delta_time)) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || !core_handle->m_Valid) { return; }
    core_handle->m_Core.SetLateUpdateCallback(callback);
}
