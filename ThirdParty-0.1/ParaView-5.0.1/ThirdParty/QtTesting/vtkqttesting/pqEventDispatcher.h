/*=========================================================================

   Program: ParaView
   Module:    pqEventDispatcher.h

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

#ifndef _pqEventDispatcher_h
#define _pqEventDispatcher_h

#include "QtTestingExport.h"

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QEventLoop>

class pqEventPlayer;
class pqEventSource;

/// pqEventDispatcher is responsible for taking each "event" from the test and
/// then "playing" it using the player. The dispatcher is the critical component
/// of this playback since it decides when it's time to dispatch the next
/// "event" from the test.
/// After an event is posted, there are two options:
/// \li the default option is to simply wait for a small amount of time before
/// processing the next event. The hope is that within that time any pending
/// requests such as timers, slots connected using Qt::QueuedConnection etc.
/// will be handled and all will work well. This however it fraught with
/// problems resulting is large number of random test failures especially in
/// large and complex applications such as ParaView. In such cases the better
/// option is the second option.
/// \li we only process already posted events (using a call to
/// QApplication::processEvents() and then rely on timers being registered using
/// registerTimer(). All these timers are explicitly timed-out, if active, before
/// processing the next event.
///
/// To enable the second mode simply set the eventPlaybackDelay to 0.
/// In either mode, all timers registered using registerTimer() will be
/// timed-out before dispatching next event.
///
/// To make it easier to register timers, one can directly use pqTimer instead
/// of QTimer.
class QTTESTING_EXPORT pqEventDispatcher : public QObject
{
  Q_OBJECT
  typedef QObject Superclass; 
public:
  pqEventDispatcher(QObject* parent=0);
  ~pqEventDispatcher();

  /// Retrieves events from the given event source, dispatching them to
  /// the given event player for test case playback. This call blocks until all
  /// the events from the source have been played back (or failure). Returns
  /// true if playback was successful.
  bool playEvents(pqEventSource& source, pqEventPlayer& player);

  /// Set the delay between dispatching successive events. Default is set using
  /// CMake variable QT_TESTING_EVENT_PLAYBACK_DELAY.
  static void setEventPlaybackDelay(int milliseconds);
  static int eventPlaybackDelay();

  /** Wait function provided for players that need to wait for the GUI
      to perform a certain action.
      Note: the minimum wait time is 100ms. This is set to avoid timiing issues
      on slow processors that hang tests.*/
  static void processEventsAndWait(int ms);

    /** proccessEvents method for widgets and paraview to use instead of
    calling Qt version, since that will break test playback*/
  static void processEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents);

  /// register a timer that needs to be ensured to have timed-out after every
  /// event dispatch.
  static void registerTimer(QTimer* timer);

  /// Disables the behavior where more test events may be dispatched
  /// if Qt starts waiting in an event loop. Warning: Setting this to
  /// true will prevent modal dialogs from functioning correctly.
  static void deferEventsIfBlocked(bool defer);

  /// Return if the Dispatcher is not playing events
  bool isPaused() const;

  /// Return Dispatcher's status
  bool status() const;

signals:

  /// signal when playback starts
  void restarted();
  /// signal when playback pauses
  void paused();

protected slots:
  /// Plays a single event. this->PlayBackFinished and this->PlayBackStatus are
  /// updated by this method.
  void playEvent(int indent=0);
  void playEventOnBlocking();

  /// Called when the mainThread is about to block.
  void aboutToBlock();

  /// Called when the mainThread wakes up.
  void awake();

public slots:
  /// Change the TimeStep
  void setTimeStep(int value);
  /// Method to be able to stop/pause/play the current playback script
  void run(bool value);
  void stop();
  void oneStep();

protected:
  bool PlayingBlockingEvent;
  bool PlayBackFinished;
  bool PlayBackPaused;
  bool PlayBackStatus;
  bool PlayBackOneStep;
  bool PlayBackStoped;
  static bool DeferMenuTimeouts;
  /// This variable says that we should not continue to process test events
  /// when the application is blocked in a Qt event loop - it is either blocked
  /// in a modal dialog or is in a long wait while also processing events
  /// (such as when waiting from Insitu server @see pqLiveInsituManager).
  static bool DeferEventsIfBlocked;

  pqEventSource* ActiveSource;
  pqEventPlayer* ActivePlayer;
  QTimer BlockTimer;
};

#endif // !_pqEventDispatcher_h
