#ifndef BCONFIG_H
#define BCONFIG_H

#include <QDialog>
#include <QMap>
#include <QVariant>
class QComboBox;
class QTextBrowser;
class QSettings;

typedef struct {
   QVariant defaultValue;
   QVariant initialValue;
   QString entry;
} SettingInfo;

class BConfig : public QWidget
{
   Q_OBJECT
public:
   BConfig(QWidget *parent = 0L);
   virtual void handleSettings(QWidget *w, const QString entry, QVariant defaultValue);
   virtual void setContextHelp(QWidget *w, QString help);
   virtual void setContextHelp(QComboBox *c, QStringList & strings);
   virtual void setInfoBrowser(QTextBrowser *browser);
   virtual void setQSetting(const QString organisation, const QString application, const QString group);
   virtual void setDefaultContextInfo(QString info);
protected:
   virtual bool eventFilter ( QObject * watched, QEvent * event );
   virtual void loadSettings(QSettings *settings = 0, bool updateInitValue = true);
   virtual void _save(QSettings *settings = 0);
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
   bool infoItemHovered;
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
   BConfigDialog(BConfig *config, QWidget *parent = 0L);
};

#endif
