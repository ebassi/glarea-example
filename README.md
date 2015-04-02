## Using GtkGLArea

A simple example of how to use OpenGL and the GtkGLArea widget available
since GTK+ 3.16.

See also: https://www.bassi.io/articles/2015/02/17/using-opengl-with-gtk/ 

## Building and running

You will need GTK+ 3.16 or later to build this example.

Clone the repository, as usual:

    $ git clone https://github.com/ebassi/glarea-example

Then run `make` inside the cloned repository:

    $ cd glarea-example
    $ make

Finally, run the example:

    $ ./glarea

If everything worked as it should, you should see this:

![glarea](/glarea-example.png)

Use the range widgets to control the rotation of the triangle, and feel
free to play around with the source.

## What goes where

This is a simple example, but it's been broken down in a way that allows
easy reuse of the parts.

  * `glarea-app.[ch]` - This is the main application singleton; just a
    thin wrapper around GtkApplication which creates an application
    window of the `GlareaAppWindow` class on activation, unless one is
    already present, in which case it just brings up into focus the
    existing instance
  * `glarea-app-menu.ui` - The `GMenu` XML description of the application
    menu, loaded by the application singleton instance
  * `glarea-app-window.[ch]` - This is the main application window, and
    where the real magic happens
  * `glarea-app-window.ui` - The `GtkBuilder` XML description of the
    `GlareaAppWindow` class
  * `glarea-error.[ch]` - A GError error domain, for our internal use
  * `glarea-fragment.glsl` - The GLSL fragment shader source, which we
    embed into the executable binary by way of `GResource`
  * `glarea-vertex.glsl` - The GLSL vertex shader source, which we
    embed into the executable binary by way of `GResource`
  * `glarea.gresource.xml` - The list of resources we want to embed
    into the executable binary
  * `main.c` - The main entry point, which creates the application
    singletong and spins the main loop

The whole application logic can be broken down into roughly four separate
parts:

  1. Initialization
  2. State updates
  3. Drawing
  4. Deinitialization

### Initialization

We use the `GtkWidget::realize` signal to know when the `GtkGLArea` widget
has created the windowing system resources associated with the GL context;
in order to use the signal, we connect the `gl_init` function to it inside
the UI description.

The basic step is to make the GL context current, so we can use the GL API:

```C
  gtk_gl_area_make_current (GTK_GL_AREA (self->gl_drawing_area));
  if (gtk_gl_area_get_error (GTK_GL_AREA (self->gl_drawing_area)) != NULL)
    return;
```

We check if the `GtkGLArea` widget is in an error state to decide whether
to bail out and not use GL API on an invalid context.

After that, we initialize our vertex and fragment shaders, as well as the
buffer objects.

### State updates

Every time the user changes the value of the three `GtkRange` widgets, the
corresponding `GtkAdjustment` is also updated; we catch the value change
in the `adjustment_changed` signal handler, which we connected using the
UI description.

The `adjustment_changed` signal recomputes the modelview-projection matrix
using a rotation transformation, and calls `gtk_widget_queue_draw()` on
the `GtkGLArea` widget, signalling that the contents of the widget should
be updated, i.e. redrawn.

### Drawing

The `gl_draw` function is connected to the `GtkGLArea::render` signal by
way of the UI description.

We do not need to call `gtk_gl_area_make_current()`, as the context is
made current before emitting the signal; we also do not need to check for
an error state, because the signal will not be emitted in that case.

We load the various GL objects we created inside `gl_init()`, like the
vertex array object and the program; we upload the `mvp` matrix to the
location of the `mvp` uniform in the vertex shader; and we call the
`glDrawArrays()` function to draw the vertices of the triangle. At the
end of the process, we reset the state.

### Deinitialization

We use the `GtkWidget::unrealize` signal to release the resources we
allocated inside the `gl_init()` function. As with the initialization
process, we need to check if the `GtkGLArea` widget is in an error
state after making the GL context current.

## Copyright and License

Copyright 2015  Emmanuele Bassi

Released under the terms of the CC0. See the `LICENSE` file for more details.
