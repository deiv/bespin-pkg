
#include "bconfig.h"
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QTextBrowser>

BConfigDialog::BConfigDialog(BConfig *config, QWidget *parent) : QDialog(parent) {
   
   QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
   QWidget *btn;
   
   btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Ok );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(save()));
   connect(btn, SIGNAL(clicked(bool)), this, SLOT(accept()));
   btn->setDisabled(true);
   connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   
   btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Save );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(save()));
   btn->setDisabled(true);
   connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   
   btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Apply );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(save()));
   btn->setDisabled(true);
   connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   
   btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Reset );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(reset()));
   btn->setDisabled(true);
   connect(config, SIGNAL(changed(bool)), btn, SLOT(setEnabled(bool)));
   
   btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::RestoreDefaults );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(defaults()));
   
   btn = (QWidget*)buttonBox->addButton ( QDialogButtonBox::Cancel );
   connect(btn, SIGNAL(clicked(bool)), this, SLOT(reject()));

   
   QVBoxLayout *vl = new QVBoxLayout;
   vl->addWidget(config);
   vl->addWidget(buttonBox);
   setLayout(vl);
}


BConfig::BConfig(QWidget *parent) : QWidget(parent) {
   infoItemHovered = false;
}

void BConfig::handleSettings(QWidget *w, QString entry, QVariant value) {
   SettingInfo info;
   info.defaultValue = value;
   info.initialValue = QVariant();
   info.entry = entry;
   _settings[w] = info;
   if (qobject_cast<QCheckBox*>(w))
      connect (w, SIGNAL(stateChanged(int)), this, SLOT(checkDirty()));
   else if (qobject_cast<QComboBox*>(w))
      connect (w, SIGNAL(currentIndexChanged(int)), this, SLOT(checkDirty()));
   else if (qobject_cast<QSlider*>(w))
      connect (w, SIGNAL(valueChanged(int)), this, SLOT(checkDirty()));
}

void BConfig::setDefaultContextInfo(QString info) {
   _defaultContextInfo = info;
}

void BConfig::setContextHelp(QWidget *w, QString help) {
   _contextHelps[w] = help;
   w->installEventFilter(this);
}

void BConfig::setContextHelp(QComboBox *c, QStringList & strings) {
   _comboHelps[c] = strings;
   ((QWidget*)c->view())->installEventFilter(this);
   c->installEventFilter(this);
   connect(c, SIGNAL(highlighted(int)), this, SLOT(setComboListInfo(int)));
   connect(c, SIGNAL(activated(int)), this, SLOT(setComboListInfo(int)));
}

void BConfig::setInfoBrowser(QTextBrowser *browser) {
   _infoBrowser = browser;
   _infoBrowser->installEventFilter(this);
}

void BConfig::checkDirty()
{
   QMap<QWidget*, SettingInfo>::iterator i;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      if (sender() == i.key())
      {
        SettingInfo *info = &(i.value());
         if (QComboBox *box = qobject_cast<QComboBox*>(i.key()))
         {
            if (box->itemData(box->currentIndex()).isValid())
               emit changed(box->itemData(box->currentIndex()) != info->initialValue);
            else
               emit changed(box->currentIndex() != info->initialValue.toInt());
         }
         else if (QCheckBox *box = qobject_cast<QCheckBox*>(i.key()))
            emit changed(box->isChecked() != info->initialValue.toBool());
         else
            qWarning("%s is not supported yet, feel free tro ask", i.key()->metaObject()->className());
         break;
      }
   }
}

void BConfig::reset() {
   QMap<QWidget*, SettingInfo>::iterator i;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      if (sender() == i.key())
      {
         SettingInfo *info = &(i.value());
         if (QComboBox *box = qobject_cast<QComboBox*>(i.key()))
         {
            int idx = box->findData(info->initialValue);
            if (idx == -1)
               box->setCurrentIndex(info->initialValue.toInt());
            else
               box->setCurrentIndex(idx);
         }
         else if (QCheckBox *box = qobject_cast<QCheckBox*>(i.key()))
            box->setChecked(info->initialValue.toBool());
         else
            qWarning("%s is not supported yet, feel free tro ask", i.key()->metaObject()->className());
         break;
      }
   }
}

