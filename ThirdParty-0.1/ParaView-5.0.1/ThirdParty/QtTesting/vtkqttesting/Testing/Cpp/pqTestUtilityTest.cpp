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
#include <QLineEdit>
#include <QMap>

// QtTesting includes
#include "pqEventObserver.h"
#include "pqEventSource.h"
#include "pqTestUtility.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqTestUtilityTester : public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void testDefaults();

  void testAddEventSource();
  void testAddEventSource_data();

  void testAddEventObserver();
  void testAddEventObserver_data();

  void testAddObjectStateProperty();
  void testAddObjectStateProperty_data();

//  void testRemoveObjectStateProperty();
//  void testRemoveObjectStateProperty_data();

  void testAddDataDirectory();
  void testAddDataDirectory_data();

  void testRemoveDataDirectory();
  void testRemoveDataDirectory_data();

  void testConvertToDataDirectory();
  void testConvertToDataDirectory_data();

  void testConvertFromDataDirectory();
  void testConvertFromDataDirectory_data();
};

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testDefaults()
{
  pqTestUtility testUtility;

  QCOMPARE(testUtility.playingTest(), false);
  QCOMPARE(testUtility.eventSources().count(), 0);
  QCOMPARE(testUtility.eventObservers().count(), 0);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddEventSource()
{
  pqTestUtility testUtility;

  QFETCH(QString, extension1);
  QFETCH(QObject*, source1);
  QFETCH(int, newCount1);
  QFETCH(QString, extension2);
  QFETCH(QObject*, source2);
  QFETCH(int, newCount2);

  testUtility.addEventSource(extension1, dynamic_cast<pqEventSource*>(source1));
  QCOMPARE(testUtility.eventSources().count(), newCount1);
  testUtility.addEventSource(extension2, dynamic_cast<pqEventSource*>(source2));
  QCOMPARE(testUtility.eventSources().count(), newCount2);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddEventSource_data()
{
  QTest::addColumn<QString>("extension1");
  QTest::addColumn<QObject*>("source1");
  QTest::addColumn<int>("newCount1");
  QTest::addColumn<QString>("extension2");
  QTest::addColumn<QObject*>("source2");
  QTest::addColumn<int>("newCount2");

  pqDummyEventSource* nullSource = NULL;
  pqDummyEventSource* dummySource1 = new pqDummyEventSource();
  pqDummyEventSource* dummySource2 = new pqDummyEventSource();
  pqDummyEventSource* dummySource3 = new pqDummyEventSource();
  pqDummyEventSource* dummySource4 = new pqDummyEventSource();
  pqDummyEventSource* dummySource5 = new pqDummyEventSource();
  QTest::newRow("null") << QString("")
                        << qobject_cast<QObject*>(nullSource) << 0
                        << QString("null")
                        << qobject_cast<QObject*>(nullSource) << 0;
  QTest::newRow("same") << QString("py")
                        << qobject_cast<QObject*>(dummySource1) << 1
                        << QString("py")
                        << qobject_cast<QObject*>(dummySource1) << 1;
  QTest::newRow("same") << QString("py")
                        << qobject_cast<QObject*>(dummySource2) << 1
                        << QString("py")
                        << qobject_cast<QObject*>(dummySource3) << 1;
  QTest::newRow("diff") << QString("py")
                        << qobject_cast<QObject*>(dummySource4) << 1
                        << QString("xml")
                        << qobject_cast<QObject*>(dummySource5) << 2;
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddEventObserver()
{
  pqTestUtility testUtility;

  QFETCH(QString, extension1);
  QFETCH(QObject*, observer1);
  QFETCH(int, newCount1);
  QFETCH(QString, extension2);
  QFETCH(QObject*, observer2);
  QFETCH(int, newCount2);

  testUtility.addEventObserver(extension1, dynamic_cast<pqEventObserver*>(observer1));
  QCOMPARE(testUtility.eventObservers().count(), newCount1);
  testUtility.addEventObserver(extension2, dynamic_cast<pqEventObserver*>(observer2));
  QCOMPARE(testUtility.eventObservers().count(), newCount2);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddEventObserver_data()
{
  QTest::addColumn<QString>("extension1");
  QTest::addColumn<QObject*>("observer1");
  QTest::addColumn<int>("newCount1");
  QTest::addColumn<QString>("extension2");
  QTest::addColumn<QObject*>("observer2");
  QTest::addColumn<int>("newCount2");

  pqDummyEventObserver* nullObserver = NULL;
  pqDummyEventObserver* dummyObserver1 = new pqDummyEventObserver();
  pqDummyEventObserver* dummyObserver2 = new pqDummyEventObserver();
  pqDummyEventObserver* dummyObserver3 = new pqDummyEventObserver();
  pqDummyEventObserver* dummyObserver4 = new pqDummyEventObserver();
  pqDummyEventObserver* dummyObserver5 = new pqDummyEventObserver();
  QTest::newRow("null") << QString("")
                        << qobject_cast<QObject*>(nullObserver) << 0
                        << QString("null")
                        << qobject_cast<QObject*>(nullObserver) << 0;
  QTest::newRow("all_same") << QString("py")
                            << qobject_cast<QObject*>(dummyObserver1) << 1
                            << QString("py")
                            << qobject_cast<QObject*>(dummyObserver1) << 1;
  QTest::newRow("only_same_key") << QString("py")
                                 << qobject_cast<QObject*>(dummyObserver2) << 1
                                 << QString("py")
                                 << qobject_cast<QObject*>(dummyObserver3) << 1;
  QTest::newRow("diff") << QString("py")
                        << qobject_cast<QObject*>(dummyObserver4) << 1
                        << QString("xml")
                        << qobject_cast<QObject*>(dummyObserver5) << 2;
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddObjectStateProperty()
{
  pqTestUtility testUtility;

  QFETCH(QObject*, object1);
  QFETCH(QString, property1);
  QFETCH(int, objectCount1);
  QFETCH(int, propertyCount1);
  QFETCH(QObject*, object2);
  QFETCH(QString, property2);
  QFETCH(int, objectCount2);
  QFETCH(int, propertyCount2);

  testUtility.addObjectStateProperty(object1, property1);
  int nbObject =  testUtility.objectStateProperty().count();
  QCOMPARE(nbObject, objectCount1);
  if (nbObject > 0)
    {
    int nbProperty1 = 0;
    foreach (QStringList list, testUtility.objectStateProperty().values())
      {
      nbProperty1 += list.count();
      }
    QCOMPARE(nbProperty1, propertyCount1);
    }
  testUtility.addObjectStateProperty(object2, property2);
  nbObject =  testUtility.objectStateProperty().count();
  QCOMPARE(nbObject, objectCount2);
  if (nbObject > 0)
    {
    int nbProperty2 = 0;
    foreach (QStringList list, testUtility.objectStateProperty().values())
      {
      nbProperty2 += list.count();
      }
    QCOMPARE(nbProperty2, propertyCount2);
    }
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddObjectStateProperty_data()
{
  QTest::addColumn<QObject*>("object1");
  QTest::addColumn<QString>("property1");
  QTest::addColumn<int>("objectCount1");
  QTest::addColumn<int>("propertyCount1");
  QTest::addColumn<QObject*>("object2");
  QTest::addColumn<QString>("property2");
  QTest::addColumn<int>("objectCount2");
  QTest::addColumn<int>("propertyCount2");

  QObject* nullObject = NULL;
  QLineEdit* lineEdit1 = new QLineEdit();
  QLineEdit* lineEdit2 = new QLineEdit();
  QLineEdit* lineEdit3 = new QLineEdit();
  QLineEdit* lineEdit4 = new QLineEdit();
  QLineEdit* lineEdit5 = new QLineEdit();

  QTest::newRow("objectNull") << nullObject
                              << QString("") << 0 << 0
                              << nullObject
                              << QString("maxLength") << 0 << 0;
  QTest::newRow("wrongProperty") << qobject_cast<QObject*>(lineEdit1)
                                 << QString("") << 0 << 0
                                 << qobject_cast<QObject*>(lineEdit1)
                                 << QString("wrongProperty") << 0 << 0;
  QTest::newRow("sameProperty") << qobject_cast<QObject*>(lineEdit2)
                                << QString("maxLength") << 1 << 1
                                << qobject_cast<QObject*>(lineEdit2)
                                << QString("maxLength") << 1 << 1;
  QTest::newRow("diffProperty") << qobject_cast<QObject*>(lineEdit3)
                                << QString("maxLength") << 1 << 1
                                << qobject_cast<QObject*>(lineEdit3)
                                << QString("readOnly") << 1 << 2;
  QTest::newRow("diffObject") << qobject_cast<QObject*>(lineEdit4)
                              << QString("maxLength") << 1 << 1
                              << qobject_cast<QObject*>(lineEdit5)
                              << QString("maxLength") << 2 << 2;
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddDataDirectory()
{
  pqTestUtility testUtility;

  QFETCH(QString, label1);
  QFETCH(QString, path1);
  QFETCH(int, result1);
  QFETCH(QString, label2);
  QFETCH(QString, path2);
  QFETCH(int, result2);

  testUtility.addDataDirectory(label1, QDir(path1));
  QCOMPARE(testUtility.dataDirectory().count(), result1);

  testUtility.addDataDirectory(label2, QDir(path2));
  QCOMPARE(testUtility.dataDirectory().count(), result2);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testAddDataDirectory_data()
{
  QTest::addColumn<QString>("label1");
  QTest::addColumn<QString>("path1");
  QTest::addColumn<int>("result1");
  QTest::addColumn<QString>("label2");
  QTest::addColumn<QString>("path2");
  QTest::addColumn<int>("result2");

  QTest::newRow("empty") << QString() << QString() << 0
                         << QString("label") << QString("") << 0 ;
  QTest::newRow("allSame") << QString("TEMPLATE") << QString("/home/") << 1
                           << QString("TEMPLATE") << QString("/home/") << 1;
  QTest::newRow("sameLabel") << QString("TEMPLATE") << QString("/home/") << 1
                             << QString("TEMPLATE") << QString("/home/BenLg") << 1;
  QTest::newRow("samePath") << QString("TEMPLATE") << QString("/home/") << 1
                            << QString("TEMP") << QString("/home/") << 2;
  QTest::newRow("diff") << QString("TEMPLATE") << QString("/home/") << 1
                        << QString("TEMP") << QString("/home/BenLg") << 2;
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testRemoveDataDirectory()
{
  pqTestUtility testUtility;

  QFETCH(QString, label);
  QFETCH(bool, result);

  testUtility.addDataDirectory(QString("TEMPLATE"), QDir("/home"));
  QCOMPARE(testUtility.removeDataDirectory(label), result);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testRemoveDataDirectory_data()
{
  QTest::addColumn<QString>("label");
  QTest::addColumn<bool>("result");

  QTest::newRow("empty") << QString() << false;
  QTest::newRow("wrong") << QString("TEMP") << false;
  QTest::newRow("right") << QString("TEMPLATE") << true;
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testConvertToDataDirectory()
{
  pqTestUtility testUtility;

  QFETCH(QString, label1);
  QFETCH(QString, dir1);
  QFETCH(QString, label2);
  QFETCH(QString, dir2);

  testUtility.addDataDirectory(QString(label1), QDir(dir1));
  testUtility.addDataDirectory(QString(label2), QDir(dir2));

  QFETCH(QString, path);
  QFETCH(QString, result);

  QString convertPath = testUtility.convertToDataDirectory(path);
  QCOMPARE(convertPath, result);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testConvertToDataDirectory_data()
{
  QTest::addColumn<QString>("label1");
  QTest::addColumn<QString>("dir1");
  QTest::addColumn<QString>("label2");
  QTest::addColumn<QString>("dir2");
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("result");

  QTest::newRow("0") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString()
                     << QString();
  QTest::newRow("1") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/home") << QString("/home");
  QTest::newRow("2") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/home/toto/")
                     << QString("/home/toto/");
  QTest::newRow("3") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/home/BenLg")
                     << QString("${ROOT}/");
  QTest::newRow("3") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/home/BenLg/toto")
                     << QString("${ROOT}/toto");
  QTest::newRow("4") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/home/BenLg/data/toto")
                     << QString("${ROOTDATA}/toto");

  // Same test as the previous one but we inverse the two dataDirectories
  // to be sure that our function chosse the right one.
  QTest::newRow("5") << "ROOTDATA" << "/home/BenLg/data"
                     << "ROOT" << "/home/BenLg"
                     << QString("/home/BenLg/data/toto")
                     << QString("${ROOTDATA}/toto");
  QTest::newRow("6") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/usr/toto/home/BenLg/toto")
                     << QString("/usr/toto/home/BenLg/toto");
  QTest::newRow("7") << "ROOT" << "/home/BenLg"
                     << "ROOTDATA" << "/home/BenLg/data"
                     << QString("/usr/home/BenLg/data/toto")
                     << QString("/usr/home/BenLg/data/toto");
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testConvertFromDataDirectory()
{
  pqTestUtility testUtility;

  testUtility.addDataDirectory(QString("ROOT"), QDir("/home/BenLg"));
  testUtility.addDataDirectory(QString("ROOTDATA"), QDir("/home/BenLg/data"));

  QFETCH(QString, path);
  QFETCH(QString, result);

  QString convertPath = testUtility.convertFromDataDirectory(path);
  QCOMPARE(convertPath, result);
}

// ----------------------------------------------------------------------------
void pqTestUtilityTester::testConvertFromDataDirectory_data()
{
  QTest::addColumn<QString>("path");
  QTest::addColumn<QString>("result");

  QTest::newRow("0") << QString("") << QString();
  QTest::newRow("1") << QString("/home/guest/data") << QString("/home/guest/data");
  QTest::newRow("2") << QString("${WRONG}/home") << QString("${WRONG}/home");
  QTest::newRow("3") << QString("${ROOT}") << QString("/home/BenLg");
  QTest::newRow("4") << QString("${ROOT}/Doc/bin") << QString("/home/BenLg/Doc/bin");
  QTest::newRow("5") << QString("${ROOTDATA}") << QString("/home/BenLg/data");
  QTest::newRow("6") << QString("${ROOTDATA}/Ex1/Patient0")
                     << QString("/home/BenLg/data/Ex1/Patient0");
}


// ----------------------------------------------------------------------------
CTK_TEST_MAIN( pqTestUtilityTest )
#include "moc_pqTestUtilityTest.cpp"
