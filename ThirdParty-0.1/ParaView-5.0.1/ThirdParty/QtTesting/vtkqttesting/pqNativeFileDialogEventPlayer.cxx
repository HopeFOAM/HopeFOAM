/*=========================================================================

   Program: ParaView
   Module:    pqNativeFileDialogEventPlayer.cxx

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

#include "pqNativeFileDialogEventPlayer.h"

#include <QEvent>
#include <QFileDialog>
#include <QApplication>
#include "pqEventDispatcher.h"
#include "pqTestUtility.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)

typedef QString (*_qt_filedialog_existing_directory_hook)(QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options options);
extern Q_DECL_IMPORT _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook;

typedef QString (*_qt_filedialog_open_filename_hook)(QWidget * parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options);
extern Q_DECL_IMPORT _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook;

typedef QStringList (*_qt_filedialog_open_filenames_hook)(QWidget * parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options);
extern Q_DECL_IMPORT _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook;

typedef QString (*_qt_filedialog_save_filename_hook)(QWidget * parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options);
extern Q_DECL_IMPORT _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook;

namespace
{
    pqNativeFileDialogEventPlayer* self;
    _qt_filedialog_existing_directory_hook old_existing_directory_hook;
    _qt_filedialog_open_filename_hook old_open_filename_hook;
    _qt_filedialog_open_filenames_hook old_open_filenames_hook;
    _qt_filedialog_save_filename_hook old_save_filename_hook;

    QEventLoop *loop = 0;
    QString filename;
    QStringList filenames;

    QString dir_hook(QWidget* pqNotUsed(parent), const QString& pqNotUsed(caption), const QString& pqNotUsed(dir),
                     QFileDialog::Options pqNotUsed(options))
    {
        // wait for next event played to give us a filename
        loop->exec();
        return filename;
    }

    QString filename_hook(QWidget* pqNotUsed(parent), const QString& pqNotUsed(caption), const QString& pqNotUsed(dir),
			  const QString& pqNotUsed(filter), QString * pqNotUsed(selectedFilter),
			  QFileDialog::Options pqNotUsed(options))
    {
        // wait for next event played to give us a filename
        loop->exec();
        return filename;
    }

  QStringList filenames_hook(QWidget* pqNotUsed(parent), const QString& pqNotUsed(caption), const QString& pqNotUsed(dir),
			     const QString& pqNotUsed(filter), QString *pqNotUsed(selectedFilter),
			     QFileDialog::Options pqNotUsed(options))
    {
        // wait for next event played to give us a filename
        loop->exec();
        return filenames;
    }


}

pqNativeFileDialogEventPlayer::pqNativeFileDialogEventPlayer(pqTestUtility* util, QObject* p)
    : pqWidgetEventPlayer(p), mUtil(util)
{
  if(!loop)
    {
    loop = new QEventLoop();
    }

    QObject::connect(util, SIGNAL(playbackStarted()), this, SLOT(start()));
    QObject::connect(util, SIGNAL(playbackStopped()), this, SLOT(stop()));
}

pqNativeFileDialogEventPlayer::~pqNativeFileDialogEventPlayer()
{
  if(loop)
    {
    delete loop;
    loop = NULL;
    }
}

void pqNativeFileDialogEventPlayer::start()
{
    self = this;

    // existing dir hook
    old_existing_directory_hook = qt_filedialog_existing_directory_hook;
    qt_filedialog_existing_directory_hook = dir_hook;

    // open file hook
    old_open_filename_hook = qt_filedialog_open_filename_hook;
    qt_filedialog_open_filename_hook = filename_hook;

    // open files hook
    old_open_filenames_hook = qt_filedialog_open_filenames_hook;
    qt_filedialog_open_filenames_hook = filenames_hook;

    // save file hook
    old_save_filename_hook = qt_filedialog_save_filename_hook;
    qt_filedialog_save_filename_hook = filename_hook;
}

void pqNativeFileDialogEventPlayer::stop()
{
    self = NULL;
    qt_filedialog_existing_directory_hook = old_existing_directory_hook;
    qt_filedialog_open_filename_hook = old_open_filename_hook;
    qt_filedialog_open_filenames_hook = old_open_filenames_hook;
    qt_filedialog_save_filename_hook = old_save_filename_hook;
}

bool pqNativeFileDialogEventPlayer::playEvent(QObject* Object, const QString& Command, const QString& Arguments, bool& pqNotUsed(Error))
{

    if(!qobject_cast<QApplication*>(Object))
    {
        return false;
    }

    QStringList normalized_files = Arguments.split(";");
    QStringList files;

    foreach(QString file, normalized_files)
    {
        files.append(mUtil->convertFromDataDirectory(file));
    }

    if(Command == "FileOpen" || Command == "DirOpen" || Command == "FileSave")
    {
        filename = files.join(";");
        loop->quit();
        return true;
    }
    else if(Command == "FilesOpen")
    {
        filenames = files;
        loop->quit();
        return true;
    }

    return false;
}
#else
//-----------------------------------------------------------------------------
pqNativeFileDialogEventPlayer::pqNativeFileDialogEventPlayer(
  pqTestUtility* util, QObject* p) : pqWidgetEventPlayer(p), mUtil(util)
{
}
pqNativeFileDialogEventPlayer::~pqNativeFileDialogEventPlayer()
{
}
void pqNativeFileDialogEventPlayer::start()
{
}
void pqNativeFileDialogEventPlayer::stop()
{
}

bool pqNativeFileDialogEventPlayer::playEvent(
  QObject* Object, const QString& Command, const QString& Arguments, bool& Error)

{
  Q_UNUSED(Object);
  Q_UNUSED(Command);
  Q_UNUSED(Arguments);
  Q_UNUSED(Error);
  return false;
}

#endif
