#include "graph_view.h"

static void
    draw_spark(Canvas* canvas, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* hist, uint8_t count, uint8_t start) {
    // Baseline, so an idle metric still has something to read against.
    canvas_draw_line(canvas, x, y + h, x + w - 1, y + h);

    if(count < 2) return;

    int16_t prev_x = 0;
    int16_t prev_y = 0;

    for(uint8_t k = 0; k < count; k++) {
        uint8_t value = hist[(uint8_t)((start + k) % HIST_LEN)];
        if(value > 100) value = 100;

        // Fixed x scale so the line grows from the left as history fills.
        int16_t px = (int16_t)(x + (uint16_t)k * (w - 1) / (HIST_LEN - 1));
        int16_t py = (int16_t)(y + h - 1 - (uint16_t)value * (h - 1) / 100);

        if(k > 0) canvas_draw_line(canvas, prev_x, prev_y, px, py);

        prev_x = px;
        prev_y = py;
    }
}

void draw_graph_view(Canvas* canvas, void* ctx) {
    PcMonitorApp* app = ctx;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontKeyboard);

    if(!app->hist_count) {
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignBottom, "Collecting...");
        return;
    }

    static const char* labels[4] = {"CPU", "RAM", "GPU", "VRAM"};
    const uint8_t* hists[4] = {app->hist_cpu, app->hist_ram, app->hist_gpu, app->hist_vram};

    uint8_t start = (uint8_t)((app->hist_index + HIST_LEN - app->hist_count) % HIST_LEN);

    for(uint8_t row = 0; row < 4; row++) {
        uint8_t y = (uint8_t)(row * 14);
        canvas_draw_str(canvas, 1, y + 9, labels[row]);
        draw_spark(canvas, 32, y, 95, 11, hists[row], app->hist_count, start);
    }
}
