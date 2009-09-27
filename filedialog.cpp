/* Bespin widget style for Qt4
    Copyright (C) 2007-2009 Thomas Luebking <thomas.luebking@web.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QProcess>

#ifdef Q_WS_WIN
#undef Q_GUI_EXPORT
#define Q_GUI_EXPORT
#endif

class Q_GUI_EXPORT QApplicationPrivate
{
    public:
        static void enterModal(QWidget*);
        static void leaveModal(QWidget*);
};

typedef QStringList
        (*_qt_filedialog_open_filenames_hook)( QWidget *parent, const QString &caption, const QString &dir,
                                               const QString &filter, QString *selectedFilter, QFileDialog::Options options );
typedef QString
        (*_qt_filedialog_open_filename_hook)( QWidget *parent, const QString &caption, const QString &dir,
                                              const QString &filter, QString *selectedFilter, QFileDialog::Options options);
typedef QString
        (*_qt_filedialog_save_filename_hook)( QWidget *parent, const QString &caption, const QString &dir,
                                              const QString &filter, QString *selectedFilter, QFileDialog::Options options);
typedef QString
        (*_qt_filedialog_existing_directory_hook)( QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options options);

enum Session { Unkown = 0, KDE, GNOME };

static Session
session()
{
    const char *env = getenv("DESKTOP_SESSION");
    if (!env) return Unkown;
    QString qSession = env;
    if (qSession == "kde4") return KDE;
    if (qSession == "gnome") return GNOME;
    env = getenv("KDE_FULL_SESSION");
    if (!env) return Unkown;
    qSession = env;
    if (qSession == "true") return KDE;
    return Unkown;
}
// #include <QtDebug>
#define MODAL_DIALOG 1
static QString
dialog(QWidget *parent, Session ses, const QStringList &args, const QString &dir )
{
//     qDebug() << parent << args;
#if MODAL_DIALOG
    QWidget modal;
    modal.setAttribute( Qt::WA_NoChildEventsForParent, true );
    modal.setParent( parent, Qt::Window );
    QApplicationPrivate::enterModal( &modal );
    QProcess proc( &modal );
#else
    QProcess proc( parent );
#endif

    proc.setWorkingDirectory( dir );
    proc.start( ses == KDE ? "kdialog" : "zenity", args );
    proc.waitForFinished( -1 /*5*60*1000*/ );

    QString result;
    if ( proc.error() == QProcess::UnknownError )
    {
        result = proc.readAllStandardOutput().trimmed();
        if ( result.isNull() )
            result = "";
    }
    
#if MODAL_DIALOG
    QApplicationPrivate::leaveModal( &modal );
#endif
    return result;
}

static QStringList
qDialog( QFileDialog::FileMode mode, QWidget *parent, const QString &caption, const QString &dir,
         const QString &filter, QString *selectedFilter, QFileDialog::Options options )
{
    QFileDialog dialog( parent, caption, dir, filter );
    dialog.setFileMode( mode );
    dialog.setConfirmOverwrite( !( options & QFileDialog::DontConfirmOverwrite ) );
    dialog.setResolveSymlinks( !( options & QFileDialog::DontResolveSymlinks ) );
    if ( selectedFilter )
        dialog.selectNameFilter( *selectedFilter );

#if QT_VERSION >= 0x040400
// this does sth. else
//     if ( mode & QFileDialog::ShowDirsOnly )
//         dialog.setFilter( QDir::Dirs );
#endif

    QStringList result;
    dialog.exec();
    if ( dialog.result() == QDialog::Accepted )
        result = dialog.selectedFiles();
    
    return result;
}

static QString
firstItem( const QStringList &list )
{
    if ( list.isEmpty() )
        return QString();
    return list.first();
}

static QString
path( const QString& dir )
{
    QString s = dir;
    if ( s.isEmpty() )
        s = QDir::currentPath();
    if ( !s.endsWith('/') )
        s += '/';
    return s;
}

