/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/
/*=========================================================================

   Program: ParaView

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
#include <QtTest/QtTest>

// QtTesting includes
#include "pqEventObserver.h"
#include "pqEventSource.h"


// ----------------------------------------------------------------------------
// Dumy pqEventObserver

class pqDummyEventObserver : public pqEventObserver
{
  Q_OBJECT

public:
  pqDummyEventObserver(QObject* p = 0) : pqEventObserver(p) {}
  ~pqDummyEventObserver() {}

  virtual void setStream(QTextStream* /*stream*/)
  {
  }

  QString Text;

public slots:
  virtual void onRecordEvent(const QString& widget,
                             const QString& command,
                             const QString& arguments)
  {
    this->Text.append(QString("%1, %2, %3#").arg(widget,
                                          command,
                                          arguments));
  }
};

// ----------------------------------------------------------------------------
// Dumy eventSource

class pqDummyEventSource : public pqEventSource
{
  typedef pqEventSource Superclass;

public:
  pqDummyEventSource(QObject* p = 0): Superclass(p) {}
  ~pqDummyEventSource() {}

protected:
  virtual void setContent(const QString& /*xmlfilename*/)
    {
    return;
    }

  int getNextEvent(QString& /*widget*/,
                   QString& /*command*/,
                   QString& /*arguments*/)
    {
    return 0;
    }
};

// ----------------------------------------------------------------------------
// MACRO

#define CTK_TEST_NOOP_MAIN(TestObject) \
int TestObject(int argc, char *argv[]) \
{ \
    QObject tc; \
    return QTest::qExec(&tc, argc, argv); \
}

#ifdef QT_GUI_LIB

//-----------------------------------------------------------------------------
#define CTK_TEST_MAIN(TestObject) \
  int TestObject(int argc, char *argv[]) \
  { \
    QApplication app(argc, argv); \
    TestObject##er tc; \
    return QTest::qExec(&tc, argc, argv); \
  }

#else

//-----------------------------------------------------------------------------
#define CTK_TEST_MAIN(TestObject) \
  int TestObject(int argc, char *argv[]) \
  { \
    QCoreApplication app(argc, argv); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    TestObject##er tc; \
    return QTest::qExec(&tc, argc, argv); \
  }

#endif // QT_GUI_LIB

namespace ctkTest
{
static void mouseEvent(QTest::MouseAction action, QWidget *widget, Qt::MouseButton button,
                       Qt::KeyboardModifiers stateKey, QPoint pos, int delay=-1)
{
  if (action != QTest::MouseMove)
    {
    QTest::mouseEvent(action, widget, button, stateKey, pos, delay);
    return;
    }
  QTEST_ASSERT(widget);
  //extern int Q_TESTLIB_EXPORT defaultMouseDelay();
  //if (delay == -1 || delay < defaultMouseDelay())
  //    delay = defaultMouseDelay();
  if (delay > 0)
      QTest::qWait(delay);

  if (pos.isNull())
      pos = widget->rect().center();

  QTEST_ASSERT(button == Qt::NoButton || button & Qt::MouseButtonMask);
  QTEST_ASSERT(stateKey == 0 || stateKey & Qt::KeyboardModifierMask);

  stateKey &= static_cast<unsigned int>(Qt::KeyboardModifierMask);

  QMouseEvent me(QEvent::User, QPoint(), Qt::LeftButton, button, stateKey);

  me = QMouseEvent(QEvent::MouseMove, pos, widget->mapToGlobal(pos), button, button, stateKey);
  QSpontaneKeyEvent::setSpontaneous(&me);
  if (!qApp->notify(widget, &me))
    {
    static const char *mouseActionNames[] =
        { "MousePress", "MouseRelease", "MouseClick", "MouseDClick", "MouseMove" };
    QString warning = QString::fromLatin1("Mouse event \"%1\" not accepted by receiving widget");
    QTest::qWarn(warning.arg(QString::fromLatin1(mouseActionNames[static_cast<int>(action)])).toLatin1().data());
    }
}

inline void mouseMove(QWidget *widget, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                      QPoint pos = QPoint(), int delay=-1)
  { ctkTest::mouseEvent(QTest::MouseMove, widget, button, stateKey, pos, delay); }

}
