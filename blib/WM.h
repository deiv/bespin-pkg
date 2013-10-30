#include <QPoint>
#include <QWidget>

namespace Bespin {
namespace WM {
    enum Direction {
        TopLeft = 0, Top = 1, TopRight = 2, Right = 3,
        BottomRight = 4, Bottom = 5, BottomLeft = 6, Left = 7,
        Move = 8, KeyboardSize = 9, KeyboardMove = 10, MoveResizeCancel = 11
    }; // copy of the NETWM enum
    BLIB_EXPORT void triggerMoveResize(WId id, QPoint p, Direction d);
    BLIB_EXPORT void triggerMove(WId id, QPoint p);
}
}