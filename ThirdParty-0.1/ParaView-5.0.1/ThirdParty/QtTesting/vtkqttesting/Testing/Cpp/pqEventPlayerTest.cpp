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
#include "pqCommentEventPlayer.h"
#include "pqEventPlayer.h"
#include "pqTestUtility.h"
#include "pqTreeViewEventPlayer.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqEventPlayerTester: public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void testDefaults();

  void testAddWidgetEventPlayer();
  void testAddWidgetEventPlayer_data();

  void testRemoveWidgetEventPlayer();
  void testRemoveWidgetEventPlayer_data();

  void testGetWidgetEventPlayer();

  void testAddDefaultWidgetEventPlayers();
  void testAddDefaultWidgetEventPlayers_data();
};

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testDefaults()
{
  pqEventPlayer eventPlayer;

  QCOMPARE(eventPlayer.players().count(), 0);
}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testAddWidgetEventPlayer()
{
  pqEventPlayer eventPlayer;

  QFETCH(QObject*, widget1);
  QFETCH(int, newCount1);
  QFETCH(QObject*, widget2);
  QFETCH(int, newCount2);

  // When we add the widgetEventPlayer into the eventPlayer, it is automaticaly
  // reparented to the eventPlayer. So its deletion would be automatic.
  eventPlayer.addWidgetEventPlayer(qobject_cast<pqWidgetEventPlayer*>(widget1));
  QCOMPARE(eventPlayer.players().count(), newCount1);

  eventPlayer.addWidgetEventPlayer(qobject_cast<pqWidgetEventPlayer*>(widget2));
  QCOMPARE(eventPlayer.players().count(), newCount2);
}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testAddWidgetEventPlayer_data()
{
  QTest::addColumn<QObject*>("widget1");
  QTest::addColumn<int>("newCount1");
  QTest::addColumn<QObject*>("widget2");
  QTest::addColumn<int>("newCount2");

  pqWidgetEventPlayer* nullWidget = NULL;
  QTest::newRow("empty_empty")
      << qobject_cast<QObject*>(nullWidget) << 0
      << qobject_cast<QObject*>(nullWidget) << 0;
  QTest::newRow("empty_pqComment")
      << qobject_cast<QObject*>(nullWidget) << 0
      << qobject_cast<QObject*>(new pqCommentEventPlayer(0)) << 1;
  QTest::newRow("pqComment_pqComment")
      << qobject_cast<QObject*>(new pqCommentEventPlayer(0)) << 1
      << qobject_cast<QObject*>(new pqCommentEventPlayer(0)) << 1;
  QTest::newRow("pqComment_pqTreeView")
      << qobject_cast<QObject*>(new pqCommentEventPlayer(0)) << 1
      << qobject_cast<QObject*>(new pqTreeViewEventPlayer()) << 2;
}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testRemoveWidgetEventPlayer()
{
  pqEventPlayer eventPlayer;

  QFETCH(QString, nameToRemove);
  QFETCH(bool, firstResult);
  QFETCH(bool, secondResult);
  QFETCH(int, newCount);
  QFETCH(bool, thirdResult);

  QCOMPARE(eventPlayer.removeWidgetEventPlayer(nameToRemove),
           firstResult);

  // When we add the widgetEventPlayer into the eventPlayer, it is automaticaly
  // reparented to the eventPlayer. So its deletion would be automatic.
  pqCommentEventPlayer* commentPlayer = new pqCommentEventPlayer(0);
  eventPlayer.addWidgetEventPlayer(commentPlayer);

  QCOMPARE(eventPlayer.players().count(), 1);
  QCOMPARE(eventPlayer.removeWidgetEventPlayer(nameToRemove),
           secondResult);
  QCOMPARE(eventPlayer.players().count(), newCount);
  QCOMPARE(eventPlayer.removeWidgetEventPlayer(nameToRemove),
           thirdResult);
}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testRemoveWidgetEventPlayer_data()
{
  QTest::addColumn<QString>("nameToRemove");
  QTest::addColumn<bool>("firstResult");
  QTest::addColumn<bool>("secondResult");
  QTest::addColumn<int>("newCount");
  QTest::addColumn<bool>("thirdResult");

  QTest::newRow("empty") << "" << false << false << 1 << false;
  QTest::newRow("wrong") << "pqTreeViewEventPlayer" << false << false << 1 << false;
  QTest::newRow("right") << "pqCommentEventPlayer" << false << true << 0 << false;
}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testGetWidgetEventPlayer()
{
  pqEventPlayer eventPlayer;

  pqWidgetEventPlayer* nullWidget = NULL;
  QCOMPARE(eventPlayer.getWidgetEventPlayer(0),
           nullWidget);
  QCOMPARE(eventPlayer.getWidgetEventPlayer("pqCommentEventPlayer"),
           nullWidget);

  // When we add the widgetEventPlayer into the eventPlayer, it is automaticaly
  // reparented to the eventPlayer. So its deletion would be automatic.
  pqCommentEventPlayer* comment = new pqCommentEventPlayer(0);
  eventPlayer.addWidgetEventPlayer(comment);

  QCOMPARE(eventPlayer.getWidgetEventPlayer(0),
           nullWidget);
  QCOMPARE(eventPlayer.getWidgetEventPlayer("pqTreeViewEventPlayer"),
           nullWidget);
  QCOMPARE(eventPlayer.getWidgetEventPlayer("pqCommentEventPlayer"),
           comment);

}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testAddDefaultWidgetEventPlayers()
{
  pqEventPlayer eventPlayer;
  pqTestUtility testUtility;

  eventPlayer.addDefaultWidgetEventPlayers(&testUtility);
  QList<pqWidgetEventPlayer*> players = eventPlayer.players();

  QFETCH(QString, widgetEventPlayerName);
  QFETCH(int, index);

  QCOMPARE(QString(players.at(index)->metaObject()->className()),
           widgetEventPlayerName);
}

// ----------------------------------------------------------------------------
void pqEventPlayerTester::testAddDefaultWidgetEventPlayers_data()
{
  QTest::addColumn<int>("index");
  QTest::addColumn<QString>("widgetEventPlayerName");

  QTest::newRow("0") << 0 << "pqNativeFileDialogEventPlayer";
  QTest::newRow("1") << 1 << "pq3DViewEventPlayer";
  QTest::newRow("2") << 2 << "pqAbstractMiscellaneousEventPlayer";
  QTest::newRow("3") << 3 << "pqTreeViewEventPlayer";
  QTest::newRow("4") << 4 << "pqTabBarEventPlayer";
  QTest::newRow("5") << 5 << "pqAbstractStringEventPlayer";
  QTest::newRow("6") << 6 << "pqAbstractItemViewEventPlayer";
  QTest::newRow("7") << 7 << "pqAbstractIntEventPlayer";
  QTest::newRow("8") << 8 << "pqAbstractDoubleEventPlayer";
  QTest::newRow("9") << 9 << "pqAbstractBooleanEventPlayer";
  QTest::newRow("10") << 10 << "pqAbstractActivateEventPlayer";
  QTest::newRow("11") << 11 << "pqBasicWidgetEventPlayer";
  QTest::newRow("12") << 12 << "pqCommentEventPlayer";
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(pqEventPlayerTest)
#include "moc_pqEventPlayerTest.cpp"

