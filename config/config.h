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
   void store();
   void restore();
   void save(); // to store colors to qt configuration - in case
   void import();
   void saveAs();
private:
   /** This is the UI created with Qt Designer and included by ui_config.h */
   Ui::Config ui;
   
   /** Just some functions to fill the comboboxes, not really of interest */
   void generateColorModes(QComboBox *box);
   void generateGradientTypes(QComboBox *box);
   
   QPalette *loadedPal;
private slots:
   void storedSettigSelected(QListWidgetItem *);
   void remove();
};

#endif
