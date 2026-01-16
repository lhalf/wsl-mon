#include "usb.h"
#include <stdio.h>
#include <string.h>

#define BUTTON_DELAY_MS 500

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

typedef struct {
  char *bus_id;
  GtkGrid *grid;
} UsbCallbackData;

static bool execute_usb_action(const char *bus_id, const UsbAction action) {
  char command[256];
  const char *base_cmd = (action == ATTACH) ? ATTACH_COMMAND : DETACH_COMMAND;

  snprintf(command, sizeof(command), "%s %s 2>&1", base_cmd, bus_id);
  FILE *pipe = popen(command, "r");

  if (!pipe)
    return false;

  int status = pclose(pipe);

  return (WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static gboolean refresh_ui_callback(gpointer user_data) {
  GtkGrid *grid = GTK_GRID(user_data);

  GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(grid));
  while (child != NULL) {
    GtkWidget *next = gtk_widget_get_next_sibling(child);
    gtk_grid_remove(grid, child);
    child = next;
  }

  usb_populate_grid(grid);

  return G_SOURCE_REMOVE;
}

static void on_usb_toggle_clicked(GtkButton *button, gpointer user_data) {
  const UsbCallbackData *data = user_data;
  const char *current_label = gtk_button_get_label(button);

  gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

  UsbAction action =
      (g_strcmp0(current_label, "Attach") == 0) ? ATTACH : DETACH;

  if (execute_usb_action(data->bus_id, action)) {
    g_timeout_add(BUTTON_DELAY_MS, refresh_ui_callback, data->grid);
  } else {
    gtk_widget_set_sensitive(GTK_WIDGET(button), TRUE);
  }
}

static void free_callback_data(gpointer data, GClosure *closure) {
  (void)closure;
  UsbCallbackData *d = data;
  g_free(d->bus_id);
  g_free(d);
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

  GtkWidget *button =
      gtk_button_new_with_label(is_attached ? "Detach" : "Attach");
  gtk_widget_set_sensitive(button, is_shared);

  UsbCallbackData *data = g_new0(UsbCallbackData, 1);
  data->bus_id = g_strdup(bus_id);
  data->grid = grid;

  g_signal_connect_data(button, "clicked", G_CALLBACK(on_usb_toggle_clicked),
                        data, free_callback_data, 0);

  gtk_grid_attach(GTK_GRID(grid), bus_label, 0, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), name_label, 1, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), status_label, 2, row, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button, 3, row, 1, 1);
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
