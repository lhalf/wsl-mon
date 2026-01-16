#include "ui.h"
#include "usb.h"

#define TITLE "WSL Monitor"
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 400
#define BUTTON_SPACING 10
#define MARGIN 20

void ui_activate(GtkApplication *app, gpointer user_data) {
  (void)user_data;

  GtkWidget *window = gtk_application_window_new(app);

  gtk_window_set_title(GTK_WINDOW(window), TITLE);
  gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);

  GtkWidget *scrolled = gtk_scrolled_window_new();

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, BUTTON_SPACING);
  gtk_widget_set_margin_start(vbox, MARGIN);
  gtk_widget_set_margin_end(vbox, MARGIN);
  gtk_widget_set_margin_top(vbox, MARGIN);
  gtk_widget_set_margin_bottom(vbox, MARGIN);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), vbox);
  gtk_window_set_child(GTK_WINDOW(window), scrolled);

  usb_populate_buttons(GTK_BOX(vbox));

  gtk_window_present(GTK_WINDOW(window));
}
