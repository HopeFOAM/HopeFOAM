/*=========================================================================

   Program: ParaView
   Module:    pqEventComment.h

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

#ifndef __pqEventComment_h
#define __pqEventComment_h

// Qt includes
#include <QObject>

// QtTesting inlcudes
#include "pqTestUtility.h"
#include "QtTestingExport.h"

/// pqEventComment is responsible for adding any kind of events that are not added
/// by widgets.
/// For exemple, you can add an event to block the playback, to show a custom
/// comment etc ...

class QTTESTING_EXPORT pqEventComment :
  public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqEventComment(pqTestUtility* util, QObject* parent = 0);
  ~pqEventComment();

  /// Call this function to add an event comment, which will display a message,
  /// during the playback in the log.
  void recordComment(const QString& arguments);

  /// Call this function to add an event comment, which will display a message,
  /// and then pause the macro during the playback.
  void recordCommentBlock(const QString& arguments);

signals:
  /// All functions should emit this signal whenever they wish to record comment event
  void recordComment(QObject* widget, const QString& type, const QString& argument);

protected:
  void recordComment(const QString& command,
                     const QString& arguments,
                     QObject* = 0);

  pqTestUtility* TestUtility;
};

#endif
