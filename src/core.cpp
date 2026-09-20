#include <liara/abi_version.h>
#include <liara/core/config.h>
#include <liara/core/core.h>
#include <liara/core/LiaraCore.h>
#include <liara/modules.h>
#include <liara/renderer/packet.h>
#include <liara/result.h>

#include <cstdint>

struct liara_core_t
{
    mutable bool m_Valid = false;
    Liara::Core::LiaraCore m_Core;
};

static constexpr liara_module_info_t LIARA_CORE_MODULE_INFO = {
    .struct_version = LIARA_MODULE_INFO_VERSION,
    .abi_version = LIARA_ABI_VERSION,
    .module_version = LIARA_CORE_VERSION,
    .reserved = 0,
    .module_name = "Core",
    .abi_version_str = LIARA_ABI_VERSION_STR,
    .module_version_str = LIARA_CORE_VERSION_STRING,
};

const liara_module_info_t* liara_core_info(void) { return &LIARA_CORE_MODULE_INFO; }

uint32_t liara_core_abi_version(void) { return LIARA_CORE_MODULE_INFO.abi_version; }

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
// NOLINTBEGIN(readability-identifier-naming)
liara_result_t liara_core_create(liara_core_handle_t** out_core) {
    // NOLINTEND(readability-identifier-naming)
    if (out_core == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    auto* core = new liara_core_handle_t();
    core->m_Valid = true;
    *out_core = core;
    return LIARA_RESULT_SUCCESS;
}  // NOLINTEND(cppcoreguidelines-owning-memory)

// NOLINTBEGIN(cppcoreguidelines-owning-memory)
// NOLINTBEGIN(readability-identifier-naming)
liara_result_t liara_core_destroy(const liara_core_handle_t* core_handle) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    if (!core_handle->m_Valid) { return LIARA_RESULT_INVALID_STATE; }
    delete core_handle;
    return LIARA_RESULT_SUCCESS;
}  // NOLINTEND(cppcoreguidelines-owning-memory)

// NOLINTBEGIN(readability-identifier-naming)
void liara_core_update(liara_core_handle_t* core_handle, float delta_time) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || !core_handle->m_Valid) { return; }
    core_handle->m_Core.Update(delta_time);
}

// NOLINTBEGIN(readability-identifier-naming)
liara_result_t liara_core_get_render_packet(const liara_core_handle_t* core_handle, liara_render_packet_t* out_packet) {
    // NOLINTEND(readability-identifier-naming)
    if (core_handle == nullptr || out_packet == nullptr) { return LIARA_RESULT_NULL_POINTER; }
    if (!core_handle->m_Valid) { return LIARA_RESULT_INVALID_STATE; }
    return core_handle->m_Core.GetRenderPacket(out_packet);
}
