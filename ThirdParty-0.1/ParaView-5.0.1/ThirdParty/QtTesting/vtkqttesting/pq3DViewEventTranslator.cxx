/*=========================================================================

   Program: ParaView
   Module:    pq3DViewEventTranslator.cxx

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

#include "pq3DViewEventTranslator.h"

#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWidget>

pq3DViewEventTranslator::pq3DViewEventTranslator(const QByteArray& classname, QObject* p)
    : pqWidgetEventTranslator(p), mClassType(classname),
      lastMoveEvent(QEvent::MouseButtonPress, QPoint(), Qt::MouseButton(),
                    Qt::MouseButtons(), Qt::KeyboardModifiers())
{
}

pq3DViewEventTranslator::~pq3DViewEventTranslator()
{
}

bool pq3DViewEventTranslator::translateEvent(QObject* Object, QEvent* Event,
                                           bool& /*Error*/)
{
    QWidget* widget = qobject_cast<QWidget*>(Object);
    if(!widget || !Object->inherits(mClassType.data()))
    {
        return false;
    }

    bool handled = false;
    switch(Event->type())
      {
    case QEvent::ContextMenu:
      handled=true;
      break;

    case QEvent::MouseButtonPress:
        {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(Event);
        if (mouseEvent)
          {
          QSize size = widget->size();
          double normalized_x = mouseEvent->x()/static_cast<double>(size.width());
          double normalized_y = mouseEvent->y()/static_cast<double>(size.height());
          int button = mouseEvent->button();
          int buttons = mouseEvent->buttons();
          int modifiers = mouseEvent->modifiers();
          emit recordEvent(Object, "mousePress", QString("(%1,%2,%3,%4,%5)")
            .arg(normalized_x)
            .arg(normalized_y)
            .arg(button)
            .arg(buttons)
            .arg(modifiers));
          }

        // reset lastMoveEvent
        QMouseEvent e(QEvent::MouseButtonPress, QPoint(), Qt::MouseButton(),
                      Qt::MouseButtons(), Qt::KeyboardModifiers());

        lastMoveEvent = e;
        }
      handled = true;
      break;

      case QEvent::MouseMove:
      {
          QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(Event);
          if (mouseEvent)
            {
            QMouseEvent e(QEvent::MouseMove, QPoint(mouseEvent->x(), mouseEvent->y()),
                          mouseEvent->button(), mouseEvent->buttons(),
                          mouseEvent->modifiers());

            lastMoveEvent = e;
        }
      }
      handled = true;
      break;

    case QEvent::MouseButtonRelease:
        {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(Event);
        if (mouseEvent)
          {
            QSize size = widget->size();

            // record last move event if it is valid
            if(lastMoveEvent.type() == QEvent::MouseMove)
            {
                double normalized_x = lastMoveEvent.x()/static_cast<double>(size.width());
                double normalized_y = lastMoveEvent.y()/static_cast<double>(size.height());
                int button = lastMoveEvent.button();
                int buttons = lastMoveEvent.buttons();
                int modifiers = lastMoveEvent.modifiers();

                emit recordEvent(Object, "mouseMove", QString("(%1,%2,%3,%4,%5)")
                  .arg(normalized_x)
                  .arg(normalized_y)
                  .arg(button)
                  .arg(buttons)
                  .arg(modifiers));
            }


          double normalized_x = mouseEvent->x()/static_cast<double>(size.width());
          double normalized_y = mouseEvent->y()/static_cast<double>(size.height());
          int button = mouseEvent->button();
          int buttons = mouseEvent->buttons();
          int modifiers = mouseEvent->modifiers();

          emit recordEvent(Object, "mouseRelease", QString("(%1,%2,%3,%4,%5)")
            .arg(normalized_x)
            .arg(normalized_y)
            .arg(button)
            .arg(buttons)
            .arg(modifiers));
          }
        }
      handled = true;
      break;

    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      {
      QKeyEvent* ke = static_cast<QKeyEvent*>(Event);
      QString data =QString("%1:%2:%3:%4:%5:%6")
        .arg(ke->type())
        .arg(ke->key())
        .arg(static_cast<int>(ke->modifiers()))
        .arg(ke->text())
        .arg(ke->isAutoRepeat())
        .arg(ke->count());
      emit recordEvent(Object, "keyEvent", data);
      }
      break;

    default:
      break;
      }

    return handled;

}
