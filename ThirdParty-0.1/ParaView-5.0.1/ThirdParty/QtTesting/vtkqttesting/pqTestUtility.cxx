/*=========================================================================

   Program: ParaView
   Module:    pqTestUtility.cxx

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
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QVariant>

// QtTesting includes
#include "pqEventComment.h"
#include "pqEventObserver.h"
#include "pqEventSource.h"
#include "pqEventTranslator.h"
#include "pqPlayBackEventsDialog.h"
#ifdef QT_TESTING_WITH_PYTHON
#include "pqPythonEventSource.h"
#include "pqPythonEventObserver.h"
#endif
#include "pqRecordEventsDialog.h"
#include "pqTestUtility.h"

#include "QtTestingConfigure.h"


//-----------------------------------------------------------------------------
pqTestUtility::pqTestUtility(QObject* p)
  : QObject(p)
{
  this->PlayingTest = false;

  this->File = 0;
  this->FileSuffix = QString();

  this->Translator.addDefaultWidgetEventTranslators(this);
  this->Translator.addDefaultEventManagers(this);
  this->Player.addDefaultWidgetEventPlayers(this);

#ifdef QT_TESTING_WITH_PYTHON
  // add a python event source
  this->addEventSource("py", new pqPythonEventSource(this));
  this->addEventObserver("py", new pqPythonEventObserver(this));
#endif
}

//-----------------------------------------------------------------------------
pqTestUtility::~pqTestUtility()
{
  this->File = 0;
}
  
//-----------------------------------------------------------------------------
pqEventDispatcher* pqTestUtility::dispatcher()
{
  return &this->Dispatcher;
}

//-----------------------------------------------------------------------------
pqEventRecorder* pqTestUtility::recorder()
{
  return &this->Recorder;
}

//-----------------------------------------------------------------------------
pqEventPlayer* pqTestUtility::eventPlayer()
{
  return &this->Player;
}

//-----------------------------------------------------------------------------
pqEventTranslator* pqTestUtility::eventTranslator()
{
  return &this->Translator;
}

//-----------------------------------------------------------------------------
void pqTestUtility::addEventSource(const QString& fileExtension,
                                   pqEventSource* source)
{
  if (!source)
    {
    return;
    }

  QMap<QString, pqEventSource*>::iterator iter;
  iter = this->EventSources.find(fileExtension);

  if(iter != this->EventSources.end())
    {
    if (iter.value() == source)
      {
      return;
      }
    pqEventSource* src = iter.value();
    this->EventSources.erase(iter);
    delete src;
    }
  this->EventSources.insert(fileExtension, source);
  source->setParent(this);
}

//-----------------------------------------------------------------------------
QMap<QString, pqEventSource*> pqTestUtility::eventSources() const
{
  return this->EventSources;
}

//-----------------------------------------------------------------------------
void pqTestUtility::addEventObserver(const QString& fileExtension,
                                     pqEventObserver* observer)
{
  if (!observer)
    {
    return;
    }

  QMap<QString, pqEventObserver*>::iterator iter;
  iter = this->EventObservers.find(fileExtension);
  if(iter != this->EventObservers.end())
    {
    if (iter.value() == observer)
      {
      return;
      }

    pqEventObserver* src = iter.value();
    this->EventObservers.erase(iter);
    delete src;
    }
  this->EventObservers.insert(fileExtension, observer);
  observer->setParent(this);
}

//-----------------------------------------------------------------------------
QMap<QString, pqEventObserver*> pqTestUtility::eventObservers() const
{
  return this->EventObservers;
}

//-----------------------------------------------------------------------------
void pqTestUtility::openPlayerDialog()
{
  pqPlayBackEventsDialog* dialog =
      new pqPlayBackEventsDialog(this->Player,
                                this->Dispatcher,
                                this,
                                QApplication::activeWindow());
  dialog->show();
}

//-----------------------------------------------------------------------------
void pqTestUtility::stopTests()
{
  this->PlayingTest = false;
  this->Dispatcher.stop();
}

//-----------------------------------------------------------------------------
void pqTestUtility::stopRecords(int value)
{
  this->Recorder.stop(value);
}

//-----------------------------------------------------------------------------
void pqTestUtility::onRecordStopped()
{
  QTemporaryFile* file = qobject_cast<QTemporaryFile*>(this->File);
  if(file)
    {
    QFileDialog* dialog = new QFileDialog(0, tr("Macro File Name"),
                                QString("macro"), tr("XML Files (*.xml)"));
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setDefaultSuffix("xml");
    if (dialog->exec() == QDialog::Rejected)
      {
      return;
      }

    QStringList newFilename = dialog->selectedFiles();
    if (newFilename[0].isEmpty())
      {
      return;
      }
    if (!newFilename[0].endsWith(QString(".%1").arg(this->FileSuffix)))
      {
      newFilename[0] += QString(".%1").arg(this->FileSuffix);
      }
    // QFile::copy doesn't overwrite if the file already exist, and return false
    if(QFile::exists(newFilename[0]))
      {
      QFile::remove(newFilename[0]);
      }
    QFile::copy(file->fileName(), newFilename[0]);
    delete dialog;
    }
  this->File->close();
}

//-----------------------------------------------------------------------------
bool pqTestUtility::playTests(const QString& filename)
{
  QStringList files;
  files << filename;
  return this->playTests(files);
}

//-----------------------------------------------------------------------------
bool pqTestUtility::playTests(const QStringList& filenames)
{
  if (this->PlayingTest)
    {
    qCritical("playTests() cannot be called recursively.");
    return false;
    }

  this->PlayingTest = true;
  emit this->playbackStarted();

  bool success = true;
  foreach (QString filename, filenames)
    {
    if(!this->playingTest())
      {
      break;
      }
    QFileInfo info(filename);
    emit this->playbackStarted(filename);
    QString suffix = info.completeSuffix();
    QMap<QString, pqEventSource*>::iterator iter;
    iter = this->EventSources.find(suffix);
    if(info.isReadable() && iter != this->EventSources.end())
      {
      iter.value()->setContent(filename);

//      QEventLoop loop;
//      QTimer::singleShot(100, &loop, SLOT(quit()));
//      loop.exec();
      QApplication::processEvents();
      if (!this->Dispatcher.playEvents(*iter.value(), this->Player))
        {
        // dispatcher returned failure, don't continue with rest of the tests
        // and flag error.
        success = false;
        emit this->playbackStopped(info.fileName(), success);
        break;
        }
      emit this->playbackStopped(info.fileName(), success);
      qDebug() << "Test " << info.fileName() << "is finished. Success = " << success;
      }
    }

  this->PlayingTest = false;
  emit this->playbackStopped();

  return success;
}

//-----------------------------------------------------------------------------
void pqTestUtility::recordTests()
{
#if defined(Q_WS_MAC)
  // check for native or non-native menu bar.
  // testing framework doesn't work with native menu bar, so let's warn if we
  // get that.
  if(!getenv("QT_MAC_NO_NATIVE_MENUBAR"))
    {
    qWarning("Recording menu events for native Mac menus doesn't work.\n"
             "Set the QT_MAC_NO_NATIVE_MENUBAR environment variable to"
             " correctly record menus");
    }
#endif

  pqEventObserver* observer = this->EventObservers.value(this->FileSuffix);
  if(!observer)
    {
    return;
    }

  if(!this->File->open(QIODevice::WriteOnly))
    {
    qCritical() << "File cannot be opened";
    return;
    }

  QObject::connect(&this->Recorder, SIGNAL(stopped()),
                   this, SLOT(onRecordStopped()), Qt::UniqueConnection);

  if (QApplication::activeWindow())
    {
    pqRecordEventsDialog* dialog = new pqRecordEventsDialog(&this->Recorder,
                                          this,
                                          QApplication::activeWindow());
    dialog->setAttribute(Qt::WA_QuitOnClose, false);

    QRect rectApp = QApplication::activeWindow()->geometry();
    QRect rectDialog(QPoint(rectApp.left(),
                            rectApp.bottom() - dialog->sizeHint().height()),
                     QSize(dialog->geometry().width(), dialog->sizeHint().height()));

    dialog->setGeometry(rectDialog);
    dialog->show();
    }
  else
    {
    qWarning() << "No acive windows has been found";
    }

  this->Recorder.recordEvents(&this->Translator, observer, this->File, true);
}

//-----------------------------------------------------------------------------
void pqTestUtility::recordTests(const QString& filename)
{
  this->File = new QFile(filename);
  QFileInfo info(filename);
  this->FileSuffix = info.completeSuffix();
  this->recordTests();
}

//-----------------------------------------------------------------------------
void pqTestUtility::recordTestsBySuffix(const QString& suffix)
{
  QString tempFilename(QString("%1/macro.%2").arg(QDir::tempPath(), suffix));
  this->File = new QTemporaryFile(tempFilename);
  this->FileSuffix = suffix;
  this->recordTests();
}

// ----------------------------------------------------------------------------
void pqTestUtility::addObjectStateProperty(QObject* object, const QString& property)
{
  if (!object)
    {
    return;
    }
  if (property.isEmpty() ||
      !object->property(property.toLatin1()).isValid())
    {
    return;
    }
  if(this->objectStatePropertyAlreadyAdded(object, property))
    {
    return;
    }

  this->ObjectStateProperty[object].append(property);
}

// ----------------------------------------------------------------------------
bool pqTestUtility::objectStatePropertyAlreadyAdded(QObject* object,
                                                    const QString& property)
{
  // Check if this property already exist for this object
  QMap<QObject*, QStringList>::iterator iter =
      this->ObjectStateProperty.find(object);
  if (iter != this->ObjectStateProperty.end())
    {
    return this->ObjectStateProperty[object].contains(property);
    }

  return false;
}

// ----------------------------------------------------------------------------
QMap<QObject*, QStringList> pqTestUtility::objectStateProperty() const
{
  return this->ObjectStateProperty;
}

//-----------------------------------------------------------------------------
void pqTestUtility::addDataDirectory(const QString& label, const QDir& path)
{
  // To simplify, we allow only absolute path for the moment.
  if (label.isEmpty())
    {
    return;
    }
  // Find better way to check if the path seams to be valid.
  // But as long as we would like to label some path, to allow a cross
  // platform use of QtTesting, a simple path.exist() seams not good.
  // not working !
  if (!path.isAbsolute())
    {
    return;
    }

  this->DataDirectories[label] = path;
}

//-----------------------------------------------------------------------------
bool pqTestUtility::removeDataDirectory(const QString& label)
{
  return this->DataDirectories.remove(label);
}

//-----------------------------------------------------------------------------
QMap<QString, QDir> pqTestUtility::dataDirectory() const
{
  return this->DataDirectories;
}

//-----------------------------------------------------------------------------
QMap<QString, QDir>::iterator pqTestUtility::findBestIterator(const QString& file,
                                               QMap<QString, QDir>::iterator startIter)
{
  QMap<QString, QDir>::iterator currentIter;
  QMap<QString, QDir>::iterator iter;
  int size = file.size();
  bool find = false;

  for(iter = startIter; iter != this->DataDirectories.end() ; ++ iter)
    {
    if (file.startsWith(iter.value().path()))
      {
      QString relativePos = iter.value().relativeFilePath(file);
      if (relativePos.size() < size)
        {
        find = true;
        size = relativePos.size();
        currentIter = iter;
        }
      }
    }

  if (!find)
    {
    return this->DataDirectories.end();
    }
  return currentIter;
}

//-----------------------------------------------------------------------------
QString pqTestUtility::convertToDataDirectory(const QString& file)
{
  QMap<QString, QDir>::iterator iter =
      this->findBestIterator(file, this->DataDirectories.begin());

  // If iterator at the end, we didn't find a match.
  if (iter == this->DataDirectories.end())
    {
    return file;
    }

  QString rel_file = iter.value().relativeFilePath(file);
  return QString("${%1}/%2").arg(iter.key()).arg(rel_file);
}

//-----------------------------------------------------------------------------
QString pqTestUtility::convertFromDataDirectory(const QString& file)
{
  QString filename = file;
  QMap<QString, QDir>::iterator iter;
  for(iter = this->DataDirectories.begin(); iter != this->DataDirectories.end(); ++iter)
    {
    QString label = QString("${%1}").arg(iter.key());
    if(filename.contains(label))
      {
      filename.replace(label, iter.value().absolutePath());
      break;
      }
    }
  return filename;
}
