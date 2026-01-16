#include "ui.h"
#include <gtk/gtk.h>

int main(int argc, char **argv) {
  GtkApplication *app =
      gtk_application_new("org.wsl.mon", G_APPLICATION_FLAGS_NONE);

  g_signal_connect(app, "activate", G_CALLBACK(ui_activate), NULL);

  const int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}