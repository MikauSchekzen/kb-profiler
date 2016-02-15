#include "interception.h"
#include "src/utils.h"

enum ScanCode
{
    SCANCODE_CAPSLOCK   = 0x3A,
    SCANCODE_LSHIFT = 0x2A,
    SCANCODE_F1   = 0x3B,
    SCANCODE_SCROLLLOCK = 0x46
};

int main()
{
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke stroke;

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP);

    while(interception_receive(context, device = interception_wait(context), (InterceptionStroke *)&stroke, 1) > 0)
    {
        if(stroke.code == SCANCODE_CAPSLOCK) {
          stroke.code = SCANCODE_SCROLLLOCK;
        }

        interception_send(context, device, (InterceptionStroke *)&stroke, 1);
    }

    interception_destroy_context(context);

    return 0;
}
