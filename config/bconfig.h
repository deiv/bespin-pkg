#ifndef BCONFIG_H
#define BCONFIG_H

#include <QDialog>
#include <QMap>
#include <QVariant>
class QComboBox;
class QTextBrowser;

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
   void handleSettings(QWidget *w, const QString entry, QVariant defaultValue);
   void setContextHelp(QWidget *w, QString help);
   void setContextHelp(QComboBox *c, QStringList & strings);
   void setInfoBrowser(QTextBrowser *browser);
   void setQSetting(const QString organisation, const QString application, const QString group);
   void setDefaultContextInfo(QString info);
protected:
   bool eventFilter ( QObject * watched, QEvent * event );
   void loadSettings();
private:
   bool infoItemHovered;
signals:
   void changed(bool);
public slots:
   void save();
   void defaults();
   void reset();
private slots:
   void checkDirty();
   void resetInfo();
   void setComboListInfo(int index);
private:
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
