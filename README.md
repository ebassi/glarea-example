## Using GtkGLArea

A simple example of how to use OpenGL and the GtkGLArea widget available
since GTK+ 3.16.

See also: https://www.bassi.io/articles/2015/02/17/using-opengl-with-gtk/ 

## Building

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

## Copyright and License

Copyright 2015  Emmanuele Bassi

Released under the terms of the CC0. See the `LICENSE` file for more details.
