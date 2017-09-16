/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevDiagnostics_h
#define __IceTDevDiagnostics_h

#include <IceT.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

ICET_EXPORT void icetRaiseDiagnostic(const char *msg, IceTEnum type,
				     IceTBitField level,
				     const char *file, int line);

#define icetRaiseError(msg, type)			\
    icetRaiseDiagnostic(msg, type, ICET_DIAG_ERRORS, __FILE__, __LINE__)

#define icetRaiseWarning(msg, type)			\
    icetRaiseDiagnostic(msg, type, ICET_DIAG_WARNINGS, __FILE__, __LINE__)

#ifdef DEBUG
#define icetRaiseDebug(msg)				\
    icetRaiseDiagnostic(msg, ICET_NO_ERROR, ICET_DIAG_DEBUG, __FILE__, __LINE__)

#define icetRaiseDebug1(msg, arg1)			\
    {							\
	char msg_string[256];				\
	sprintf(msg_string, msg, arg1);			\
	icetRaiseDebug(msg_string);			\
    }

#define icetRaiseDebug2(msg, arg1, arg2)		\
    {							\
	char msg_string[256];				\
	sprintf(msg_string, msg, arg1, arg2);		\
	icetRaiseDebug(msg_string);			\
    }

#define icetRaiseDebug4(msg, arg1, arg2, arg3, arg4)	\
    {							\
	char msg_string[256];				\
	sprintf(msg_string, msg, arg1,arg2,arg3,arg4);	\
	icetRaiseDebug(msg_string);			\
    }
#else /* DEBUG */
#define icetRaiseDebug(msg)
#define icetRaiseDebug1(msg,arg1)
#define icetRaiseDebug2(msg,arg1,arg2)
#define icetRaiseDebug4(msg,arg1,arg2,arg3,arg4)
#endif /* DEBUG */

ICET_EXPORT void icetDebugBreak(void);

#ifdef __cplusplus
}
#endif

#endif /* __IceTDevDiagnostics_h */
