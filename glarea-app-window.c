#include "glarea-app-window.h"
#include <epoxy/gl.h>
#include <math.h>

enum { X_AXIS, Y_AXIS, Z_AXIS, N_AXES };

struct _GlareaAppWindow
{
  GtkApplicationWindow parent_instance;

  GtkAdjustment *x_adjustment;
  GtkAdjustment *y_adjustment;
  GtkAdjustment *z_adjustment;

  GtkWidget *gl_drawing_area;
  GtkWidget *quit_button;

  double rotation_angles[N_AXES];
  float mvp[16];

  guint vao;
  guint vertex_buffer;
  guint program;
  guint mvp_location;
  guint position_index;
  guint color_index;
};

struct _GlareaAppWindowClass
{
  GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE (GlareaAppWindow, glarea_app_window, GTK_TYPE_APPLICATION_WINDOW)

/* the vertex data is constant */
static const float vertex_data[] = {
  /* vertices */
   0.f,   0.5f,   0.f,
   0.5f, -0.366f, 0.f,
  -0.5f, -0.366f, 0.f,

  /* colors */
  1.f, 0.f, 0.f,
  0.f, 1.f, 0.f,
  0.f, 0.f, 1.f,
};

static void
init_buffers (guint *vao_out,
              guint *vertex_buffer_out)
{
  guint vao, buffer;

  /* we need to create a VAO to store the other buffers */
  glGenVertexArrays (1, &vao);
  glBindVertexArray (vao);

  /* this is the buffer that holds the vertices */
  glGenBuffers (1, &buffer);
  glBindBuffer (GL_ARRAY_BUFFER, buffer);
  glBufferData (GL_ARRAY_BUFFER, sizeof (vertex_data), vertex_data, GL_STATIC_DRAW);

  /* reset the state */
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindVertexArray (0);

  if (vao_out != NULL)
    *vao_out = vao;

  if (vertex_buffer_out != NULL)
    *vertex_buffer_out = buffer;
}

static guint
create_shader (int         shader_type,
               const char *source)
{
  guint shader = glCreateShader (shader_type);
  glShaderSource (shader, 1, &source, NULL);
  glCompileShader (shader);

  int status;
  glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len;
      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_len);

      char *buffer = g_malloc (log_len + 1);
      glGetShaderInfoLog (shader, log_len, NULL, buffer);

      g_warning ("Compilation failure in %s shader:\n%s\n",
                 shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                 buffer);

      g_free (buffer);

      glDeleteShader (shader);

      return 0;
    }

  return shader;
}

static void
init_shaders (guint *program_out,
              guint *mvp_location_out,
              guint *position_location_out,
              guint *color_location_out)
{
  GBytes *source;
  guint program = 0;
  guint mvp_location = 0;
  guint vertex = 0, fragment = 0;
  guint position_location = 0;
  guint color_location = 0;

  /* load the vertex shader */
  source = g_resources_lookup_data ("/io/bassi/glarea/glarea-vertex.glsl", 0, NULL);
  vertex = create_shader (GL_VERTEX_SHADER, g_bytes_get_data (source, NULL));
  g_bytes_unref (source);
  if (vertex == 0)
    goto out;

  /* load the fragment shader */
  source = g_resources_lookup_data ("/io/bassi/glarea/glarea-fragment.glsl", 0, NULL);
  fragment = create_shader (GL_FRAGMENT_SHADER, g_bytes_get_data (source, NULL));
  g_bytes_unref (source);
  if (fragment == 0)
    goto out;

  /* link the vertex and fragment shaders together */
  program = glCreateProgram ();
  glAttachShader (program, vertex);
  glAttachShader (program, fragment);
  glLinkProgram (program);

  int status = 0;
  glGetProgramiv (program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len = 0;
      glGetProgramiv (program, GL_INFO_LOG_LENGTH, &log_len);

      char * buffer = g_malloc (log_len + 1);
      glGetProgramInfoLog (program, log_len, NULL, buffer);

      g_warning ("Linking failure:\n%s\n", buffer);

      g_free (buffer);

      glDeleteProgram (program);
      program = 0;
      mvp_location = 0;

      goto out;
    }

  /* get the location of the "mvp" uniform */
  mvp_location = glGetUniformLocation (program, "mvp");

  /* get the location of the "position" and "color" attributes */
  position_location = glGetAttribLocation (program, "position");
  color_location = glGetAttribLocation (program, "color");

  /* the individual shaders can be detached and destroyed */
  glDetachShader (program, vertex);
  glDetachShader (program, fragment);

out:
  if (vertex != 0)
    glDeleteShader (vertex);
  if (fragment != 0)
    glDeleteShader (fragment);

  if (program_out != NULL)
    *program_out = program;
  if (mvp_location_out != NULL)
    *mvp_location_out = mvp_location;
  if (position_location_out != NULL)
    *position_location_out = position_location;
  if (color_location_out != NULL)
    *color_location_out = color_location;
}

