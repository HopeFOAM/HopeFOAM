/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2015 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef _TEST_MPI_OPENGL_H
#define _TEST_MPI_OPENGL_H

#define ICET_NO_MPI_RENDERING_FUNCTIONS
#include "test_mpi.h"

#include "test_config.h"

#include <IceTGL.h>

#ifdef ICET_TESTS_USE_GLUT
#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif
#endif

#ifdef ICET_TESTS_USE_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef ICET_TESTS_USE_GLUT
static int windowId;
#endif

#ifdef ICET_TESTS_USE_GLFW
static GLFWwindow *window;
#endif

#include "test_codes.h"

static void checkOglError(void)
{
    GLenum error = glGetError();

#define CASE_ERROR(ename)                                               \
    case ename: printrank("## Current IceT error = " #ename "\n"); break;

    switch (error) {
      case GL_NO_ERROR: break;
      CASE_ERROR(GL_INVALID_ENUM);
      CASE_ERROR(GL_INVALID_VALUE);
      CASE_ERROR(GL_INVALID_OPERATION);
      CASE_ERROR(GL_STACK_OVERFLOW);
      CASE_ERROR(GL_STACK_UNDERFLOW);
      CASE_ERROR(GL_OUT_OF_MEMORY);
#ifdef GL_TABLE_TOO_LARGE
      CASE_ERROR(GL_TABLE_TOO_LARGE);
#endif
      default:
          printrank("## UNKNOWN OPENGL ERROR CODE!!!!!!\n");
          break;
    }

#undef CASE_ERROR
}

void init_mpi_opengl(int *argcp, char ***argvp)
{
#ifdef ICET_TESTS_USE_GLUT
    /* Let Glut have first pass at the arguments to grab any that it can use. */
    glutInit(argcp, *argvp);
#endif

#ifdef ICET_TESTS_USE_GLFW
    if (!glfwInit()) { exit(1); }
#endif

    init_mpi(argcp, argvp);
}

#if defined(ICET_TESTS_USE_GLUT)
static int (*g_test_function)(void);

static void no_op()
{
}

static void glut_draw()
{
    int result;

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT);
    swap_buffers();

    result = run_test_base(g_test_function);

    glutDestroyWindow(windowId);

    exit(result);
}

int run_test(int (*test_function)(void))
{
  /* Record the test function so we can run it in the Glut draw callback. */
    g_test_function = test_function;

    glutDisplayFunc(no_op);
    glutIdleFunc(glut_draw);

  /* Glut will reliably create the OpenGL context only after the main loop is
   * started.  This will create the window and then call our glut_draw function
   * to populate it.  It will never return, which is why we call exit in
   * glut_draw. */
    glutMainLoop();

  /* We do not expect to be here.  Raise an alert to signal that the tests are
   * not running as expected. */
    return TEST_NOT_PASSED;
}

void initialize_render_window(int width, int height)
{
    IceTInt rank, num_proc;

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    /* Create a renderable window. */
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(width, height);

    {
        char title[256];
        sprintf(title, "IceT Test %d of %d", rank, num_proc);
        windowId = glutCreateWindow(title);
    }

    icetGLInitialize();
}

void swap_buffers(void)
{
    glutSwapBuffers();
}

#elif defined(ICET_TESTS_USE_GLFW)

int run_test(int (*test_function)())
{
    int result;

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT);
    swap_buffers();

    result = run_test_base(test_function);

    glfwDestroyWindow(window);
    glfwTerminate();

    return result;
}

void initialize_render_window(int width, int height)
{
    IceTInt rank, num_proc;

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    /* Create a renderable window. */
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_SAMPLES, 0);

    {
        char title[256];
        sprintf(title, "IceT Test %d of %d", rank, num_proc);
        window = glfwCreateWindow(width, height, title, NULL, NULL);
    }

    glfwMakeContextCurrent(window);

    icetGLInitialize();
}

void swap_buffers(void)
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

#else

#error "ICET_TESTS_USE_OPENGL defined but no window library is defined."

#endif

void finalize_rendering()
{
    checkOglError();
}

#endif /* _TEST_MPI_OPENGL_H */
