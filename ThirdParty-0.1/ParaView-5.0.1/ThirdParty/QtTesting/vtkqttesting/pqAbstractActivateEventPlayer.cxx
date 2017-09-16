/*=========================================================================

   Program: ParaView
   Module:    pqAbstractActivateEventPlayer.cxx

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

#include "pqAbstractActivateEventPlayer.h"

#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QtDebug>
#include <QToolButton>

#include "pqEventDispatcher.h"

pqAbstractActivateEventPlayer::pqAbstractActivateEventPlayer(QObject * p)
  : pqWidgetEventPlayer(p)
{
}

bool pqAbstractActivateEventPlayer::playEvent(QObject* Object,
                                              const QString& Command,
                                              const QString& Arguments,
                                              bool& Error)
{
    if(Command != "activate" && Command != "longActivate")
      return false;

  if (QMenuBar* const menu_bar  = qobject_cast<QMenuBar*>(Object))
    {
    QAction* action = findAction(menu_bar, Arguments);
    if (action)
      {
      menu_bar->setActiveAction(action);
      return true;
      }

    qCritical() << "couldn't find action " << Arguments;
    Error = true;
    return true;
    }

  if(QMenu* const object = qobject_cast<QMenu*>(Object))
    {
    QAction* action = findAction(object, Arguments);

    if(!action)
      {
      qCritical() << "couldn't find action " << Arguments;
      Error = true;
      return true;
      }

    // get a list of menus that must be navigated to
    // click on the action
    QObjectList menus;
    for(QObject* p = object;
        qobject_cast<QMenu*>(p) || qobject_cast<QMenuBar*>(p);
        p = p->parent())
      {
      menus.push_front(p);
      }

    // unfold menus to make action visible
    int i;
    int numMenus = menus.size() - 1;
    for(i=0; i < numMenus; ++i)
      {
      QObject* p = menus[i];
      QMenu* next = qobject_cast<QMenu*>(menus[i+1]);
      if(QMenuBar* menu_bar = qobject_cast<QMenuBar*>(p))
        {
        menu_bar->setActiveAction(next->menuAction());
        int max_wait = 0;
        while(!next->isVisible() && (++max_wait) <= 2)
          {
          pqEventDispatcher::processEventsAndWait(100);
          }
        }
      else if(QMenu* menu = qobject_cast<QMenu*>(p))
        {
        menu->setActiveAction(next->menuAction());
        int max_wait = 0;
        while(!next->isVisible() && (++max_wait) <= 2)
          {
          pqEventDispatcher::processEventsAndWait(100);
          }
        }
      }

    // set active action, will cause scrollable menus to scroll
    // to make action visible
    object->setActiveAction(action);

    // activate the action item
    QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    QKeyEvent keyUp(QEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);

    qApp->notify(object, &keyDown);
    qApp->notify(object, &keyUp);

    //QApplication::processEvents();
    return true;
    }

  if(QAbstractButton* const object = qobject_cast<QAbstractButton*>(Object))
    {
    if (Command == "activate")
      {
      object->click();
      //QApplication::processEvents();
      return true;
      }
    if(Command == "longActivate")
      {
      QToolButton* tButton = qobject_cast<QToolButton*>(Object);
      if(tButton)
        {
        tButton->showMenu();
        return true;
        }
      }
    }

  if (QAction* const action = qobject_cast<QAction*>(Object))
    {
    action->activate(QAction::Trigger);
    //QApplication::processEvents();
    return true;
    }

  qCritical() << "calling activate on unhandled type " << Object;
  Error = true;
  return true;
}

QAction* pqAbstractActivateEventPlayer::findAction(QMenuBar* p, const QString& name)
{
  QList<QAction*> actions = p->actions();
  QAction* action = NULL;
  foreach(QAction* a, actions)
    {
    if(a->menu()->objectName() == name)
      {
      action = a;
      break;
      }
    }

  if(!action)
    {
    foreach(QAction* a, actions)
      {
      if(a->text() == name)
        {
        action = a;
        break;
        }
      }
    }

  return action;
}

QAction* pqAbstractActivateEventPlayer::findAction(QMenu* p, const QString& name)
{
  QList<QAction*> actions = p->actions();
  QAction* action = NULL;
  foreach(QAction* a, actions)
    {
    if(a->objectName() == name)
      {
      action = a;
      break;
      }
    }

  if(!action)
    {
    foreach(QAction* a, actions)
      {
      if(a->text() == name)
        {
        action = a;
        break;
        }
      }
    }

  return action;
}
