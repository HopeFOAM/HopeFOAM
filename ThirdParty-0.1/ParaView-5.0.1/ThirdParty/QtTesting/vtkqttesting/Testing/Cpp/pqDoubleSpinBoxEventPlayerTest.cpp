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
class pqDoubleSpinBoxEventPlayerTester: public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

  void testPlayBackCommandSetDouble();
  void testPlayBackCommandSetDouble_data();

private:
  QDoubleSpinBox*     DoubleSpinBox;

  pqTestUtility*      TestUtility;
};

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventPlayerTester::initTestCase()
{
  this->TestUtility = new pqTestUtility();
  this->TestUtility->addEventObserver("xml", new pqDummyEventObserver);
  this->TestUtility->addEventSource("xml", new pqDummyEventSource());

  this->DoubleSpinBox = 0;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventPlayerTester::init()
{
  // Init the QSpinBox
  this->DoubleSpinBox = new QDoubleSpinBox();
  this->DoubleSpinBox->setObjectName("doubleSpinBoxTest");
  this->DoubleSpinBox->setMinimum(-99.99);
  this->DoubleSpinBox->setValue(0);
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventPlayerTester::cleanup()
{
  delete this->DoubleSpinBox;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventPlayerTester::cleanupTestCase()
{
  delete this->TestUtility;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventPlayerTester::testPlayBackCommandSetDouble()
{
  QFETCH(double, value);
  QFETCH(double, result);
  QFETCH(int, count);

  QSignalSpy spy(this->DoubleSpinBox, SIGNAL(valueChanged(double)));

  bool error;
  this->TestUtility->eventPlayer()->playEvent(
      QString("doubleSpinBoxTest"), QString("set_double"),
      QString::number(value), error);

  QCOMPARE(this->DoubleSpinBox->value(), result);
  QCOMPARE(spy.count(), count);
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventPlayerTester::testPlayBackCommandSetDouble_data()
{
  QTest::addColumn<double>("value");
  QTest::addColumn<double>("result");
  QTest::addColumn<int>("count");

  QTest::newRow("negative") << -10.5 << -10.5 << 1;
  QTest::newRow("positive") << 33.25 << 33.25 << 1;
  QTest::newRow("negativeoutside") << -200.00 << -99.99 << 1;
  QTest::newRow("positiveoutside") << 200.00 << 99.99 << 1;
  QTest::newRow("negativeboundary") << -99.99 << -99.99 << 1;
  QTest::newRow("positiveboundary") << 99.99 << 99.99 << 1;
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN( pqDoubleSpinBoxEventPlayerTest )
#include "moc_pqDoubleSpinBoxEventPlayerTest.cpp"
