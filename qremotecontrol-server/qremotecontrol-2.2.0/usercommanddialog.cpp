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
#include "usercommanddialog.h"
#include "ui_usercommanddialog.h"

UserCommandDialog::UserCommandDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserCommandDialog)
{
    ui->setupUi(this);
    ui->nameEdit->setFocus();
}

UserCommandDialog::~UserCommandDialog()
{
    delete ui;
}

void UserCommandDialog::setName(QString arg)
{
    m_name = arg;
    ui->nameEdit->setText(arg);
}

void UserCommandDialog::setCommand(QString arg)
{
    m_command = arg;
    //ui->commandEdit->setText(arg);
}

void UserCommandDialog::setParameters(QString arg)
{
    m_parameters = arg;
    //ui->parameterEdit->setText(arg);
}

void UserCommandDialog::on_nameEdit_editingFinished()
{
    m_name = ui->nameEdit->text();
}

void UserCommandDialog::on_commandEdit_editingFinished()
{
    //m_command = ui->commandEdit->text();
}

void UserCommandDialog::on_parameterEdit_editingFinished()
{
    //m_parameters = ui->parameterEdit->text();
}

void UserCommandDialog::on_browseCommandButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName( this,
                                                     tr("Open file"),
                                                     QDesktopServices::storageLocation(QDesktopServices::HomeLocation),
                                                     tr("All Files (*)"));
    if (!fileName.isNull()) {
        //ui->commandEdit->setText(fileName);
    }
}
