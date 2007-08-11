
#include "config.h"
#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QTimer>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>

#if ! EXECUTABLE

/** This function declares the kstyle config plugin, you may need to adjust it
for other plugins or won't need it at all, if you're not interested in a plugin */
extern "C"
{
   Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
   {
      /**Create our config dialog and reply it as plugin
      This is slightly different from the setup in a standalone dialog at the
      bottom of this file*/
      return new Config(parent);
   }
}

#endif

/** Gradient enumeration for the comboboxes, so that i don't have to handle the
integers - not of interest for you*/
enum GradientType {
   GradNone = 0, GradSimple, GradSunken, GradGloss,
      GradGlass, GradButton
};

static const char* defInfo =
"<div align=\"center\">\
   <img src=\":/bespin.png\"/><br>\
</div>\
<b>Bespin Style</b><hr>\
<p>\
   &copy;&nbsp;2006/2007 by Thomas L&uuml;bking<br>\
   Includes Design Ideas by\
   <ul type=\"disc\">\
      <li>Nuno Pinheiro</li>\
      <li>David Vignoni</li>\
      <li>Kenneth Wimer</li>\
   </ul>\
</p>\
   <hr>\
Visit <a href=\"http://cloudcity.sourceforge.net\">CloudCity.SourceForge.Net</a>";

static const char* manageInfo =
"<b>Settings management</b><hr>\
<p>\
   You can save your current settings (including colors from qconfig!) and\
   restore them later here.\
</p><p>\
   It's also possible to im and export settings from external files and share\
   them with others.\
</p><p>\
   You can also call the config dialog with the paramater \"demo\"\
<pre>\
   bespin-config demo [some style]\
</pre>\
   to test your settings on various widgets.\
</p><p>\
   If you want to test settings before importing them, call\
<pre>\
   bespin-config try &lt;some_settings.conf&gt;\
</pre>\
</p>";

