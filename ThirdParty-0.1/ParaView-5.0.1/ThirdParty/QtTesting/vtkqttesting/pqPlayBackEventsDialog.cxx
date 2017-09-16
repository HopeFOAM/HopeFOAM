/*=========================================================================

   Program: ParaView
   Module:    pqPlayBackEventsDialog.cxx

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

#include "pqCommentEventPlayer.h"
#include "pqEventDispatcher.h"
#include "pqEventPlayer.h"
#include "pqPlayBackEventsDialog.h"
#include "pqTestUtility.h"

#include "ui_pqPlayBackEventsDialog.h"

#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMoveEvent>
#include <QTableWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QStringListModel>
#include <QTextStream>
#include <QTimer>

#include <QDebug>

//////////////////////////////////////////////////////////////////////////////////
// pqImplementation

struct pqPlayBackEventsDialog::pqImplementation
{
public:
  pqImplementation(pqEventPlayer& player,
                   pqEventDispatcher& dispatcher,
                   pqTestUtility* testUtility);
  ~pqImplementation();
  void init(pqPlayBackEventsDialog* dialog);
  void setProgressBarsValue(int value);
  void setProgressBarValue(int row, int value);
  QString setMaxLenght(const QString& name, int max);

  Ui::pqPlayBackEventsDialog Ui;

  pqEventPlayer&      Player;
  pqEventDispatcher&  Dispatcher;
  pqTestUtility*      TestUtility;

  int           CurrentLine; // Add counter to the Dispatcher
  int           MaxLines;
  int           CurrentFile;
  QStringList   Filenames;
  QStringList   CurrentEvent;
  QRect         OldRect;
};

// ----------------------------------------------------------------------------
pqPlayBackEventsDialog::pqImplementation::pqImplementation(pqEventPlayer& player,
                                                           pqEventDispatcher& dispatcher,
                                                           pqTestUtility* testUtility)
  : Player(player)
  , Dispatcher(dispatcher)
  , TestUtility(testUtility)
{
  this->CurrentLine = 0;
  this->MaxLines = 0;
  this->CurrentFile = 0;
  this->Filenames = QStringList();
  this->CurrentEvent = QStringList();
}

// ----------------------------------------------------------------------------
pqPlayBackEventsDialog::pqImplementation::~pqImplementation()
{
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::pqImplementation::init(pqPlayBackEventsDialog* dialog)
{
  this->Ui.setupUi(dialog);

  this->Ui.loadFileButton->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));

  this->Ui.playerErrorTextLabel->setVisible(false);
  this->Ui.playerErrorIconLabel->setVisible(false);
  this->Ui.infoErrorTextLabel->setVisible(false);
  this->Ui.infoErrorIconLabel->setVisible(false);
  this->Ui.logBrowser->setVisible(false);

  pqWidgetEventPlayer* widgetPlayer =
      this->Player.getWidgetEventPlayer(QString("pqCommentEventPlayer"));
  pqCommentEventPlayer* commentPlayer =
      qobject_cast<pqCommentEventPlayer*>(widgetPlayer);
  if (commentPlayer)
    {
    QObject::connect(commentPlayer, SIGNAL(comment(QString)),
                     this->Ui.logBrowser, SLOT(append(QString)));
    }

  dialog->setMaximumHeight(dialog->minimumSizeHint().height());

  QObject::connect(&this->Player, SIGNAL(eventAboutToBePlayed(QString, QString, QString)),
                   dialog, SLOT(onEventAboutToBePlayed(QString, QString, QString)));

  QObject::connect(this->Ui.timeStepSpinBox, SIGNAL(valueChanged(int)),
                   &this->Dispatcher, SLOT(setTimeStep(int)));

  QObject::connect(this->Ui.loadFileButton, SIGNAL(clicked()),
                   dialog, SLOT(loadFiles()));
  QObject::connect(this->Ui.plusButton, SIGNAL(clicked()),
                   dialog, SLOT(insertFiles()));
  QObject::connect(this->Ui.minusButton, SIGNAL(clicked()),
                   dialog, SLOT(removeFiles()));

  QObject::connect(this->Ui.playPauseButton, SIGNAL(toggled(bool)),
                   dialog, SLOT(onPlayOrPause(bool)));
  QObject::connect(this->Ui.playPauseButton, SIGNAL(toggled(bool)),
                   &this->Dispatcher, SLOT(run(bool)));
  QObject::connect(this->Ui.stopButton, SIGNAL(clicked()),
                   this->TestUtility, SLOT(stopTests()));
  QObject::connect(this->Ui.stepButton, SIGNAL(clicked()),
                   dialog, SLOT(onOneStep()));

  QObject::connect(this->TestUtility, SIGNAL(playbackStarted(QString)),
                   dialog, SLOT(onStarted(QString)));

  QObject::connect(this->TestUtility, SIGNAL(playbackStarted()),
                   dialog, SLOT(onStarted()));
  QObject::connect(this->TestUtility, SIGNAL(playbackStopped()),
                   dialog, SLOT(onStopped()));
  QObject::connect(&this->Dispatcher, SIGNAL(paused()),
                   dialog, SLOT(updateUi()));
  QObject::connect(&this->Dispatcher, SIGNAL(restarted()),
                   dialog, SLOT(updateUi()));

  QObject::connect(&this->Player, SIGNAL(errorMessage(QString)),
                   this->Ui.logBrowser, SLOT(append(QString)));

}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::pqImplementation::setProgressBarsValue(int value)
{
  for(int i = 0; i < this->Ui.tableWidget->rowCount(); ++i)
    {
    this->setProgressBarValue(i, value);
    }
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::pqImplementation::setProgressBarValue(int row,
                                                                   int value)
{
  QWidget* widget = this->Ui.tableWidget->cellWidget(row, 2);
  QProgressBar* progressBar = qobject_cast<QProgressBar*>(widget);
  progressBar->setValue(value);
}

// ----------------------------------------------------------------------------
QString pqPlayBackEventsDialog::pqImplementation::setMaxLenght(const QString& name,
                                                               int max)
{
  if(name.length() > max)
    {
    return name.left(max/2) + "..." + name.right(max/2);
    }
  return name;
}

///////////////////////////////////////////////////////////////////////////////////
// pqPlayBackEventsDialog

// ----------------------------------------------------------------------------
pqPlayBackEventsDialog::pqPlayBackEventsDialog(pqEventPlayer& Player,
                                               pqEventDispatcher& Dispatcher,
                                               pqTestUtility* TestUtility,
                                               QWidget* Parent)
  : QDialog(Parent)
  , Implementation(new pqImplementation(Player, Dispatcher, TestUtility))
{
  this->Implementation->init(this);
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->loadFiles();
}

// ----------------------------------------------------------------------------
pqPlayBackEventsDialog::~pqPlayBackEventsDialog()
{
  delete Implementation;
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::done(int value)
{
  this->Implementation->TestUtility->stopTests();
  QDialog::done(value);
}

// ----------------------------------------------------------------------------
QStringList pqPlayBackEventsDialog::selectedFileNames() const
{
  QStringList list;
  for(int i = 0; i < this->Implementation->Ui.tableWidget->rowCount(); ++i)
    {
    QCheckBox* box = qobject_cast<QCheckBox*>(
        this->Implementation->Ui.tableWidget->cellWidget(i, 0));
    if (box->isChecked())
      {
      list << this->Implementation->Filenames[i];
      }
    }
  return list;
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onEventAboutToBePlayed(const QString& Object,
                                                    const QString& Command,
                                                    const QString& Arguments)
{
  ++this->Implementation->CurrentLine;
  QStringList newEvent;
  newEvent << Object << Command << Arguments;
  this->Implementation->CurrentEvent = newEvent;
  this->updateUi();
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::loadFiles()
{
  QFileDialog* dialog = new QFileDialog(this, "Macro File Name",
    QString(), "XML Files (*.xml)");
  dialog->setFileMode(QFileDialog::ExistingFiles);
  if (dialog->exec())
    {
    this->Implementation->Filenames = dialog->selectedFiles();
    this->Implementation->Ui.tableWidget->setRowCount(0);
    this->loadFiles(this->Implementation->Filenames);
    }
  delete dialog;
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::insertFiles()
{
  QFileDialog* dialog = new QFileDialog(this, "Macro File Name",
    QString(), "XML Files (*.xml)");
  dialog->setFileMode(QFileDialog::ExistingFiles);
  if (dialog->exec())
    {
    this->Implementation->Filenames << dialog->selectedFiles();
    this->loadFiles(dialog->selectedFiles());
    }
  delete dialog;
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::removeFiles()
{
  if (QMessageBox::Ok == QMessageBox::warning(this, QString("Remove files"),
                                             QString("Are you sure you want to \n"
                                                     "remove all checked files ?\n"),
                                             QMessageBox::Ok,
                                             QMessageBox::Cancel))
    {
    foreach(QString file, this->selectedFileNames())
      {
      int index = this->Implementation->Filenames.indexOf(file);
      this->Implementation->Ui.tableWidget->removeRow(index);
      this->Implementation->Filenames.removeAt(index);
      }
    this->updateUi();
    }
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::loadFiles(const QStringList& filenames)
{
  for(int i = 0 ; i < filenames.count() ; i++)
    {
    this->addFile(filenames[i]);
    }
  this->Implementation->Ui.tableWidget->resizeColumnToContents(0);
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::addFile(const QString& filename)
{
  QFileInfo info(filename);
  int newIndex = this->Implementation->Ui.tableWidget->rowCount();
  this->Implementation->Ui.tableWidget->insertRow(newIndex);
  this->Implementation->Ui.tableWidget->setItem(
      newIndex, 1, new QTableWidgetItem(info.fileName()));
  this->Implementation->Ui.tableWidget->setCellWidget(
      newIndex, 2, new QProgressBar(this->Implementation->Ui.tableWidget));
  this->Implementation->setProgressBarValue(newIndex, 0);
  QCheckBox* check = new QCheckBox(this->Implementation->Ui.tableWidget);
  check->setChecked(true);
  QObject::connect(check, SIGNAL(toggled(bool)), this, SLOT(updateUi()));
  this->Implementation->Ui.tableWidget->setCellWidget(newIndex, 0, check);
  this->updateUi();
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onPlayOrPause(bool playOrPause)
{
  if (this->Implementation->TestUtility->playingTest() ||
      !playOrPause)
    {
    return;
    }

  QStringList newList = this->selectedFileNames();
  this->Implementation->TestUtility->playTests(newList);
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onOneStep()
{
  this->Implementation->Ui.playPauseButton->setChecked(false);
  if (!this->Implementation->TestUtility->playingTest())
    {
    this->Implementation->Dispatcher.run(false);
    this->Implementation->Dispatcher.oneStep();
    QStringList newList = this->selectedFileNames();
    this->Implementation->TestUtility->playTests(newList);
    }
  else
    {
    this->Implementation->Dispatcher.oneStep();
    }
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onStarted(const QString& filename)
{
  this->Implementation->CurrentFile =
      this->Implementation->Filenames.indexOf(filename);
  this->Implementation->Ui.tableWidget->setCurrentCell(
      this->Implementation->CurrentFile, 1,
      QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent);
  this->Implementation->Ui.logBrowser->clear();

  this->Implementation->MaxLines = 0;
  this->Implementation->CurrentLine = 0;

  QFile file(filename);
  QFileInfo infoFile(file);
  file.open(QIODevice::ReadOnly);
  this->Implementation->Ui.logBrowser->append(QString("Start file : %1").arg(
                                                infoFile.fileName()));
  QTextStream stream(&file);
  this->Implementation->Ui.currentFileLabel->setText(infoFile.fileName());
  while(!stream.atEnd())
    {
    QString line = stream.readLine();
    if(line.trimmed().startsWith("<event"))
      {
      ++this->Implementation->MaxLines;
      }
    }
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onStarted()
{
  this->Implementation->setProgressBarsValue(0);
  this->updateUi();
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onStopped()
{
  this->Implementation->Ui.playPauseButton->setChecked(false);
  this->updateUi();
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::updateUi()
{
  // Update Moda/Modeless
  this->onModal(this->Implementation->TestUtility->playingTest() &&
                !(this->Implementation->TestUtility->playingTest() &&
                  this->Implementation->Dispatcher.isPaused()));

  // Update player buttons
  this->Implementation->Ui.playPauseButton->setChecked(
      this->Implementation->TestUtility->playingTest() &&
      !this->Implementation->Dispatcher.isPaused());
  this->Implementation->Ui.playPauseButton->setEnabled(
      !this->Implementation->Filenames.isEmpty() &&
      this->selectedFileNames().count() > 0);
  this->Implementation->Ui.stepButton->setEnabled(
      !this->Implementation->Filenames.isEmpty() &&
      this->selectedFileNames().count() > 0);
  this->Implementation->Ui.stopButton->setEnabled(
      this->Implementation->TestUtility->playingTest());

  // loadFile, plus and minus buttons
  this->Implementation->Ui.loadFileButton->setEnabled(
      !this->Implementation->TestUtility->playingTest());
  this->Implementation->Ui.plusButton->setEnabled(
      !this->Implementation->TestUtility->playingTest());
  this->Implementation->Ui.minusButton->setEnabled(
      !this->Implementation->TestUtility->playingTest());

  // Time step
  this->Implementation->Ui.timeStepSpinBox->setEnabled(
      !this->Implementation->Filenames.isEmpty());

  // Error feedback
  this->Implementation->Ui.playerErrorTextLabel->setVisible(
      !this->Implementation->Dispatcher.status());
  this->Implementation->Ui.playerErrorIconLabel->setVisible(
      !this->Implementation->Dispatcher.status());
  this->Implementation->Ui.infoErrorTextLabel->setVisible(
      !this->Implementation->Dispatcher.status());
  this->Implementation->Ui.infoErrorIconLabel->setVisible(
      !this->Implementation->Dispatcher.status());

  QString command = tr("Command : ");
  QString argument = tr("Argument(s) : ");
  QString object = tr("Object : ");
  if(this->Implementation->TestUtility->playingTest() &&
     !this->Implementation->CurrentEvent.isEmpty())
    {
    command += this->Implementation->setMaxLenght(
        this->Implementation->CurrentEvent[1], 40);
    argument += this->Implementation->setMaxLenght(
        this->Implementation->CurrentEvent[2], 40);
    object += this->Implementation->setMaxLenght(
        this->Implementation->CurrentEvent[0], 40);
    this->Implementation->setProgressBarValue(this->Implementation->CurrentFile,
      static_cast<int>((static_cast<double>(
          this->Implementation->CurrentLine)/static_cast<double>(
              this->Implementation->MaxLines-1))*100));
    }
  else
    {
    this->Implementation->Ui.currentFileLabel->setText(
        QString("No Test is playing ..."));
    }

  this->Implementation->Ui.commandLabel->setText(command);
  this->Implementation->Ui.argumentsLabel->setText(argument);
  this->Implementation->Ui.objectLabel->setText(object);

  this->update();
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::onModal(bool value)
{
  // From modal to modeless we don't need to hide() show() the dialog
  if (value)
    {
    this->setAttribute(Qt::WA_WState_Visible, false);
    this->setAttribute(Qt::WA_WState_Hidden, true);
    }
  this->setModal(value);
  if (value)
    {
    this->Implementation->OldRect = this->frameGeometry();
    this->setVisible(true);
    this->Implementation->OldRect = QRect();
    }
  this->raise();
}

// ----------------------------------------------------------------------------
void pqPlayBackEventsDialog::moveEvent(QMoveEvent* event)
{
  if(this->Implementation->OldRect.isValid())
    {
    QPoint oldPos = this->Implementation->OldRect.topLeft();
    this->Implementation->OldRect = QRect();
    this->move(oldPos);
    }
  else
    {
    this->Superclass::moveEvent(event);
    }
}
