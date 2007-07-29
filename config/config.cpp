
#include "config.h"
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>

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
      GradGlass, GradButton
};

/** The Constructor - your first job! */
Config::Config(QWidget *parent) : BConfig(parent), loadedPal(0) {
   
   /** Setup the UI and geometry */
   ui.setupUi(this);
   ui.info->setMinimumWidth( 160 ); /** min width for the info browser */
   ui.info->setOpenExternalLinks( true ); /** i've an internet link here */
   resize(640,-1); /** sets dialog width to 640 and height to auto */
   
   /** Prepare the settings store, not of interest */
   ui.store->hide(); // don't show by default
   QSettings settings("Bespin", "Store");
   ui.store->addItems( settings.childGroups() );
   ui.store->sortItems();
   connect (ui.store, SIGNAL( itemClicked(QListWidgetItem *) ),
            this, SLOT( import2(QListWidgetItem *) ) );
   
   /** fill some comboboxes, not of interest */
   generateColorModes(ui.crProgressBar);
   generateColorModes(ui.crTabBar);
   generateColorModes(ui.crTabBarActive);
   
   generateGradientTypes(ui.gradButton);
   generateGradientTypes(ui.gradChoose);
   generateGradientTypes(ui.gradTab);
   
   /** 1. name the info browser, you'll need it to show up context help
   Can be any QTextBrowser on your UI form */
   setInfoBrowser(ui.info);
   /** 2. Define a context info that is displayed when no other context help is
   demanded */
   setDefaultContextInfo("<div align=\"center\">\
      <img src=\":/bespin.png\"/><br>\
   </div>\
   <b>Bespin Style</b><hr>\
   &copy;&nbsp;2006/2007 by Thomas L&uuml;bking<br>\
   Includes Design Ideas by\
   <ul type=\"disc\">\
      <li>Nuno Pinheiro</li>\
      <li>David Vignoni</li>\
      <li>Kenneth Wimer</li>\
   </ul>\
   <hr>\
   Visit <a href=\"http://cloudcity.sourceforge.net\">CloudCity.SourceForge.Net</a>");
   
   /** handleSettings(.) tells BConfig to take care (savwe load) of a widget
   In this case "ui.bgMode" is the widget on the form,
   "BackgroundMode" specifies the entry in the ini style config file and
   "3" is the default value for this entry*/
   handleSettings(ui.bgMode, "BackgroundMode", 3);
   
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
   
   handleSettings(ui.tabTransition, "TabTransition", 1);
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
   
   handleSettings(ui.checkMark, "CheckType", 1);
   handleSettings(ui.showMenuIcons, "ShowMenuIcons", false);
   handleSettings(ui.showScrollButtons, "ShowScrollButtons", false);
   setContextHelp(ui.showScrollButtons, "<b>Show Scrollbar buttons</b><hr>\
                  Seriously, honestly: when do you ever use the buttons to move\
                  a scrollbar slider? (ok, notebooks don't have a mousewheel...)");
   handleSettings(ui.menuShadow, "MenuShadow", false); // i have a compmgr running :P
   handleSettings(ui.crProgressBar, "role_progress", QPalette::Button);
   /** setContextHelp(.) attaches a context help string to a widget on your form */
   setContextHelp(ui.crProgressBar, "<b>ProgressBar Roles</b><hr>\
                  This is the \"done\" part of the Progressbar<br>\
                  Choose any mode you like - the other part is like the window");
   handleSettings(ui.crTabBar, "role_tab", QPalette::WindowText);
   setContextHelp(ui.crTabBar, "<b>Tabbar Role</b><hr>\
                  The color of the tabbar background<br>\
                  The Text color is chosen automatically");
   handleSettings(ui.crTabBarActive, "role_tabActive", QPalette::Button);
   setContextHelp(ui.crTabBarActive, "<b>Tabbar Active Item Role</b><hr>\
                  The color of the hovered or selected tab<br>\
                  The Text color is chosen automatically");
   handleSettings(ui.gradButton, "GradButton", GradNone);
   handleSettings(ui.gradChoose, "GradChoose", GradGlass);
   handleSettings(ui.gradTab, "GradTab", GradGloss);
   handleSettings(ui.fullButtonHover, "FullButtonHover", false);
   setContextHelp(ui.fullButtonHover, "<b>Fully filled hovered buttons</b><hr>\
                  This is especially a good idea if the contrast between the\
                  button and Window color is low and also looks ok with Glass/Gloss\
                  gradient settings - but may be toggled whenever you want");
   handleSettings(ui.sunkenButtons, "SunkenButtons", false);
   setContextHelp(ui.sunkenButtons, "<b>Sunken Buttons</b><hr>\
                  Somewhat experimental, at least i'm not happy with the look.\
                  Should be avoided with dark (near black) window and/or button\
                  color");
   
   /** setQSetting(.) tells BConfig to store values at
   "Company, Application, Group" - these strings are passed to QSettings */
   setQSetting("Bespin", "Style", "Style");
   
   /** you can call loadSettings() whenever you want, but (obviously)
   only items that have been propagated with handleSettings(.) are handled !!*/
   loadSettings();
}

/** reimplementation of the import functionality
1. we won't present a file dialog, but a listview
2. we wanna im/export the current palette as well
*/
void Config::import() {
   ui.info->hide();
   ui.store->show();
}

static QStringList colors(const QPalette &pal, QPalette::ColorGroup group) {
   QStringList list;
   for (int i = 0; i < QPalette::NColorRoles; i++)
      list << pal.color(group, (QPalette::ColorRole) i).name();
   return list;
}

void Config::import2(QListWidgetItem *item) {
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
   
   list = settings.value ( "disabled", colors(pal, QPalette::Inactive) ).toStringList();
   for (i = 0; i < QPalette::NColorRoles; i++)
      loadedPal->setColor ( QPalette::Disabled, (QPalette::ColorRole) i, QColor(list.at(i)) );

   settings.endGroup();
   settings.endGroup();
   
   ui.store->hide();
   ui.info->show();
}

void Config::save() {
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
void Config::saveAs() {
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
   QList<QListWidgetItem *> present =
      ui.store->findItems ( string, Qt::MatchExactly );
   if (!present.isEmpty()) {
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

int main(int argc, char *argv[])
{
   /** First make an application */
   QApplication app(argc, argv);
   /** Next make a config widget (from the constructor above) */
   Config *config = new Config;
   /** Next make a dialog from the widget */
   BConfigDialog *window = new BConfigDialog(config);
   /** Show up the dialog */
   window->show();
   /** run the application! */
   return app.exec();
}
