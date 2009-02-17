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
    return Unkown;
}

static QString
dialog(QWidget *parent, const QString &cmd, const QStringList &args, const QString &dir )
{
    QWidget modal;
    modal.setAttribute( Qt::WA_NoChildEventsForParent, true );
    modal.setParent( parent, Qt::Window );
    QApplicationPrivate::enterModal( &modal );
    
    QProcess proc( &modal );
    proc.setWorkingDirectory( dir );
    proc.start( cmd, args );
    proc.waitForFinished( -1 );

    QString result;
    if ( proc.error() == QProcess::UnknownError )
    {
        result = proc.readAllStandardOutput().trimmed();
        if ( result.isNull() )
            result = "";
    }

    QApplicationPrivate::leaveModal( &modal );
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

static void
append( QStringList &list, const QString &string, const QString &def = QString() )
{
    if ( !string.isEmpty() )
        list << string;
    else if ( !def.isNull() )
        list << def;
}

static QString
simpleFilter( const QString& filter )
{
    QStringList list = filter.split( ';', QString::SkipEmptyParts );
    for ( int i = 0; i < list.count(); ++i )
    {
        list[i].replace(QRegExp(".*\\((.*\\*.*)\\).*"), "\\1");
    }
    return list.join(" ");
}

#define PREPARE_KDE \
cmd = "kdialog";\
if ( parent ) args << "--attach" << QString::number( parent->winId() );\
if ( !caption.isEmpty() ) args << "--title" << caption

#define PREPARE_GNOME \
cmd = "zenity";\
if ( !caption.isEmpty() ) args << "--title=" + caption;\
args << "--file-selection"

static QString
openFilename( QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options )
{
    Session ses = session();
    if ( ses == Unkown || options & QFileDialog::DontUseNativeDialog )
        return firstItem( qDialog( QFileDialog::ExistingFile, parent, caption, dir, filter, selectedFilter, options ) );
    
    QString cmd; QStringList args;
    
    if ( ses == KDE )
    {
        PREPARE_KDE;
        args << "--getopenfilename"; append( args, dir, QDir::currentPath() ); append( args, simpleFilter(filter) );
    }

    else if ( ses == GNOME )
    {
        PREPARE_GNOME;
//         --file-filter='All | *'
    }

    QString filename = dialog( parent, cmd, args, dir );

    if ( filename.isNull() )
        return firstItem( qDialog( QFileDialog::ExistingFile, parent, caption, dir, filter, selectedFilter, options ) );

    return filename;

}

static QStringList
openFilenames(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
    Session ses = session();
    if ( ses == Unkown || options & QFileDialog::DontUseNativeDialog )
        return qDialog( QFileDialog::ExistingFiles, parent, caption, dir, filter, selectedFilter, options );

    QString cmd; QStringList args;
    args << "--multiple";
    
    char splitter = '\n';
    if ( ses == KDE )
    {
        PREPARE_KDE;
        args << "--separate-output" << "--getopenfilename";
        append( args, dir, QDir::currentPath() ); append( args, simpleFilter(filter) );
    }

    else if ( ses == GNOME )
    {
        PREPARE_GNOME;
        splitter = '|';
    }

    QString string = dialog( parent, cmd, args, dir );
    if ( string.isNull() )
        return qDialog( QFileDialog::ExistingFiles, parent, caption, dir, filter, selectedFilter, options );
    
    return string.split(splitter);
}


static QString
saveFilename(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
    Session ses = session();
    if ( ses == Unkown || options & QFileDialog::DontUseNativeDialog )
        return firstItem( qDialog( QFileDialog::AnyFile, parent, caption, dir, filter, selectedFilter, options ) );

    QString cmd; QStringList args;
    
    if ( ses == KDE )
    {
        PREPARE_KDE;
        args << "--getsavefilename"; append( args, dir + " ", QDir::currentPath() + " " ); append( args, simpleFilter(filter) );
    }

    else if ( ses == GNOME )
    {
        PREPARE_GNOME;
        args << "--save";
        if ( !( options & QFileDialog::DontConfirmOverwrite ) )
            args << "--confirm-overwrite";
    }
    
    QString filename = dialog( parent, cmd, args, dir );
    
    if ( filename.isNull() )
        return firstItem( qDialog( QFileDialog::AnyFile, parent, caption, dir, filter, selectedFilter, options ) );
    
    return filename;
}

static QString
openDirectory( QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options options )
{
    Session ses = session();
    if ( ses == Unkown || options & QFileDialog::DontUseNativeDialog )
        return firstItem( qDialog( QFileDialog::DirectoryOnly, parent, caption, dir, QString(), 0, options ) );

    QString cmd; QStringList args;

    if ( ses == KDE )
    {
        PREPARE_KDE;
        args << "--getexistingdirectory"; append( args, dir, QDir::currentPath() );
    }

    else if ( ses == GNOME )
    {
        PREPARE_GNOME;
        args << "--directory";
    }

    QString filename = dialog( parent, cmd, args, dir );

    if ( filename.isNull() )
        return firstItem( qDialog( QFileDialog::DirectoryOnly, parent, caption, dir, QString(), 0, options ) );
    
    return filename;
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
