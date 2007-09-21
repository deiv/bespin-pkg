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
#include <QDir>
#include <QFile>
#include <QSettings>

#if PUSHER
#include <QX11Info>
#include "bpp.h"
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

class BStyle : public QStyle {
public:
   BStyle() : QStyle (){}
   virtual void init(const QSettings *settings) = 0;
};

static int error(const QString & string) {
   qWarning("Error: %s", string.toLatin1().data());
   return 1;
}

static int usage(const char* appname) {
   qWarning(
"Usage:\n\
==========================\n\
%s [config]\t\t\t\tConfigure the Bespin Style\n\
%s demo [style]\t\t\tLaunch a demo, you can pass other stylenames\n\
%s try <some_config.bespin.conf>\tTry out an exported setting\n\
%s import <some_config.bespin.conf>\tImport an exported setting\n\
%s %s", appname, appname, appname, appname,
#if PUSHER
appname, "pusher [stop]\t\t\tThe Bg Pixmap daemon - you should not have to call this\n"
#else
"",""
#endif
   );
   return 1;
}

enum Mode {Invalid = 0,  Configure, Import, Demo, Try, Pusher };

int main(int argc, char *argv[])
{
   Mode mode = Configure;
   QApplication *app = 0;
   if (argc > 1) {
      mode = Invalid;
      if (!qstrcmp( argv[1], "config" )) mode = Configure;
      if (!qstrcmp( argv[1], "import" )) mode = Import;
      else if (!qstrcmp( argv[1], "demo" )) mode = Demo;
      else if (!qstrcmp( argv[1], "try" )) mode = Try;
#if PUSHER
      else if (!qstrcmp( argv[1], "pusher" )) mode = Pusher;
#endif
   }

   switch (mode) {
   case Configure: {
      app = new QApplication(argc, argv);
      Config *config = new Config;
      BConfigDialog *window =
         new BConfigDialog(config, BConfigDialog::All &
                           ~(BConfigDialog::Import | BConfigDialog::Export));
      window->resize(640,-1);
      window->show();
      return app->exec();
   }
   case Import: {
      if (argc < 3)
         return error("you lack <some_config.bespin.conf>");
      if (!QFile::exists(argv[2]))
         return error(QString("The file %1 does not exist").arg(argv[2]));
      return Config::sImport(argv[2]).isNull();
   }
   case Try: {
      app = new QApplication(argc, argv);
      if (argc < 3)
         return error("you lack <some_config.bespin.conf>");
      
      if (!QFile::exists(argv[2]))
         return error(QString("The file %1 does not exist").arg(argv[2]));
      
      QSettings file(argv[2], QSettings::IniFormat);
      if (!file.childGroups().contains("BespinStyle"))
         return error(QString("%1 is not a valid Bespin configuration").arg(argv[2]));
      
      if (!app->setStyle("Bespin"))
         return error("Fatal: Bespin Style not found or loadable!");
      
      file.beginGroup("BespinStyle");
            // palette update =============================
      QPalette pal = app->palette();
      file.beginGroup("QPalette");
      QStringList list =
         file.value ( "active", Config::colors(pal, QPalette::Active) ).toStringList();
      Config::updatePalette(pal, QPalette::Active, list);
      list =
         file.value ( "inactive", Config::colors(pal, QPalette::Inactive) ).toStringList();
      Config::updatePalette(pal, QPalette::Inactive, list);
      list =
         file.value ( "disabled", Config::colors(pal, QPalette::Disabled) ).toStringList();
      Config::updatePalette(pal, QPalette::Disabled, list);
      file.endGroup();
      app->setPalette(pal);
            // ================================================
      static_cast<BStyle*>(app->style())->init(&file);
      
      file.endGroup();
   }
   case Demo: {
      if (!app)
         app = new QApplication(argc, argv);
      if (mode == Demo && argc > 2) // allow setting another style
         app->setStyle(argv[2]);
      Ui::Demo ui;
      Dialog *window = new Dialog;
      ui.setupUi(window);
      ui.tabWidget->setCurrentIndex(0);
      QObject::connect (ui.rtl, SIGNAL(toggled(bool)),
                        window, SLOT(setLayoutDirection(bool)));
      window->show();
      return app->exec();
   }
#if PUSHER
   case Pusher: {
      QFile lock(QDir::tempPath() + "bespinPP.lock");
      
      if (argc > 2 && !qstrcmp( argv[2], "stop" )) {
         if (!lock.exists()) // nope...
            return 1;
         lock.open(QIODevice::ReadOnly);
         QString pid = lock.readLine();
         lock.close();
         kill(pid.toUInt(), SIGTERM);
         return 0;
      }
      
      if (lock.exists()) // we've allready an instance
         return 1;
      
      // make a lock!
      lock.open(QIODevice::WriteOnly);
      lock.write(QString::number(getpid()).toAscii());
      lock.close();
      
      int argc_ = argc;
      BespinPP app(argc_, argv);
      signal (SIGTERM, BespinPP::cleanup); signal (SIGINT, BespinPP::cleanup);
      signal (SIGQUIT, BespinPP::cleanup); signal (SIGKILL, BespinPP::cleanup);
      signal (SIGHUP, BespinPP::cleanup); signal (SIGSEGV, BespinPP::cleanup);
      int ret = app.exec();
      return ret;
   }
#endif
   default:
      return usage(argv[0]);
   }
}
