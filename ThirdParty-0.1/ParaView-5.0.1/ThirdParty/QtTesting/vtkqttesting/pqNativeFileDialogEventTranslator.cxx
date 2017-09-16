/*=========================================================================

   Program: ParaView
   Module:    pqNativeFileDialogEventTranslator.cxx

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

#include "pqNativeFileDialogEventTranslator.h"

#include <QEvent>
#include <QFileDialog>
#include <QApplication>
#include "pqEventTranslator.h"
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
    pqNativeFileDialogEventTranslator* self;
    _qt_filedialog_existing_directory_hook old_existing_directory_hook;
    _qt_filedialog_open_filename_hook old_open_filename_hook;
    _qt_filedialog_open_filenames_hook old_open_filenames_hook;
    _qt_filedialog_save_filename_hook old_save_filename_hook;

    QString existing_directory_hook(QWidget* parent, const QString& caption, const QString& dir,
                               QFileDialog::Options options)
    {
        qt_filedialog_existing_directory_hook = 0;

        QString path = QFileDialog::getExistingDirectory(parent, caption, dir, options);
        self->record("DirOpen", path);

        qt_filedialog_existing_directory_hook = existing_directory_hook;
        return path;
    }


    QString open_filename_hook(QWidget* parent, const QString& caption, const QString& dir,
                               const QString& filter, QString *selectedFilter,
                               QFileDialog::Options options)
    {
        qt_filedialog_open_filename_hook = 0;

        QString file = QFileDialog::getOpenFileName(parent, caption, dir, filter, selectedFilter, options);
        self->record("FileOpen", file);

        qt_filedialog_open_filename_hook = open_filename_hook;
        return file;
    }

    QStringList open_filenames_hook(QWidget* parent, const QString& caption, const QString& dir,
                               const QString& filter, QString *selectedFilter,
                               QFileDialog::Options options)
    {
        qt_filedialog_open_filenames_hook = 0;

        QStringList files = QFileDialog::getOpenFileNames(parent, caption, dir, filter, selectedFilter, options);
        self->record("FilesOpen", files.join(";"));

        qt_filedialog_open_filenames_hook = open_filenames_hook;
        return files;
    }

    QString save_filename_hook(QWidget* parent, const QString& caption, const QString& dir,
                               const QString& filter, QString *selectedFilter,
                               QFileDialog::Options options)
    {
        qt_filedialog_save_filename_hook = 0;

        QString file = QFileDialog::getSaveFileName(parent, caption, dir, filter, selectedFilter, options);
        self->record("FileSave", file);

        qt_filedialog_save_filename_hook = save_filename_hook;
        return file;
    }

}


pqNativeFileDialogEventTranslator::pqNativeFileDialogEventTranslator(pqTestUtility* util, QObject* p)
    : pqWidgetEventTranslator(p), mUtil(util)
{
    QObject::connect(mUtil->eventTranslator(), SIGNAL(started()), this, SLOT(start()));
    QObject::connect(mUtil->eventTranslator(), SIGNAL(stopped()), this, SLOT(stop()));
}

pqNativeFileDialogEventTranslator::~pqNativeFileDialogEventTranslator()
{
}

void pqNativeFileDialogEventTranslator::start()
{
    self = this;

    old_existing_directory_hook = qt_filedialog_existing_directory_hook;
    qt_filedialog_existing_directory_hook = existing_directory_hook;

    old_open_filename_hook = qt_filedialog_open_filename_hook;
    qt_filedialog_open_filename_hook = open_filename_hook;

    old_open_filenames_hook = qt_filedialog_open_filenames_hook;
    qt_filedialog_open_filenames_hook = open_filenames_hook;

    old_save_filename_hook = qt_filedialog_save_filename_hook;
    qt_filedialog_save_filename_hook = save_filename_hook;

}

void pqNativeFileDialogEventTranslator::stop()
{
    self = NULL;

    qt_filedialog_existing_directory_hook = old_existing_directory_hook;
    qt_filedialog_open_filename_hook = old_open_filename_hook;
    qt_filedialog_open_filenames_hook = old_open_filenames_hook;
    qt_filedialog_save_filename_hook = old_save_filename_hook;
}

bool pqNativeFileDialogEventTranslator::translateEvent(QObject* Object, QEvent* pqNotUsed(Event),
						       bool& pqNotUsed(Error))
{
    // capture events under a filedialog and consume them
    QObject* tmp = Object;
    while(tmp)
    {
        if(qobject_cast<QFileDialog*>(tmp))
        {
            return true;
        }
        tmp = tmp->parent();
    }

    return false;
}

void pqNativeFileDialogEventTranslator::record(const QString& command, const QString& args)
{
    QStringList files = args.split(";");
    QStringList normalized_files;

    foreach(QString file, files)
    {
        normalized_files.append(mUtil->convertToDataDirectory(file));
    }

    emit this->recordEvent(QApplication::instance(), command, normalized_files.join(";"));
}

#else
pqNativeFileDialogEventTranslator::pqNativeFileDialogEventTranslator(
  pqTestUtility* util, QObject* p) : pqWidgetEventTranslator(p), mUtil(util)
{
}
pqNativeFileDialogEventTranslator::~pqNativeFileDialogEventTranslator()
{
}

void pqNativeFileDialogEventTranslator::start()
{
}

void pqNativeFileDialogEventTranslator::stop()
{
}

bool pqNativeFileDialogEventTranslator::translateEvent(
  QObject* pqNotUsed(Object), QEvent* pqNotUsed(Event), bool& pqNotUsed(Error))
{
    return false;
}

void pqNativeFileDialogEventTranslator::record(const QString& command, const QString& args)
{
  Q_UNUSED(command);
  Q_UNUSED(args);
}
#endif
