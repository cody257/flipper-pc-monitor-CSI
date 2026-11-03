#include "net_view.h"

static const char* band_str(uint8_t band) {
    switch(band) {
    case 1:
        return "2.4GHz";
    case 2:
        return "5GHz";
    case 3:
        return "6GHz";
    default:
        return NULL;
    }
}

static void draw_row(Canvas* canvas, uint8_t y, const char* label, const char* value) {
    canvas_draw_str_aligned(canvas, 1, y, AlignLeft, AlignBottom, label);
    canvas_draw_str_aligned(canvas, 127, y, AlignRight, AlignBottom, value);
}

void draw_net_view(Canvas* canvas, void* ctx) {
    PcMonitorApp* app = ctx;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontKeyboard);

    // Older backend: only the 16-byte resource frame is being sent.
    if(!app->net_valid) {
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignBottom, "No network data");
        canvas_draw_str_aligned(canvas, 64, 44, AlignCenter, AlignBottom, "Update the backend");
        return;
    }

    char value[24];

    if(app->net.flags & NET_FLAG_WIFI) {
        snprintf(value, sizeof(value), "%u%%", app->net.wifi_signal);
    } else {
        snprintf(value, sizeof(value), "n/a");
    }
    draw_row(canvas, 11, "Wi-Fi", value);

    snprintf(value, sizeof(value), "%u Mbps", app->net.wifi_mbps);
    draw_row(canvas, 22, "Link", value);

    const char* band = band_str(app->net.band);
    if(band && app->net.channel) {
        snprintf(value, sizeof(value), "%s ch%u", band, app->net.channel);
    } else if(app->net.channel) {
        snprintf(value, sizeof(value), "ch%u", app->net.channel);
    } else {
        snprintf(value, sizeof(value), "n/a");
    }
    draw_row(canvas, 33, "Band", value);

    if(app->net.flags & NET_FLAG_HAS_IP) {
        snprintf(
            value,
            sizeof(value),
            "%u.%u.%u.%u",
            app->net.ip[0],
            app->net.ip[1],
            app->net.ip[2],
            app->net.ip[3]);
    } else if(app->net.flags & NET_FLAG_IPV6) {
        // Only the first two hextets fit; the rest would not add much here.
        snprintf(
            value,
            sizeof(value),
            "%02x%02x:%02x%02x",
            app->net.ip[0],
            app->net.ip[1],
            app->net.ip[2],
            app->net.ip[3]);
    } else {
        snprintf(value, sizeof(value), "n/a");
    }
    draw_row(canvas, 44, "IP", value);

    draw_row(canvas, 55, "VPN", (app->net.flags & NET_FLAG_VPN) ? "on" : "off");
}
