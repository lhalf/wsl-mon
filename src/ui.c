#include "ui.h"
#include "usb.h"

#define TITLE "WSL Monitor"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 400

#define GRID_ROW_SPACING 10
#define GRID_COLUMN_SPACING 10
#define GRID_MARGIN 20

void ui_activate(GtkApplication *app, gpointer user_data) {
  (void)user_data;

  GtkWidget *window = gtk_application_window_new(app);

  gtk_window_set_title(GTK_WINDOW(window), TITLE);
  gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_window_present(GTK_WINDOW(window));

  GtkWidget *scrolled = gtk_scrolled_window_new();

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_window_set_child(GTK_WINDOW(window), scrolled);

  GtkWidget *grid = gtk_grid_new();

  gtk_grid_set_row_spacing(GTK_GRID(grid), GRID_ROW_SPACING);
  gtk_grid_set_column_spacing(GTK_GRID(grid), GRID_COLUMN_SPACING);
  gtk_widget_set_margin_start(grid, GRID_MARGIN);
  gtk_widget_set_margin_end(grid, GRID_MARGIN);
  gtk_widget_set_margin_top(grid, GRID_MARGIN);
  gtk_widget_set_margin_bottom(grid, GRID_MARGIN);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), grid);

  usb_populate_grid(GTK_GRID(grid));
}