static QString
simpleFilter( const QString& filter )
{
    if ( filter.isEmpty() )
        return "*";
    
    QStringList list = filter.split( ';', QString::SkipEmptyParts );
    for ( int i = 0; i < list.count(); ++i )
    {
        list[i].replace(QRegExp(".*\\((.*\\*.*)\\).*"), "\\1");
    }
    return list.join(" ");
}

#ifdef Q_WS_X11
#define PREPARE_KDE \
if ( !caption.isEmpty() ) args << "--title" << caption; \
if ( parent ) args << "--attach" << QString::number( parent->winId() );
#else
#define PREPARE_KDE \
if ( !caption.isEmpty() ) args << "--title" << caption;
#endif

#define PREPARE_GNOME \
if ( !caption.isEmpty() ) args << "--title=" + caption;\
args << "--file-selection"

static QString
openFilename( QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options )
{
    if ( Session ses = session() )
    if ( !(options & QFileDialog::DontUseNativeDialog) )
    {
    
        QStringList args;

        if ( ses == KDE )
        {
            PREPARE_KDE;
            args << "--getopenfilename" << path( dir ) << simpleFilter(filter);
        }

        else if ( ses == GNOME )
        {
            PREPARE_GNOME;
    //         --file-filter='All | *'
        }

        QString filename = dialog( parent, ses, args, dir );
        if ( !filename.isNull() )
            return filename;
    }

    return firstItem( qDialog( QFileDialog::ExistingFile, parent, caption, dir, filter, selectedFilter, options ) );

}

static QStringList
openFilenames(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
    if ( Session ses = session() )
    if ( !(options & QFileDialog::DontUseNativeDialog) )
    {
        QStringList args;
        args << "--multiple";

        char splitter = '\n';
        if ( ses == KDE )
        {
            PREPARE_KDE;
            args << "--separate-output" << "--getopenfilename" << path( dir ) << simpleFilter(filter);
        }

        else if ( ses == GNOME )
        {
            PREPARE_GNOME;
            splitter = '|';
        }

        QString string = dialog( parent, ses, args, dir );
        if ( !string.isNull() )
            return string.split(splitter);
    }

    return qDialog( QFileDialog::ExistingFiles, parent, caption, dir, filter, selectedFilter, options );
}


static QString
saveFilename(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
    if ( Session ses = session() )
    if ( !(options & QFileDialog::DontUseNativeDialog) )
    {
        QStringList args;

        if ( ses == KDE )
        {
            PREPARE_KDE;
            args << "--getsavefilename" << (dir.isEmpty() ? QDir::currentPath() + "/ " : dir) << simpleFilter(filter);
        }

        else if ( ses == GNOME )
        {
            PREPARE_GNOME;
            args << "--save";
            if ( !( options & QFileDialog::DontConfirmOverwrite ) )
                args << "--confirm-overwrite";
        }

        QString filename = dialog( parent, ses, args, dir );
        if ( !filename.isNull() )
            return filename;
    }

    return firstItem( qDialog( QFileDialog::AnyFile, parent, caption, dir, filter, selectedFilter, options ) );
}

static QString
openDirectory( QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options options )
{
    if ( Session ses = session() )
    if ( !(options & QFileDialog::DontUseNativeDialog) )
    {
        QStringList args;

        if ( ses == KDE )
        {
            PREPARE_KDE;
            args << "--getexistingdirectory" << path( dir);
        }

        else if ( ses == GNOME )
        {
            PREPARE_GNOME;
            args << "--directory";
        }

        QString filename = dialog( parent, ses, args, dir );
        if ( !filename.isNull() )
            return filename;
    }
    
    return firstItem( qDialog( QFileDialog::DirectoryOnly, parent, caption, dir, QString(), 0, options ) );
}

extern Q_GUI_EXPORT _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook;
extern Q_GUI_EXPORT _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook;
extern Q_GUI_EXPORT _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook;
extern Q_GUI_EXPORT _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook;

void
register_dialog_functions()
{
    qt_filedialog_open_filename_hook = &openFilename;
    qt_filedialog_open_filenames_hook = &openFilenames;
    qt_filedialog_save_filename_hook = &saveFilename;
    qt_filedialog_existing_directory_hook = &openDirectory;
}
