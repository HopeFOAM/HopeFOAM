/*=========================================================================

   Program: ParaView
   Module:    pqPlayBackEventsDialog.h

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

#ifndef _pqPlayBackEventsDialog_h
#define _pqPlayBackEventsDialog_h

#include "QtTestingExport.h"
#include <QDialog>

class pqEventPlayer;
class pqEventDispatcher;
class pqTestUtility;

/// Provides a standard dialog that will PlayBack user input to an XML file as long as the dialog remains open
class QTTESTING_EXPORT pqPlayBackEventsDialog : public QDialog
{
  Q_OBJECT
  
  typedef QDialog Superclass;
public:
  /**
  Creates the dialog and begins translating user input with the supplied translator.
  */
  pqPlayBackEventsDialog(pqEventPlayer& Player,pqEventDispatcher& Source,
                         pqTestUtility* TestUtility, QWidget* Parent);
  ~pqPlayBackEventsDialog();

private slots:
  void onEventAboutToBePlayed(const QString&,const QString&,const QString&);
  void loadFiles();
  void insertFiles();
  void removeFiles();
  void onPlayOrPause(bool);
  void onOneStep();
  void onStarted();
  void onStarted(const QString&);
  void onStopped();
  void onModal(bool value);

public slots:
  virtual void done(int);
  void updateUi();

protected:
  virtual void moveEvent(QMoveEvent* event);

private:
  void loadFiles(const QStringList& filenames);
  void addFile(const QString& filename);
  QStringList selectedFileNames() const;

  pqPlayBackEventsDialog(const pqPlayBackEventsDialog&);            // Not Implemented
  pqPlayBackEventsDialog& operator=(const pqPlayBackEventsDialog&); // Not Implemented

  struct pqImplementation;
  pqImplementation* const Implementation;
};

#endif // !_pqPlayBackEventsDialog_h

