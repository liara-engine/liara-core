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

    constexpr uint32_t DimColor(const uint32_t COLOR, const float FACTOR) {
        const auto R = static_cast<uint32_t>(static_cast<float>((COLOR >> 16U) & 0xFFU) * FACTOR);
        const auto G = static_cast<uint32_t>(static_cast<float>((COLOR >> 8U) & 0xFFU) * FACTOR);
        const auto B = static_cast<uint32_t>(static_cast<float>(COLOR & 0xFFU) * FACTOR);
        return 0xFF000000U | (R << 16U) | (G << 8U) | B;
    }

    class LiaraCore
    {
    public:
        LiaraCore()
            : m_Core(nullptr) {
            m_Drawables.reserve(SUN_OFFSETS.size() + m_Bodies.size() * (TRAIL_LENGTH + 1));

            for (size_t i = 0; i < m_Bodies.size(); ++i) {
                OrbitingBody& body = m_Bodies.at(i);
                body.angle =
                    static_cast<float>(i) * (2.0F * std::numbers::pi_v<float> / static_cast<float>(m_Bodies.size()))
                    + static_cast<float>(i) * 0.35F;

                const float X = CENTER_X + body.radius * std::cos(body.angle);
                const float Y = CENTER_Y + body.radius * std::sin(body.angle);
                body.trailX.fill(X);
                body.trailY.fill(Y);
            }

            BuildRenderPacket();
        }

        ~LiaraCore() = default;

        [[nodiscard]] liara_core_run_mode GetRunMode() const { return m_RunMode; }

        [[nodiscard]] float GetFixedTimeStep() const { return m_FixedTimeStep; }

        [[nodiscard]] bool IsStopRequested() const { return m_StopRequested; }

        void SetCore(liara_core_handle_t* core) { m_Core = core; }

        void SetRunMode(const liara_core_run_mode RUN_MODE, const float FIXED_TIME_STEP) {
            m_RunMode = RUN_MODE;
            m_FixedTimeStep = FIXED_TIME_STEP;
        }

        void StopRequested() { m_StopRequested = true; }

        void SetLateUpdateCallback(void (*callback)(liara_core_handle_t* core, float deltaTime)) {
            m_LateUpdateCallback = callback;
        }

        void Update(const float DELTA_TIME) {
            Simulate(DELTA_TIME);
            BuildRenderPacket();

            if (m_LateUpdateCallback != nullptr) { m_LateUpdateCallback(m_Core, DELTA_TIME); }
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
            float radius;
            float angularSpeed;
            uint32_t color;
            float angle = 0.0F;
            std::array<float, TRAIL_LENGTH> trailX {};
            std::array<float, TRAIL_LENGTH> trailY {};
        };

        liara_core_run_mode m_RunMode = LIARA_CORE_RUN_MODE_AUTOMATIC;
        float m_FixedTimeStep = 0.016F;
        bool m_StopRequested = false;
        void (*m_LateUpdateCallback)(liara_core_handle_t* core, float deltaTime) = nullptr;

        liara_core_handle_t* m_Core;

        std::array<OrbitingBody, BODY_COUNT> m_Bodies {
            {{.radius = 5.0F, .angularSpeed = 1.6F, .color = 0xFF6EC6FFU},
             {.radius = 9.0F, .angularSpeed = 1.0F, .color = 0xFFFFA65CU},
             {.radius = 12.0F, .angularSpeed = 0.7F, .color = 0xFF81C784U},
             {.radius = 15.0F, .angularSpeed = 0.5F, .color = 0xFFCE93D8U}}
        };

        std::vector<liara_render_drawable_t> m_Drawables;
        liara_render_packet_t m_Packet {};

        void Simulate(const float DELTA_TIME) {
            for (OrbitingBody& body : m_Bodies) {
                for (size_t age = TRAIL_LENGTH - 1; age > 0; --age) {
                    body.trailX.at(age) = body.trailX.at(age - 1);
                    body.trailY.at(age) = body.trailY.at(age - 1);
                }
                body.trailX.at(0) = CENTER_X + body.radius * std::cos(body.angle);
                body.trailY.at(0) = CENTER_Y + body.radius * std::sin(body.angle);

                body.angle = std::fmod(body.angle + body.angularSpeed * DELTA_TIME, 2.0F * std::numbers::pi_v<float>);
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
                    const size_t INDEX = age - 1;
                    m_Drawables.push_back(liara_render_drawable_t {
                        .x = body.trailX.at(INDEX),
                        .y = body.trailY.at(INDEX),
                        .color = DimColor(body.color, TRAIL_DIM_FACTORS.at(INDEX)),
                    });
                }
                m_Drawables.push_back(liara_render_drawable_t {
                    .x = CENTER_X + body.radius * std::cos(body.angle),
                    .y = CENTER_Y + body.radius * std::sin(body.angle),
                    .color = body.color,
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
