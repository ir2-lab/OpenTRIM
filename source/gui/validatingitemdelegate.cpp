#include "validatingitemdelegate.h"

#include <QKeyEvent>
#include <QLineEdit>

bool ValidatingItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            auto *lineEdit = qobject_cast<QLineEdit *>(object);
            if (lineEdit && !lineEdit->hasAcceptableInput()) {
                // Deliver the key straight to the editor, bypassing the
                // base class's own eventFilter() (which would otherwise
                // swallow it here -- see header comment).
                lineEdit->event(event);
                return true;
            }
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}
