/* -*- c -*- *****************************************************************
** Copyright (C) 2003 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test provides a simple example of using IceT to perform parallel
** rendering.
*****************************************************************************/

#include <IceTGL.h>
#include "test_util.h"
#include "test_codes.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#endif

IceTInt rank;
IceTInt num_proc;

#define SPHERE_RESOLUTION 20
#define SPHERE_RADIUS 0.5f
#define SPHERE_NUM_QUADS (SPHERE_RESOLUTION*SPHERE_RESOLUTION/2)
#define SPHERE_NUM_VERTICES (SPHERE_NUM_QUADS*4)

static void GetSphereArrays(GLfloat **vertex_array_p, GLfloat **normal_array_p)
{
    static GLfloat vertex_array[SPHERE_NUM_VERTICES*3];
    static GLfloat normal_array[SPHERE_NUM_VERTICES*3];
    static GLboolean arrays_initialized = 0;

    if (!arrays_initialized)
    {
        GLfloat cos_table[SPHERE_RESOLUTION+1];
        GLfloat sin_table[SPHERE_RESOLUTION+1];
        int theta_index, phi_index;

        // Fill out tables.
        for (theta_index = 0; theta_index <= SPHERE_RESOLUTION; theta_index++)
        {
            cos_table[theta_index]=cosf((2*M_PI/SPHERE_RESOLUTION)*theta_index);
            sin_table[theta_index]=sinf((2*M_PI/SPHERE_RESOLUTION)*theta_index);
        }

        // Compute the vertices and normals.
        GLfloat *vertex_p = vertex_array;
        GLfloat *normal_p = normal_array;
        for (phi_index = 0; phi_index < SPHERE_RESOLUTION/2; phi_index++)
        {
            for (theta_index = 0; theta_index<SPHERE_RESOLUTION; theta_index++)
            {
#define SET_VERT(t_index, p_index) \
    normal_p[0] = cos_table[t_index]*sin_table[p_index]; \
    normal_p[1] = sin_table[t_index]*sin_table[p_index]; \
    normal_p[2] = cos_table[p_index]; \
    vertex_p[0] = normal_p[0]*SPHERE_RADIUS; \
    vertex_p[1] = normal_p[1]*SPHERE_RADIUS; \
    vertex_p[2] = normal_p[2]*SPHERE_RADIUS; \
    normal_p += 3; vertex_p += 3;
                SET_VERT(theta_index, phi_index);
                SET_VERT(theta_index, phi_index+1);
                SET_VERT(theta_index+1, phi_index+1);
                SET_VERT(theta_index+1, phi_index);
            }
        }

        arrays_initialized = 1;
    }

    *vertex_array_p = vertex_array;
    *normal_array_p = normal_array;
}

static void draw(void)
{
    GLfloat *vertex_array;
    GLfloat *normal_array;

    GetSphereArrays(&vertex_array, &normal_array);
    glVertexPointer(3, GL_FLOAT, 0, vertex_array);
    glNormalPointer(GL_FLOAT, 0, vertex_array);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* When changing the modelview matric in the draw function, you must be
   * wary of two things.  First, make sure the modelview matrix is restored
   * to what is was when the function is called.  Remember, the draw
   * function may be called multiple times and transformations may be
   * commuted.  Also, the bounds of the drawn geometry must be correctly
   * transformed before given to IceT.  IceT has no way of knowing about
   * transformations done here.  It is an error to change the projection
   * matrix in the draw function. */
    glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glTranslatef((float)rank, 0, 0);
      glDrawArrays(GL_QUADS, 0, SPHERE_NUM_VERTICES);
    glPopMatrix();
}

