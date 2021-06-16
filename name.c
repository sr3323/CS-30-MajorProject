/*
 * This file sets the name of the device
 * so that it appears as "Foot Controller" in list of devices.
 */

#include "usb_names.h"

// Edit these lines to create your own name.  The length must
// match the number of characters in your custom name.

#define MIDI_NAME   {'F','o','o','t',' ','C','o','n','t','r','o','l','l','e','r'}
#define MIDI_NAME_LEN  15

// Do not change this part.  This exact format is required by USB.

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + MIDI_NAME_LEN * 2,
        3,
        MIDI_NAME
};
