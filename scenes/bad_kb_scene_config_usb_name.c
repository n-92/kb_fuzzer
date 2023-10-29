#include "../bad_kb_app.h"

static void bad_kb_scene_config_usb_name_text_input_callback(void* context) {
    BadKbApp* bad_kb = context;

    view_dispatcher_send_custom_event(bad_kb->view_dispatcher, BadKbAppCustomEventTextInputDone);
}

void bad_kb_scene_config_usb_name_on_enter(void* context) {
    BadKbApp* bad_kb = context;
    TextInput* text_input = bad_kb->text_input;

    if(scene_manager_get_scene_state(bad_kb->scene_manager, BadKbSceneConfigUsbName)) {
        strlcpy(bad_kb->usb_name_buf, bad_kb->config.usb_cfg.manuf, BAD_KB_USB_LEN);
        text_input_set_header_text(text_input, "Set USB manufacturer name");
    } else {
        strlcpy(bad_kb->usb_name_buf, bad_kb->config.usb_cfg.product, BAD_KB_USB_LEN);
        text_input_set_header_text(text_input, "Set USB product name");
    }

    text_input_set_result_callback(
        text_input,
        bad_kb_scene_config_usb_name_text_input_callback,
        bad_kb,
        bad_kb->usb_name_buf,
        BAD_KB_USB_LEN,
        true);

    view_dispatcher_switch_to_view(bad_kb->view_dispatcher, BadKbAppViewTextInput);
}

bool bad_kb_scene_config_usb_name_on_event(void* context, SceneManagerEvent event) {
    BadKbApp* bad_kb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == BadKbAppCustomEventTextInputDone) {
            if(scene_manager_get_scene_state(bad_kb->scene_manager, BadKbSceneConfigUsbName)) {
                strlcpy(bad_kb->config.usb_cfg.manuf, bad_kb->usb_name_buf, BAD_KB_USB_LEN);
            } else {
                strlcpy(bad_kb->config.usb_cfg.product, bad_kb->usb_name_buf, BAD_KB_USB_LEN);
            }
            bad_kb_config_refresh(bad_kb);
        }
        scene_manager_previous_scene(bad_kb->scene_manager);
    }
    return consumed;
}

void bad_kb_scene_config_usb_name_on_exit(void* context) {
    BadKbApp* bad_kb = context;
    TextInput* text_input = bad_kb->text_input;

    text_input_reset(text_input);
}
