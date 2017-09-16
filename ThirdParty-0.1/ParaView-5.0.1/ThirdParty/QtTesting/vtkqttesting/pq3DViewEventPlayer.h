/*=========================================================================

   Program: ParaView
   Module:    pq3DViewEventPlayer.h

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#ifndef _pq3DViewEventPlayer_h
#define _pq3DViewEventPlayer_h

#include "pqWidgetEventPlayer.h"

/**
Concrete implementation of pqWidgetEventPlayer that handles playback of "activate" events for 3d views.
It is not registered by default, and user can register with their own 3d view type.

\sa pqEventPlayer
*/
class QTTESTING_EXPORT pq3DViewEventPlayer :
  public pqWidgetEventPlayer
{
  Q_OBJECT

public:
  pq3DViewEventPlayer(const QByteArray& classname, QObject* p = 0);

  bool playEvent(QObject* Object, const QString& Command, const QString& Arguments, bool& Error);

protected:
  QByteArray mClassType;

private:

  pq3DViewEventPlayer(const pq3DViewEventPlayer&);
  pq3DViewEventPlayer& operator=(const pq3DViewEventPlayer&);
};

#endif // !_pq3DViewEventPlayer_h
