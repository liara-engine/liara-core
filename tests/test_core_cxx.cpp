// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/core/core.h>
#include <liara/renderer/packet.h>
#include <liara/result.h>

#include <doctest/doctest.h>

TEST_CASE("liara_core_create - success") {
    liara_core_handle_t* core = nullptr;
    const liara_result_t result = liara_core_create(&core);

    CHECK(result == LIARA_RESULT_SUCCESS);
    CHECK(core != nullptr);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_create - null out_core") {
    const liara_result_t result = liara_core_create(nullptr);
    CHECK(result == LIARA_RESULT_NULL_POINTER);
}

TEST_CASE("liara_core_destroy - success") {
    liara_core_handle_t* core = nullptr;
    liara_core_create(&core);

    const liara_result_t result = liara_core_destroy(core);
    CHECK(result == LIARA_RESULT_SUCCESS);
}

TEST_CASE("liara_core_destroy - null pointer") {
    const liara_result_t result = liara_core_destroy(nullptr);
    CHECK(result == LIARA_RESULT_NULL_POINTER);
}

TEST_CASE("liara_core_get_render_packet - null core_handle") {
    liara_render_packet_t packet {};
    const liara_result_t result = liara_core_get_render_packet(nullptr, &packet);
    CHECK(result == LIARA_RESULT_NULL_POINTER);
}

TEST_CASE("liara_core_get_render_packet - null out_packet") {
    liara_core_handle_t* core = nullptr;
    liara_core_create(&core);

    const liara_result_t result = liara_core_get_render_packet(core, nullptr);
    CHECK(result == LIARA_RESULT_NULL_POINTER);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_get_render_packet - populated after a manual update") {
    liara_core_handle_t* core = nullptr;
    liara_core_create(&core);

    liara_core_set_run_mode(core, LIARA_CORE_RUN_MODE_MANUAL, 0.0F);
    liara_core_update(core, 1.0F / 60.0F);

    liara_render_packet_t packet {};
    const liara_result_t result = liara_core_get_render_packet(core, &packet);

    CHECK(result == LIARA_RESULT_SUCCESS);
    CHECK(packet.struct_version == LIARA_RENDER_PACKET_VERSION);
    CHECK(packet.grid_width > 0);
    CHECK(packet.grid_height > 0);
    CHECK(packet.drawables != nullptr);
    CHECK(packet.drawable_count > 0);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_update - manual mode is a no-op without run mode set to manual") {
    liara_core_handle_t* core = nullptr;
    liara_core_create(&core);

    // Default run mode is AUTOMATIC; liara_core_update() should be a no-op until MANUAL is selected explicitly.
    liara_render_packet_t before {};
    liara_core_get_render_packet(core, &before);

    liara_core_update(core, 1.0F / 60.0F);

    liara_render_packet_t after {};
    liara_core_get_render_packet(core, &after);
    CHECK(after.drawable_count == before.drawable_count);

    liara_core_destroy(core);
}

// NOLINTEND(readability-identifier-naming)
