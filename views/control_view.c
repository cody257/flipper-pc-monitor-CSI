#include "control_view.h"

const ControlItem pc_control_items[] = {
    {"Volume +", CmdVolUp},
    {"Volume -", CmdVolDown},
    {"Mute", CmdMute},
    {"Play / Pause", CmdPlayPause},
    {"Next track", CmdNext},
    {"Prev track", CmdPrev},
    {"Lock PC", CmdLock},
};

const uint8_t pc_control_count =
    (uint8_t)(sizeof(pc_control_items) / sizeof(pc_control_items[0]));

void draw_control_view(Canvas* canvas, void* ctx) {
    PcMonitorApp* app = ctx;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontKeyboard);

    // Five rows fit above the page dots; scroll a window around the selection.
    const uint8_t visible = 5;
    uint8_t top = 0;

    if(app->control_index > 2) top = (uint8_t)(app->control_index - 2);
    if((uint16_t)(top + visible) > pc_control_count) {
        top = pc_control_count > visible ? (uint8_t)(pc_control_count - visible) : 0;
    }

    for(uint8_t i = 0; i < visible && (uint8_t)(top + i) < pc_control_count; i++) {
        uint8_t y = (uint8_t)(i * 11);
        bool selected = (uint8_t)(top + i) == app->control_index;

        if(selected) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, y, 128, 11);
            canvas_set_color(canvas, ColorWhite);
        }

        canvas_draw_str(canvas, 3, y + 9, pc_control_items[top + i].label);
        canvas_set_color(canvas, ColorBlack);
    }
}
