// Author(s): Bas Ploeger, Carst Tankink, Ruud Koolen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MARKDIALOG_H
#define MARKDIALOG_H

#include "ui_markdialog.h"

#include <QDialog>
#include <QMap>
#include <QString>

#include "markmanager.h"

class MarkListItem : public QListWidgetItem
{
  public:
    MarkListItem(QString text, MarkRuleIndex index_): QListWidgetItem(text), index(index_) {}
    MarkRuleIndex index;
};

class MarkDialog : public QDialog
{
  Q_OBJECT

  public:
    MarkDialog(QWidget *parent, MarkManager *markManager);

  protected slots:
    void loadLts();
    void markStyleClicked();
    void setMarkStyle(MarkStyle style);
    void clusterMatchStyleChanged(int index);
    void setClusterMatchStyle(MatchStyle style);
    void stateMatchStyleChanged(int index);
    void setStateMatchStyle(MatchStyle style);
    void addMarkRule();
    void markRuleAdded(MarkRuleIndex index);
    void editMarkRule(QListWidgetItem *item);
    void enableMarkRule(QListWidgetItem *item);
    void markRuleChanged(MarkRuleIndex index);
    void removeMarkRule();
    void markRuleRemoved(MarkRuleIndex index);
    void actionLabelChanged(QListWidgetItem *item);
    void setActionMarked(int action, bool marked);

  protected:
    QString markRuleDescription(MarkRuleIndex index) const;

  private:
    Ui::MarkDialog m_ui;
    MarkManager *m_markManager;
    QVector<QString> m_actions;
    QList<QColor> m_markRuleColors;
    int m_markRuleNextColorIndex;
    QMap<MarkRuleIndex, MarkListItem *> m_markListItems;
    QMap<QString, int> m_actionNumbers;
    QMap<int, int> m_actionPositions;
};

#endif
