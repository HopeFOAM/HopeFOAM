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
#include <QtTest/QtTest>

// QtTesting includes
#include "pqTestUtility.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqSpinBoxEventPlayerTester: public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

  void testPlayBackCommandSetInt();
  void testPlayBackCommandSetInt_data();

private:
  QSpinBox*         SpinBox;

  pqTestUtility*    TestUtility;
};

// ----------------------------------------------------------------------------
void pqSpinBoxEventPlayerTester::initTestCase()
{
  this->TestUtility = new pqTestUtility();
  this->TestUtility->addEventObserver("xml", new pqDummyEventObserver);
  this->TestUtility->addEventSource("xml", new pqDummyEventSource());

  this->SpinBox = 0;
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventPlayerTester::init()
{
  // Init the QSpinBox
  this->SpinBox = new QSpinBox();
  this->SpinBox->setObjectName("spinBoxTest");
  this->SpinBox->setMinimum(-99);
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventPlayerTester::cleanup()
{
  delete this->SpinBox;
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventPlayerTester::cleanupTestCase()
{
  delete this->TestUtility;
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventPlayerTester::testPlayBackCommandSetInt()
{
  QFETCH(int, value);
  QFETCH(int, result);
  QFETCH(int, count);

  QSignalSpy spy(this->SpinBox, SIGNAL(valueChanged(int)));

  bool error;
  this->TestUtility->eventPlayer()->playEvent(
      QString("spinBoxTest"), QString("set_int"), QString::number(value), error);

  QCOMPARE(this->SpinBox->value(), result);
  QCOMPARE(spy.count(), count);
}

// ----------------------------------------------------------------------------
void pqSpinBoxEventPlayerTester::testPlayBackCommandSetInt_data()
{
  QTest::addColumn<int>("value");
  QTest::addColumn<int>("result");
  QTest::addColumn<int>("count");

  QTest::newRow("positive") << 10 << 10 << 1;
  QTest::newRow("negative") << -33 << -33 << 1;
  QTest::newRow("negativeoutside") << -200 << -99 << 1;
  QTest::newRow("positiveoutside") << 200 << 99 << 1;
  QTest::newRow("negativeboundary") << -99 << -99 << 1;
  QTest::newRow("positiveboundary") << 99 << 99 << 1;
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN( pqSpinBoxEventPlayerTest )
#include "moc_pqSpinBoxEventPlayerTest.cpp"
