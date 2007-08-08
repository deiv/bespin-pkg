
#include "bconfig.h"
#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QDir>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QTextBrowser>

BConfigDialog::BConfigDialog(BConfig *config, QWidget *parent) :
QDialog(parent, Qt::Window) {
   
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
   
   btn = (QWidget*)buttonBox->addButton ( tr("Save As..."), QDialogButtonBox::ApplyRole );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(saveAs()));
   
   btn = (QWidget*)buttonBox->addButton ( tr("Load..."), QDialogButtonBox::ActionRole );
   connect(btn, SIGNAL(clicked(bool)), config, SLOT(import()));
   
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

void BConfig::saveAs() {
   QString filename = QFileDialog::getSaveFileName(parentWidget(),
      tr("Save Configuration"), QDir::home().path(), tr("Config Files (*.conf *.ini)"));
   QSettings settings(filename, QSettings::IniFormat);
   _save(&settings, false);
}

void BConfig::import() {
   QString filename = QFileDialog::getOpenFileName(parentWidget(),
      tr("Import Configuration"), QDir::home().path(), tr("Config Files (*.conf *.ini)"));
   QSettings settings(filename, QSettings::IniFormat);
   loadSettings(&settings, false);
}

QVariant BConfig::defaultValue(QWidget *w) const {
   return _settings.value(w).defaultValue;
}

QVariant BConfig::initialValue(QWidget *w) const {
   return _settings.value(w).initialValue;
}

QVariant BConfig::savedValue(QWidget *w) const {
   return _settings.value(w).savedValue;
}

void BConfig::handleSettings(QWidget *w, QString entry, QVariant value) {
   SettingInfo info;
   info.defaultValue = value;
   info.initialValue = info.savedValue = QVariant();
   info.entry = entry;
   _settings[w] = info;
   if (qobject_cast<QAbstractButton*>(w))
      connect (w, SIGNAL(toggled(bool)), this, SLOT(checkDirty()));
   else if (qobject_cast<QComboBox*>(w))
      connect (w, SIGNAL(currentIndexChanged(int)), this, SLOT(checkDirty()));
   else if (qobject_cast<QAbstractSlider*>(w))
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
   bool dirty = false;
   QMap<QWidget*, SettingInfo>::iterator i;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      if (!sender() || (sender() == i.key()))
      {
         SettingInfo *info = &(i.value());
         dirty = dirty || variant(i.key()) != info->savedValue;
         
         if (sender() || dirty)
            break;
      }
   }
   emit changed(dirty);
}

void BConfig::reset() {
   QMap<QWidget*, SettingInfo>::iterator i;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      if (sender() == i.key())
      {
         setVariant(i.key(), (&(i.value()))->defaultValue);
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
         setVariant(i.key(), (&(i.value()))->defaultValue);
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
#include <QtDebug>
void BConfig::loadSettings(QSettings *settings, bool updateInit) {
   _infoBrowser->setHtml(_defaultContextInfo);
   bool delSettings = false;
   if (!settings) {
      delSettings = true;
      settings = new QSettings(_qsetting[0], _qsetting[1]);
   }
   
   settings->beginGroup(_qsetting[2]);
   
   QMap<QWidget*, SettingInfo>::iterator i;
   SettingInfo *info; QVariant value;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      info = &(i.value());
      value = settings->value( info->entry, info->defaultValue);
      if (updateInit)
         info->savedValue = info->initialValue = value;
      setVariant(i.key(), value);
   }
   
   settings->endGroup();
   if (delSettings)
      delete settings;
}

void BConfig::save() {
   QSettings settings(_qsetting[0], _qsetting[1]);
   _save(&settings);
}

QVariant BConfig::variant(const QWidget *w) const {
   if (const QComboBox *box = qobject_cast<const QComboBox*>(w)) {
      if (box->itemData(box->currentIndex()).isValid())
         return box->itemData(box->currentIndex());
      return box->currentIndex();
   }
   else if (const QCheckBox *box = qobject_cast<const QCheckBox*>(w))
      return box->isChecked();
   
   qWarning("%s is not supported yet, feel free tro ask", w->metaObject()->className());
   return QVariant();
}

bool BConfig::setVariant(QWidget *w, const QVariant &v) const {
   if (QComboBox *box = qobject_cast<QComboBox*>(w)) {
      int idx = box->findData(v);
      if (idx == -1)
         box->setCurrentIndex(v.toInt());
      else
         box->setCurrentIndex(idx);
   }
   else if (QCheckBox *box = qobject_cast<QCheckBox*>(w))
      box->setChecked(v.toBool());
   else {
      qWarning("%s is not supported yet, feel free tro ask", w->metaObject()->className());
      return false;
   }
   return true;
}

void BConfig::_save(QSettings *settings, bool makeDirty) {
   
   bool delSettings = false;
   if (!settings) {
      delSettings = true;
      settings = new QSettings(_qsetting[0], _qsetting[1]);
   }
   
   settings->beginGroup(_qsetting[2]);
   
   QMap<QWidget*, SettingInfo>::iterator i;
   SettingInfo *info;
   for (i = _settings.begin(); i != _settings.end(); ++i)
   {
      QVariant value = variant(i.key());
      if (value.isValid()) {
         info = &(i.value());
         settings->setValue(info->entry, value);
         if (makeDirty)
            info->savedValue = value;
      }
   }
   settings->endGroup();
   
   if (delSettings)
      delete settings;
   
   if (makeDirty)
      emit changed(false);
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