/** The Constructor - your first job! */
Config::Config(QWidget *parent) : BConfig(parent), loadedPal(0), infoIsManage(false) {
   
   /** Setup the UI and geometry */
   ui.setupUi(this);
   ui.info->setMinimumWidth( 200 ); /** min width for the info browser */
   ui.info->setOpenExternalLinks( true ); /** i've an internet link here */
   
   /** Prepare the settings store, not of interest */
   QSettings settings("Bespin", "Store");
   ui.store->addItems( settings.childGroups() );
   ui.store->sortItems();
   connect (ui.btnStore, SIGNAL(clicked()), this, SLOT(store()));
   connect (ui.btnRestore, SIGNAL(clicked()), this, SLOT(restore()));
   connect (ui.store, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(restore()));
   connect (ui.btnImport, SIGNAL(clicked()), this, SLOT(import()));
   connect (ui.btnExport, SIGNAL(clicked()), this, SLOT(saveAs()));
   connect (ui.store, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(storedSettigSelected(QListWidgetItem *)));
   connect (ui.btnDelete, SIGNAL(clicked()), this, SLOT(remove()));
   connect (ui.generalTab, SIGNAL(currentChanged(int)), this, SLOT(handleDefInfo(int)));
   ui.btnRestore->setEnabled(false);
   ui.btnExport->setEnabled(false);
   ui.btnDelete->setEnabled(false);
   
   /** fill some comboboxes, not of interest */
   generateColorModes(ui.crProgressBar);
   generateColorModes(ui.crTabBar);
   generateColorModes(ui.crTabBarActive);
   generateColorModes(ui.crPopup);
   generateColorModes(ui.crMenuActive);
   
   generateGradientTypes(ui.gradButton);
   generateGradientTypes(ui.gradChoose);
   generateGradientTypes(ui.gradMenuItem);
   generateGradientTypes(ui.gradProgress);
   generateGradientTypes(ui.gradTab);
   
/** connection between the bgmode and the structure combo -
   not of interest*/
   connect(ui.bgMode, SIGNAL(currentIndexChanged(int)), this, SLOT(handleBgMode(int)));
   
   /** 1. name the info browser, you'll need it to show up context help
   Can be any QTextBrowser on your UI form */
   setInfoBrowser(ui.info);
   /** 2. Define a context info that is displayed when no other context help is
   demanded */
   setDefaultContextInfo(defInfo);
   
   /** handleSettings(.) tells BConfig to take care (savwe load) of a widget
   In this case "ui.bgMode" is the widget on the form,
   "BackgroundMode" specifies the entry in the ini style config file and
   "3" is the default value for this entry*/
   handleSettings(ui.bgMode, "Bg.Mode", 3);
   handleSettings(ui.structure, "Bg.Structure", 0);

   handleSettings(ui.sunkenButtons, "Btn.3dPos", 0);
   handleSettings(ui.checkMark, "Btn.CheckType", 0);
   handleSettings(ui.cushion, "Btn.Cushion", true);
   handleSettings(ui.fullButtonHover, "Btn.FullHover", true);
   handleSettings(ui.gradButton, "Btn.Gradient", GradButton);
   
   handleSettings(ui.gradChoose, "Chooser.Gradient", GradSunken);
   
   handleSettings(ui.crMenuActive, "Menu.ActiveRole", QPalette::Highlight);
   handleSettings(ui.gradMenuItem, "Menu.ItemGradient", GradNone);
   handleSettings(ui.showMenuIcons, "Menu.ShowIcons", false);
   handleSettings(ui.menuShadow, "Menu.Shadow", false); // i have a compmgr running :P
   handleSettings(ui.crPopup, "Menu.Role", QPalette::Window);
   
   handleSettings(ui.gradProgress, "Progress.Gradient", GradGloss);
   handleSettings(ui.crProgressBar, "Progress.Role", QPalette::Highlight);
   
   handleSettings(ui.showScrollButtons, "Scroll.ShowButtons", false);
   
   handleSettings(ui.crTabBarActive, "Tab.ActiveRole", QPalette::Highlight);
   handleSettings(ui.tabAnimSteps, "Tab.AnimSteps", 4);
   handleSettings(ui.gradTab, "Tab.Gradient", GradButton);
   handleSettings(ui.crTabBar, "Tab.Role", QPalette::Window);
   handleSettings(ui.tabTransition, "Tab.Transition", 1);
   
   QStringList strList;
   strList <<
      "<b>Plain (color)</b><hr>Select if you have a really lousy \
      machine or just hate structured backgrounds." <<
      
      "<b>Scanlines</b><hr>Wanna Aqua feeling?" <<
      
      "<b>Complex</b><hr>Several light gradients covering the whole window." <<
      
      "<b>Vertical Top/Bottom Gradient</b><hr>Simple gradient that brightens \
      on the upper and darkens on the lower end<br>(cheap, fallback suggestion 1)" <<
      
      "<b>Horizontal Left/Right Gradient</b><hr>Simple gradient that darkens \
      on left and right side." <<
      
      "<b>Vertical Center Gradient</b><hr>The window vertically brightens \
      to the center" <<
      
      "<b>Horizontal Center Gradient</b><hr>The window horizontally brightens \
      to the center (similar to Apples Brushed Metal, less cheap, \
      fallback suggestion 2)";
   
   /** if you call setContextHelp(.) with a combobox and pass a stringlist,
   the strings are attached to the combo entries and shown on select/hover */
   setContextHelp(ui.bgMode, strList);
   strList.clear();
   
   strList <<
      "<b>Jump</b><hr>No transition at all - fastest but looks stupid" <<
      
      "<b>ScanlineBlend</b><hr>Similar to CrossFade, but won't require \
      Hardware acceleration." <<
      
      "<b>SlideIn</b><hr>The new tab falls down from top" <<
      
      "<b>SlideOut</b><hr>The new tab rises from bottom" <<
      
      "<b>RollIn</b><hr>The new tab appears from Top/Bottom to center" <<
      
      "<b>RollOut</b><hr>The new tab appears from Center to Top/Bottom" <<
      
      "<b>OpenVertically</b><hr>The <b>old</b> Tab slides <b>out</b> \
      to Top and Bottom" <<
      
      "<b>CloseVertically</b><hr>The <b>new</b> Tab slides <b>in</b> \
      from Top and Bottom" <<
      
      "<b>OpenHorizontally</b><hr>The <b>old</b> Tab slides <b>out</b> \
      to Left and Right" <<
      
      "<b>CloseHorizontally</b><hr>The <b>new</b> Tab slides <b>in</b> \
      from Left and Right" <<
      
      "<b>CrossFade</b><hr>What you would expect - one fades out while the \
      other fades in.<br>\
      This is CPU hungry - better have GPU Hardware acceleration.";
   setContextHelp(ui.tabTransition, strList);
   
   setContextHelp(ui.tabAnimSteps, "<b>Tab Transition Steps</b><hr>\
                  How many steps the transition has.<br><b>Notice:</b> that this has\
                  impact on the animation speed - more steps result in a slower animation!");
   
   
   setContextHelp(ui.cushion, "<b>Cushion Mode</b><hr>\
                  By default, the buttons are kinda solid and will move towards\
                  the background when pressed.<br>\
                  If you check this, you'll get a more cushion kind look, i.e.\
                  the Button will be \"pressed in\"");

   setContextHelp(ui.showScrollButtons, "<b>Show Scrollbar buttons</b><hr>\
                  Seriously, honestly: when do you ever use the buttons to move\
                  a scrollbar slider? (ok, notebooks don't have a mousewheel...)");

   /** setContextHelp(.) attaches a context help string to a widget on your form */
   setContextHelp(ui.crProgressBar, "<b>ProgressBar Roles</b><hr>\
                  This is the \"done\" part of the Progressbar<br>\
                  Choose any mode you like - the other part is like the window");

   setContextHelp(ui.crPopup, "<b>Popup Menu Role</b><hr>\
                  Choose anything you like (hint: saturated colors annoy me :)<br>\
                  The Text color is chosen automatically<br>\
                  Selected items are like selected menubar items if you choose \"Window\" here,\
                  otherwise they appear inverted");

   setContextHelp(ui.crMenuActive, "<b>Selected Menubar Item Role</b><hr>\
                  You may choose any role here<br>\
                  Select \"WindowText\" if you want inversion.<br>\
                  <b>Warning!</b><br>If you select \"Window\" here and \"None\" \
                  below, the hovering is hardly indicated!");

   setContextHelp(ui.crTabBar, "<b>Tabbar Role</b><hr>\
                  The color of the tabbar background<br>\
                  The Text color is chosen automatically");

   setContextHelp(ui.crTabBarActive, "<b>Tabbar Active Item Role</b><hr>\
                  The color of the hovered or selected tab<br>\
                  The Text color is chosen automatically");

   setContextHelp(ui.fullButtonHover, "<b>Fully filled hovered buttons</b><hr>\
                  This is especially a good idea if the contrast between the\
                  button and Window color is low and also looks ok with Glass/Gloss\
                  gradient settings - but may be toggled whenever you want");

   
   /** setQSetting(.) tells BConfig to store values at
   "Company, Application, Group" - these strings are passed to QSettings */
   setQSetting("Bespin", "Style", "Style");
   
   /** you can call loadSettings() whenever you want, but (obviously)
   only items that have been propagated with handleSettings(.) are handled !!*/
   loadSettings();
   
   /** ===========================================
   You're pretty much done here - simple eh? ;)
    The following code reimplemets some BConfig functions
   (mainly to manage Qt color setttings)
   
   if you want a standalone app you may want to check the main() funtion
   at the end of this file as well - but there's nothing special about it...
    =========================================== */
}


