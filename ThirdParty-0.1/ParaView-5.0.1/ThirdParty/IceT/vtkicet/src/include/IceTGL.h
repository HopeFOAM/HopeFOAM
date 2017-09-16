/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTGL_h
#define __IceTGL_h

#include <IceT.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

ICET_GL_EXPORT void icetGLInitialize(void);

ICET_GL_EXPORT IceTBoolean icetGLIsInitialized(void);

ICET_GL_EXPORT void icetGLSetReadBuffer(GLenum mode);

ICET_GL_EXPORT IceTImage icetGLDrawFrame(void);

typedef void (*IceTGLDrawCallbackType)(void);

ICET_GL_EXPORT void icetGLDrawCallback(IceTGLDrawCallbackType callback);

#define ICET_GL_STATE_START ICET_RENDER_LAYER_STATE_START
#define ICET_GL_STATE_END   ICET_RENDER_LAYER_STATE_END

#define ICET_GL_INITIALIZED     (ICET_GL_STATE_START | (IceTEnum)0x0001)

#define ICET_GL_READ_BUFFER     (ICET_GL_STATE_START | (IceTEnum)0x0010)

#define ICET_GL_DRAW_FUNCTION   (ICET_GL_STATE_START | (IceTEnum)0x0020)
#define ICET_GL_INFLATE_TEXTURE (ICET_GL_STATE_START | (IceTEnum)0x0021)

#define ICET_GL_STATE_ENABLE_START ICET_RENDER_LAYER_ENABLE_START
#define ICET_GL_STATE_ENABLE_END   ICET_RENDER_LAYER_ENABLE_END

#define ICET_GL_DISPLAY         (ICET_GL_STATE_ENABLE_START | (IceTEnum)0x0000)
#define ICET_GL_DISPLAY_COLORED_BACKGROUND (ICET_GL_STATE_ENABLE_START | (IceTEnum)0x0001)
#define ICET_GL_DISPLAY_INFLATE (ICET_GL_STATE_ENABLE_START | (IceTEnum)0x0002)
#define ICET_GL_DISPLAY_INFLATE_WITH_HARDWARE (ICET_GL_STATE_ENABLE_START | (IceTEnum)0x0003)

#define ICET_GL_BUFFER_START    ICET_RENDER_LAYER_BUFFER_START
#define ICET_GL_BUFFER_END      ICET_RENDER_LAYER_BUFFER_END

#define ICET_GL_DISPLAY_BUFFER  (ICET_GL_BUFFER_START | (IceTEnum)0x0000)
#define ICET_GL_INFLATE_BUFFER  (ICET_GL_BUFFER_START | (IceTEnum)0x0001)


#ifdef __cplusplus
}
#endif

#endif /* __IceTGL_h */
