#include <QString>

const QString bespin_revision()
{
    QString rev("$Revision$");
    rev.remove("$Revision: ");
    rev.remove(" $");
    return rev;
}
