#include "../bad_kb_app.h"

static void
    bad_kb_scene_error_event_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    BadKbApp* app = context;

    if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, BadKbCustomEventErrorBack);
    }
}

void bad_kb_scene_error_on_enter(void* context) {
    BadKbApp* app = context;

    if(app->error == BadKbAppErrorNoFiles) {
        widget_add_icon_element(app->widget, 0, 0, &I_SDQuestion_35x43);
        widget_add_string_multiline_element(
            app->widget,
            81,
            4,
            AlignCenter,
            AlignTop,
            FontSecondary,
            "No SD card or\napp data found.\nThis app will not\nwork without\nrequired files.");
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Back", bad_kb_scene_error_event_callback, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, BadKbAppViewWidget);
}

bool bad_kb_scene_error_on_event(void* context, SceneManagerEvent event) {
    BadKbApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BadKbCustomEventErrorBack) {
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
        }
    }
    return consumed;
}

void bad_kb_scene_error_on_exit(void* context) {
    BadKbApp* app = context;
    widget_reset(app->widget);
}
