#include "usb.h"
#include <stdio.h>
#include <string.h>

#define BUTTON_WIDTH 80
#define STATUS_WIDTH 100

#define GRID_SPACING 5
#define GRID_MARGIN 5

#define LIST_COMMAND "usbipd.exe list 2>/dev/null"
#define ATTACH_COMMAND "usbipd.exe attach --wsl --busid"
#define DETACH_COMMAND "usbipd.exe detach --busid"

typedef enum { ATTACH, DETACH } UsbAction;

typedef struct {
  char bus_id[16];
  char name[256];
  char status[64];
} UsbDevice;

static bool execute_usb_action(const char *bus_id, const UsbAction action) {
  char command[256];
  const char *base_cmd = action == ATTACH ? ATTACH_COMMAND : DETACH_COMMAND;

  g_debug("%s", base_cmd);

  snprintf(command, sizeof(command), "%s %s 2>&1", base_cmd, bus_id);

  FILE *pipe = popen(command, "r");

  if (!pipe) {
    g_printerr("usbipd not found.");
    return false;
  }

  g_debug("success");

  return pclose(pipe);
}

static void on_usb_toggle_clicked(GtkButton *button, gpointer user_data) {
  char *bus_id = (char *)user_data;
  const char *current_label = gtk_button_get_label(button);

  if (g_strcmp0(current_label, "Attach") == 0) {
    if (execute_usb_action(bus_id, ATTACH)) {
      gtk_button_set_label(button, "Detach");
    }
  } else {
    if (execute_usb_action(bus_id, DETACH)) {
      gtk_button_set_label(button, "Attach");
    }
  }

  gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
}

static void free_bus_id(gpointer data, GClosure *closure) {
  (void)closure;
  g_free(data);
}

static void add_usb_device_row(GtkGrid *grid, const int row, const char *bus_id,
                               const char *name, const char *status) {
  GtkWidget *bus_label = gtk_label_new(bus_id);
  gtk_label_set_xalign(GTK_LABEL(bus_label), 0);
  gtk_widget_set_hexpand(bus_label, TRUE);

  GtkWidget *name_label = gtk_label_new(name);
  gtk_label_set_xalign(GTK_LABEL(name_label), 0);
  gtk_widget_set_hexpand(name_label, TRUE);

  GtkWidget *status_label = gtk_label_new(status);
  gtk_label_set_xalign(GTK_LABEL(status_label), 0);
  gtk_widget_set_size_request(status_label, STATUS_WIDTH, -1);

  bool is_shared =
      (strstr(status, "Shared") != NULL || strstr(status, "Attached") != NULL);
  bool is_attached = (strstr(status, "Attached") != NULL);
  GtkWidget *btn = gtk_button_new_with_label(is_attached ? "Detach" : "Attach");
  gtk_widget_set_sensitive(btn, is_shared);
  gtk_widget_set_size_request(btn, BUTTON_WIDTH, -1);

  if (!is_shared) {
    gtk_widget_set_tooltip_text(btn, "Device must be bound in Windows first.");
  }

  g_signal_connect_data(btn, "clicked", G_CALLBACK(on_usb_toggle_clicked),
                        g_strdup(bus_id), free_bus_id, 0);

  gtk_grid_attach(GTK_GRID(grid), bus_label, 0, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), name_label, 1, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), status_label, 2, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn, 3, row, 1, 1);
}

static bool parse_usb_line(const char *line, UsbDevice *out_dev) {
  if (!g_ascii_isdigit(line[0]))
    return false;

  char *buf = g_strdup(line);
  g_strchomp(buf);

  char *status_start = g_strrstr(buf, "  ");
  if (!status_start) {
    g_free(buf);
    return false;
  }
  strncpy(out_dev->status, g_strstrip(status_start),
          sizeof(out_dev->status) - 1);
  *status_start = '\0';

  char *first_space = strchr(buf, ' ');
  if (!first_space) {
    g_free(buf);
    return false;
  }
  *first_space = '\0';
  strncpy(out_dev->bus_id, buf, sizeof(out_dev->bus_id) - 1);

  char *p = first_space + 1;
  while (*p == ' ')
    p++;

  char *name_start = strchr(p, ' ');
  if (!name_start) {
    g_free(buf);
    return false;
  }

  strncpy(out_dev->name, g_strstrip(name_start), sizeof(out_dev->name) - 1);

  g_free(buf);
  return true;
}

void usb_populate_grid(GtkGrid *grid) {
  FILE *pipe = popen(LIST_COMMAND, "r");

  if (!pipe) {
    g_printerr("usbipd not found.");
    return;
  }

  char line[512];
  int row = 0;
  UsbDevice usb;

  while (fgets(line, sizeof(line), pipe)) {
    if (parse_usb_line(line, &usb)) {
      add_usb_device_row(grid, row, usb.bus_id, usb.name, usb.status);
      row++;
    }
  }

  pclose(pipe);
}
