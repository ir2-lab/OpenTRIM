#ifndef VALIDATINGITEMDELEGATE_H
#define VALIDATINGITEMDELEGATE_H

#include <QStyledItemDelegate>

// QAbstractItemDelegate installs itself as an event filter on every editor
// it creates, and that filter's own Return/Enter handling swallows the key
// itself (QAbstractItemDelegatePrivate::handleEditorEvent() -> tryFixup())
// whenever the editor is a QLineEdit with hasAcceptableInput() == false --
// before the editor's own keyPressEvent() ever runs. That breaks any
// QLineEdit-based editor (e.g. QtVectorEdit's QNumberEdit/QVectorEdit) that
// relies on keyPressEvent() to show its own validation feedback on Enter:
// inside an item view the feedback never appears, even though the same
// widget works fine standalone. See QtVectorEdit's README ("Known issues")
// for the full Qt-side explanation.
//
// Subclass this instead of QStyledItemDelegate to get that key event
// delivered to the editor as normal, for every QLineEdit-based editor with
// an active validator -- not just QtVectorEdit's widgets.
class ValidatingItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif // VALIDATINGITEMDELEGATE_H
