#ifndef WSL_MON_USB_H
#define WSL_MON_USB_H

#include <gtk/gtk.h>

typedef struct {
  char bus_id[16];
  char name[256];
  char status[64];
} UsbDevice;

void usb_populate_grid(GtkGrid *grid);
bool parse_usb_line(const char *line, UsbDevice *usb);

#endif // WSL_MON_USB_H