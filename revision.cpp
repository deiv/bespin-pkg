#include <QString>
// comment counter 1
const QString bespin_revision()
{
    QString rev("$Revision$");
    rev.remove("$Revision: ");
    rev.remove(" $");
    return rev;
}
