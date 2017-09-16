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

// Qt includes
#include <QIODevice>
#include <QTextStream>

// QtTesting includes
#include "pqEventObserver.h"
#include "pqEventRecorder.h"
#include "pqEventTranslator.h"

// ----------------------------------------------------------------------------
pqEventRecorder::pqEventRecorder(QObject *parent)
  : Superclass(parent)
{
  this->ActiveObserver = 0;
  this->ActiveTranslator = 0;
  this->File = 0;

  this->Recording = false;
  this->ContinuousFlush = false;
}

// ----------------------------------------------------------------------------
pqEventRecorder::~pqEventRecorder()
{
  this->ActiveObserver = 0;
  this->ActiveTranslator = 0;
}

// ----------------------------------------------------------------------------
void pqEventRecorder::setContinuousFlush(bool value)
{
  if (!this->ActiveObserver)
    {
    return;
    }

  if (value)
    {
    QObject::connect(this->ActiveObserver,
                     SIGNAL(eventRecorded(QString,QString,QString)),
                     this, SLOT(flush()));
    }
  else
    {
    QObject::disconnect(this->ActiveObserver,
                        SIGNAL(eventRecorded(QString,QString,QString)),
                        this, SLOT(flush()));
    }
  this->ContinuousFlush = value;
}

// ----------------------------------------------------------------------------
bool pqEventRecorder::continuousFlush() const
{
  return this->ContinuousFlush;
}

// ----------------------------------------------------------------------------
void pqEventRecorder::setFile(QIODevice* file)
{
  this->File = file;
}

// ----------------------------------------------------------------------------
QIODevice* pqEventRecorder::file() const
{
  return this->File;
}

// ----------------------------------------------------------------------------
void pqEventRecorder::setObserver(pqEventObserver* observer)
{
  this->ActiveObserver = observer;
}

// ----------------------------------------------------------------------------
pqEventObserver* pqEventRecorder::observer() const
{
  return this->ActiveObserver;
}

// ----------------------------------------------------------------------------
void pqEventRecorder::setTranslator(pqEventTranslator* translator)
{
  this->ActiveTranslator = translator;
}

// ----------------------------------------------------------------------------
pqEventTranslator* pqEventRecorder::translator() const
{
  return this->ActiveTranslator;
}

// ----------------------------------------------------------------------------
bool pqEventRecorder::isRecording() const
{
  return this->Recording;
}

// ----------------------------------------------------------------------------
void pqEventRecorder::recordEvents(pqEventTranslator* translator,
                                   pqEventObserver* observer,
                                   QIODevice* file,
                                   bool continuousFlush)
{
  this->setTranslator(translator);
  this->setObserver(observer);
  this->setFile(file);
  this->setContinuousFlush(continuousFlush);

  this->start();
}

// ----------------------------------------------------------------------------
void pqEventRecorder::flush()
{
  this->Stream.flush();
}

// ----------------------------------------------------------------------------
void pqEventRecorder::start()
{
  if (!this->File || !this->ActiveObserver || !this->ActiveTranslator)
    {
    return;
    }

  QObject::connect(
    this->ActiveTranslator,
    SIGNAL(recordEvent(QString,QString,QString)),
    this->ActiveObserver,
    SLOT(onRecordEvent(QString,QString,QString)));

  // Set the device
  this->Stream.setDevice(this->File);

  // Set the Stream to the Observer
  this->ActiveObserver->setStream(&this->Stream);

  // Start the Translator
  this->ActiveTranslator->start();

  this->Recording = true;
  emit this->started();
}

// ----------------------------------------------------------------------------
void pqEventRecorder::stop(int value)
{
  QObject::disconnect(
    this->ActiveTranslator,
    SIGNAL(recordEvent(QString,QString,QString)),
    this->ActiveObserver,
    SLOT(onRecordEvent(QString,QString,QString)));

  this->ActiveObserver->setStream(NULL);
  this->ActiveTranslator->stop();

  this->Recording = false;

  if (!value)
    {
    return;
    }

  this->flush();
  emit this->stopped();
}

// ----------------------------------------------------------------------------
void pqEventRecorder::pause(bool value)
{
  if (!value)
    {
    QObject::disconnect(
      this->ActiveTranslator,
      SIGNAL(recordEvent(QString,QString,QString)),
      this->ActiveObserver,
      SLOT(onRecordEvent(QString,QString,QString)));
    }
  else
    {
    QObject::connect(
      this->ActiveTranslator,
      SIGNAL(recordEvent(QString,QString,QString)),
      this->ActiveObserver,
      SLOT(onRecordEvent(QString,QString,QString)),
      Qt::UniqueConnection);
    }

  this->Recording = value;
  emit this->paused(value);
}
