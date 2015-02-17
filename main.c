#include <gtk/gtk.h>

#include "glarea-app.h"

int
main (int argc, char *argv[])
{
  return g_application_run (G_APPLICATION (glarea_app_new ()), argc, argv);
}
