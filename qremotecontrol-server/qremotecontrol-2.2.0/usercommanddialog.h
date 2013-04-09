/**************************************************************************
**
** This file is part of QRemoteControl-Server.
**
** QRemoteControl-Server is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** QRemoteControl-Server is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with PhyxCalc.  If not, see <http://www.gnu.org/licenses/>.
**
***************************************************************************/
#ifndef USERCOMMANDDIALOG_H
#define USERCOMMANDDIALOG_H

#include <QDialog>
#include <QDesktopServices>
#include <QFileDialog>

namespace Ui {
class UserCommandDialog;
}

class UserCommandDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString command READ command WRITE setCommand)
    Q_PROPERTY(QString parameters READ parameters WRITE setParameters)
    
public:
    explicit UserCommandDialog(QWidget *parent = 0);
    ~UserCommandDialog();
    
    QString name() const
    {
        return m_name;
    }

    QString command() const
    {
        return m_command;
    }

    QString parameters() const
    {
        return m_parameters;
    }

public slots:
    void setName(QString arg);
    void setCommand(QString arg);
    void setParameters(QString arg);

private slots:
    void on_nameEdit_editingFinished();

    void on_commandEdit_editingFinished();

    void on_parameterEdit_editingFinished();

    void on_browseCommandButton_clicked();

private:
    Ui::UserCommandDialog *ui;
    QString m_name;
    QString m_command;
    QString m_parameters;
};

#endif // USERCOMMANDDIALOG_H