static void
gl_init (GlareaAppWindow *self)
{
  /* we need to ensure that the GdkGLContext is set before calling GL API */
  gtk_gl_area_make_current (GTK_GL_AREA (self->gl_drawing_area));

  /* initialize the vertex buffers */
  init_buffers (&self->vao, &self->vertex_buffer);

  /* initialize the shaders and retrieve the program data */
  init_shaders (&self->program,
                &self->mvp_location,
                &self->position_index,
                &self->color_index);
}

static void
gl_fini (GlareaAppWindow *self)
{
  gtk_gl_area_make_current (GTK_GL_AREA (self->gl_drawing_area));

  /* destroy all the resources we created */
  glDeleteBuffers (1, &self->vertex_buffer);
  glDeleteVertexArrays (1, &self->vao);
  glDeleteProgram (self->program);
}

static void
draw_triangle (GlareaAppWindow *self)
{
  /* load our program */
  glUseProgram (self->program);

  /* update the "mvp" matrix we use in the shader */
  glUniformMatrix4fv (self->mvp_location, 1, GL_FALSE, &(self->mvp[0]));

  /* use the vertices in our buffer */
  glBindVertexArray (self->vao);
  glBindBuffer (GL_ARRAY_BUFFER, self->vertex_buffer);

  glEnableVertexAttribArray (self->position_index);
  glEnableVertexAttribArray (self->color_index);

  /* set the position attribute */
  glVertexAttribPointer (self->position_index, 3, GL_FLOAT, GL_FALSE, 0, 0);

  /* set the color attribute */
  glVertexAttribPointer (self->color_index, 3, GL_FLOAT, GL_FALSE, 0, (char *) 0 + (sizeof (float) * 3 * 3));

  /* draw the three vertices as a triangle */
  glDrawArrays (GL_TRIANGLES, 0, 3);

  /* we finished using the buffers and program */
  glDisableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindVertexArray (0);
  glUseProgram (0);
}

static gboolean
gl_draw (GlareaAppWindow *self)
{
  /* clear the viewport; the viewport is automatically resized when
   * the GtkGLArea gets an allocation
   */
  glClearColor (0.5, 0.5, 0.5, 1.0);
  glClear (GL_COLOR_BUFFER_BIT);

  /* draw our object */
  draw_triangle (self);

  /* flush the contents of the pipeline */
  glFlush ();

  return FALSE;
}

static void
init_identity (float *res)
{
  /* initialize a matrix as an identity matrix */
  res[0] = 1.f; res[4] = 0.f;  res[8] = 0.f; res[12] = 0.f;
  res[1] = 0.f; res[5] = 1.f;  res[9] = 0.f; res[13] = 0.f;
  res[2] = 0.f; res[6] = 0.f; res[10] = 1.f; res[14] = 0.f;
  res[3] = 0.f; res[7] = 0.f; res[11] = 0.f; res[15] = 1.f;
}

