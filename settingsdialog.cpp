/****************************************************************************
**
** Copyright 2010 Adminotech Ltd
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
*****************************************************************************/

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "ui_remoteom.h"
#include "settings.h"
#include "remoteom.h"
#include <QList>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->nameLine->hide();
    ui->labelName->hide();
    ui->backButton->hide();

    Settings s;

    if (!s.readXmlFile(items))
    {                
        return;
    }

    ui->comboSettings->setCurrentIndex(0);
    ui->txtIPAddress->setText(set_ipaddress);
    ui->comboSettings->addItems(items.keys());

    connect(ui->comboSettings, SIGNAL(currentIndexChanged(QString)), this, SLOT(activeComboBox(QString)));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;    
}

void SettingsDialog::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::on_saveSettingsButton_clicked()
{

    Settings s;
    if (ui->nameLine->text().isEmpty() || ui->txtIPAddress->text().isEmpty() || ui->txtPort->text().isEmpty() )
    {
        QMessageBox::warning(this, tr("Remote OM"), tr("Please enter all required settings"));
        return;
    }

    //Adding new entry
    if (ui->newentrySettingsButton->isHidden() && !ui->nameLine->text().isEmpty())
    {
        //Check name if exists
        for (int i = 0; i < items.count(); i++ )
        {
            int t = QString::compare(items.keys().at(i), ui->nameLine->text(), Qt::CaseInsensitive);
            if (t == 0)
            {
                QMessageBox::warning(this, tr("Remote OM"), tr("Setting with same name already exists"));
                return;
            }
        }
    }

    bool found = false;
    if (!items.isEmpty())
    {
        for (int i = 0; i < items.count(); i++ )
        {
            int t = QString::compare(items.keys().at(i), ui->nameLine->text(), Qt::CaseInsensitive);
            if (t == 0)
            {
                found = true;
                break;
            }
        }
    }

    if (found)
    {
        //Update existing item
        items[ui->nameLine->text()] = ui->txtIPAddress->text()+";"+ui->txtPort->text()+";"+ui->txtPassword->text();
    }
    else
    {
        //Add new item
        items.insert(ui->nameLine->text(), ui->txtIPAddress->text()+";"+ui->txtPort->text()+";"+ui->txtPassword->text());
    }


    if (!s.saveXmlFile(items))
    {
        QMessageBox::warning(this, tr("Remote OM"), tr("Saving settings failed."));
        return;
    }

    /*if (!s.saveXmlFile(ui->nameLine->text(), ui->txtIPAddress->text(), ui->txtPort->text(), ui->txtPassword->text()))
    {
        QMessageBox::warning(this, tr("Remote OM"), tr("Saving settings failed."));
        return;
    }*/

    QMessageBox::information(this, tr("Remote OM"), tr("Settings saved."));

    if (ui->comboSettings->currentIndex() == -1)
    {
        connect(ui->comboSettings, SIGNAL(currentIndexChanged(QString)), this, SLOT(activeComboBox(QString)));
    }

    if (found == false)
    {
        ui->comboSettings->clear();
        ui->comboSettings->addItems(items.keys());

        ui->newentrySettingsButton->show();
        ui->delete_entrySettingsButton->show();
        ui->backButton->hide();
    }

    ui->nameLine->hide();
    ui->labelName->hide();
    ui->comboSettings->show();
    ui->labelSettings->show();

    if (!s.readXmlFile(items))
    {
        return;
    }

    QStringList list = items[ui->comboSettings->currentText()].split(";");
    if (!list.isEmpty())
    {
        ui->txtIPAddress->setText(list[0]);
        ui->txtPort->setText(list[1]);
        ui->txtPassword->setText(list[2]);

        set_ipaddress = list[0];
        set_port = list[1];
        set_password = list[2];
    }


}

