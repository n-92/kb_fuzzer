#include "../bad_kb_app.h"
#include "../helpers/ducky_script.h"
#include "furi_hal_power.h"
#include "furi_hal_usb.h"
#include <xtreme.h>

enum VarItemListIndex {
    VarItemListIndexKeyboardLayout,
    VarItemListIndexConnection,
};

enum VarItemListIndexBt {
    VarItemListIndexBtRemember = VarItemListIndexConnection + 1,
    VarItemListIndexBtDeviceName,
    VarItemListIndexBtMacAddress,
    VarItemListIndexBtRandomizeMac,
};

enum VarItemListIndexUsb {
    VarItemListIndexUsbManufacturer = VarItemListIndexConnection + 1,
    VarItemListIndexUsbProductName,
    VarItemListIndexUsbVidPid,
    VarItemListIndexUsbRandomizeVidPid,
};

void bad_kb_scene_config_connection_callback(VariableItem* item) {
    BadKbApp* bad_kb = variable_item_get_context(item);
    bad_kb->is_bt = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, bad_kb->is_bt ? "BT" : "USB");
    view_dispatcher_send_custom_event(bad_kb->view_dispatcher, VarItemListIndexConnection);
}

void bad_kb_scene_config_bt_remember_callback(VariableItem* item) {
    BadKbApp* bad_kb = variable_item_get_context(item);
    bad_kb->bt_remember = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, bad_kb->bt_remember ? "ON" : "OFF");
    view_dispatcher_send_custom_event(bad_kb->view_dispatcher, VarItemListIndexBtRemember);
}

void bad_kb_scene_config_var_item_list_callback(void* context, uint32_t index) {
    BadKbApp* bad_kb = context;
    view_dispatcher_send_custom_event(bad_kb->view_dispatcher, index);
}

void bad_kb_scene_config_on_enter(void* context) {
    BadKbApp* bad_kb = context;
    VariableItemList* var_item_list = bad_kb->var_item_list;
    VariableItem* item;

    item = variable_item_list_add(var_item_list, "Keyboard layout", 0, NULL, bad_kb);

    item = variable_item_list_add(
        var_item_list, "Connection", 2, bad_kb_scene_config_connection_callback, bad_kb);
    variable_item_set_current_value_index(item, bad_kb->is_bt);
    variable_item_set_current_value_text(item, bad_kb->is_bt ? "BT" : "USB");
    if(bad_kb->has_usb_id) {
        variable_item_set_locked(item, true, "Script has\nID cmd!\nLocked to\nUSB Mode!");
    } else if(bad_kb->has_bt_id) {
        variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nBT Mode!");
    }

    if(bad_kb->is_bt) {
        item = variable_item_list_add(
            var_item_list, "BT Remember", 2, bad_kb_scene_config_bt_remember_callback, bad_kb);
        variable_item_set_current_value_index(item, bad_kb->bt_remember);
        variable_item_set_current_value_text(item, bad_kb->bt_remember ? "ON" : "OFF");

        item = variable_item_list_add(var_item_list, "BT Device Name", 0, NULL, bad_kb);
        if(bad_kb->set_bt_id) {
            variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nset Name!");
        }

        item = variable_item_list_add(var_item_list, "BT MAC Address", 0, NULL, bad_kb);
        if(bad_kb->bt_remember) {
            variable_item_set_locked(item, true, "Remember\nmust be Off!");
        } else if(bad_kb->set_bt_id) {
            variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nset MAC!");
        }

        item = variable_item_list_add(var_item_list, "Randomize BT MAC", 0, NULL, bad_kb);
        if(bad_kb->bt_remember) {
            variable_item_set_locked(item, true, "Remember\nmust be Off!");
        } else if(bad_kb->set_bt_id) {
            variable_item_set_locked(item, true, "Script has\nBT_ID cmd!\nLocked to\nset MAC!");
        }
    } else {
        item = variable_item_list_add(var_item_list, "USB Manufacturer", 0, NULL, bad_kb);
        if(bad_kb->set_usb_id) {
            variable_item_set_locked(item, true, "Script has\nID cmd!\nLocked to\nset Mname!");
        }

        item = variable_item_list_add(var_item_list, "USB Product Name", 0, NULL, bad_kb);
        if(bad_kb->set_usb_id) {
            variable_item_set_locked(item, true, "Script has\nID cmd!\nLocked to\nset Pname!");
        }

        item = variable_item_list_add(var_item_list, "USB VID and PID", 0, NULL, bad_kb);
        if(bad_kb->set_usb_id) {
            variable_item_set_locked(item, true, "Script has\nID cmd!\nLocked to\nset IDs!");
        }

        item = variable_item_list_add(var_item_list, "Randomize USB VID:PID", 0, NULL, bad_kb);
        if(bad_kb->set_usb_id) {
            variable_item_set_locked(item, true, "Script has\nID cmd!\nLocked to\nset IDs!");
        }
    }

    variable_item_list_set_enter_callback(
        var_item_list, bad_kb_scene_config_var_item_list_callback, bad_kb);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(bad_kb->scene_manager, BadKbSceneConfig));

    view_dispatcher_switch_to_view(bad_kb->view_dispatcher, BadKbAppViewVarItemList);
}