void BConfig::defaults() {
   QMap<QWidget*, SettingInfo>::iterator i;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      if (sender() == i.key())
      {
         SettingInfo *info = &(i.value());
         if (QComboBox *box = qobject_cast<QComboBox*>(i.key()))
         {
            int idx = box->findData(info->defaultValue);
            if (idx == -1)
               box->setCurrentIndex(info->defaultValue.toInt());
            else
               box->setCurrentIndex(idx);
         }
         else if (QCheckBox *box = qobject_cast<QCheckBox*>(i.key()))
            box->setChecked(info->defaultValue.toBool());
         else
            qWarning("%s is not supported yet, feel free tro ask", i.key()->metaObject()->className());
         break;
      }
   }
}

void BConfig::setComboListInfo(int index) {
   if (index < 0)
      return;
   if (QComboBox *box = qobject_cast<QComboBox*>(sender())) {
      if (_comboHelps.value(box).count() < index + 1)
         return;
      infoItemHovered = true;
      _infoBrowser->setHtml(_comboHelps.value(box).at(index));
   }
}

void BConfig::resetInfo() {
   if (!infoItemHovered)
      _infoBrowser->setHtml(_defaultContextInfo);
}

void BConfig::setQSetting(const QString organisation, const QString application, const QString group) {
   _qsetting[0] = organisation;
   _qsetting[1] = application;
   _qsetting[2] = group;
}

void BConfig::loadSettings() {
   _infoBrowser->setHtml(_defaultContextInfo);
   
   QSettings settings(_qsetting[0], _qsetting[1]);
   settings.beginGroup(_qsetting[2]);
   
   QMap<QWidget*, SettingInfo>::iterator i;
   SettingInfo *info;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      info = &(i.value());
      info->initialValue = settings.value( info->entry, info->defaultValue);
      if (QComboBox *box = qobject_cast<QComboBox*>(i.key()))
      {
         int idx = box->findData(info->initialValue);
         if (idx == -1)
            box->setCurrentIndex(info->initialValue.toInt());
         else
            box->setCurrentIndex(idx);
      }
      else if (QCheckBox *box = qobject_cast<QCheckBox*>(i.key()))
         box->setChecked(info->initialValue.toBool());
      else
         qWarning("%s is not supported yet, feel free tro ask", i.key()->metaObject()->className());
   }
   
   settings.endGroup();
}

void BConfig::save() {
   
   QSettings settings(_qsetting[0], _qsetting[1]);
   settings.beginGroup(_qsetting[2]);
   
   QMap<QWidget*, SettingInfo>::iterator i;
   SettingInfo *info;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      info = &(i.value());
      info->initialValue = settings.value( info->entry, info->defaultValue);
      if (QComboBox *box = qobject_cast<QComboBox*>(i.key()))
      {
         if (box->itemData(box->currentIndex()).isValid())
            settings.setValue(info->entry, box->itemData(box->currentIndex()));
         else
            settings.setValue(info->entry, box->currentIndex());
      }
      else if (QCheckBox *box = qobject_cast<QCheckBox*>(i.key()))
         settings.setValue(info->entry, box->isChecked());
      else
         qWarning("%s is not supported yet, feel free tro ask", i.key()->metaObject()->className());
   }
   
   settings.endGroup();
}

bool BConfig::eventFilter ( QObject * o, QEvent * e) {
   if (e->type() == QEvent::Enter) {
      if (o == _infoBrowser) {
         infoItemHovered = true;
         return false;
      }
      infoItemHovered = false;
      if (QComboBox *box = qobject_cast<QComboBox*>(o)) {
         QMap<QComboBox*, QStringList>::iterator i;
         for (i = _comboHelps.begin(); i != _comboHelps.end(); ++i)
            if (o == i.key()) {
               infoItemHovered = true;
               _infoBrowser->setHtml(i.value().at(box->currentIndex()));
               return false;
            }
      }
      QMap<QWidget*, QString>::iterator i;
      for (i = _contextHelps.begin(); i != _contextHelps.end(); ++i)
         if (o == i.key()) {
            infoItemHovered = true;
            _infoBrowser->setHtml(i.value());
            return false;
         }
      return false;
   }
   else if (e->type() == QEvent::Leave) {
      infoItemHovered = false;
      QTimer::singleShot(300, this, SLOT(resetInfo()));
      return false;
   }
   return false;
}
