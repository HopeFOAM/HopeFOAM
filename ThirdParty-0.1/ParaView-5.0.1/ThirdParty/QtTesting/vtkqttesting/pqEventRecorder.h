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

#ifndef _pqEventRecorder_h
#define _pqEventRecorder_h

// Qt includes
#include <QIODevice>
#include <QObject>
#include <QTextStream>

// QtTesting includes
#include "QtTestingExport.h"

class pqEventObserver;
class pqEventTranslator;

/// Class to provide the recording.
/// Two behaviors :
///   - Either Continuous Flush is true, the file is written after each new actions
/// Indeed if the QtTesting/Application crashs, part of the script is written.
///   - Or the file is written only when we stop the record.

/// A simple call to the function "recordEvent" with good parameters, will start
/// the record.

class QTTESTING_EXPORT pqEventRecorder : public QObject
{
  Q_OBJECT
  Q_PROPERTY(bool continuousFlush READ continuousFlush WRITE setContinuousFlush)
  typedef QObject Superclass;

public:
  explicit pqEventRecorder(QObject *parent = 0);
  ~pqEventRecorder();

  void setContinuousFlush(bool value);
  bool continuousFlush() const;

  void setFile(QIODevice* file);
  QIODevice* file() const;

  void setObserver(pqEventObserver* observer);
  pqEventObserver* observer() const;

  void setTranslator(pqEventTranslator* translator);
  pqEventTranslator* translator() const;

  bool isRecording() const;

  void recordEvents(pqEventTranslator* translator,
                    pqEventObserver* observer,
                    QIODevice* file,
                    bool continuousFlush);

signals:
  void started();
  void stopped();
  void paused(bool);

public slots:
  void flush();

  void start();
  void stop(int value);
  void pause(bool);

protected:
  pqEventObserver*    ActiveObserver;
  pqEventTranslator*  ActiveTranslator;
  QIODevice*          File;

  bool                Recording;
  bool                ContinuousFlush;
  QTextStream         Stream;
};

#endif // !_pqEventRecorder_h
