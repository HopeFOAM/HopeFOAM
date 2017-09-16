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
#include <QApplication>
#include <QSpinBox>
#include <QStyle>
#include <QtTest/QtTest>

// QtTesting includes
#include "pqTestUtility.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqSpinBoxEventTranslatorTester: public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

  void testRecordLeftMouseClick();
  void testRecordLeftMouseClick_data();

  void testRecordRightMouseClick();
  void testRecordRightMouseClick_data();

  void testRecordMiddleMouseClick();
  void testRecordMiddleMouseClick_data();

  void testRecordKeyBoardClick();
  void testRecordKeyBoardClick_data();

  void testRecordComplexClick();

private:
  QSpinBox*           SpinBox;

  pqTestUtility*      TestUtility;
  pqDummyEventObserver* EventObserver;
};

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::initTestCase()
{
  this->TestUtility = new pqTestUtility();
  this->EventObserver = new pqDummyEventObserver();
  this->TestUtility->addEventObserver("xml", this->EventObserver);
  this->TestUtility->addEventSource("xml", new pqDummyEventSource());

  this->SpinBox = 0;
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::init()
{
  // Init SpinBox
  this->SpinBox = new QSpinBox();
  this->SpinBox->setObjectName("spinBoxTest");
  this->SpinBox->setMinimum(-100);

  // Start to record events
  this->TestUtility->recordTestsBySuffix("xml");

  // Fire the event "enter" to connect spinbox signals to the translator slots
  QEvent enter(QEvent::Enter);
  qApp->notify(this->SpinBox, &enter);
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::cleanup()
{
  this->TestUtility->stopRecords(0);
  delete this->SpinBox;
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::cleanupTestCase()
{
  this->EventObserver = 0;
  delete this->TestUtility;
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordLeftMouseClick()
{
  QFETCH(int, X);
  QFETCH(int, Y);
  QFETCH(QString, recordEmitted);

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier, QPoint(X,Y));
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordLeftMouseClick_data()
{
  QSpinBox spinBox;

  QTest::addColumn<int>("X");
  QTest::addColumn<int>("Y");
  QTest::addColumn<QString>("recordEmitted");

  QSize size = spinBox.size();
  int frameWidth =
      spinBox.style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) + 1;

  QTest::newRow("0,0") << 0 << 0 << QString();
  QTest::newRow("invalid") << - size.rwidth() <<  frameWidth << QString();
  QTest::newRow("middle") << size.rwidth() / 2 << size.rheight() / 2
                          << QString();
  QTest::newRow("arrow up") << size.rwidth()- frameWidth
                            << frameWidth
                            << QString("spinBoxTest, set_int, 1#");
  QTest::newRow("arrow down") << size.rwidth() - frameWidth
                              << size.rheight() - frameWidth
                              << QString("spinBoxTest, set_int, -1#");
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordRightMouseClick()
{
  QFETCH(int, X);
  QFETCH(int, Y);
  QFETCH(QString, recordEmitted);

  QTest::mouseClick(this->SpinBox, Qt::RightButton, Qt::NoModifier, QPoint(X,Y));
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordRightMouseClick_data()
{
  QSpinBox spinBox;

  QTest::addColumn<int>("X");
  QTest::addColumn<int>("Y");
  QTest::addColumn<QString>("recordEmitted");

  QSize size = spinBox.size();
  int frameWidth =
      spinBox.style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) + 1;

  QTest::newRow("0,0") << 0 << 0 << QString();
  QTest::newRow("invalid") << - size.rwidth() <<  frameWidth << QString();
  QTest::newRow("middle") << size.rwidth() / 2 << size.rheight() / 2
                          << QString();
  QTest::newRow("arrow down") << size.rwidth() - frameWidth
                              << size.rheight() - frameWidth
                              << QString();
  QTest::newRow("arrow up") << size.rwidth() - frameWidth
                            << frameWidth
                            << QString();
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordMiddleMouseClick()
{
  QFETCH(int, X);
  QFETCH(int, Y);
  QFETCH(QString, recordEmitted);

  QTest::mouseClick(this->SpinBox, Qt::MiddleButton, Qt::NoModifier, QPoint(X,Y));
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordMiddleMouseClick_data()
{
  this->testRecordRightMouseClick_data();
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordKeyBoardClick()
{
  QFETCH(QString, number);
  QFETCH(QString, recordEmitted);
  this->SpinBox->clear();

  QTest::keyClicks(this->SpinBox, number);
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordKeyBoardClick_data()
{
  QTest::addColumn<QString>("number");
  QTest::addColumn<QString>("recordEmitted");

  QTest::newRow("invalid") << QString("aa") << QString();
  QTest::newRow("valid & invalid") << QString("2.aa") << QString("spinBoxTest, set_int, 2#");
  QTest::newRow("valid positif") << QString::number(33) << QString("spinBoxTest, set_int, 3#spinBoxTest, set_int, 33#");
  QTest::newRow("valid negatif") << QString::number(-5) << QString("spinBoxTest, set_int, -5#");
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventTranslatorTester::testRecordComplexClick()
{
  QSize size = this->SpinBox->size();
  int frameWidth =
      this->SpinBox->style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) + 1;

  QString recordExpected;

  QTest::keyClicks(this->SpinBox, "10");
  recordExpected.append(QString("spinBoxTest, set_int, 1#"));
  recordExpected.append(QString("spinBoxTest, set_int, 10#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , size.rheight() - frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 9#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , size.rheight() - frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 8#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 9#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 10#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 11#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 12#"));

  QTest::mouseClick(this->SpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("spinBoxTest, set_int, 13#"));

  QTest::keyClicks(this->SpinBox, "0");
  recordExpected.append(QString("spinBoxTest, set_int, 0#"));

  QCOMPARE(this->EventObserver->Text, recordExpected);
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN( pqSpinBoxEventTranslatorTest )
#include "moc_pqSpinBoxEventTranslatorTest.cpp"
