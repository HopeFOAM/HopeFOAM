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
#include "pqTimer.h"

#include "pqEventDispatcher.h"

//-----------------------------------------------------------------------------
pqTimer::pqTimer(QObject* parentObject)
  : Superclass(parentObject)
{
  pqEventDispatcher::registerTimer(this);
}

//-----------------------------------------------------------------------------
pqTimer::~pqTimer()
{
}

//-----------------------------------------------------------------------------
void pqTimer::timerEvent(QTimerEvent* evt)
{
  // here we can consume the timer event and reschedule it for another time if
  // timers are blocked.
  this->Superclass::timerEvent(evt);
}

//-----------------------------------------------------------------------------
void pqTimer::singleShot(int msec, QObject* receiver, const char* member)
{
  if (receiver && member)
    {
    pqTimer* timer = new pqTimer(receiver);
    QObject::connect(timer, SIGNAL(timeout()), receiver, member);
    QObject::connect(timer, SIGNAL(timeout()), timer, SLOT(deleteLater()));
    timer->setSingleShot(true);
    timer->start(msec);
    }
}
