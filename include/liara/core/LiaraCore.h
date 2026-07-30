#pragma once

#include <liara/core/core.h>
#include <liara/renderer/packet.h>
#include <liara/result.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <utility>
#include <vector>

namespace Liara::Core
{
    constexpr uint16_t GRID_WIDTH = 64;
    constexpr uint16_t GRID_HEIGHT = 36;
    constexpr float CENTER_X = static_cast<float>(GRID_WIDTH) / 2.0F;
    constexpr float CENTER_Y = static_cast<float>(GRID_HEIGHT) / 2.0F;
    constexpr uint32_t BACKGROUND_COLOR = 0xFF000000U;
    constexpr uint32_t SUN_COLOR = 0xFFFFE066U;

    constexpr size_t BODY_COUNT = 4;
    constexpr size_t TRAIL_LENGTH = 4;

    constexpr std::array<float, TRAIL_LENGTH> TRAIL_DIM_FACTORS = {0.55F, 0.35F, 0.20F, 0.10F};

    constexpr std::array<std::pair<float, float>, 5> SUN_OFFSETS = {
        {{0.0F, 0.0F}, {1.0F, 0.0F}, {-1.0F, 0.0F}, {0.0F, 1.0F}, {0.0F, -1.0F}}
    };

    constexpr uint32_t DimColor(const uint32_t color, const float factor) {
        const auto r = static_cast<uint32_t>(static_cast<float>((color >> 16U) & 0xFFU) * factor);
        const auto g = static_cast<uint32_t>(static_cast<float>((color >> 8U) & 0xFFU) * factor);
        const auto b = static_cast<uint32_t>(static_cast<float>(color & 0xFFU) * factor);
        return 0xFF000000U | (r << 16U) | (g << 8U) | b;
    }

    class LiaraCore
    {
    public:
        LiaraCore() {
            m_Drawables.reserve(SUN_OFFSETS.size() + (m_Bodies.size() * (TRAIL_LENGTH + 1)));

            for (size_t i = 0; i < m_Bodies.size(); ++i) {
                OrbitingBody& body = m_Bodies.at(i);
                body.m_Angle =
                    (static_cast<float>(i) * (2.0F * std::numbers::pi_v<float> / static_cast<float>(m_Bodies.size())))
                    + (static_cast<float>(i) * 0.35F);

                const float x = CENTER_X + (body.m_Radius * std::cos(body.m_Angle));
                const float y = CENTER_Y + (body.m_Radius * std::sin(body.m_Angle));
                body.m_TrailX.fill(x);
                body.m_TrailY.fill(y);
            }

            BuildRenderPacket();
        }

        ~LiaraCore() = default;

        LiaraCore(const LiaraCore&) = delete;
        LiaraCore& operator=(const LiaraCore&) = delete;
        LiaraCore(LiaraCore&&) = delete;
        LiaraCore& operator=(LiaraCore&&) = delete;

        [[nodiscard]] liara_core_run_mode GetRunMode() const { return m_RunMode; }

        [[nodiscard]] float GetFixedTimeStep() const { return m_FixedTimeStep; }

        [[nodiscard]] bool IsStopRequested() const { return m_StopRequested; }

        void SetCore(liara_core_handle_t* core) { m_Core = core; }

        void SetRunMode(const liara_core_run_mode runMode, const float fixedTimeStep) {
            m_RunMode = runMode;
            m_FixedTimeStep = fixedTimeStep;
        }

        void StopRequested() { m_StopRequested = true; }

        void SetLateUpdateCallback(void (*callback)(liara_core_handle_t* core, float deltaTime)) {
            m_LateUpdateCallback = callback;
        }

        void Update(const float deltaTime) {
            Simulate(deltaTime);
            BuildRenderPacket();

            if (m_LateUpdateCallback != nullptr) { m_LateUpdateCallback(m_Core, deltaTime); }
        }

