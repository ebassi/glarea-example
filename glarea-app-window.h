#ifndef __GLAREA_APP_WINDOW_H__
#define __GLAREA_APP_WINDOW_H__

#include <gtk/gtk.h>
#include "glarea-app.h"

G_BEGIN_DECLS

#define GLAREA_TYPE_APP_WINDOW (glarea_app_window_get_type ())

G_DECLARE_FINAL_TYPE (GlareaAppWindow, glarea_app_window, GLAREA, APP_WINDOW, GtkApplicationWindow)

GtkWidget *glarea_app_window_new (GlareaApp *app);

G_END_DECLS

#endif /* __GLAREA_APP_WINDOW_H__ */
