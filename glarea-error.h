#ifndef __GLAREA_ERROR_H__
#define __GLAREA_ERROR_H__

#include <glib.h>

G_BEGIN_DECLS

#define GLAREA_ERROR (glarea_error_quark ())

typedef enum {
  GLAREA_ERROR_SHADER_COMPILATION,
  GLAREA_ERROR_SHADER_LINK
} GlareaError;

GQuark glarea_error_quark (void);

G_END_DECLS

#endif /* __GLAREA_ERROR_H__ */
