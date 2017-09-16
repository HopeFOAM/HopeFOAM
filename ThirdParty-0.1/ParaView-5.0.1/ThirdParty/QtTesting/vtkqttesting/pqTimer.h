/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
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

========================================================================*/
#ifndef __pqTimer_h
#define __pqTimer_h

#include <QTimer>
#include "QtTestingExport.h"

/// pqTimer is a extension for QTimer which ensures that the timer is registered
/// with the pqEventDispatcher. Register timers with pqEventDispatcher ensures
/// that when tests are being played back, the timer will be ensured to have
/// timed out if active after every step in the playback. This provides a means
/// to reproduce the timer behaviour in the real world, during test playback.
class QTTESTING_EXPORT pqTimer : public QTimer
{
  Q_OBJECT
  typedef QTimer Superclass;
public:
  pqTimer(QObject* parent=0);
  virtual ~pqTimer();

  /// This static function calls a slot after a given time interval.
  static void singleShot(int msec, QObject* receiver, const char* member);

protected:
  /// overridden to support blocking timer events in future (current
  /// implementation merely forwards to superclass).
  virtual void timerEvent(QTimerEvent* evt);

private:
  Q_DISABLE_COPY(pqTimer)
};

#endif
