/***************************************************************************
 *   Copyright (C) 2006 by Zi Ree   *
 *   Zi Ree @ SecondLife   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "settings.h"
#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
  qDebug("SettingsDialog::SettingsDialog()");

  setupUi(this);

  useFogCheckbox->setChecked(Settings::Instance()->fog());
  floorTranslucencySpin->setValue(Settings::Instance()->floorTranslucency());
  easeInCheckbox->setChecked(Settings::Instance()->easeIn());
  easeOutCheckbox->setChecked(Settings::Instance()->easeOut());
  tPoseWarningCheckBox->setChecked(Settings::Instance()->tPoseWarning());
  debugCheckBox->setChecked(Settings::Instance()->Debug());
}

SettingsDialog::~SettingsDialog()
{
  qDebug("SettingsDialog::~SettingsDialog()");
}

void SettingsDialog::on_applyButton_clicked()
{
  qDebug("accept()");

  Settings::Instance()->setFog(useFogCheckbox->isChecked());
  Settings::Instance()->setFloorTranslucency(floorTranslucencySpin->value());
/*TODO: this should be per-animation
  Settings::setEaseIn(easeInCheckbox->isChecked());
  Settings::setEaseOut(easeOutCheckbox->isChecked());     */

  Settings::Instance()->setTPoseWarning(tPoseWarningCheckBox->isChecked());
  Settings::Instance()->setDebug(debugCheckBox->isChecked());

  Settings::Instance()->WriteSettings();

  emit configChanged();
  qApp->processEvents();
}

void SettingsDialog::on_okButton_clicked()
{
  qDebug("acceptOk()");
  on_applyButton_clicked();
  accept();
}

void SettingsDialog::on_cancelButton_clicked()
{
  qDebug("reject()");
  reject();
}

void SettingsDialog::on_useFogCheckbox_toggled(bool state)
{
  qDebug("useFogToggled(%d)",state);
}

void SettingsDialog::on_floorTranslucencySpin_valueChanged(int value)
{
  qDebug("floorTranslucencyChanged(%d)",value);
}

void SettingsDialog::on_easeInCheckbox_toggled(bool state)
{
  qDebug("easeInToggled(%d)",state);
}

void SettingsDialog::on_easeOutCheckbox_toggled(bool state)
{
  qDebug("easeOutToggled(%d)",state);
}

void SettingsDialog::on_tPoseWarningCheckBox_toggled(bool checked)
{
  qDebug("tPoseWarning toggled(%d)", checked);
}


void SettingsDialog::on_debugCheckBox_toggled(bool checked)
{
  qDebug("DEBUG mode toggled(%d)", checked);
}
