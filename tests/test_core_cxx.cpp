// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/core/core.h>
#include <liara/renderer/packet.h>
#include <liara/result.h>

#include <cmath>
#include <cstddef>
#include <vector>

#include <doctest/doctest.h>

namespace
{
    constexpr float TICK_SECONDS = 1.0F / 60.0F;
    constexpr float MOVED_EPSILON = 1e-5F;

    bool AnyDrawableMoved(const liara_render_packet_t& after, const std::vector<liara_render_drawable_t>& before) {
        if (after.drawable_count != before.size()) { return true; }
        for (size_t i = 0; i < before.size(); ++i) {
            if (std::fabs(after.drawables[i].x - before.at(i).x) > MOVED_EPSILON
                || std::fabs(after.drawables[i].y - before.at(i).y) > MOVED_EPSILON) {
                return true;
            }
        }
        return false;
    }
}  // namespace

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

TEST_CASE("liara_core_get_render_packet - populated after an update") {
    liara_core_handle_t* core = nullptr;
    liara_core_create(&core);

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

TEST_CASE("liara_core_update - every call advances the simulation") {
    liara_core_handle_t* core = nullptr;
    liara_core_create(&core);

    liara_render_packet_t before {};
    REQUIRE(liara_core_get_render_packet(core, &before) == LIARA_RESULT_SUCCESS);
    REQUIRE(before.drawable_count > 0);

    const std::vector<liara_render_drawable_t> snapshot(before.drawables, before.drawables + before.drawable_count);

    liara_core_update(core, TICK_SECONDS);

    liara_render_packet_t after {};
    REQUIRE(liara_core_get_render_packet(core, &after) == LIARA_RESULT_SUCCESS);
    REQUIRE(after.drawable_count == snapshot.size());

    CHECK(AnyDrawableMoved(after, snapshot));

    liara_core_destroy(core);
}

TEST_CASE("liara_core_update - a null handle is ignored rather than crashing") {
    liara_core_update(nullptr, TICK_SECONDS);

    // Nothing to assert beyond reaching this line. `liara_core_update` returns void, so the contract
    // it guards is "no-op, no segfault", and the only way to observe it is to survive the call.
    CHECK(true);
}

// NOLINTEND(readability-identifier-naming)
