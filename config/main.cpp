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
#include <QTimer>

#if PUSHER
#include <QX11Info>
#include "bpp.h"
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#define CHAR(_QSTRING_) _QSTRING_.toLatin1().data()

class BStyle : public QStyle {
public:
   BStyle() : QStyle (){}
   virtual void init(const QSettings *settings) = 0;
};

static int error(const QString & string) {
   qWarning("Error: %s", CHAR(string));
   return 1;
}

static int usage(const char* appname) {
   printf(
"Usage:\n\
==========================\n\
%s [config]\t\t\t\tConfigure the Bespin Style\n\
%s presets\t\t\t\tList the available Presets\n\
%s demo [style]\t\t\tLaunch a demo, you can pass other stylenames\n\
%s try <some_config.bespin>\tTry out an exported setting\n\
%s sshot <some_file.png> [preset] [width]\tSave a screenshot\n\
%s load <some_preset>\tLoad a preset to current settings\n\
%s import <some_config.bespin>\tImport an exported setting\n\
%s export <some_preset> <some_config.bespin>\tImport an exported setting\n\
%s %s", appname, appname, appname, appname, appname, appname, appname, appname,
#if PUSHER
appname, "pusher [stop]\t\t\tThe Bg Pixmap daemon - you should not have to call this\n"
#else
"",""
#endif
   );
   return 1;
}

enum Mode {
   Invalid = 0, Configure, Presets,
      Import, Export, Load,
      Demo, Try, Screenshot, Pusher };
#include <QMessageBox>
int main(int argc, char *argv[])
{
   Mode mode = Configure;
   QApplication *app = 0;
   QString title;
   if (argc > 1) {
      mode = Invalid;
      if (!qstrcmp( argv[1], "config" )) mode = Configure;
      else if (!qstrcmp( argv[1], "presets" )) mode = Presets;
      else if (!qstrcmp( argv[1], "import" )) mode = Import;
      else if (!qstrcmp( argv[1], "export" )) mode = Export;
      else if (!qstrcmp( argv[1], "demo" )) mode = Demo;
      else if (!qstrcmp( argv[1], "try" )) mode = Try;
      else if (!qstrcmp( argv[1], "sshot" )) mode = Screenshot;
      else if (!qstrcmp( argv[1], "load" )) mode = Load;
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
//       window->resize(848,480);
      window->adjustSize();
//       QMessageBox::warning(0,"bespin error!", "this is a really bad error - look: there's a white rabbit behind you!");
      window->show();
      return app->exec();
   }
   case Presets: {
      QSettings store("Bespin", "Store");
      foreach (QString name, store.childGroups())
         printf("%s\n", CHAR(name));
      return 0;
   }
   case Load: {
      if (argc < 3)
         return error("you lack <some_preset>");
      return Config::load(argv[2]);
   }
   case Import: {
      if (argc < 3)
         return error("you lack <some_config.bespin>");
      if (!QFile::exists(argv[2]))
         return error(QString("The file %1 does not exist").arg(argv[2]));
      return Config::sImport(argv[2]).isNull();
   }
   case Export: {
      if (argc < 3)
         return error("you lack <some_preset> <some_config.bespin>");
      if (argc < 4)
         return error("you lack <some_config.bespin>");
      bool suc = Config::sExport(argv[2], argv[3]);
      if (!suc)
         return error("export failed (invalid preset name?)");
      return 0;
   }
   case Try: {
      app = new QApplication(argc, argv);
      if (argc < 3)
         return error("you lack <some_config.bespin>");
      
      if (!QFile::exists(argv[2]))
         return error(QString("The file %1 does not exist").arg(argv[2]));
      
      QSettings file(argv[2], QSettings::IniFormat);
      if (!file.childGroups().contains("BespinStyle"))
         return error(QString("%1 is not a valid Bespin configuration").arg(argv[2]));
      
      if (!app->setStyle("Bespin"))
         return error("Fatal: Bespin Style not found or loadable!");
      
      file.beginGroup("BespinStyle");
      title = file.value ( "StoreName", "" ).toString();
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
      if (title.isEmpty())
         title = "Bespin Demo";
      if (!app)
         app = new QApplication(argc, argv);
      Dialog *window = new Dialog;
      if (mode == Demo && argc > 2) {// allow setting another style
         app->setStyle(argv[2]);
         app->setPalette(app->style()->standardPalette());
         title = QString("Bespin Demo / %1 Style").arg(argv[2]);
      }
      else {
         char *preset = getenv("BESPIN_PRESET");
         if (preset) title = QString("%1 @ Bespin Demo").arg(preset);
      }
      Ui::Demo ui;
      ui.setupUi(window);
      ui.tabWidget->setCurrentIndex(0);
      QObject::connect (ui.rtl, SIGNAL(toggled(bool)),
                        window, SLOT(setLayoutDirection(bool)));
      window->setWindowTitle ( title );
      window->show();
      return app->exec();
   }
   case Screenshot: {
      if (argc < 3)
         return error("you lack <some_output.png>");
#ifndef Q_WS_WIN
      if (argc > 3)
         setenv("BESPIN_PRESET", argv[3], 1);
#endif
      app = new QApplication(argc, argv);
      Dialog *window = new Dialog;
      Ui::Demo ui;
      ui.setupUi(window);
      ui.tabWidget->setCurrentIndex(0);
      ui.buttonBox->button(QDialogButtonBox::Ok)->setAttribute( Qt::WA_UnderMouse );
      ui.demoBrowser->setAttribute( Qt::WA_UnderMouse );
//       ui.demoLineEdit->setFocus(Qt::MouseFocusReason);
      
      QImage image(window->size(), QImage::Format_RGB32);
      window->showMinimized();
      window->QDialog::render(&image);
      if (argc > 4)
         image = image.scaledToWidth(atoi(argv[4]), Qt::SmoothTransformation);
      image.save ( argv[2], "png" );
      return 0;
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
