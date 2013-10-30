#include <QString>
// comment counter 2
const QString bespin_revision()
{
    QString rev("$Revision: 1616 $");
    rev.remove("$Revision: ");
    rev.remove(" $");
    return rev;
}
