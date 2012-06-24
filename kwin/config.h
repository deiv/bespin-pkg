/*
 *   Bespin window decoration for KWin
 *   Copyright 2008-2012 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "../config/bconfig.h"
#include "ui_configdialog.h"

class KConfigGroup;
class QListWidgetItem;

class Config : public BConfig
{
    Q_OBJECT
public:
    Config(QWidget* parent);
public slots:
    void createNewPreset();
    void deleteCurrentPreset();
    void presetChanged(QListWidgetItem*, QListWidgetItem*);
    void save(KConfigGroup&);
private:
    void loadPresets();
    void savePresets();
private slots:
    void catchClones(QListWidgetItem*);
    void watchBgMode();
    void watchDecoGradient();
    void watchShadowSize(int);
private:
   Ui::Config ui;
};

#endif // KNIFTYCONFIG_H