        // The returned packet's `drawables` pointer aliases m_Drawables and stays valid only until the next Simulate()/
        // BuildRenderPacket() (i.e. the next Update()). See liara_core_get_render_packet()'s docs in core.h.
        [[nodiscard]] liara_result_t GetRenderPacket(liara_render_packet_t* outPacket) const {
            if (outPacket == nullptr) { return LIARA_RESULT_NULL_POINTER; }
            *outPacket = m_Packet;
            return LIARA_RESULT_SUCCESS;
        }

    private:
        struct OrbitingBody
        {
            float m_Radius;
            float m_AngularSpeed;
            uint32_t m_Color;
            float m_Angle = 0.0F;
            std::array<float, TRAIL_LENGTH> m_TrailX {};
            std::array<float, TRAIL_LENGTH> m_TrailY {};
        };

        liara_core_run_mode m_RunMode = LIARA_CORE_RUN_MODE_AUTOMATIC;
        float m_FixedTimeStep = 0.016F;
        bool m_StopRequested = false;
        void (*m_LateUpdateCallback)(liara_core_handle_t* core, float deltaTime) = nullptr;

        liara_core_handle_t* m_Core {nullptr};

        std::array<OrbitingBody, BODY_COUNT> m_Bodies {
            {{.m_Radius = 5.0F, .m_AngularSpeed = 1.6F, .m_Color = 0xFF6EC6FFU},
             {.m_Radius = 9.0F, .m_AngularSpeed = 1.0F, .m_Color = 0xFFFFA65CU},
             {.m_Radius = 12.0F, .m_AngularSpeed = 0.7F, .m_Color = 0xFF81C784U},
             {.m_Radius = 15.0F, .m_AngularSpeed = 0.5F, .m_Color = 0xFFCE93D8U}}
        };

        std::vector<liara_render_drawable_t> m_Drawables;
        liara_render_packet_t m_Packet {};

        void Simulate(const float deltaTime) {
            for (OrbitingBody& body : m_Bodies) {
                for (size_t age = TRAIL_LENGTH - 1; age > 0; --age) {
                    body.m_TrailX.at(age) = body.m_TrailX.at(age - 1);
                    body.m_TrailY.at(age) = body.m_TrailY.at(age - 1);
                }
                body.m_TrailX.at(0) = CENTER_X + (body.m_Radius * std::cos(body.m_Angle));
                body.m_TrailY.at(0) = CENTER_Y + (body.m_Radius * std::sin(body.m_Angle));

                body.m_Angle =
                    std::fmod(body.m_Angle + (body.m_AngularSpeed * deltaTime), 2.0F * std::numbers::pi_v<float>);
            }
        }

        void BuildRenderPacket() {
            m_Drawables.clear();

            for (const auto& [OFFSET_X, OFFSET_Y] : SUN_OFFSETS) {
                m_Drawables.push_back(liara_render_drawable_t {
                    .x = CENTER_X + OFFSET_X,
                    .y = CENTER_Y + OFFSET_Y,
                    .color = SUN_COLOR,
                });
            }

            for (const OrbitingBody& body : m_Bodies) {
                for (size_t age = TRAIL_LENGTH; age > 0; --age) {
                    const size_t index = age - 1;
                    m_Drawables.push_back(liara_render_drawable_t {
                        .x = body.m_TrailX.at(index),
                        .y = body.m_TrailY.at(index),
                        .color = DimColor(body.m_Color, TRAIL_DIM_FACTORS.at(index)),
                    });
                }
                m_Drawables.push_back(liara_render_drawable_t {
                    .x = CENTER_X + (body.m_Radius * std::cos(body.m_Angle)),
                    .y = CENTER_Y + (body.m_Radius * std::sin(body.m_Angle)),
                    .color = body.m_Color,
                });
            }

            m_Packet.struct_version = LIARA_RENDER_PACKET_VERSION;
            m_Packet.grid_width = GRID_WIDTH;
            m_Packet.grid_height = GRID_HEIGHT;
            m_Packet.background_color = BACKGROUND_COLOR;
            m_Packet.drawables = m_Drawables.data();
            m_Packet.drawable_count = m_Drawables.size();
        }
    };
}  // namespace Liara::Core
