/*=========================================================================

   Program: ParaView
   Module:    pqEventPlayer.h

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

// QtTesting includes
#include "pqEventTranslator.h"
#include "pqSpinBoxEventTranslator.h"
#include "pqTestUtility.h"
#include "pqTreeViewEventTranslator.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqEventTranslatorTester : public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void testDefaults();

  void testAddWidgetEventTranslator();
  void testAddWidgetEventTranslator_data();

  void testRemoveWidgetEventTranslator();
  void testRemoveWidgetEventTranslator_data();

  void testGetWidgetEventTranslator();

  void testAddDefaultWidgetEventTranslators();
  void testAddDefaultWidgetEventTranslators_data();
};

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testDefaults()
{
  pqEventTranslator eventTranslator;

  QCOMPARE(eventTranslator.translators().count(), 0);
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testAddWidgetEventTranslator()
{
  pqEventTranslator eventTranslator;

  QFETCH(QObject*, widget1);
  QFETCH(int, newCount1);
  QFETCH(QObject*, widget2);
  QFETCH(int, newCount2);

  // When we add the widgetEventPlayer into the eventPlayer, it is automaticaly
  // reparented to the eventPlayer. So its deletion would be automatic.
  eventTranslator.addWidgetEventTranslator(
      dynamic_cast<pqWidgetEventTranslator*>(widget1));
  QCOMPARE(eventTranslator.translators().count(), newCount1);

  eventTranslator.addWidgetEventTranslator(
      dynamic_cast<pqWidgetEventTranslator*>(widget2));
  QCOMPARE(eventTranslator.translators().count(), newCount2);
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testAddWidgetEventTranslator_data()
{
  QTest::addColumn<QObject*>("widget1");
  QTest::addColumn<int>("newCount1");
  QTest::addColumn<QObject*>("widget2");
  QTest::addColumn<int>("newCount2");

  pqWidgetEventTranslator* nullWidget = NULL;
  QTest::newRow("empty_empty")
      << qobject_cast<QObject*>(nullWidget) << 0
      << qobject_cast<QObject*>(nullWidget) << 0;
  QTest::newRow("empty_pqSpinBox")
      << qobject_cast<QObject*>(nullWidget) << 0
      << qobject_cast<QObject*>(new pqSpinBoxEventTranslator()) << 1;
  QTest::newRow("pqSpinBox_pqSpinBox")
      << qobject_cast<QObject*>(new pqSpinBoxEventTranslator()) << 1
      << qobject_cast<QObject*>(new pqSpinBoxEventTranslator()) << 1;
  QTest::newRow("pqSpinBox_pqTreeView")
      << qobject_cast<QObject*>(new pqSpinBoxEventTranslator()) << 1
      << qobject_cast<QObject*>(new pqTreeViewEventTranslator()) << 2;
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testRemoveWidgetEventTranslator()
{
  pqEventTranslator eventTranslator;

  QFETCH(QString, nameToRemove);
  QFETCH(bool, firstResult);
  QFETCH(bool, secondResult);
  QFETCH(int, newCount);
  QFETCH(bool, thirdResult);

  QCOMPARE(eventTranslator.removeWidgetEventTranslator(nameToRemove),
           firstResult);

  // When we add the widgetEventPlayer into the eventPlayer, it is automaticaly
  // reparented to the eventPlayer. So its deletion would be automatic.
  pqTreeViewEventTranslator* treeView = new pqTreeViewEventTranslator();
  eventTranslator.addWidgetEventTranslator(treeView);

  QCOMPARE(eventTranslator.translators().count(), 1);
  QCOMPARE(eventTranslator.removeWidgetEventTranslator(nameToRemove),
           secondResult);
  QCOMPARE(eventTranslator.translators().count(), newCount);
  QCOMPARE(eventTranslator.removeWidgetEventTranslator(nameToRemove),
           thirdResult);
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testRemoveWidgetEventTranslator_data()
{
  QTest::addColumn<QString>("nameToRemove");
  QTest::addColumn<bool>("firstResult");
  QTest::addColumn<bool>("secondResult");
  QTest::addColumn<int>("newCount");
  QTest::addColumn<bool>("thirdResult");

  QTest::newRow("empty") << "" << false << false << 1 << false;
  QTest::newRow("wrong") << "pqSpinBoxEventTranslator" << false << false << 1 << false;
  QTest::newRow("right") << "pqTreeViewEventTranslator" << false << true << 0 << false;
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testGetWidgetEventTranslator()
{
  pqEventTranslator eventTranslator;

  pqSpinBoxEventTranslator* nullWidget = NULL;
  QCOMPARE(eventTranslator.getWidgetEventTranslator(0),
           nullWidget);
  QCOMPARE(eventTranslator.getWidgetEventTranslator("pqSpinBoxEventTranslator"),
           nullWidget);

  // When we add the widgetEventPlayer into the eventPlayer, it is automaticaly
  // reparented to the eventPlayer. So its deletion would be automatic.
  pqSpinBoxEventTranslator* spinBox = new pqSpinBoxEventTranslator();
  eventTranslator.addWidgetEventTranslator(spinBox);

  QCOMPARE(eventTranslator.getWidgetEventTranslator(0),
           nullWidget);
  QCOMPARE(eventTranslator.getWidgetEventTranslator("pqTreeViewEventTranslator"),
           nullWidget);
  QCOMPARE(eventTranslator.getWidgetEventTranslator("pqSpinBoxEventTranslator"),
           spinBox);
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testAddDefaultWidgetEventTranslators()
{
  pqEventTranslator eventTranslator;
  pqTestUtility testUtility;
  eventTranslator.addDefaultWidgetEventTranslators(&testUtility);
  QList<pqWidgetEventTranslator*> translators = eventTranslator.translators();

  QFETCH(int, index);
  QFETCH(QString, widgetEventTranslatorName);

  QCOMPARE(QString(translators.at(index)->metaObject()->className()),
           widgetEventTranslatorName);
}

// ----------------------------------------------------------------------------
void pqEventTranslatorTester::testAddDefaultWidgetEventTranslators_data()
{
  QTest::addColumn<int>("index");
  QTest::addColumn<QString>("widgetEventTranslatorName");

  QTest::newRow("0") << 0 << "pqNativeFileDialogEventTranslator";
  QTest::newRow("1") << 1 << "pq3DViewEventTranslator";
  QTest::newRow("2") << 2 << "pqTreeViewEventTranslator";
  QTest::newRow("3") << 3 << "pqTabBarEventTranslator";
  QTest::newRow("4") << 4 << "pqSpinBoxEventTranslator";
  QTest::newRow("5") << 5 << "pqMenuEventTranslator";
  QTest::newRow("6") << 6 << "pqLineEditEventTranslator";
  QTest::newRow("7") << 7 << "pqDoubleSpinBoxEventTranslator";
  QTest::newRow("8") << 8 << "pqComboBoxEventTranslator";
  QTest::newRow("9") << 9 << "pqAbstractSliderEventTranslator";
  QTest::newRow("10") << 10 << "pqAbstractItemViewEventTranslator";
  QTest::newRow("11") << 11 << "pqAbstractButtonEventTranslator";
  QTest::newRow("12") << 12 << "pqBasicWidgetEventTranslator";
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(pqEventTranslatorTest)
#include "moc_pqEventTranslatorTest.cpp"
