
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
#include <QValidator>

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

/** Gradient enumeration for the comboboxes, so that i don't have to handle the
integers - not of interest for you*/
enum GradientType {
   GradNone = 0, GradSimple, GradSunken, GradGloss,
      GradGlass, GradButton, GradCloud
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
   bespin demo [some style]\
</pre>\
   to test your settings on various widgets.\
</p><p>\
   If you want to test settings before importing them, call\
<pre>\
   bespin try &lt;some_settings.conf&gt;\
</pre>\
</p>";

/** Intenal class for the PW Char entry - not of interest */

static ushort unicode(const QString &string) {
   if (string.length() == 1)
      return string.at(0).unicode();
   uint n = string.toUShort();
   if (!n)
      n = string.toUShort(0,16);
   if (!n)
      n = string.toUShort(0,8);
   return n;
}

class UniCharValidator : public QValidator {
public:
   UniCharValidator( QObject * parent ) : QValidator(parent){}
   virtual State validate ( QString & input, int & ) const {
      if (input.length() == 0)
         return Intermediate;
      if (input.length() == 1)
         return Acceptable;
      if (input.length() == 2 && input.at(0) == '0' && input.at(1).toLower() == 'x')
         return Intermediate;
      if (unicode(input))
         return Acceptable;
      return Invalid;
   }
};

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
   ui.btnStore->setAutoDefault ( false );
   ui.btnRestore->setAutoDefault ( false );
   ui.btnImport->setAutoDefault ( false );
   ui.btnExport->setAutoDefault ( false );
   ui.btnDelete->setAutoDefault ( false );
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
   ui.storeLine->hide();
   
   /** fill some comboboxes, not of interest */
   generateColorModes(ui.crProgressBar);
   generateColorModes(ui.crTabBar);
   generateColorModes(ui.crTabBarActive);
   generateColorModes(ui.crPopup);
   generateColorModes(ui.crMenuActive);
   generateColorModes(ui.btnRole);
   generateColorModes(ui.btnActiveRole);
   generateColorModes(ui.headerRole);
   
   generateGradientTypes(ui.gradButton);
   generateGradientTypes(ui.gradChoose);
   generateGradientTypes(ui.gradMenuItem);
   generateGradientTypes(ui.gradProgress);
   generateGradientTypes(ui.gradTab);
   generateGradientTypes(ui.gradScroll);
   
   
   QSettings csettings("Bespin", "Config");
   QStringList strList =
      csettings.value ( "UserPwChars", QStringList() ).toStringList();
   ushort n;
   foreach (QString str, strList) {
      n = str.toUShort(0,16);
      if (n)
         ui.pwEchoChar->addItem(QChar(n), n);
   }
   strList.clear();
   ui.pwEchoChar->addItem(QChar(0x26AB), 0x26AB);
   ui.pwEchoChar->addItem(QChar(0x2022), 0x2022);
   ui.pwEchoChar->addItem(QChar(0x2055), 0x2055);
   ui.pwEchoChar->addItem(QChar(0x220E), 0x220E);
   ui.pwEchoChar->addItem(QChar(0x224E), 0x224E);
   ui.pwEchoChar->addItem(QChar(0x25AA), 0x25AA);
   ui.pwEchoChar->addItem(QChar(0x25AC), 0x25AC);
   ui.pwEchoChar->addItem(QChar(0x25AC), 0x25AC);
   ui.pwEchoChar->addItem(QChar(0x25A0), 0x25A0);
   ui.pwEchoChar->addItem(QChar(0x25CF), 0x25CF);
   ui.pwEchoChar->addItem(QChar(0x2605), 0x2605);
   ui.pwEchoChar->addItem(QChar(0x2613), 0x2613);
   ui.pwEchoChar->addItem(QChar(0x26A1), 0x26A1);
   ui.pwEchoChar->addItem(QChar(0x2717), 0x2717);
   ui.pwEchoChar->addItem(QChar(0x2726), 0x2726);
   ui.pwEchoChar->addItem(QChar(0x2756), 0x2756);
   ui.pwEchoChar->addItem(QChar(0x2756), 0x2756);
   ui.pwEchoChar->addItem(QChar(0x27A4), 0x27A4);
   ui.pwEchoChar->addItem(QChar(0xa4), 0xa4);
   ui.pwEchoChar->addItem("|", '|');
   ui.pwEchoChar->addItem(":", ':');
   ui.pwEchoChar->addItem("*", '*');
   ui.pwEchoChar->lineEdit()->
      setValidator(new UniCharValidator(ui.pwEchoChar->lineEdit()));
   connect (ui.pwEchoChar->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT (learnPwChar()));
   ui.pwEchoChar->setInsertPolicy(QComboBox::NoInsert);
   
   
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

   handleSettings(ui.sunkenButtons, "Btn.Layer", 0);
   handleSettings(ui.checkMark, "Btn.CheckType", 0);
   handleSettings(ui.cushion, "Btn.Cushion", true);
   handleSettings(ui.fullButtonHover, "Btn.FullHover", true);
   handleSettings(ui.gradButton, "Btn.Gradient", GradButton);
   handleSettings(ui.btnRole, "Btn.Role", QPalette::Window);
   handleSettings(ui.btnActiveRole, "Btn.ActiveRole", QPalette::Button);
   
   handleSettings(ui.gradChoose, "Chooser.Gradient", GradSunken);
   
   handleSettings(ui.pwEchoChar, "Input.PwEchoChar", 0x26AB);
   
