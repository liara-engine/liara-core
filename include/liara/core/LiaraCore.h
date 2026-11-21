#pragma once

#include <liara/mesh.h>
#include <liara/renderer/packet.h>
#include <liara/result.h>

#include <array>
#include <cstdint>

namespace Liara::Core
{
    /** The colour the core's one view clears the window to: dark blue, so that it cannot pass for a window nobody
     * presents to. */
    constexpr uint32_t BACKGROUND_COLOR = 0xFF1A2440U;

    /** The whole of the target. */
    constexpr liara_render_viewport_t FULL_FRAME {.x = 0.0F, .y = 0.0F, .width = 1.0F, .height = 1.0F};

    /**
     * The placeholder core of v0.1: it simulates nothing, and builds the experimental packet of one view on the window
     * which, once the host names a mesh, draws that mesh.
     */
    class LiaraCore
    {
    public:
        LiaraCore() { BuildRenderPacket(); }

        ~LiaraCore() = default;

        LiaraCore(const LiaraCore&) = delete;
        LiaraCore& operator=(const LiaraCore&) = delete;
        LiaraCore(LiaraCore&&) = delete;
        LiaraCore& operator=(LiaraCore&&) = delete;

        /** Nothing simulates yet, so the delta is unused; the packet is rebuilt as the final form will on each tick. */
        void Update([[maybe_unused]] const float deltaTime) { BuildRenderPacket(); }

        /** The mesh the packets draw from the next update on; LIARA_MESH_ID_NONE for none. */
        void DrawMesh(const liara_mesh_id_t mesh) noexcept { m_Mesh = mesh; }

        // The returned packet's pointers alias m_Views and m_Drawables and stay valid only until the next
        // BuildRenderPacket(), that is the next Update(). See liara_core_get_render_packet()'s docs in core.h.
        [[nodiscard]] liara_result_t GetRenderPacket(liara_render_packet_t* outPacket) const {
            if (outPacket == nullptr) { return LIARA_RESULT_NULL_POINTER; }
            *outPacket = m_Packet;
            return LIARA_RESULT_SUCCESS;
        }

    private:
        liara_mesh_id_t m_Mesh = LIARA_MESH_ID_NONE;
        std::array<liara_render_view_t, 1> m_Views {};
        std::array<liara_render_drawable_t, 1> m_Drawables {};
        liara_render_packet_t m_Packet {};

        void BuildRenderPacket() {
            const bool drawing = m_Mesh != LIARA_MESH_ID_NONE;
            m_Drawables.at(0) = liara_render_drawable_t {.mesh = m_Mesh};
            m_Views.at(0) = liara_render_view_t {
                .target = LIARA_RENDER_TARGET_WINDOW,
                .viewport = FULL_FRAME,
                .clear_color = BACKGROUND_COLOR,
                .reserved = 0,
                .drawables = drawing ? m_Drawables.data() : nullptr,
                .drawable_count = drawing ? m_Drawables.size() : 0,
            };
            m_Packet = liara_render_packet_t {
                .struct_version = LIARA_RENDER_PACKET_VERSION,
                .reserved = 0,
                .views = m_Views.data(),
                .view_count = m_Views.size(),
            };
        }
    };
}  // namespace Liara::Core
