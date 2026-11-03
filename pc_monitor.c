#include "pc_monitor.h"

static void draw_page_indicator(Canvas* canvas, PcMonitorApp* app) {
    uint8_t x = 64 - (PageCount * PAGE_DOTS_STEP) / 2 + 1;

    canvas_set_color(canvas, ColorBlack);
    for(uint8_t i = 0; i < PageCount; i++) {
        if(i == (uint8_t)app->page) {
            canvas_draw_box(
                canvas, x + i * PAGE_DOTS_STEP, PAGE_DOTS_Y, PAGE_DOTS_SIZE, PAGE_DOTS_SIZE);
        } else {
            canvas_draw_frame(
                canvas, x + i * PAGE_DOTS_STEP, PAGE_DOTS_Y, PAGE_DOTS_SIZE, PAGE_DOTS_SIZE);
        }
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    PcMonitorApp* app = ctx;

    switch(app->bt_state) {
    case BtStateWaiting:
        draw_connect_view(canvas);
        return;

    case BtStateRecieving:
        if(app->page == PageNet) {
            draw_net_view(canvas, app);
        } else {
            draw_bars_view(canvas, app);
        }
        break;

    default:
        draw_status_view(canvas, app);
        return;
    }

    draw_page_indicator(canvas, app);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static uint16_t bt_serial_callback(SerialServiceEvent event, void* ctx) {
    furi_assert(ctx);
    PcMonitorApp* app = ctx;

    if(event.event == SerialServiceEventTypeDataReceived) {
        FURI_LOG_D(
            TAG,
            "SerialServiceEventTypeDataReceived. Size: %u/%u. Data: %s",
            event.data.size,
            sizeof(DataStruct),
            (char*)event.data.buffer);

        if(event.data.size == sizeof(DataStruct)) {
            memcpy(&app->data, event.data.buffer, sizeof(DataStruct));
            app->bt_state = BtStateRecieving;
            app->last_packet = furi_hal_rtc_get_timestamp();

            // Elegant solution, the backlight is only on when there is continuous communication
            notification_message(app->notification, &sequence_display_backlight_on);

            notification_message(app->notification, &sequence_blink_blue_10);

            view_port_update(app->view_port);
        } else if(event.data.size == sizeof(NetStruct)) {
            // Second frame type. Copied out first so a stray packet with the
            // right length but wrong magic can never be mistaken for this one.
            NetStruct net;
            memcpy(&net, event.data.buffer, sizeof(NetStruct));
            if(net.magic == NET_MAGIC) {
                app->net = net;
                app->net_valid = true;
                view_port_update(app->view_port);
            }
        }
    }

    return 0;
}

static PcMonitorApp* pc_monitor_alloc() {
    PcMonitorApp* app = malloc(sizeof(PcMonitorApp));
    memset(app, 0, sizeof(PcMonitorApp));
    app->view_port = view_port_alloc();
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    app->gui = furi_record_open(RECORD_GUI);
    app->bt = furi_record_open(RECORD_BT);

    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app->event_queue);
    return app;
}

static void pc_monitor_free(PcMonitorApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_BT);
    free(app);
}

int32_t pc_monitor_app(void* p) {
    UNUSED(p);
    PcMonitorApp* app = pc_monitor_alloc();

    bt_disconnect(app->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    bt_keys_storage_set_storage_path(app->bt, APP_DATA_PATH(".bt_serial.keys"));

    BleProfileSerialParams params = {
        .device_name_prefix = "PC Mon",
        .mac_xor = 0x0002,
    };
    app->ble_serial_profile = bt_profile_start(app->bt, ble_profile_serial, &params);

    furi_check(app->ble_serial_profile);

    ble_profile_serial_set_event_callback(
        app->ble_serial_profile, BT_SERIAL_BUFFER_SIZE, bt_serial_callback, app);
    furi_hal_bt_start_advertising();

    app->bt_state = BtStateWaiting;
    FURI_LOG_D(TAG, "Bluetooth is active!");

    // Main loop
    InputEvent event;
    while(true) {
        if(furi_message_queue_get(app->event_queue, &event, 1) == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                if(event.key == InputKeyBack) break;

                if(event.key == InputKeyLeft) {
                    app->page = (PageId)((app->page + PageCount - 1) % PageCount);
                    view_port_update(app->view_port);
                } else if(event.key == InputKeyRight) {
                    app->page = (PageId)((app->page + 1) % PageCount);
                    view_port_update(app->view_port);
                }
            }
        }

        if(app->bt_state == BtStateRecieving &&
           (furi_hal_rtc_get_timestamp() - app->last_packet > 5))
            app->bt_state = BtStateLost;
    }

    ble_profile_serial_set_event_callback(app->ble_serial_profile, 0, NULL, NULL);

    bt_disconnect(app->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);

    furi_check(bt_profile_restore_default(app->bt));

    pc_monitor_free(app);

    return 0;
}
