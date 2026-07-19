#pragma once

#include "liara/core/core.h"

#include <liara/renderer/renderer.h>
#include <liara/result.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>

class LiaraCore
{
public:
    LiaraCore()
        : m_Core(nullptr)
        , m_Renderer(nullptr)
        , m_ScreenWidth(64)
        , m_ScreenHeight(36) {
        m_Screen.resize(m_ScreenHeight, std::vector<uint32_t>(m_ScreenWidth, 0xFF000000));  // Black background
        constexpr uint8_t NUM_PIXELS = 20;
        m_PixelPosX.resize(NUM_PIXELS);
        m_PixelPosY.resize(NUM_PIXELS);
        m_PixelVelocityX.resize(NUM_PIXELS);
        m_PixelVelocityY.resize(NUM_PIXELS);
        m_PixelColors.resize(NUM_PIXELS);

        for (size_t i = 0; i < NUM_PIXELS; ++i) {
            m_PixelPosX.at(i) = static_cast<uint8_t>(rand() % m_ScreenWidth);
            m_PixelPosY.at(i) = static_cast<uint8_t>(rand() % m_ScreenHeight);
            m_PixelVelocityX.at(i) = static_cast<uint8_t>((rand() % 3) - 1);  // -1, 0, or 1
            m_PixelVelocityY.at(i) = static_cast<uint8_t>((rand() % 3) - 1);  // -1, 0, or 1
            m_PixelColors.at(i) =
                static_cast<uint32_t>(rand() % 0xFFFFFF) | 0xFF000000;  // Random color with full alpha
        }
    }

    ~LiaraCore() = default;

    [[nodiscard]] liara_core_run_mode GetRunMode() const { return m_RunMode; }

    [[nodiscard]] float GetFixedTimeStep() const { return m_FixedTimeStep; }

    [[nodiscard]] bool IsStopRequested() const { return m_StopRequested; }

    void SetCore(liara_core_handle_t* core) { m_Core = core; }

    void SetRenderer(liara_renderer_handle_t* renderer) { m_Renderer = renderer; }

    void SetRunMode(const liara_core_run_mode RUN_MODE, const float FIXED_TIME_STEP) {
        m_RunMode = RUN_MODE;
        m_FixedTimeStep = FIXED_TIME_STEP;
    }

    void StopRequested() { m_StopRequested = true; }

    void SetLateUpdateCallback(void (*callback)(liara_core_handle_t* core, float deltaTime)) {
        m_LateUpdateCallback = callback;
    }

    void Update(const float DELTA_TIME) {
        UpdateScreen(DELTA_TIME);
        RenderScreen();

        if (m_LateUpdateCallback != nullptr) { m_LateUpdateCallback(m_Core, DELTA_TIME); }
    }

private:
    liara_core_run_mode m_RunMode = LIARA_CORE_RUN_MODE_AUTOMATIC;
    float m_FixedTimeStep = 0.016F;
    bool m_StopRequested = false;
    void (*m_LateUpdateCallback)(liara_core_handle_t* core, float deltaTime) = nullptr;

    liara_core_handle_t* m_Core;
    liara_renderer_handle_t* m_Renderer;
    uint16_t m_ScreenWidth;
    uint16_t m_ScreenHeight;
    std::vector<std::vector<uint32_t>> m_Screen;
    std::vector<uint8_t> m_PixelPosX;
    std::vector<uint8_t> m_PixelPosY;
    std::vector<uint8_t> m_PixelVelocityX;
    std::vector<uint8_t> m_PixelVelocityY;
    std::vector<uint32_t> m_PixelColors;

    void UpdateScreen(const float /*DELTA_TIME*/) {
        for (size_t i = 0; i < m_PixelPosX.size(); ++i) {
            const uint8_t PIXEL_X = m_PixelPosX.at(i);
            const uint8_t PIXEL_Y = m_PixelPosY.at(i);
            m_PixelPosX.at(i) += m_PixelVelocityX.at(i);
            m_PixelPosY.at(i) += m_PixelVelocityY.at(i);

            if (m_PixelPosX.at(i) >= m_ScreenWidth || m_PixelPosX.at(i) < 0) {
                m_PixelVelocityX.at(i) = -m_PixelVelocityX.at(i);
                m_PixelPosX.at(i) += m_PixelVelocityX.at(i);
            }

            if (m_PixelPosY.at(i) >= m_ScreenHeight || m_PixelPosY.at(i) < 0) {
                m_PixelVelocityY.at(i) = -m_PixelVelocityY.at(i);
                m_PixelPosY.at(i) += m_PixelVelocityY.at(i);
            }

            m_Screen.at(m_PixelPosY.at(i)).at(m_PixelPosX.at(i)) = m_PixelColors.at(i);
            m_Screen.at(PIXEL_Y).at(PIXEL_X) = 0xFF000000;
        }
    }

    void ClearScreen() const {
        liara_renderer_set_text_color(m_Renderer, 0xFFFFFFFF);  // Set text color to whited
        for (size_t i = 0; i < m_ScreenHeight; ++i) { liara_renderer_print(m_Renderer, "\033[0m\r\033[2K\033[A", 13); }
    }

    void RenderScreen() const {
        static bool firstRender = true;
        if (!firstRender) { ClearScreen(); }
        firstRender = false;

        for (const auto& row : m_Screen) {
            for (const auto& pixel : row) {
                liara_renderer_set_text_color(m_Renderer, pixel);
                liara_renderer_print(m_Renderer, "██", 6);  // Print two block characters to represent a pixel
            }
            liara_renderer_print(m_Renderer, "\n", 1);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
};