static int SimpleExampleRun()
{
    float angle;

  /* Normally, the first thing that you do is set up your communication and
   * then create at least one IceT context.  This has already been done in
   * the calling function (i.e. icetTests_mpi.c).  See the init_mpi in
   * test_mpi.h for an example.
   */

  /* If we had set up the communication layer ourselves, we could have
   * gotten these parameters directly from it.  Since we did not, this
   * provides an alternate way. */
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

  /* We should be able to set any color we want, but we should do it BEFORE
   * icetGLDrawFrame() is called, not in the callback drawing function.
   * There may also be limitations on the background color when performing
   * color blending. */
    glClearColor(0.2f, 0.5f, 0.1f, 1.0f);

  /* Give IceT a function that will issue the OpenGL drawing commands. */
    icetGLDrawCallback(draw);

  /* Give IceT the bounds of the polygons that will be drawn.  Note that
   * we must take into account any transformation that happens within the
   * draw function (but IceT will take care of any transformation that
   * happens before icetGLDrawFrame). */
    icetBoundingBoxd(-0.5+rank, 0.5+rank, -0.5, 0.5, -0.5, 0.5);

  /* Set up the tiled display.  Normally, the display will be fixed for a
   * given installation, but since this is a demo, we give two specific
   * examples. */
    if (num_proc < 4) {
      /* Here is an example of a "1 tile" case.  This is functionally
       * identical to a traditional sort last algorithm. */
        icetResetTiles();
        icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    } else {
      /* Here is an example of a 4x4 tile layout.  The tiles are displayed
       * with the following ranks:
       *
       *               +---+---+
       *               | 0 | 1 |
       *               +---+---+
       *               | 2 | 3 |
       *               +---+---+
       *
       * Each tile is simply defined by grabing a viewport in an infinite
       * global display screen.  The global viewport projection is
       * automatically set to the smallest region containing all tiles.
       *
       * This example also shows tiles abutted against each other.
       * Mullions and overlaps can be implemented by simply shifting tiles
       * on top of or away from each other.
       */
        icetResetTiles();
        icetAddTile(0, (IceTInt)SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        icetAddTile((IceTInt)SCREEN_WIDTH,
                    (IceTInt)SCREEN_HEIGHT,
                    SCREEN_WIDTH,
                    SCREEN_HEIGHT,
                    1);
        icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 2);
        icetAddTile((IceTInt)SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 3);
    }

  /* Tell IceT what strategy to use.  The REDUCE strategy is an all-around
   * good performer. */
    icetStrategy(ICET_STRATEGY_REDUCE);

  /* Set up the projection matrix as you normally would. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.75, 0.75, -0.75, 0.75, -0.75, 0.75);

  /* Other normal OpenGL setup. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    if (rank%8 != 0) {
        GLfloat color[4];
        color[0] = (float)(rank%2);
        color[1] = (float)((rank/2)%2);
        color[2] = (float)((rank/4)%2);
        color[3] = 1.0f;
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    }

  /* Here is an example of an animation loop. */
    for (angle = 0; angle < 360; angle += 10) {
      /* We can set up a modelview matrix here and IceT will factor this
       * in determining the screen projection of the geometry.  Note that
       * there is further transformation in the draw function that IceT
       * cannot take into account.  That transformation is handled in the
       * application by deforming the bounds before giving them to
       * IceT. */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotated(angle, 0.0, 1.0, 0.0);
        glScaled(1.0f/num_proc, 1.0, 1.0);
        glTranslated(-(num_proc-1)/2.0, 0.0, 0.0);

      /* Instead of calling draw() directly, call it indirectly through
       * icetDrawFrame().  IceT will automatically handle image
       * compositing. */
        icetGLDrawFrame();

      /* For obvious reasons, IceT should be run in double-buffered frame
       * mode.  After calling icetDrawFrame, the application should do a
       * synchronize (a barrier is often about as good as you can do) and
       * then a swap buffers. */
        swap_buffers();
    }

    return TEST_PASSED;
}

int SimpleExample(int argc, char * argv[])
{
    /* To remove warning */
    (void)argc;
    (void)argv;

    return run_test(SimpleExampleRun);
}
