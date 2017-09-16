/*=========================================================================

   Program: ParaView
   Module:    pqNativeFileDialogEventTranslator.h

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

#ifndef _pqNativeFileDialogEventTranslator_h
#define _pqNativeFileDialogEventTranslator_h

#include "pqWidgetEventTranslator.h"
#include <QMouseEvent>

class pqTestUtility;

/**
Records usage of native file dialogs in test cases.

\sa pqEventTranslator
*/

class QTTESTING_EXPORT pqNativeFileDialogEventTranslator :
  public pqWidgetEventTranslator
{
  Q_OBJECT

public:
  pqNativeFileDialogEventTranslator(pqTestUtility* util, QObject* p=0);
  ~pqNativeFileDialogEventTranslator();

  virtual bool translateEvent(QObject* Object, QEvent* Event, bool& Error);

  void record(const QString& command, const QString& args);

protected slots:
  void start();
  void stop();

protected:

  pqTestUtility* mUtil;

private:
  pqNativeFileDialogEventTranslator(const pqNativeFileDialogEventTranslator&);
  pqNativeFileDialogEventTranslator& operator=(const pqNativeFileDialogEventTranslator&);
};

#endif // !_pqNativeFileDialogEventTranslator_h
