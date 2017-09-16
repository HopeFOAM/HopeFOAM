/*=========================================================================

   Program: ParaView
   Module:    pqEventDispatcher.cxx

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

#include "pqEventDispatcher.h"

#include "pqEventPlayer.h"
#include "pqEventSource.h"

#include <QAbstractEventDispatcher>
#include <QtDebug>
#include <QApplication>
#include <QEventLoop>
#include <QThread>
#include <QDialog>
#include <QMainWindow>
#include <QList>
#include <QPointer>
#include <QTimer>

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
namespace
{
  static QList<QPointer<QTimer> > RegisteredTimers;
  
  void processTimers()
    {
    foreach (QTimer* timer, RegisteredTimers)
      {
      if (timer && timer->isActive())
        {
        QTimerEvent event(timer->timerId());
        qApp->notify(timer, &event);
        }
      }
    }

  static int EventPlaybackDelay = QT_TESTING_EVENT_PLAYBACK_DELAY;
};

bool pqEventDispatcher::DeferMenuTimeouts = false;
bool pqEventDispatcher::DeferEventsIfBlocked = false;

//-----------------------------------------------------------------------------
pqEventDispatcher::pqEventDispatcher(QObject* parentObject) :
  Superclass(parentObject)
{
  this->ActiveSource = NULL;
  this->ActivePlayer = NULL;

  this->PlayingBlockingEvent = false;

  this->PlayBackStatus = true;
  this->PlayBackFinished = false;
  this->PlayBackPaused = false;
  this->PlayBackOneStep = false;
  this->PlayBackStoped = false;

#ifdef __APPLE__
  this->BlockTimer.setInterval(1000);
#else
  this->BlockTimer.setInterval(100);
#endif
  this->BlockTimer.setSingleShot(true);
  QObject::connect(&this->BlockTimer, SIGNAL(timeout()),
                   this, SLOT(playEventOnBlocking()));
}

//-----------------------------------------------------------------------------
pqEventDispatcher::~pqEventDispatcher()
{
}


//-----------------------------------------------------------------------------
void pqEventDispatcher::setEventPlaybackDelay(int milliseconds)
{
  EventPlaybackDelay = (milliseconds <= 0)? 0 : milliseconds;
}

//-----------------------------------------------------------------------------
int pqEventDispatcher::eventPlaybackDelay()
{
  return EventPlaybackDelay;
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::registerTimer(QTimer* timer)
{
  if (timer)
    {
    RegisteredTimers.push_back(timer);
    }
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::deferEventsIfBlocked(bool enable)
{
  pqEventDispatcher::DeferEventsIfBlocked = enable;
}


//-----------------------------------------------------------------------------
void pqEventDispatcher::aboutToBlock()
{
  // if (!pqEventDispatcher::DeferMenuTimeouts)
    {
    if (!this->BlockTimer.isActive())
      {
      // cout << "aboutToBlock" << endl;
      // Request a delayed playback for an event.
      this->BlockTimer.start();
      }
    }
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::awake()
{
  //if (!pqEventDispatcher::DeferMenuTimeouts)
  //  {
  //  // cout << "awake" << endl;
  //  // this->BlockTimer.stop();
  //  }
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::setTimeStep(int value)
{
  EventPlaybackDelay = value;
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::run(bool value)
{
  this->PlayBackPaused = !value;
  if (value)
    {
    emit this->restarted();
    }
  else
    {
    emit this->paused();
    }
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::stop()
{
  this->PlayBackPaused = false;
  this->PlayBackFinished = true;
}

//-----------------------------------------------------------------------------
bool pqEventDispatcher::isPaused() const
{
  return this->PlayBackPaused;
}

//-----------------------------------------------------------------------------
bool pqEventDispatcher::status() const
{
  return this->PlayBackStatus;
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::oneStep()
{
  this->PlayBackOneStep = true;
}

//-----------------------------------------------------------------------------
bool pqEventDispatcher::playEvents(pqEventSource& source, pqEventPlayer& player)
{
  if (this->ActiveSource || this->ActivePlayer)
    {
    qCritical() << "Event dispatcher is already playing";
    return false;
    }

  this->ActiveSource = &source;
  this->ActivePlayer = &player;

  QApplication::setEffectEnabled(Qt::UI_General, false);

  QObject::connect(QAbstractEventDispatcher::instance(), SIGNAL(aboutToBlock()),
                   this, SLOT(aboutToBlock()));
  QObject::connect(QAbstractEventDispatcher::instance(), SIGNAL(awake()),
                   this, SLOT(awake()));

  // This is how the playback logic works:
  // * In here, we continuously keep on playing one event after another until
  //   we are done processing all the events.
  // * If a modal dialog pops up, then this->aboutToBlock() gets called. To me
  //   accurate, aboutToBlock() is called everytime the sub-event loop is entered
  //   and more modal dialogs that loop is entered after processing of each event
  //   (not merely when the dialog pops up).
  // * In this->aboutToBlock() we start a timer which on timeout processes just 1 event.
  // * After executing that event, if the dialog still is up, them aboutToBlock() will
  //   be called again and the cycle continues. If not, the control returns to this main
  //   playback loop, and it continues.
  this->PlayBackStatus = true; // success.
  this->PlayBackFinished = false;
  while (!this->PlayBackFinished)
    {
    if(!this->PlayBackPaused)
      {
    //cerr << "=== playEvent(1) ===" << endl;
    this->playEvent();
    }
    else
      {
      if (this->PlayBackOneStep)
        {
        this->PlayBackOneStep = false;
        this->playEvent();
        }
      else
        {
        this->processEventsAndWait(100);
        }
      }
    }
  this->ActiveSource = NULL;
  this->ActivePlayer = NULL;

  QObject::disconnect(QAbstractEventDispatcher::instance(), SIGNAL(aboutToBlock()),
                   this, SLOT(aboutToBlock()));
  QObject::disconnect(QAbstractEventDispatcher::instance(), SIGNAL(awake()),
                   this, SLOT(awake()));

  return this->PlayBackStatus;
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::playEventOnBlocking()
{
//  if(this->PlayingBlockingEvent)
//    {
//    qDebug() << "Event blocking already playing ....";
//    return;
//    }

  if (pqEventDispatcher::DeferMenuTimeouts || pqEventDispatcher::DeferEventsIfBlocked)
    {
    //cerr << "=== playEventOnBlocking ===" << endl;
    this->BlockTimer.start();
    return;
    }

  //cout << "---blocked event: " << endl;
  // if needed for debugging, I can print blocking annotation here.
  //cerr << "=== playEvent(2) ===" << endl;
  this->playEvent(1);

  //if (!this->BlockTimer.isActive())
  //  {
  //  this->BlockTimer.start();
  //  }
  this->PlayingBlockingEvent = false;
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::playEvent(int indent)
{
  this->BlockTimer.stop();
  if (this->PlayBackFinished)
    {
    return;
    }

  if (!this->ActiveSource)
    {
    this->PlayBackFinished = true;
    this->PlayBackStatus = false; // failure.
    qCritical("Internal error: playEvent called without a valid event source.");
    return;
    }

  QString object;
  QString command;
  QString arguments;
  
  int result = this->ActiveSource->getNextEvent(object, command, arguments);
  if (result == pqEventSource::DONE)
    {
    this->PlayBackFinished = true;
    return;
    }
  else if(result == pqEventSource::FAILURE)
    {
    this->PlayBackFinished = true;
    this->PlayBackStatus = false; // failure.
    return;
    }
    
  static unsigned long counter=0;
  unsigned long local_counter = counter++;
  QString pretty_name = object.mid(object.lastIndexOf('/'));
  bool print_debug = getenv("PV_DEBUG_TEST") != NULL;
#if defined(WIN32) || defined(__APPLE__) // temporary debugging on both platforms.
  print_debug = true;
#endif
  if (print_debug)
    {
    cout <<  QTime::currentTime().toString("hh:mm:ss").toStdString().c_str()
         << " : "
         << QString().fill(' ', 4*indent).toStdString().c_str()
         << local_counter << ": Test (" << indent << "): "
         << pretty_name.toStdString().c_str() << ": "
         << command.toStdString().c_str() << " : "
         << arguments.toStdString().c_str() << endl;
    }

  bool error = false;
  this->ActivePlayer->playEvent(object, command, arguments, error);
  this->BlockTimer.stop();

  // process any posted events. We call processEvents() so that any slots
  // connected using QueuedConnection also get handled.
  this->processEventsAndWait(EventPlaybackDelay);

  // We explicitly timeout timers that have been registered with us.
  processTimers();

  this->BlockTimer.stop();

  if (print_debug)
    {
    cout << QTime::currentTime().toString("hh:mm:ss").toStdString().c_str()
         << " : "
         << QString().fill(' ', 4*indent).toStdString().c_str()
         << local_counter << ": Done" << endl;
    }

  if (error)
    {
    this->PlayBackStatus  = false;
    this->PlayBackFinished = true;
    return;
    }

}

//-----------------------------------------------------------------------------
void pqEventDispatcher::processEventsAndWait(int ms)
{
  bool prev = pqEventDispatcher::DeferMenuTimeouts;
  pqEventDispatcher::DeferMenuTimeouts = true;
  if (ms > 0)
    {
//    ms = (ms < 100) ? 100 : ms;
    QApplication::processEvents();
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
    }
  QApplication::processEvents();
  QApplication::sendPostedEvents();
  QApplication::processEvents();
  pqEventDispatcher::DeferMenuTimeouts = prev;
}

//-----------------------------------------------------------------------------
void pqEventDispatcher::processEvents(QEventLoop::ProcessEventsFlags flags)
{
  bool prev = pqEventDispatcher::DeferMenuTimeouts;
  pqEventDispatcher::DeferMenuTimeouts = true;
  QApplication::processEvents(flags);
  pqEventDispatcher::DeferMenuTimeouts = prev;
}