void SettingsDialog::on_newentrySettingsButton_clicked()
{
    ui->nameLine->show();
    ui->labelName->show();
    ui->comboSettings->hide();
    ui->labelSettings->hide();
    ui->saveSettingsButton->show();
    ui->txtIPAddress->clear();
    ui->txtPassword->clear();
    ui->txtPort->clear();
    ui->nameLine->clear();
    ui->backButton->show();
    ui->delete_entrySettingsButton->hide();
    ui->newentrySettingsButton->hide();


}

void SettingsDialog::on_delete_entrySettingsButton_clicked()
{

    QString delete_name = ui->comboSettings->currentText();
    QString delete_IP = ui->txtIPAddress->text();
    Settings s;
    s.deleteXml(delete_name, delete_IP);
    ui->txtIPAddress->clear();
    ui->txtPassword->clear();
    ui->txtPort->clear();
    ui->nameLine->clear();
    ui->comboSettings->removeItem(ui->comboSettings->currentIndex());
    set_ipaddress = "";
    set_port = "";
    set_password = "";

    items.remove(delete_name);
    ui->comboSettings->clear();
    ui->comboSettings->addItems(items.keys());

    if (items.count() > 0)
    {
        activeComboBox(items.keys().at(0));
    }
}

void SettingsDialog::on_backButton_clicked()
{

    ui->nameLine->hide();
    ui->labelName->hide();
    ui->comboSettings->show();
    ui->labelSettings->show();
    ui->saveSettingsButton->show();
    ui->txtIPAddress->setText(set_ipaddress);
    ui->txtPassword->setText(set_password);
    ui->txtPort->setText(set_port);
    ui->nameLine->clear();

    ui->delete_entrySettingsButton->show();
    ui->newentrySettingsButton->show();
    ui->backButton->hide();
}

void SettingsDialog::activeComboBox(QString name)
{

    if(name != "")
    {
        if (items.contains(name))
        {

            QString val = items[name];
            QStringList list = val.split(";");

            ui->txtIPAddress->setText(list[0]);
            set_ipaddress = list[0];
            ui->txtPort->setText(list[1]);
            set_port = list[1];
            ui->txtPassword->setText(list[2]);
            set_password = list[2];
            ui->nameLine->setText(name);

        }
    }
}

void SettingsDialog::FirstChoice(QString profile)
{
    Settings s;    
    if (!s.readXmlFile(items))
    {
        return;
    }

    QString name = items.keys().at(0);
    if (!profile.isEmpty())
    {
        if (items[profile].isNull() || items[profile].isEmpty())
        {
            name = items.keys().at(0);
        }
        else
        {
            name = profile;
        }
    }

    QStringList list = items[name].split(";");

    ui->txtIPAddress->setText(list[0]);
    set_ipaddress = list[0];
    ui->txtPassword->setText(list[2]);
    set_password = list[2];
    ui->txtPort->setText(list[1]);
    set_port = list[1];
    ui->nameLine->setText(name);

}

void SettingsDialog::Init()
{
    if (ui->txtIPAddress->text().isEmpty())
    {
        ui->txtIPAddress->setText(set_ipaddress);
        ui->txtPort->setText(set_port);
        ui->txtPassword->setText(set_password);
    }
}

void SettingsDialog::SetInitialSettings()
{
    Settings s;

    if (!s.readXmlFile(items))
    {
        return;
    }

    QStringList list = items[0].split(";");

    ui->txtIPAddress->setText(list[0]);
    set_ipaddress = list[0];
    ui->txtPassword->setText(list[2]);
    set_password = list[2];
    ui->txtPort->setText(list[1]);
    set_port = list[1];
    //ui->nameLine->setText(items[0].);
}

void SettingsDialog::on_closeSettingsButton_clicked()
{
    if (ui->nameLine->isVisible())
    {
        ui->nameLine->hide();
        ui->labelName->hide();
        ui->comboSettings->show();
        ui->labelSettings->show();

        ui->delete_entrySettingsButton->show();
        ui->newentrySettingsButton->show();
        ui->backButton->hide();

    }

    this->close();
}

