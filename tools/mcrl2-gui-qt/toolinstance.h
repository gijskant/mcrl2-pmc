// Author(s): Rimco Boudewijns
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef TOOLINSTANCE_H
#define TOOLINSTANCE_H

#include <QWidget>
#include "toolinformation.h"
#include "optionvalue.h"

namespace Ui {
  class ToolInstance;
}

class ToolInstance : public QWidget
{
    Q_OBJECT
    
  public:
    explicit ToolInstance(QString filename, ToolInformation information, QWidget *parent = 0);
    ~ToolInstance();

    ToolInformation information() { return m_info; }
    QString executable();
    QString arguments();

  public slots:
    void onStateChange(QProcess::ProcessState state);
    void onStandardOutput();
    void onStandardError();
    void onRun();
    void onAbort();
    void onSave();

  private:
    QString m_filename;
    ToolInformation m_info;
    Ui::ToolInstance *ui;

    QList<OptionValue> m_optionValues;
    QProcess m_process;

  signals:
    void titleChanged(QString title);

};

#endif // TOOLINSTANCE_H
