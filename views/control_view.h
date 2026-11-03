#pragma once

#include <stdint.h>
#include <gui/gui.h>
#include "../pc_monitor.h"

typedef struct {
    const char* label;
    uint8_t command;
} ControlItem;

extern const ControlItem pc_control_items[];
extern const uint8_t pc_control_count;

void draw_control_view(Canvas* canvas, void* ctx);