static void
compute_mvp (float *res,
             float  phi,
             float  theta,
             float  psi)
{
  float x = phi * (G_PI / 180.f);
  float y = theta * (G_PI / 180.f);
  float z = psi * (G_PI / 180.f);
  float c1 = cosf (x), s1 = sinf (x);
  float c2 = cosf (y), s2 = sinf (y);
  float c3 = cosf (z), s3 = sinf (z);
  float c3c2 = c3 * c2;
  float s3c1 = s3 * c1;
  float c3s2s1 = c3 * s2 * s1;
  float s3s1 = s3 * s1;
  float c3s2c1 = c3 * s2 * c1;
  float s3c2 = s3 * c2;
  float c3c1 = c3 * c1;
  float s3s2s1 = s3 * s2 * s1;
  float c3s1 = c3 * s1;
  float s3s2c1 = s3 * s2 * c1;
  float c2s1 = c2 * s1;
  float c2c1 = c2 * c1;
  
  /* apply all three rotations using the three matrices:
   *
   * ⎡  c3 s3 0 ⎤ ⎡ c2  0 -s2 ⎤ ⎡ 1   0  0 ⎤
   * ⎢ -s3 c3 0 ⎥ ⎢  0  1   0 ⎥ ⎢ 0  c1 s1 ⎥
   * ⎣   0  0 1 ⎦ ⎣ s2  0  c2 ⎦ ⎣ 0 -s1 c1 ⎦
   */
  res[0] = c3c2;  res[4] = s3c1 + c3s2s1;  res[8] = s3s1 - c3s2c1; res[12] = 0.f;
  res[1] = -s3c2; res[5] = c3c1 - s3s2s1;  res[9] = c3s1 + s3s2c1; res[13] = 0.f;
  res[2] = s2;    res[6] = -c2s1;         res[10] = c2c1;          res[14] = 0.f;
  res[3] = 0.f;   res[7] = 0.f;           res[11] = 0.f;           res[15] = 1.f;
}

static void
adjustment_changed (GlareaAppWindow *self,
                    GtkAdjustment   *adj)
{
  double value = gtk_adjustment_get_value (adj);

  /* update the rotation angles */
  if (adj == self->x_adjustment)
    self->rotation_angles[X_AXIS] = value;

  if (adj == self->y_adjustment)
    self->rotation_angles[Y_AXIS] = value;

  if (adj == self->z_adjustment)
    self->rotation_angles[Z_AXIS] = value;

  /* recompute the mvp matrix */
  compute_mvp (self->mvp,
               self->rotation_angles[X_AXIS],
               self->rotation_angles[Y_AXIS],
               self->rotation_angles[Z_AXIS]);

  /* queue a redraw on the GtkGLArea */
  gtk_widget_queue_draw (self->gl_drawing_area);
}

static void
glarea_app_window_class_init (GlareaAppWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/io/bassi/glarea/glarea-app-window.ui");

  gtk_widget_class_bind_template_child (widget_class, GlareaAppWindow, gl_drawing_area);
  gtk_widget_class_bind_template_child (widget_class, GlareaAppWindow, quit_button);
  gtk_widget_class_bind_template_child (widget_class, GlareaAppWindow, x_adjustment);
  gtk_widget_class_bind_template_child (widget_class, GlareaAppWindow, y_adjustment);
  gtk_widget_class_bind_template_child (widget_class, GlareaAppWindow, z_adjustment);

  gtk_widget_class_bind_template_callback (widget_class, adjustment_changed);
  gtk_widget_class_bind_template_callback (widget_class, gl_init);
  gtk_widget_class_bind_template_callback (widget_class, gl_draw);
  gtk_widget_class_bind_template_callback (widget_class, gl_fini);
}

static void
glarea_app_window_init (GlareaAppWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  /* reset the mvp matrix */
  init_identity (self->mvp);
}

GtkWidget *
glarea_app_window_new (GlareaApp *app)
{
  return g_object_new (glarea_app_window_get_type (), "application", app, NULL);
}