bool bad_kb_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadKbApp* bad_kb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(bad_kb->scene_manager, BadKbSceneConfig, event.event);
        consumed = true;
        switch(event.event) {
        case VarItemListIndexKeyboardLayout:
            scene_manager_next_scene(bad_kb->scene_manager, BadKbSceneConfigLayout);
            break;
        case VarItemListIndexConnection:
            bad_kb_config_refresh(bad_kb);
            break;
        default:
            break;
        }
        if(bad_kb->is_bt) {
            switch(event.event) {
            case VarItemListIndexBtRemember:
                bad_kb_config_refresh(bad_kb);
                break;
            case VarItemListIndexBtDeviceName:
                scene_manager_next_scene(bad_kb->scene_manager, BadKbSceneConfigBtName);
                break;
            case VarItemListIndexBtMacAddress:
                scene_manager_next_scene(bad_kb->scene_manager, BadKbSceneConfigBtMac);
                break;
            case VarItemListIndexBtRandomizeMac:
                furi_hal_random_fill_buf(bad_kb->config.bt_mac, BAD_KB_MAC_LEN);
                bad_kb_config_refresh(bad_kb);
                break;
            default:
                break;
            }
        } else {
            switch(event.event) {
            case VarItemListIndexUsbManufacturer:
                scene_manager_set_scene_state(
                    bad_kb->scene_manager, BadKbSceneConfigUsbName, true);
                scene_manager_next_scene(bad_kb->scene_manager, BadKbSceneConfigUsbName);
                break;
            case VarItemListIndexUsbProductName:
                scene_manager_set_scene_state(
                    bad_kb->scene_manager, BadKbSceneConfigUsbName, false);
                scene_manager_next_scene(bad_kb->scene_manager, BadKbSceneConfigUsbName);
                break;
            case VarItemListIndexUsbVidPid:
                scene_manager_next_scene(bad_kb->scene_manager, BadKbSceneConfigUsbVidPid);
                break;
            case VarItemListIndexUsbRandomizeVidPid:
                furi_hal_random_fill_buf(
                    (void*)bad_kb->usb_vidpid_buf, sizeof(bad_kb->usb_vidpid_buf));
                bad_kb->config.usb_cfg.vid = bad_kb->usb_vidpid_buf[0];
                bad_kb->config.usb_cfg.pid = bad_kb->usb_vidpid_buf[1];
                bad_kb_config_refresh(bad_kb);
                break;
            default:
                break;
            }
        }
    }

    return consumed;
}

void bad_kb_scene_config_on_exit(void* context) {
    BadKbApp* bad_kb = context;
    VariableItemList* var_item_list = bad_kb->var_item_list;

    variable_item_list_reset(var_item_list);
}
