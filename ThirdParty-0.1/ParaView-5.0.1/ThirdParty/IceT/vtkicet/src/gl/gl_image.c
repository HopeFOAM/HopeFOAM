/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevGLImage.h>

#include <IceTGL.h>

#include <IceTDevDiagnostics.h>
#include <IceTDevState.h>
#include <IceTDevTiming.h>

void icetGLDrawCallbackFunction(const IceTDouble *projection_matrix,
                                const IceTDouble *modelview_matrix,
                                const IceTFloat *background_color,
                                const IceTInt *readback_viewport,
                                IceTImage result)
{
    IceTSizeType width = icetImageGetWidth(result);
    IceTSizeType height = icetImageGetHeight(result);
    GLint gl_viewport[4];
    glGetIntegerv(GL_VIEWPORT, gl_viewport);

  /* Check OpenGL state. */
    {
        if ((gl_viewport[2] != width) || (gl_viewport[3] != height)) {
            icetRaiseError("OpenGL viewport different than expected."
                           " Was it changed?", ICET_SANITY_CHECK_FAIL);
        }
    }

  /* Set up OpenGL. */
    {
      /* Load the matrices. */
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(projection_matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixd(modelview_matrix);

      /* Set the clear color as the background IceT currently wants. */
        glClearColor(background_color[0], background_color[1],
                     background_color[2], background_color[3]);
    }

  /* Call the rendering callback. */
    {
        IceTVoid *value;
        IceTGLDrawCallbackType callback;
        icetRaiseDebug("Calling OpenGL draw function.");
        icetGetPointerv(ICET_GL_DRAW_FUNCTION, &value);
        callback = (IceTGLDrawCallbackType)value;
        (*callback)();
    }

    /* Temporarily stop render time while reading back buffer. */
    icetTimingRenderEnd();

    icetTimingBufferReadBegin();

  /* Read the OpenGL buffers. */
    {
        IceTEnum color_format = icetImageGetColorFormat(result);
        IceTEnum depth_format = icetImageGetDepthFormat(result);
        IceTEnum readbuffer;
        IceTSizeType x_offset = gl_viewport[0] + readback_viewport[0];
        IceTSizeType y_offset = gl_viewport[1] + readback_viewport[1];

        glPixelStorei(GL_PACK_ROW_LENGTH, (GLint)icetImageGetWidth(result));

      /* These pixel store parameters are not working on one of the platforms
       * I am testing on (thank you Mac).  Instead of using these, just offset
       * the buffers we read in from. */
        /* glPixelStorei(GL_PACK_SKIP_PIXELS, readback_viewport[0]); */
        /* glPixelStorei(GL_PACK_SKIP_ROWS, readback_viewport[1]); */

        icetGetEnumv(ICET_GL_READ_BUFFER, &readbuffer);
        glReadBuffer(readbuffer);

        if (color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
            IceTUInt *colorBuffer = icetImageGetColorui(result);
            glReadPixels((GLint)x_offset,
                         (GLint)y_offset,
                         (GLsizei)readback_viewport[2],
                         (GLsizei)readback_viewport[3],
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         colorBuffer + (  readback_viewport[0]
                                        + width*readback_viewport[1]));
        } else if (color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
            IceTFloat *colorBuffer = icetImageGetColorf(result);
            glReadPixels((GLint)x_offset,
                         (GLint)y_offset,
                         (GLsizei)readback_viewport[2],
                         (GLsizei)readback_viewport[3],
                         GL_RGBA,
                         GL_FLOAT,
                         colorBuffer + 4*(  readback_viewport[0]
                                          + width*readback_viewport[1]));
        } else if (color_format != ICET_IMAGE_COLOR_NONE) {
            icetRaiseError("Invalid color format.", ICET_SANITY_CHECK_FAIL);
        }

        if (depth_format == ICET_IMAGE_DEPTH_FLOAT) {
            IceTFloat *depthBuffer = icetImageGetDepthf(result);;
            glReadPixels((GLint)x_offset,
                         (GLint)y_offset,
                         (GLsizei)readback_viewport[2],
                         (GLsizei)readback_viewport[3],
                         GL_DEPTH_COMPONENT,
                         GL_FLOAT,
                         depthBuffer + (  readback_viewport[0]
                                        + width*readback_viewport[1]));
        } else if (depth_format != ICET_IMAGE_DEPTH_NONE) {
            icetRaiseError("Invalid depth format.", ICET_SANITY_CHECK_FAIL);
        }

        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        /* glPixelStorei(GL_PACK_SKIP_PIXELS, 0); */
        /* glPixelStorei(GL_PACK_SKIP_ROWS, 0); */
    }

    icetTimingBufferReadEnd();

    /* Start render timer again.  It's going to be shut off immediately on
       return anyway, but the calling function expects it to be running. */
    icetTimingRenderBegin();
}
