#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <notification/notification.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include "views/usb_hid_keyboard.h"

typedef struct {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    DialogEx* dialog;
    UsbHidKeyboard* usb_hid_keyboard;
    uint32_t view_id;
} UsbHid;

typedef enum {
    UsbHidViewSubmenu,
    UsbHidViewKeyboard,
    UsbHidViewExitConfirm,
} UsbHidView;
