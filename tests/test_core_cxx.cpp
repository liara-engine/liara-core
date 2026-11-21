// NOLINTBEGIN(readability-identifier-naming)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <liara/core/core.h>
#include <liara/mesh.h>
#include <liara/renderer/packet.h>
#include <liara/result.h>

#include <cstdint>

#include <doctest/doctest.h>

namespace
{
    constexpr float TICK_SECONDS = 1.0F / 60.0F;
    constexpr uint32_t DARK_BLUE = 0xFF1A2440U;

    /** The packet after one more update. */
    liara_render_packet_t PacketAfterUpdate(liara_core_handle_t* core) {
        liara_core_update(core, TICK_SECONDS);
        liara_render_packet_t packet {};
        REQUIRE(liara_core_get_render_packet(core, &packet) == LIARA_RESULT_SUCCESS);
        return packet;
    }

    /** The packet's only view, copied out; a default view when the packet does not have exactly one. */
    liara_render_view_t OnlyView(const liara_render_packet_t& packet) {
        REQUIRE(packet.view_count == 1);
        return packet.view_count == 1 && packet.views != nullptr ? packet.views[0] : liara_render_view_t {};
    }

    /** The mesh of the view's only drawable; LIARA_MESH_ID_NONE when it does not have exactly one. */
    liara_mesh_id_t OnlyMesh(const liara_render_view_t& view) {
        return view.drawable_count == 1 && view.drawables != nullptr ? view.drawables[0].mesh : LIARA_MESH_ID_NONE;
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

TEST_CASE("liara_core_destroy - destroys a created core") {
    liara_core_handle_t* core = nullptr;
    REQUIRE(liara_core_create(&core) == LIARA_RESULT_SUCCESS);
    REQUIRE(core != nullptr);
    liara_core_destroy(core);
}

TEST_CASE("liara_core_destroy - a null handle is a no-op") { liara_core_destroy(nullptr); }

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

TEST_CASE("liara_core_get_render_packet - one view on the window, cleared to the background, with no drawable") {
    liara_core_handle_t* core = nullptr;
    REQUIRE(liara_core_create(&core) == LIARA_RESULT_SUCCESS);

    const liara_render_packet_t packet = PacketAfterUpdate(core);
    CHECK(packet.struct_version == LIARA_RENDER_PACKET_VERSION);
    const liara_render_view_t view = OnlyView(packet);
    CHECK(view.target == LIARA_RENDER_TARGET_WINDOW);
    CHECK(view.viewport.x == 0.0F);
    CHECK(view.viewport.y == 0.0F);
    CHECK(view.viewport.width == 1.0F);
    CHECK(view.viewport.height == 1.0F);
    CHECK(view.clear_color == DARK_BLUE);
    CHECK(view.drawable_count == 0);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_draw_mesh - the next update's view draws that mesh") {
    liara_core_handle_t* core = nullptr;
    REQUIRE(liara_core_create(&core) == LIARA_RESULT_SUCCESS);

    REQUIRE(liara_core_draw_mesh(core, 7) == LIARA_RESULT_SUCCESS);
    const liara_render_view_t view = OnlyView(PacketAfterUpdate(core));
    CHECK(view.drawable_count == 1);
    CHECK(OnlyMesh(view) == 7);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_draw_mesh - takes effect at the next update, not before") {
    liara_core_handle_t* core = nullptr;
    REQUIRE(liara_core_create(&core) == LIARA_RESULT_SUCCESS);

    REQUIRE(liara_core_draw_mesh(core, 7) == LIARA_RESULT_SUCCESS);
    liara_render_packet_t packet {};
    REQUIRE(liara_core_get_render_packet(core, &packet) == LIARA_RESULT_SUCCESS);
    CHECK(OnlyView(packet).drawable_count == 0);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_draw_mesh - LIARA_MESH_ID_NONE draws nothing again") {
    liara_core_handle_t* core = nullptr;
    REQUIRE(liara_core_create(&core) == LIARA_RESULT_SUCCESS);

    REQUIRE(liara_core_draw_mesh(core, 7) == LIARA_RESULT_SUCCESS);
    REQUIRE(OnlyView(PacketAfterUpdate(core)).drawable_count == 1);
    REQUIRE(liara_core_draw_mesh(core, LIARA_MESH_ID_NONE) == LIARA_RESULT_SUCCESS);
    CHECK(OnlyView(PacketAfterUpdate(core)).drawable_count == 0);

    liara_core_destroy(core);
}

TEST_CASE("liara_core_draw_mesh - null core") { CHECK(liara_core_draw_mesh(nullptr, 7) == LIARA_RESULT_NULL_POINTER); }

TEST_CASE("liara_core_update - a null handle is ignored rather than crashing") {
    liara_core_update(nullptr, TICK_SECONDS);

    // Nothing to assert beyond reaching this line. `liara_core_update` returns void, so the contract
    // it guards is "no-op, no segfault", and the only way to observe it is to survive the call.
    CHECK(true);
}

// NOLINTEND(readability-identifier-naming)