static QStringList colors(const QPalette &pal, QPalette::ColorGroup group) {
   QStringList list;
   for (int i = 0; i < QPalette::NColorRoles; i++)
      list << pal.color(group, (QPalette::ColorRole) i).name();
   return list;
}

static void updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list) {
      for (int i = 0; i < QPalette::NColorRoles; i++)
         pal.setColor(group, (QPalette::ColorRole) i, list.at(i));
}

/** reimplemented - i just want to extract the data from the store */
void Config::saveAs() {
   
   QListWidgetItem *item = ui.store->currentItem();
   if (!item) return;
   
   QString filename = QFileDialog::getSaveFileName(parentWidget(),
      tr("Save Configuration"), QDir::home().path(), tr("Config Files (*.conf *.ini)"));
   
   
   QSettings store("Bespin", "Store");
   store.beginGroup(item->text());
   
   QSettings file(filename, QSettings::IniFormat);
   file.beginGroup("BespinStyle");
   
   file.setValue("StoreName", item->text());
   
   foreach (QString key, store.allKeys())
      file.setValue(key, store.value(key));
   
   store.endGroup();
   file.endGroup();

}

static QString sImport(const QString &filename) {
   
   if (!QFile::exists(filename))
      return QString();
   
   QSettings file(filename, QSettings::IniFormat);
   
   if (!file.childGroups().contains("BespinStyle"))
      return QString();
   
   file.beginGroup("BespinStyle");
   
   QString demandedName;
   QString storeName = demandedName = file.value("StoreName", "Imported").toString();
   
   QSettings store("Bespin", "Store");
   
   int i = 2;
   QStringList entries = store.childGroups();
   while (entries.contains(storeName))
      storeName = demandedName + '#' + QString::number(i++);
   
   store.beginGroup(storeName);
   foreach (QString key, file.allKeys()) {
      if (key == "StoreName")
         continue;
      else
         store.setValue(key, file.value(key));
   }
   
   store.endGroup();
   file.endGroup();
   return storeName;
}

