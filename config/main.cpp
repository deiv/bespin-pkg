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

#include "config.h"
#include "ui_uiDemo.h"
#include "dialog.h"
#include <QFile>
#include <QSettings>

class BStyle : public QStyle {
public:
   BStyle() : QStyle (){}
   virtual void init(const QSettings *settings) = 0;
};

static int error(const QString & string) {
   qWarning("Error: %s", string.toLatin1().data());
   return 0;
}

int main(int argc, char *argv[])
{
   /** ==========================================
   This is just a command line tool for importing data - GUI part below*/
   if (argc > 1) {
      
      int mode = 0;
      if (!qstrcmp( argv[1], "import" ))
         mode = 1;
      else if (!qstrcmp( argv[1], "demo" ))
         mode = 2;
      else if (!qstrcmp( argv[1], "try" ))
         mode = 3;
      
      if (mode > 1) { // launch demo widget
         QApplication app(argc, argv);
         if (mode == 3) { // try external
            
            if (argc < 3)
               error("Usage: bespin-config try <some_config.bespin.conf>");
            
            if (!QFile::exists(argv[2]))
               error(QString("The file %1 does not exist").arg(argv[2]));
            
            QSettings file(argv[2], QSettings::IniFormat);
            if (!file.childGroups().contains("BespinStyle"))
               error(QString("%1 is not a valid Bespin configuration").arg(argv[2]));
            
            if (!app.setStyle("Bespin"))
               error("Fatal: Bespin Style not found or loadable!");
            
            file.beginGroup("BespinStyle");
            // palette update =============================
            QPalette pal = app.palette();
            file.beginGroup("QPalette");
            QStringList list =
               file.value ( "active", Config::colors(pal, QPalette::Active) ).toStringList();
            Config::updatePalette(pal, QPalette::Active, list);
            list =
               file.value ( "active", Config::colors(pal, QPalette::Inactive) ).toStringList();
            Config::updatePalette(pal, QPalette::Inactive, list);
            list =
               file.value ( "active", Config::colors(pal, QPalette::Disabled) ).toStringList();
            Config::updatePalette(pal, QPalette::Disabled, list);
            file.endGroup();
            app.setPalette(pal);
            // ================================================
            static_cast<BStyle*>(app.style())->init(&file);
            
            file.endGroup();
            
         }
         else if (argc > 2) // demo - allow setting another style, but don't load custom settings
            app.setStyle(argv[2]);
         
         Ui::Demo ui;
         Dialog *window = new Dialog;
         ui.setupUi(window);
         QObject::connect (ui.rtl, SIGNAL(toggled(bool)),
                           window, SLOT(setLayoutDirection(bool)));
         window->show();
         return app.exec();
      }
      if (mode == 1 && argc == 3)
         return Config::sImport(argv[2]).isNull();
   }
   /** ================================================ */
   
   /** First make an application */
   QApplication app(argc, argv);
   /** Next make a config widget (from the constructor above) */
   Config *config = new Config;
   /** Next make a dialog from the widget - i wanna handle im and export in an extra tab*/
   BConfigDialog *window =
      new BConfigDialog(config, BConfigDialog::All &
                        ~(BConfigDialog::Import | BConfigDialog::Export));
   /** sets dialog width to 640 and height to auto */
   window->resize(640,-1);
   /** Show up the dialog */
   window->show();
   /** run the application! */
   return app.exec();
}
