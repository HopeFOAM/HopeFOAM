/*=========================================================================

   Program: ParaView
   Module:    pqTreeViewEventTranslator.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
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

========================================================================*/
#include "pqTreeViewEventTranslator.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QTreeView>
#include <QVariant>

//-----------------------------------------------------------------------------
pqTreeViewEventTranslator::pqTreeViewEventTranslator(QObject* parentObject)
  : Superclass(parentObject)
{
  this->Editing = false;
}

//-----------------------------------------------------------------------------
pqTreeViewEventTranslator::~pqTreeViewEventTranslator()
{
}

//-----------------------------------------------------------------------------
bool pqTreeViewEventTranslator::translateEvent(
  QObject* object, QEvent* tr_event, bool& /*error*/)
{
  QTreeView* treeWidget = qobject_cast<QTreeView*>(object);
  if(!treeWidget)
    {
    // mouse events go to the viewport widget
    treeWidget = qobject_cast<QTreeView*>(object->parent());
    }
  if(!treeWidget)
    {
    return false;
    }

  if (tr_event->type() == QEvent::KeyRelease)
    {
    QKeyEvent* ke = static_cast<QKeyEvent*>(tr_event);
    QModelIndex index = treeWidget->currentIndex();
    QString str_index = this->getIndexAsString(index);
    if (this->Editing)
      {
      if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
        {
        QVariant value = treeWidget->model()->data(index);
        this->Editing = false;
        emit this->recordEvent(treeWidget, "editAccepted",
                               QString("%1,%2").arg(str_index, value.toString()));
        }
      if (ke->key() == Qt::Key_Escape)
        {
        this->Editing = false;
        emit this->recordEvent(treeWidget, "editCancel", str_index);
        }
      }
    if (ke->key() == Qt::Key_F2)
      {
      this->Editing = true;
      emit this->recordEvent(treeWidget, "edit", str_index);
      }
    }

  if (tr_event->type() == QEvent::Enter && treeWidget == object)
    {
    qDebug() << "Focus IN";
    if (this->TreeView != object)
      {
      if(this->TreeView)
        {
        qDebug() << "disconnected";
        QObject::disconnect(this->TreeView, 0, this, 0);
        QObject::disconnect(this->TreeView->selectionModel(), 0, this, 0);
        }
      qDebug() << "connect";
      QObject::connect(treeWidget, SIGNAL(clicked(const QModelIndex&)),
        this, SLOT(onClicked(const QModelIndex&)));
      QObject::connect(treeWidget, SIGNAL(activated(const QModelIndex&)),
        this, SLOT(onActivated(const QModelIndex&)));
      QObject::connect(treeWidget, SIGNAL(doubleClicked(const QModelIndex&)),
        this, SLOT(onDoubleClicked(const QModelIndex&)));
      QObject::connect(treeWidget, SIGNAL(expanded(const QModelIndex&)),
        this, SLOT(onExpanded(const QModelIndex&)));
      QObject::connect(treeWidget, SIGNAL(collapsed(const QModelIndex&)),
        this, SLOT(onCollapsed(const QModelIndex&)));
      QObject::connect(treeWidget->selectionModel(),
        SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
        this, SLOT(onCurrentChanged(const QModelIndex&)));
      this->TreeView = treeWidget;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
void pqTreeViewEventTranslator::onClicked(
  const QModelIndex& index)
{
  static QModelIndex oldIndex;
  QTreeView* treeWidget = qobject_cast<QTreeView*>(this->sender());
  QString str_index = this->getIndexAsString(index);
  if ( (index.model()->flags(index) & Qt::ItemIsUserCheckable) != 0)
    {
    // record the check state change if the item is user-checkable.
    emit this->recordEvent( treeWidget, "setCheckState",
      QString("%1,%3").arg(str_index).arg(
        index.model()->data(index,Qt::CheckStateRole).toInt()));
    }
  if ((treeWidget->editTriggers() & QAbstractItemView::SelectedClicked)
        == QAbstractItemView::SelectedClicked &&
      index == oldIndex)
    {
    this->Editing = true;
    emit this->recordEvent(treeWidget, "edit", str_index);
    }
  oldIndex = index;
}

//-----------------------------------------------------------------------------
void pqTreeViewEventTranslator::onActivated(const QModelIndex & index)
{
  QTreeView* treeWidget = qobject_cast<QTreeView*>(this->sender());
  QString str_index = this->getIndexAsString(index);
  emit this->recordEvent(treeWidget, "activate", str_index);
}

//-----------------------------------------------------------------------------
void pqTreeViewEventTranslator::onDoubleClicked(const QModelIndex& index)
{
  QTreeView* treeWidget = qobject_cast<QTreeView*>(this->sender());
  QString str_index = this->getIndexAsString(index);
  // record the check state change if the item is user-checkable.
  if ((treeWidget->editTriggers() & QAbstractItemView::DoubleClicked)
      == QAbstractItemView::DoubleClicked)
    {
    this->Editing = true;
    emit this->recordEvent(treeWidget, "edit", str_index);
    }
}

//-----------------------------------------------------------------------------
void pqTreeViewEventTranslator::onExpanded(const QModelIndex& index)
{
  QTreeView* treeWidget = qobject_cast<QTreeView*>(this->sender());

  // record the check state change if the item is user-checkable.
  emit this->recordEvent( treeWidget, "expand",
    this->getIndexAsString(index));
}

//-----------------------------------------------------------------------------
void pqTreeViewEventTranslator::onCollapsed(const QModelIndex& index)
{
  QTreeView* treeWidget = qobject_cast<QTreeView*>(this->sender());

  // record the check state change if the item is user-checkable.
  emit this->recordEvent( treeWidget, "collapse",
    this->getIndexAsString(index));
}

//-----------------------------------------------------------------------------
QString pqTreeViewEventTranslator::getIndexAsString(const QModelIndex& index)
{
  QModelIndex curIndex = index;
  QString str_index;
  while (curIndex.isValid())
    {
    str_index.prepend(QString("%1.%2.").arg(curIndex.row()).arg(curIndex.column()));
    curIndex = curIndex.parent();
    }

  // remove the last ".".
  str_index.chop(1);
  return str_index;
}

//-----------------------------------------------------------------------------
void pqTreeViewEventTranslator::onCurrentChanged(const QModelIndex& index)
{
  QTreeView* treeWidget = this->TreeView;

  if (treeWidget)
    {
    // record the check state change if the item is user-checkable.
    emit this->recordEvent(treeWidget,
      "setCurrent", this->getIndexAsString(index));
    }
}
