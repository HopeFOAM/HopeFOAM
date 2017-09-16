// Qt includes
#include <QEvent>

#include "Template.h"
#include "TemplateEventTranslator.h"

// ----------------------------------------------------------------------------
TemplateEventTranslator::TemplateEventTranslator(QObject *parent)
  : pqWidgetEventTranslator(parent)
{
  this->CurrentObject = 0;
}

// ----------------------------------------------------------------------------
bool TemplateEventTranslator::translateEvent(QObject *Object,
                                             QEvent *Event,
                                             bool &Error)
{
  Q_UNUSED(Error);
  
  Template* object = qobject_cast<Template*>(Object);
  if(!object)
    {
    return false;
    }

  if(Event->type() == QEvent::Enter && Object == object)
    {
    if(this->CurrentObject != Object)
      {
      if(this->CurrentObject)
        {
        disconnect(this->CurrentObject, 0, this, 0);
        }
      this->CurrentObject = Object;

      connect(object, SIGNAL(destroyed(QObject*)),
              this, SLOT(onDestroyed()));
      // Add your connect and edit the slots functions associated
      }
    }

  return true;
}

// ----------------------------------------------------------------------------
void TemplateEventTranslator::onDestroyed()
{
  if(this->CurrentObject)
    {
    this->CurrentObject = 0;
    }
}

