#ifndef BCONFIG_H
#define BCONFIG_H

#include <QDialog>
#include <QMap>
#include <QVariant>
class QComboBox;
class QTextBrowser;
class QSettings;

class BConfig : public QWidget
{
   Q_OBJECT
public:
   BConfig(QWidget *parent = 0L);
   virtual QVariant defaultValue(QWidget *) const;
   virtual void handleSettings(QWidget *w, const QString entry, QVariant defaultValue);
   virtual QVariant initialValue(QWidget *) const;
   virtual QVariant savedValue(QWidget *) const;
   virtual void setContextHelp(QWidget *w, QString help);
   virtual void setContextHelp(QComboBox *c, QStringList & strings);
   virtual void setInfoBrowser(QTextBrowser *browser);
   virtual void setQSetting(const QString organisation, const QString application, const QString group);
   virtual void setDefaultContextInfo(QString info);
protected:
   typedef struct {
      QVariant defaultValue;
      QVariant initialValue;
      QVariant savedValue;
      QString entry;
   } SettingInfo;
   virtual bool eventFilter ( QObject * watched, QEvent * event );
   virtual void loadSettings(QSettings *settings = 0, bool updateInitValue = true);
   virtual void _save(QSettings *settings = 0, bool makeDirty = true);
signals:
   void changed(bool);
public slots:
   virtual void save();
   virtual void defaults();
   virtual void reset();
   virtual void import();
   virtual void saveAs();
protected slots:
   void checkDirty();
private slots:
   void resetInfo();
   void setComboListInfo(int index);
private:
   QVariant variant(const QWidget *w) const;
   bool setVariant(QWidget *w, const QVariant &v) const;
   bool infoItemHovered, infoDirty;
   QTextBrowser *_infoBrowser;
   QMap<QWidget*, SettingInfo> _settings;
   QMap<QWidget*, QString> _contextHelps;
   QMap<QComboBox*, QStringList> _comboHelps;
   QString _qsetting[3];
   QString _defaultContextInfo;
};

class BConfigDialog : public QDialog {
   Q_OBJECT
public:
   enum ButtonType {
      Ok = 1, Cancel = 2, Save = 4, Reset = 8,
         Defaults = 16, Import = 32, Export = 64, All = 127
   };
   BConfigDialog(BConfig *config, uint btns = All, QWidget *parent = 0L);
};

#endif
