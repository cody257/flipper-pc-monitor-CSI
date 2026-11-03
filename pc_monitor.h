#pragma once

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_bt.h>
#include "helpers/ble_serial.h"
#include <bt/bt_service/bt.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>
#include <input/input.h>
#include <storage/storage.h>

#include "views/bars_view.h"
#include "views/connect_view.h"
#include "views/net_view.h"
#include "views/status_view.h"

#define TAG                   "PCMonitor"
#define BT_SERIAL_BUFFER_SIZE 128

#define SCREEN_HEIGHT 64
#define LINE_HEIGHT   11

// Keep the bars clear of the page-dot strip at the bottom.
#define BARS_HEIGHT      (SCREEN_HEIGHT - 4)
#define PAGE_DOTS_Y      60
#define PAGE_DOTS_SIZE   3
#define PAGE_DOTS_STEP   6

#define BAR_X     30
#define BAR_WIDTH 97

typedef enum {
    BtStateChecking,
    BtStateInactive,
    BtStateWaiting,
    BtStateRecieving,
    BtStateNoData,
    BtStateLost
} BtState;

#pragma pack(push, 1)
typedef struct {
    uint8_t cpu_usage;
    uint16_t ram_max;
    uint8_t ram_usage;
    char ram_unit[4];
    uint8_t gpu_usage;
    uint16_t vram_max;
    uint8_t vram_usage;
    char vram_unit[4];
} DataStruct;
#pragma pack(pop)

// Second frame type, sent as its own BLE message. 11 bytes on the wire.
// Must stay in sync with `NetInfo` in the backend's src/net_info.rs.
#pragma pack(push, 1)
typedef struct {
    uint8_t magic;
    uint8_t wifi_signal;
    uint16_t wifi_mbps;
    uint8_t band;
    uint8_t channel;
    uint8_t ip[4];
    uint8_t flags;
} NetStruct;
#pragma pack(pop)

#define NET_MAGIC       0xA2
#define NET_FLAG_HAS_IP (1 << 0)
#define NET_FLAG_VPN    (1 << 1)
#define NET_FLAG_WIFI   (1 << 2)
#define NET_FLAG_IPV6   (1 << 3)

// These sizes are the contract with the backend's bincode output. If a field
// is ever added to either side alone, the build fails here instead of the app
// silently showing nothing.
_Static_assert(sizeof(DataStruct) == 16, "DataStruct must stay 16 bytes");
_Static_assert(sizeof(NetStruct) == 11, "NetStruct must stay 11 bytes");

typedef enum {
    PageBars,
    PageNet,
    PageCount,
} PageId;

typedef struct {
    Bt* bt;
    Gui* gui;
    ViewPort* view_port;
    FuriMutex* app_mutex;
    FuriMessageQueue* event_queue;
    NotificationApp* notification;
    FuriHalBleProfileBase* ble_serial_profile;

    BtState bt_state;
    PageId page;
    DataStruct data;
    NetStruct net;
    bool net_valid;
    uint8_t lines_count;
    uint32_t last_packet;
} PcMonitorApp;
