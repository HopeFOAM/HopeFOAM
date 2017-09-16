/*=========================================================================

   Program: ParaView
   Module:    pqDoubleSpinBoxEventTranslator.cxx

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

#include "pqDoubleSpinBoxEventTranslator.h"

#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QStyle>
#include <QStyleOptionSpinBox>

// ----------------------------------------------------------------------------
pqDoubleSpinBoxEventTranslator::pqDoubleSpinBoxEventTranslator(QObject* p)
  : pqWidgetEventTranslator(p),
  CurrentObject(0)
{
}

// ----------------------------------------------------------------------------
bool pqDoubleSpinBoxEventTranslator::translateEvent(QObject* Object, QEvent* Event, bool& /*Error*/)
{
  QDoubleSpinBox* const object = qobject_cast<QDoubleSpinBox*>(Object);
  
  // consume line edit events if part of spin box
  if(!object && qobject_cast<QDoubleSpinBox*>(Object->parent()))
    {
    return true;
    }
  
  if(!object)
    return false;
    
  if(Event->type() == QEvent::Enter && Object==object)
    {
    if(this->CurrentObject != Object)
      {
      if(this->CurrentObject)
        {
        disconnect(this->CurrentObject, 0, this, 0);
        }

      this->CurrentObject = Object;
      this->Value = object->value();
      connect(object, SIGNAL(valueChanged(double)),this, SLOT(onValueChanged(double)));
      connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyed(QObject*)));
      }
    }

  if(Event->type() == QEvent::KeyRelease && Object==object)
    {
    QKeyEvent* ke = static_cast<QKeyEvent*>(Event);
    QString keyText = ke->text();
    if(keyText.length() && keyText.at(0).isLetterOrNumber())
      {
      emit recordEvent(object, "set_double", QString("%1").arg(object->value()));
      }
    else
      {
      emit recordEvent(object, "key", QString("%1").arg(ke->key()));
      }
    }

  return true;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslator::onDestroyed(QObject* /*Object*/)
{
  this->CurrentObject = 0;
}

// ----------------------------------------------------------------------------
void pqDoubleSpinBoxEventTranslator::onValueChanged(double number)
{
  emit recordEvent(this->CurrentObject, "set_double", QString("%1")
                   .arg(number));
}
