/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevContext.h>

#include <IceT.h>

#include <IceTDevDiagnostics.h>
#include <IceTDevImage.h>

#include <stdlib.h>
#include <string.h>

#define CONTEXT_MAGIC_NUMBER ((IceTEnum)0x12358D15)

struct IceTContextStruct {
    IceTEnum magic_number;
    IceTState state;
    IceTCommunicator communicator;
};

static IceTContext icet_current_context = NULL;

IceTContext icetCreateContext(IceTCommunicator comm)
{
    IceTContext context = malloc(sizeof(struct IceTContextStruct));

    if (context == NULL) {
        icetRaiseError("Could not allocate memory for IceT context.",
                       ICET_OUT_OF_MEMORY);
        return NULL;
    }

    context->magic_number = CONTEXT_MAGIC_NUMBER;

    context->communicator = comm->Duplicate(comm);

    context->state = icetStateCreate();

    icetSetContext(context);
    icetStateSetDefaults();

    return context;
}

static void callDestructor(IceTEnum dtor_variable)
{
    IceTVoid *void_dtor_pointer;
    void (*dtor_function)(void);

    icetGetPointerv(dtor_variable, &void_dtor_pointer);
    dtor_function = (void (*)(void))void_dtor_pointer;

    if (dtor_function) {
        (*dtor_function)();
    }
}

void icetDestroyContext(IceTContext context)
{
    IceTContext saved_current_context;

    saved_current_context = icetGetContext();
    if (context == saved_current_context) {
        icetRaiseDebug("Destroying current context.");
        saved_current_context = NULL;
    }

  /* Temporarily make the context to be destroyed current. */
    icetSetContext(context);

  /* Call destructors for other dependent units. */
    callDestructor(ICET_RENDER_LAYER_DESTRUCTOR);

  /* From here on out be careful.  We are invalidating the context. */
    context->magic_number = 0;

    icetStateDestroy(context->state);
    context->state = NULL;

    context->communicator->Destroy(context->communicator);

  /* The context is now completely destroyed and now null.  Restore saved
     context. */
    icetSetContext(saved_current_context);

    free(context);
}

IceTContext icetGetContext(void)
{
    return icet_current_context;
}

void icetSetContext(IceTContext context)
{
    if (context && (context->magic_number != CONTEXT_MAGIC_NUMBER)) {
        icetRaiseError("Invalid context.", ICET_INVALID_VALUE);
        return;
    }
    icet_current_context = context;
}

IceTState icetGetState()
{
    return icet_current_context->state;
}

IceTCommunicator icetGetCommunicator()
{
    return icet_current_context->communicator;
}

void icetCopyState(IceTContext dest, const IceTContext src)
{
    icetStateCopy(dest->state, src->state);
}