/** reimplemented - i just want to merge the data into the store */
void Config::import() {
   
   QString filename = QFileDialog::getOpenFileName(parentWidget(),
      tr("Import Configuration"), QDir::home().path(), tr("Config Files (*.conf *.ini)"));
   
   QString storeName = sImport(filename);
   if (storeName.isNull()) {
      ui.store->addItem(storeName);
      ui.store->sortItems();
   }
}

/** addition to the import functionality
1. we won't present a file dialog, but a listview
2. we wanna im/export the current palette as well
*/
void Config::restore() {
   QListWidgetItem *item = ui.store->currentItem();
   setQSetting("Bespin", "Store", item->text());
   loadSettings(0, false);
   setQSetting("Bespin", "Style", "Style");
   
   /** import the color settings as well */
   if (!loadedPal)
      loadedPal = new QPalette;
   else
      emit changed(true); // we must update casue we loded probably different colors before
   
   QStringList list; int i;
   const QPalette &pal = QApplication::palette();
   QSettings settings("Bespin", "Store");
   settings.beginGroup(item->text());
   settings.beginGroup("QPalette");
   
   list = settings.value ( "active", colors(pal, QPalette::Active) ).toStringList();
   for (i = 0; i < QPalette::NColorRoles; i++)
      loadedPal->setColor ( QPalette::Active, (QPalette::ColorRole) i, QColor(list.at(i)) );
   
   list = settings.value ( "inactive", colors(pal, QPalette::Inactive) ).toStringList();
   for (i = 0; i < QPalette::NColorRoles; i++)
      loadedPal->setColor ( QPalette::Inactive, (QPalette::ColorRole) i, QColor(list.at(i)) );
   
   list = settings.value ( "disabled", colors(pal, QPalette::Disabled) ).toStringList();
   for (i = 0; i < QPalette::NColorRoles; i++)
      loadedPal->setColor ( QPalette::Disabled, (QPalette::ColorRole) i, QColor(list.at(i)) );

   settings.endGroup();
   settings.endGroup();
}
#include <QtDebug>
void Config::save() {
   /** manage the daemon */
   int former = savedValue(ui.bgMode).toInt();
   int current = ui.bgMode->currentIndex();
   if (current != former) {
      QSettings settings("Bespin");
      settings.beginGroup("Style");
      if (former == 2) // was complex -> stop daemon
         QProcess::startDetached ( settings.value("BgDaemon", "bespinPP").toString(),
                                   QStringList() << "stop" );
      else if (current == 2) // IS complex -> start daemon
         QProcess::startDetached ( settings.value("BgDaemon", "bespinPP").toString() );
      settings.endGroup();
   }
   BConfig::save();
   /** save the palette loaded from store to qt configuration */
   if (loadedPal) {
      QSettings settings("Trolltech");
      settings.beginGroup("Qt");
      settings.beginGroup("Palette");
      
      settings.setValue ( "active", colors(*loadedPal, QPalette::Active) );
      settings.setValue ( "inactive", colors(*loadedPal, QPalette::Inactive) );
      settings.setValue ( "disabled", colors(*loadedPal, QPalette::Disabled) );
      
      settings.endGroup();
      settings.endGroup();
   }
}

