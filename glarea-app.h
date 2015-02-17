#ifndef __GLAREA_APP_H__
#define __GLAREA_APP_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GLAREA_TYPE_APP (glarea_app_get_type ())

G_DECLARE_FINAL_TYPE (GlareaApp, glarea_app, GLAREA, APP, GtkApplication)

GtkApplication *glarea_app_new (void);

G_END_DECLS

#endif /* __GLAREA_APP_H__ */
