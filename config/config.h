/* Bespin widget style configurator for Qt4
   Copyright (C) 2007 Thomas Luebking <thomas.luebking@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "bconfig.h"
#include "ui_config.h"

class Config : public BConfig /** <-- inherit BConfig */
{
    Q_OBJECT
public:
    /** The constructor... */
    Config(QWidget *parent = 0L);
    static QString sImport(const QString &file, bool override = false);
    static bool sExport(const QString &preset, const QString &file);
    static QStringList colors(const QPalette &pal, QPalette::ColorGroup group);
    static bool load(const QString&);
    static void updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list);
protected:
    void changeEvent(QEvent *event);
public slots:
    /** We'll reimplement the im/export functions to handle color settings as well*/
    void store();
    void restore();
    void restore(QTreeWidgetItem*, int);
    bool save(); // to store colors to qt configuration - in case
    void import();
    void saveAs();
private:
    Q_DISABLE_COPY(Config)
    /** This is the UI created with Qt Designer and included by ui_config.h */
    Ui::Config ui;

    /** Just some functions to fill the comboboxes, not really of interest */
    void generateColorModes(QComboBox *box, QList<int> *roles = 0);
    void generateGradientTypes(QComboBox *box);

    QPalette *loadedPal;
    bool infoIsManage;
    void store3(const QString &, bool);
    static void savePalette(const QPalette &pal);
private slots:
    void setHeader(const QString&);
    void storedSettigSelected(QTreeWidgetItem *);
    void store2a();
    void store2b(QTreeWidgetItem *);
    void remove();
    void handleBgMode(int);
    void handleGrooveMode(int);
    void learnPwChar();
    void filterPresets(const QString & text);
    void presetAppsChanged(QTreeWidgetItem *, int);
};

#endif
