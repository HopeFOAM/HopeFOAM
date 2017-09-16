/* -*- c -*- *****************************************************************
** Copyright (C) 2005 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This tests a corner case in which the bounds of an object extens from
** the viewing frustum to behind the viewpoint in perspective mode.
*****************************************************************************/

#include <IceTGL.h>
#include "test_util.h"
#include "test_codes.h"

#include <stdlib.h>
#include <stdio.h>

static void draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);
      glVertex3d(-1.0, -1.0, 0.0);
      glVertex3d(1.0, -1.0, 0.0);
      glVertex3d(1.0, 1.0, 0.0);
      glVertex3d(-1.0, 1.0, 0.0);
    glEnd();
}

static void PrintMatrix(float *mat)
{
    int r, c;

    for (c = 0; c < 4; c++) {
        for (r = 0; r < 4; r++) {
            printstat("%f ", mat[4*r + c]);
        }
        printstat("\n");
    }
}

static int BoundsBehindViewerRun()
{
    float mat[16];
    IceTImage image;

    IceTInt rank;
    icetGetIntegerv(ICET_RANK, &rank);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    icetGLDrawCallback(draw);
    icetStrategy(ICET_STRATEGY_REDUCE);

    icetBoundingBoxd(-1.0, 1.0, -1.0, 1.0, -0.0, 0.0);

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT);

  /* We're just going to use one tile. */
    icetResetTiles();
    icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

  /* Set up the transformation such that the quad in draw should cover the
     entire screen, but part of it extends behind the viewpoint.  Furthermore, a
     naive division by w will show all points to the right of the screen (which,
     of course, is wrong). */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -1.5);
    glRotated(10.0, 0.0, 1.0, 0.0);
    glScaled(10.0, 10.0, 10.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 2.0);

    printstat("Modelview matrix:\n");
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    PrintMatrix(mat);
    printstat("Projection matrix:\n");
    glGetFloatv(GL_PROJECTION_MATRIX, mat);
    PrintMatrix(mat);

  /* Other normal OpenGL setup. */
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor3d(1.0, 1.0, 1.0);

  /* All the processes have the same data.  Go ahead and tell IceT. */
    icetDataReplicationGroupColor(0);

    image = icetGLDrawFrame();

  /* Test the resulting image to make sure the polygon was drawn over it. */
    if (rank == 0) {
        IceTUInt *cb = icetImageGetColorui(image);
        if (cb[0] != 0xFFFFFFFF) {
            printstat("First pixel in color buffer wrong: 0x%x\n", cb[0]);
            return TEST_FAILED;
        }
    }

    return TEST_PASSED;
}

int BoundsBehindViewer(int argc, char *argv[])
{
    /* To remove warning */
    (void)argc;
    (void)argv;

    return run_test(BoundsBehindViewerRun);
}
