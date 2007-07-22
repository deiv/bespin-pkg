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
public slots:
   /** We'll reimplement the im/export functions to handle color settings as well*/
   void import();
   void saveAs();
   void save(); // to store colors to qt configuration - in case
private:
   /** This is the UI created with Qt Designer and included by ui_config.h */
   Ui::Config ui;
   
   /** Just some functions to fill the comboboxes, not really of interest */
   void generateColorModes(QComboBox *box);
   void generateGradientTypes(QComboBox *box);
   
   QPalette *loadedPal;
   
private slots:
   /** Does the import job for teh import reimplemetation */
   void import2(QListWidgetItem *);
};

#endif
