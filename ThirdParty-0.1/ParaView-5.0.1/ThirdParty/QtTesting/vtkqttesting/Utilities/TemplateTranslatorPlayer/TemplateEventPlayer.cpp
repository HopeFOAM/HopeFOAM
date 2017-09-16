// Qt includes
#include <QDebug>

#include "Template.h"
#include "TemplateEventPlayer.h"

// ----------------------------------------------------------------------------
TemplateEventPlayer::TemplateEventPlayer(QObject *parent)
  : pqWidgetEventPlayer(parent)
{
}

// ----------------------------------------------------------------------------
bool TemplateEventPlayer::playEvent(QObject *Object,
                                    const QString &Command,
                                    const QString &Arguments,
                                    bool &Error)
{
  if(Command != "")
    {
    return false;
    }

  if(Template* const object = 
       qobject_cast<Template*>(Object))
    {
    if(Command == "")
      {
      // Edit what the widget "object" should do with this command and argument
      return true;
      }
    }

  qCritical() << "calling ....'your commands'..... on unhandled type " << Object;
  Error = true;
  return true;
}

