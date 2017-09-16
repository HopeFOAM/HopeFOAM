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
#include <QStyle>
#include <QtTest/QtTest>

#include <QTemplate>

// QtTesting includes
#include "pqTestUtility.h"

#include "pqTest.h"

// ----------------------------------------------------------------------------
class pqTemplateEventTranslatorTester: public QObject
{
  Q_OBJECT

private Q_SLOTS:

  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

private:
  QTemplate*            Template;

  pqTestUtility*        TestUtility;
  pqDummyEventObserver* EventObserver;
};

// ----------------------------------------------------------------------------
void pqTemplateEventTranslatorTester::initTestCase()
{
  this->TestUtility = new pqTestUtility();
  this->EventObserver = new pqDummyEventObserver();
  this->TestUtility->addEventObserver("xml", this->EventObserver);
  this->TestUtility->addEventSource("xml", new pqDummyEventSource());

  this->Template = 0;
}

// ----------------------------------------------------------------------------
void pqTemplateEventTranslatorTester::init()
{
  // Init the Template
  this->Template = new QTemplate();
  this->Template->setObjectName("TemplateTest");

  // Start to record events
  this->TestUtility->recordTestsBySuffix("xml");

  // Fire the event "enter" to connect Template signals to the translator slots
  QEvent enter(QEvent::Enter);
  qApp->notify(this->Template, &enter);
}

// ----------------------------------------------------------------------------
void pqTemplateEventTranslatorTester::cleanup()
{
  this->TestUtility->stopRecords(0);
  delete this->Template;
}

// ----------------------------------------------------------------------------
void pqTemplateEventTranslatorTester::cleanupTestCase()
{
  this->EventObserver = 0;

  delete this->TestUtility;
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN( pqTemplateEventTranslatorTest )
#include "moc_pqTemplateEventTranslatorTest.cpp"
