#include <glib.h>
#include "usb.h"

static void test_parse_valid_line(void) {
    UsbDevice usb = {0};
    const char *line = "1-7    22d4:1803  USB Input Device                                              Not shared";

    bool result = parse_usb_line(line, &usb);

    g_assert_true(result);
    g_assert_cmpstr(usb.bus_id, ==, "1-7");
    g_assert_cmpstr(usb.name, ==, "USB Input Device");
    g_assert_cmpstr(usb.status, ==, "Not shared");
}

static void test_parse_attached_device(void) {
    UsbDevice usb = {0};
    const char *line = "1-7    22d4:1803  USB Input Device                                              Attached";

    bool result = parse_usb_line(line, &usb);

    g_assert_true(result);
    g_assert_cmpstr(usb.status, ==, "Attached");
}

static void test_parse_invalid_header(void) {
    UsbDevice usb = {0};

    const char *line = "BUSID  DEVICE                           STATE";

    bool result = parse_usb_line(line, &usb);

    g_assert_false(result);
}

static void test_parse_malformed_line(void) {
    UsbDevice usb = {0};
    const char *line = "9-9";

    bool result = parse_usb_line(line, &usb);

    g_assert_false(result);
}

int main(int argc, char *argv[]) {
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/usb/parse_valid_line", test_parse_valid_line);
    g_test_add_func("/usb/parse_attached", test_parse_attached_device);
    g_test_add_func("/usb/parse_invalid_header", test_parse_invalid_header);
    g_test_add_func("/usb/parse_malformed", test_parse_malformed_line);

    return g_test_run();
}