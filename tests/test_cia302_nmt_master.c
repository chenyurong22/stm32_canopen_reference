#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cia302_nmt_master.h"

typedef struct {
    cia302_frame_t frames[16];
    size_t frame_count;
    cia302_event_t events[32];
    size_t event_count;
} test_context_t;

static bool send_frame(void *context, const cia302_frame_t *frame) {
    test_context_t *ctx = context;
    assert(ctx->frame_count < 16U);
    ctx->frames[ctx->frame_count++] = *frame;
    return true;
}

static void receive_event(void *context, const cia302_event_t *event) {
    test_context_t *ctx = context;
    assert(ctx->event_count < 32U);
    ctx->events[ctx->event_count++] = *event;
}

static bool saw_event(const test_context_t *ctx, cia302_event_type_t type, uint8_t node_id) {
    for (size_t i = 0U; i < ctx->event_count; ++i) {
        if (ctx->events[i].type == type && ctx->events[i].node_id == node_id) {
            return true;
        }
    }
    return false;
}

int main(void) {
    test_context_t ctx = {0};
    cia302_nmt_master_t master;
    cia302_nmt_master_init(&master, 1U, send_frame, receive_event, &ctx);
    master.nmt_startup = 0x01U;
    master.boot_time_ms = 1000U;
    assert(cia302_nmt_master_configure(&master, 2U, true, true, 100U));
    assert(cia302_nmt_master_configure(&master, 3U, false, true, 100U));
    assert(!cia302_nmt_master_configure(&master, 1U, true, true, 100U));
    assert(cia302_nmt_master_start(&master, 0U));

    uint8_t bootup = 0U;
    cia302_nmt_master_receive(&master, 0x702U, &bootup, 1U, 100U);
    cia302_nmt_master_receive(&master, 0x703U, &bootup, 1U, 110U);
    cia302_nmt_master_process(&master, 120U);
    assert(master.network_ready);
    assert(ctx.frame_count == 2U);
    assert(ctx.frames[0].can_id == 0U && ctx.frames[0].dlc == 2U);
    assert(ctx.frames[0].data[0] == CIA302_NMT_START && ctx.frames[0].data[1] == 2U);
    assert(ctx.frames[1].data[0] == CIA302_NMT_START && ctx.frames[1].data[1] == 3U);
    assert(saw_event(&ctx, CIA302_EVENT_BOOTUP, 2U));
    assert(saw_event(&ctx, CIA302_EVENT_NETWORK_READY, 0U));

    cia302_nmt_master_process(&master, 250U);
    assert(saw_event(&ctx, CIA302_EVENT_HEARTBEAT_TIMEOUT, 2U));
    assert(saw_event(&ctx, CIA302_EVENT_HEARTBEAT_TIMEOUT, 3U));

    uint8_t malformed[2] = {CIA302_HEARTBEAT_OPERATIONAL, 0U};
    size_t before = ctx.event_count;
    cia302_nmt_master_receive(&master, 0x702U, malformed, 2U, 300U);
    assert(ctx.event_count == before + 1U);
    assert(ctx.events[ctx.event_count - 1U].type == CIA302_EVENT_INVALID_FRAME);

    assert(cia302_nmt_master_request(&master, CIA302_NMT_RESET_COMMUNICATION, 0U));
    assert(ctx.frames[ctx.frame_count - 1U].data[0] == CIA302_NMT_RESET_COMMUNICATION);
    assert(ctx.frames[ctx.frame_count - 1U].data[1] == 0U);
    assert(!cia302_nmt_master_request(&master, 0x7FU, 2U));

    test_context_t timeout_ctx = {0};
    cia302_nmt_master_t timeout_master;
    cia302_nmt_master_init(&timeout_master, 1U, send_frame, receive_event, &timeout_ctx);
    timeout_master.boot_time_ms = 50U;
    timeout_master.nmt_startup = CIA302_NMT_START;
    assert(cia302_nmt_master_configure(&timeout_master, 4U, true, true, 100U));
    assert(cia302_nmt_master_start(&timeout_master, 1000U));
    cia302_nmt_master_process(&timeout_master, 1051U);
    assert(!timeout_master.network_ready);
    assert(saw_event(&timeout_ctx, CIA302_EVENT_BOOT_TIMEOUT, 4U));

    puts("cia302_nmt_master: PASS");
    return 0;
}
