#ifndef CONFIG_H
#define CONFIG_H

#include "bconfig.h"
#include "ui_config.h"

class Config : public BConfig /** <-- inherit BConfig */
{
   Q_OBJECT
public:
   Config(QWidget *parent = 0L);
private:
   /** This is the UI created with Qt Designer and included by ui_config.h */
   Ui::Config ui;
   
   /** Just some functions to fill the comboboxes, not really of interest */
   void generateColorModes(QComboBox *box);
   void generateGradientTypes(QComboBox *box);
};

#endif
