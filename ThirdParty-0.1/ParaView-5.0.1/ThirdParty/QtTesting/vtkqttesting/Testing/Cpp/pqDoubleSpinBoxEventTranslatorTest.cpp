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
#include <QDoubleSpinBox>
#include <QStyle>
#include <QtTest/QtTest>

// QtTesting includes
#include "pqTestUtility.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqDoubleSpinBoxEventTranslatorTester: public QObject
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
  QDoubleSpinBox*     DoubleSpinBox;

  pqTestUtility*      TestUtility;
  pqDummyEventObserver* EventObserver;
};

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::initTestCase()
{
  this->TestUtility = new pqTestUtility();
  this->EventObserver = new pqDummyEventObserver();
  this->TestUtility->addEventObserver("xml", this->EventObserver);
  this->TestUtility->addEventSource("xml", new pqDummyEventSource());

  this->DoubleSpinBox = 0;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::init()
{
  // Init the DoubleSpinBox
  this->DoubleSpinBox = new QDoubleSpinBox();
  this->DoubleSpinBox->setObjectName("doubleSpinBoxTest");
  this->DoubleSpinBox->setMinimum(-100);
  this->DoubleSpinBox->setSingleStep(0.25);

  // Start to record events
  this->TestUtility->recordTestsBySuffix("xml");

  // Fire the event "enter" to connect DoubleSpinBox signals to the translator slots
  QEvent enter(QEvent::Enter);
  qApp->notify(this->DoubleSpinBox, &enter);
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::cleanup()
{
  this->TestUtility->stopRecords(0);
  delete this->DoubleSpinBox;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::cleanupTestCase()
{
  this->EventObserver = 0;

  delete this->TestUtility;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordLeftMouseClick()
{
  QFETCH(int, X);
  QFETCH(int, Y);
  QFETCH(QString, recordEmitted);

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier, QPoint(X,Y));
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordLeftMouseClick_data()
{
  QDoubleSpinBox doubleSpinBox;

  QTest::addColumn<int>("X");
  QTest::addColumn<int>("Y");
  QTest::addColumn<QString>("recordEmitted");

  QSize size = doubleSpinBox.size();
  int frameWidth =
      doubleSpinBox.style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) + 1;

  QTest::newRow("0,0") << 0 << 0 << QString();
  QTest::newRow("invalid") << - size.rwidth()<<  0 << QString();
  QTest::newRow("middle") << size.rwidth()/2 << size.rheight()/2 << QString();
  QTest::newRow("arrow up") << size.rwidth()- frameWidth
                            << frameWidth
                            << QString("doubleSpinBoxTest, set_double, 0.25#");
  QTest::newRow("arrow down") << size.rwidth()- frameWidth
                              << size.rheight() - frameWidth
                              << QString("doubleSpinBoxTest, set_double, -0.25#");
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordRightMouseClick()
{
  QFETCH(int, X);
  QFETCH(int, Y);
  QFETCH(QString, recordEmitted);

  QTest::mouseClick(this->DoubleSpinBox, Qt::RightButton, Qt::NoModifier, QPoint(X,Y));
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordRightMouseClick_data()
{
  QDoubleSpinBox doubleSpinBox;

  QTest::addColumn<int>("X");
  QTest::addColumn<int>("Y");
  QTest::addColumn<QString>("recordEmitted");

  QSize size = doubleSpinBox.size();
  int frameWidth =
      doubleSpinBox.style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) + 1;

  QTest::newRow("0,0") << 0 << 0 << QString();
  QTest::newRow("invalid") << - size.rwidth()<<  0 << QString();
  QTest::newRow("middle") << size.rwidth()/2 << size.rheight()/2 << QString();
  QTest::newRow("arrow up") << size.rwidth()- frameWidth
                            << frameWidth
                            << QString();
  QTest::newRow("arrow down") << size.rwidth()- frameWidth
                              << size.rheight() - frameWidth
                              << QString();
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordMiddleMouseClick()
{
  QFETCH(int, X);
  QFETCH(int, Y);
  QFETCH(QString, recordEmitted);

  QTest::mouseClick(this->DoubleSpinBox, Qt::MiddleButton,
                    Qt::NoModifier, QPoint(X,Y));
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordMiddleMouseClick_data()
{
  this->testRecordRightMouseClick_data();
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordKeyBoardClick()
{
  QFETCH(QString, number);
  QFETCH(QString, recordEmitted);
  this->DoubleSpinBox->clear();

  QTest::keyClicks(this->DoubleSpinBox, number);
  QCOMPARE(this->EventObserver->Text, recordEmitted);

  this->EventObserver->Text = QString();
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordKeyBoardClick_data()
{
  QTest::addColumn<QString>("number");
  QTest::addColumn<QString>("recordEmitted");

  QTest::newRow("33.5") << QString::number(33.5)
                        << QString("%1#%2#%3#")
                           .arg(QString("doubleSpinBoxTest, set_double, 3"),
                                QString("doubleSpinBoxTest, set_double, 33"),
                                QString("doubleSpinBoxTest, set_double, 33.5"));
  QTest::newRow("-5.23") << QString::number(-5.23)
                     << QString("%1#%2#%3#")
                        .arg(QString("doubleSpinBoxTest, set_double, -5"),
                             QString("doubleSpinBoxTest, set_double, -5.2"),
                             QString("doubleSpinBoxTest, set_double, -5.23"));
  QTest::newRow("aa") << QString("aa")
                      << QString();
  QTest::newRow("2.aa") << QString("2.aa")
                        << QString("doubleSpinBoxTest, set_double, 2#");
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslatorTester::testRecordComplexClick()
{
  QSize size = this->DoubleSpinBox->size();
  int frameWidth =
      this->DoubleSpinBox->style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth) + 1;

  QString recordExpected;

  this->DoubleSpinBox->clear();
  QTest::keyClicks(this->DoubleSpinBox, QString::number(-5.25));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5#"));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5.2#"));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5.25#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , size.rheight() - frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5.5#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , size.rheight() - frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5.75#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5.5#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5.25#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -5#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -4.75#"));

  QTest::mouseClick(this->DoubleSpinBox, Qt::LeftButton, Qt::NoModifier,
                    QPoint(size.rwidth()- frameWidth , frameWidth));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, -4.5#"));

  QTest::keyClicks(this->DoubleSpinBox, QString::number(0));
  recordExpected.append(QString("doubleSpinBoxTest, set_double, 0#"));

  QCOMPARE(this->EventObserver->Text, recordExpected);
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN( pqDoubleSpinBoxEventTranslatorTest )
#include "moc_pqDoubleSpinBoxEventTranslatorTest.cpp"