   handleSettings(ui.crMenuActive, "Menu.ActiveRole", QPalette::Highlight);
   handleSettings(ui.gradMenuItem, "Menu.ItemGradient", GradNone);
   handleSettings(ui.showMenuIcons, "Menu.ShowIcons", false);
   handleSettings(ui.menuShadow, "Menu.Shadow", false); // i have a compmgr running :P
   handleSettings(ui.crPopup, "Menu.Role", QPalette::Window);
   
   handleSettings(ui.gradProgress, "Progress.Gradient", GradGloss);
   handleSettings(ui.crProgressBar, "Progress.Role", QPalette::Highlight);
   
   handleSettings(ui.showScrollButtons, "Scroll.ShowButtons", false);
   handleSettings(ui.scrollSunken, "Scroll.Sunken", false);
   handleSettings(ui.gradScroll, "Scroll.Gradient", GradButton);
   handleSettings(ui.scrollGroove, "Scroll.Groove", false);
   
   handleSettings(ui.crTabBarActive, "Tab.ActiveRole", QPalette::Highlight);
   handleSettings(ui.tabAnimSteps, "Tab.AnimSteps", 4);
   handleSettings(ui.gradTab, "Tab.Gradient", GradButton);
   handleSettings(ui.crTabBar, "Tab.Role", QPalette::Window);
   handleSettings(ui.tabTransition, "Tab.Transition", 1);

   handleSettings(ui.headerRole, "View.HeaderRole", QPalette::Text);
   
   /** setContextHelp(.) attaches a context help string to a widget on your form */
   setContextHelp(ui.btnRoles, "<b>Button Colors</b><hr>\
                  The default and the hovered color of a button.<br>\
                  <b>Notice:</b> It's strongly suggested to select \"Button\" to\
                  (at least and best excatly) one of the states!");
   
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
   
   setContextHelp(ui.pwEchoChar, "<b>Pasword Echo Character</b><hr>\
                  The character that is displayed instead of the real text when\
                  you enter e.g. a password.<br>\
                  You can enter any char or unicode number here.\
                  <b>Notice:</b> That not all fontsets may provide all unicode characters!");
                  
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

   ui.generalTab->setCurrentIndex(0);
}


QStringList Config::colors(const QPalette &pal, QPalette::ColorGroup group) {
   QStringList list;
   for (int i = 0; i < QPalette::NColorRoles; i++)
      list << pal.color(group, (QPalette::ColorRole) i).name();
   return list;
}

void Config::updatePalette(QPalette &pal, QPalette::ColorGroup group, const QStringList &list) {
      for (int i = 0; i < QPalette::NColorRoles; i++)
         pal.setColor(group, (QPalette::ColorRole) i, list.at(i));
}

/** reimplemented - i just want to extract the data from the store */
static QString lastPath = QDir::home().path();
void Config::saveAs() {
   
   QListWidgetItem *item = ui.store->currentItem();
   if (!item) return;
   
   QString filename = QFileDialog::getSaveFileName(parentWidget(),
      tr("Save Configuration"), lastPath, tr("Config Files (*.conf *.ini)"));
   
   
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

QString Config::sImport(const QString &filename) {
   
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
      tr("Import Configuration"), lastPath, tr("Config Files (*.conf *.ini)"));
   
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
         QProcess::startDetached ( settings.value("BgDaemon", "bespin pusher").toString().append(" stop") );
      else if (current == 2) // IS complex -> start daemon
         QProcess::startDetached ( settings.value("BgDaemon", "bespin pusher").toString() );
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
   ui.storeLine->setText("Enter a name or select an item above");
   ui.storeLine->selectAll();
   ui.storeLine->show();
   ui.storeLine->setFocus();
   connect (ui.storeLine, SIGNAL(returnPressed()),
            this, SLOT(store2a()));
   connect (ui.store, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(store2b(QListWidgetItem *)));
}

void Config::store2a() {
   if (sender() != ui.storeLine)
      return;
   const QString &string = ui.storeLine->text();
   if (string.isEmpty()) {
      ui.storeLine->setText("Valid names have some chars...");
      return;
   }
   if (!ui.store->findItems ( string, Qt::MatchExactly ).isEmpty()) {
      ui.storeLine->setText("Item allready exists, please click it to replace it!");
      return;
   }
   disconnect (ui.storeLine, SIGNAL(returnPressed()),
            this, SLOT(store2a()));
   disconnect (ui.store, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(store2b(QListWidgetItem *)));
   ui.storeLine->hide();
   store3( string, true );
}

void Config::store2b(QListWidgetItem* item) {
   disconnect (ui.storeLine, SIGNAL(returnPressed()),
            this, SLOT(store2a()));
   disconnect (ui.store, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(store2b(QListWidgetItem *)));
   ui.storeLine->hide();
   store3( item->text(), false );
}
/** real action */
void Config::store3( const QString &string, bool addItem ) {

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
   if (idx == 0) {
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

void Config::learnPwChar() {
   ushort n = unicode(ui.pwEchoChar->lineEdit()->text());
   if (ui.pwEchoChar->findData(n) != -1)
      return;
   ui.pwEchoChar->insertItem(0, QChar(n), n);
   QSettings settings("Bespin", "Config");
   QStringList list = settings.value ( "UserPwChars", QStringList() ).toStringList();
   list << QString::number( n, 16 );
   settings.setValue("UserPwChars", list);
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
   box->addItem("Cloudy");
}
