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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QHash>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QWidget {
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    void FirstChoice(QString profile);
    void SetInitialSettings();
    void Init();

    QHash<QString, QString> items;
    QString set_ipaddress;
    QString set_port;
    QString set_password;

protected:
    void changeEvent(QEvent *e);


private:
    Ui::SettingsDialog *ui;

private slots:
    void on_closeSettingsButton_clicked();
    void on_saveSettingsButton_clicked();
    void on_newentrySettingsButton_clicked();
    void on_delete_entrySettingsButton_clicked();
    void on_backButton_clicked();
    void activeComboBox(QString);


};

#endif // SETTINGSDIALOG_H
