#include "usb_hid_keyboard.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <gui/elements.h>
#include <gui/icon_i.h>


struct UsbHidKeyboard {
    View* view;
};

typedef struct {
    bool shift;
    bool alt;
    bool ctrl;
    bool gui;
    uint8_t x;
    uint8_t y;
    uint8_t last_key_code;
    uint16_t modifier_code;
    bool ok_pressed;
    bool back_pressed;
    bool connected;
    char key_string[5];
} UsbHidKeyboardModel;

typedef struct {
    uint8_t width;
    char* key;
    const Icon* icon;
    char* shift_key;
    uint8_t value;
} UsbHidKeyboardKey;

typedef struct {
    int8_t x;
    int8_t y;
} UsbHidKeyboardPoint;

static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 `~!@#$%^&*()-_=+[]{};:\'\",.<>/?";

// 4 BY 12
#define MARGIN_TOP 0
#define MARGIN_LEFT 4
#define KEY_WIDTH 9
#define KEY_HEIGHT 12
#define KEY_PADDING 1
#define ROW_COUNT 2
#define COLUMN_COUNT 12
#define LOWER_LIMIT_STRING_LENGTH 1
#define UPPER_LIMIT_STRING_LENGTH 20


// 0 width items are not drawn, but there value is used
const UsbHidKeyboardKey usb_hid_keyboard_keyset[ROW_COUNT][COLUMN_COUNT] = {
    {
        {.width = 12, .icon = NULL, .key = "Pump!", .shift_key = "", .value = 0},
    },
   
};

bool send_string(const char* param) {
    uint32_t i = 0;
    while (param[i] != '\0') {
        if (param[i] != '\n') {
            uint16_t keycode = HID_ASCII_TO_KEY(param[i]);
            if (keycode != HID_KEYBOARD_NONE) {
                furi_hal_hid_kb_press(keycode);
                furi_hal_hid_kb_release(keycode);
            }
        }
        i++;
    }
    return true;
}

static char* create_rand_string(char* str, size_t size)
{
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int)(sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

static void usb_hid_keyboard_to_upper(char* str) {
    while(*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

static void usb_hid_keyboard_draw_key(
    Canvas* canvas,
    UsbHidKeyboardModel* model,
    uint8_t x,
    uint8_t y,
    UsbHidKeyboardKey key,
    bool selected) {
    if(!key.width) return;

    canvas_set_color(canvas, ColorBlack);
    uint8_t keyWidth = KEY_WIDTH * key.width + KEY_PADDING * (key.width - 1);
    if(selected) {
        // Draw a filled box
        elements_slightly_rounded_box(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING),
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING),
            keyWidth,
            KEY_HEIGHT);
        canvas_set_color(canvas, ColorWhite);
    } else {
        // Draw a framed box
        elements_slightly_rounded_frame(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING),
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING),
            keyWidth,
            KEY_HEIGHT);
    }
    if(key.icon != NULL) {
        // Draw the icon centered on the button
        canvas_draw_icon(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 - key.icon->width / 2,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + KEY_HEIGHT / 2 - key.icon->height / 2,
            key.icon);
    } else {
        // If shift is toggled use the shift key when available
        strcpy(model->key_string, (model->shift && key.shift_key != 0) ? key.shift_key : key.key);
        // Upper case if ctrl or alt was toggled true
        if((model->ctrl && key.value == HID_KEYBOARD_L_CTRL) ||
           (model->alt && key.value == HID_KEYBOARD_L_ALT) ||
           (model->gui && key.value == HID_KEYBOARD_L_GUI)) {
            usb_hid_keyboard_to_upper(model->key_string);
        }
        canvas_draw_str_aligned(
            canvas,
            MARGIN_LEFT + x * (KEY_WIDTH + KEY_PADDING) + keyWidth / 2 + 1,
            MARGIN_TOP + y * (KEY_HEIGHT + KEY_PADDING) + KEY_HEIGHT / 2,
            AlignCenter,
            AlignCenter,
            model->key_string);
    }
}

static void usb_hid_keyboard_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    UsbHidKeyboardModel* model = context;

    canvas_set_font(canvas, FontKeyboard);
    // Start shifting the all keys up if on the next row (Scrolling)
    uint8_t initY = model->y - 4 > 0 ? model->y - 4 : 0;
    for(uint8_t y = initY; y < ROW_COUNT; y++) {
        const UsbHidKeyboardKey* keyboardKeyRow = usb_hid_keyboard_keyset[y];
        uint8_t x = 0;
        for(uint8_t i = 0; i < COLUMN_COUNT; i++) {
            UsbHidKeyboardKey key = keyboardKeyRow[i];
            // Select when the button is hovered
            // Select if the button is hovered within its width
            // Select if back is clicked and its the backspace key
            // Deselect when the button clicked or not hovered
            bool keySelected = (x <= model->x && model->x < (x + key.width)) && y == model->y;
            // Revert selection for function keys
            keySelected = y == ROW_COUNT - 1 ? !keySelected : keySelected;
            bool backSelected = model->back_pressed && key.value == HID_KEYBOARD_DELETE;
            usb_hid_keyboard_draw_key(
                canvas,
                model,
                x,
                y - initY,
                key,
                (!model->ok_pressed && keySelected) || backSelected);
            x += key.width;
        }
    }
}

static void usb_hid_keyboard_process(UsbHidKeyboard* usb_hid_keyboard, InputEvent* event) {
    with_view_model(
        usb_hid_keyboard->view,
        UsbHidKeyboardModel * model,
        {
            int random_length = (rand() % (UPPER_LIMIT_STRING_LENGTH - LOWER_LIMIT_STRING_LENGTH + 1)) + LOWER_LIMIT_STRING_LENGTH;
            char* ptr_rand_string = malloc(random_length);
            send_string(create_rand_string(ptr_rand_string, random_length));
            if (ptr_rand_string != NULL) {
                free(ptr_rand_string);
            }
            
            furi_hal_hid_kb_press(HID_KEYBOARD_RETURN);
            furi_hal_hid_kb_release_all();
            

            if(event->key == InputKeyBack) {
                // If back is pressed for a short time, backspace
                if(event->type == InputTypePress) {
                    model->back_pressed = true; 
                }else if(event->type == InputTypeRelease) {
                    model->back_pressed = false;
                }
            }
        },
        true);
}

static bool usb_hid_keyboard_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    UsbHidKeyboard* usb_hid_keyboard = context;
    bool consumed = false;

    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        furi_hal_hid_kb_release_all();
    } else {
        usb_hid_keyboard_process(usb_hid_keyboard, event);
        consumed = true;
    }

    return consumed;
}

UsbHidKeyboard* usb_hid_keyboard_alloc() {
    UsbHidKeyboard* usb_hid_keyboard = malloc(sizeof(UsbHidKeyboard));

    usb_hid_keyboard->view = view_alloc();
    view_set_context(usb_hid_keyboard->view, usb_hid_keyboard);
    view_allocate_model(usb_hid_keyboard->view, ViewModelTypeLocking, sizeof(UsbHidKeyboardModel));
    view_set_draw_callback(usb_hid_keyboard->view, usb_hid_keyboard_draw_callback);
    view_set_input_callback(usb_hid_keyboard->view, usb_hid_keyboard_input_callback);

    with_view_model(usb_hid_keyboard->view, UsbHidKeyboardModel * model, { model->connected = true; }, true);

    return usb_hid_keyboard;
}

void usb_hid_keyboard_free(UsbHidKeyboard* usb_hid_keyboard) {
    furi_assert(usb_hid_keyboard);
    view_free(usb_hid_keyboard->view);
    free(usb_hid_keyboard);
}

View* usb_hid_keyboard_get_view(UsbHidKeyboard* usb_hid_keyboard) {
    furi_assert(usb_hid_keyboard);
    return usb_hid_keyboard->view;
}