/** see above, we'll present a name input dialog here */
void Config::store() {
   bool ok, addItem = true;
   QString string =
      QInputDialog::getText ( parentWidget(), tr("Enter a Name"),
                              tr(""),
                              QLineEdit::Normal, "", &ok);
   if (!ok) return;
   if (string.isEmpty()) {
      QMessageBox::information ( parentWidget(), tr("Empty name"),
                                 tr("You entered an empty name - nothing will be saved!") );
      return;
   }
   if (!ui.store->findItems ( string, Qt::MatchExactly ).isEmpty()) {
      QMessageBox::StandardButton btn =
         QMessageBox::question ( parentWidget(), tr("Allready exists!"),
                                 tr("The name you entered (%1) allready exists.<br>\
                                    <b>Do you want to replace it?</b>").arg(string),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No );
      if (btn == QMessageBox::No) {
         saveAs();
         return;
      }
      addItem = false;
   }
   if (addItem) {
      ui.store->addItem(string);
      ui.store->sortItems();
   }
   setQSetting("Bespin", "Store", string);
   save();
   setQSetting("Bespin", "Style", "Style");
   
   /** Now let's save colors as well */
   QSettings settings("Bespin", "Store");
   settings.beginGroup(string);
   settings.beginGroup("QPalette");
   
   const QPalette &pal = QApplication::palette();
   settings.setValue ( "active", colors(pal, QPalette::Active) );
   settings.setValue ( "inactive", colors(pal, QPalette::Inactive) );
   settings.setValue ( "disabled", colors(pal, QPalette::Disabled) );
   
   settings.endGroup();
   settings.endGroup();
}

void Config::remove() {
   QListWidgetItem *item = ui.store->currentItem();
   if (!item) return;
   
   QSettings store("Bespin", "Store");
   store.beginGroup(item->text());
   store.remove("");
   store.endGroup();
   delete item;
}

void Config::storedSettigSelected(QListWidgetItem *item) {
   ui.btnRestore->setEnabled(item);
   ui.btnExport->setEnabled(item);
   ui.btnDelete->setEnabled(item);
}

void Config::handleBgMode(int idx) {
   ui.structure->setVisible(idx == 1);
   ui.labelStructure->setVisible(idx == 1);
}

void Config::handleDefInfo(int idx) {
   if (idx == 4) {
      infoIsManage = true;
      setDefaultContextInfo(manageInfo);
      ui.info->setHtml(manageInfo);
   }
   else if (infoIsManage) {
      setDefaultContextInfo(defInfo);
      ui.info->setHtml(defInfo);
      infoIsManage = false;
   }
}

/** The combobox filler you've read of several times before ;) */
void Config::generateColorModes(QComboBox *box) {
   box->clear();
   box->addItem("Window", QPalette::Window);
   box->addItem("Window Text", QPalette::WindowText);
   box->addItem("Base (text editor)", QPalette::Base);
   box->addItem("Text (text editor)", QPalette::Text);
   box->addItem("Button", QPalette::Button);
   box->addItem("Button Text", QPalette::ButtonText);
   box->addItem("Highlight", QPalette::Highlight);
   box->addItem("Highlighted Text", QPalette::HighlightedText);
}

void Config::generateGradientTypes(QComboBox *box) {
   box->clear();
   box->addItem("None");
   box->addItem("Simple (pretty dull)");
   box->addItem("Sunken");
   box->addItem("Gloss");
   box->addItem("Glass");
   box->addItem("Button (Flat)");
}

/** The main function, you must provide this if you want an executable*/

#if EXECUTABLE

#include "ui_uiDemo.h"
#include "dialog.h"

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
               file.value ( "active", colors(pal, QPalette::Active) ).toStringList();
            updatePalette(pal, QPalette::Active, list);
            list =
               file.value ( "active", colors(pal, QPalette::Inactive) ).toStringList();
            updatePalette(pal, QPalette::Inactive, list);
            list =
               file.value ( "active", colors(pal, QPalette::Disabled) ).toStringList();
            updatePalette(pal, QPalette::Disabled, list);
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
         return sImport(argv[2]).isNull();
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
#endif
