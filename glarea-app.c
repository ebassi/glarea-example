#include "glarea-app.h"
#include "glarea-app-window.h"

struct _GlareaApp
{
  GtkApplication parent_instance;

  GtkWidget *window;
};

struct _GlareaAppClass
{
  GtkApplicationClass parent_class;
};

G_DEFINE_TYPE (GlareaApp, glarea_app, GTK_TYPE_APPLICATION)

static void
quit_activated (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       app)
{
  g_application_quit (G_APPLICATION (app));
}

static GActionEntry app_entries[] =
{
  { "quit", quit_activated, NULL, NULL, NULL }
};

static void
glarea_app_startup (GApplication *app)
{
  GtkBuilder *builder;
  GMenuModel *app_menu;

  G_APPLICATION_CLASS (glarea_app_parent_class)->startup (app);

  g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);

  builder = gtk_builder_new_from_resource ("/io/bassi/glarea/glarea-app-menu.ui");
  app_menu = G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
  gtk_application_set_app_menu (GTK_APPLICATION (app), app_menu);
  g_object_unref (builder);
}

static void
glarea_app_activate (GApplication *app)
{
  GlareaApp *self = GLAREA_APP (app);

  if (self->window == NULL)
    self->window = glarea_app_window_new (GLAREA_APP (app));

  gtk_window_present (GTK_WINDOW (self->window));
}


static void
glarea_app_class_init (GlareaAppClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->startup = glarea_app_startup;
  app_class->activate = glarea_app_activate;
}

static void
glarea_app_init (GlareaApp *self)
{
}

GtkApplication *
glarea_app_new (void)
{
  return g_object_new (glarea_app_get_type (), "application-id", "io.bassi.Glarea", NULL);
}
