#include "usb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_COMMAND "usbipd.exe list 2>/dev/null"
#define ATTACH_COMMAND "usbipd.exe attach --wsl --busid"

static void free_bus_id(gpointer data, GClosure *closure) {
  (void)closure;
  g_free(data);
}

static void on_usb_button_clicked(GtkButton *button, gpointer user_data) {
  (void)button;

  char command[128];
  char *bus_id = (char *)user_data;

  g_print("Attaching USB device with bus ID: %s ...\n", bus_id);

  snprintf(command, sizeof(command), "%s %s", ATTACH_COMMAND, bus_id);

  int result = system(command);
  (void)result;
}

static bool parse_usb_line(char *line, char *bus_id, char *device_name) {
  if (!strstr(line, "Not shared") && !strstr(line, "Shared")) {
    return false;
  }

  char vid_pid[16];

  if (sscanf(line, "%15s %15s %127[^\n]", bus_id, vid_pid, device_name) >= 3) {
    char *status_pos = strstr(device_name, "  ");
    if (status_pos)
      *status_pos = '\0';
    return true;
  }

  return false;
}

static void add_usb_button(GtkBox *vbox, const char *bus_id, const char *name) {
  char label[256];
  snprintf(label, sizeof(label), "Attach: %s (%s)", name, bus_id);

  GtkWidget *button = gtk_button_new_with_label(label);

  g_signal_connect_data(button, "clicked", G_CALLBACK(on_usb_button_clicked),
                        g_strdup(bus_id), free_bus_id, 0);

  gtk_box_append(vbox, button);
}

void usb_populate_buttons(GtkBox *vbox) {
  FILE *file = popen(LIST_COMMAND, "r");

  if (!file) {
    gtk_box_append(vbox, gtk_label_new("usbipd not found."));
    return;
  }

  char line[256];
  char bus_id[16];
  char device_name[128];

  while (fgets(line, sizeof(line), file)) {
    if (parse_usb_line(line, bus_id, device_name)) {
      add_usb_button(vbox, bus_id, device_name);
    }
  }

  pclose(file);
}