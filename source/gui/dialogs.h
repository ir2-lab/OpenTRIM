#ifndef DIALOGS_H
#define DIALOGS_H

#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QStringList>

class QWidget;
class QShowEvent;

/**
 * @brief Unified message / input / file / progress dialogs for opentrim-gui.
 *
 * All dialogs get a uniform window title ("OpenTRIM - <windowTitle>"), a uniform
 * appearance (the context-help "?" and min/max window buttons are removed) and a
 * more generous default width (based on font metrics).
 *
 * Static helpers mirror the QMessageBox / QInputDialog / QFileDialog convenience
 * API. The nested builder classes (MessageBox, InputDialog, ProgressDialog) apply
 * the same styling for cases that need direct access to the underlying Qt widget.
 *
 * Parameter conventions for message dialogs:
 *  - windowTitle : subtitle only; "OpenTRIM - " is prepended automatically.
 *  - heading     : bold first row of the body.
 *  - text        : zero or more extra rows, one per QStringList entry. This is how
 *                  multi-line body text is supplied.
 *  - detailedText: plain, collapsible text (QMessageBox::setDetailedText).
 */
class Dialogs
{
public:
    // ---- title helper -----------------------------------------------------
    /// "" -> project name only; otherwise "<project> - <windowTitle>"
    static QString appTitle(const QString &windowTitle = QString());

    // ---- message boxes --------------------------------------------------
    static QMessageBox::StandardButton
    message(QWidget *parent, QMessageBox::Icon icon, const QString &windowTitle,
            const QString &heading, const QStringList &text = QStringList(),
            const QString &detailedText = QString(),
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton
    information(QWidget *parent, const QString &windowTitle, const QString &heading,
               const QStringList &text = QStringList(), const QString &detailedText = QString(),
               QMessageBox::StandardButtons buttons = QMessageBox::Ok,
               QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton
    warning(QWidget *parent, const QString &windowTitle, const QString &heading,
            const QStringList &text = QStringList(), const QString &detailedText = QString(),
            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton
    critical(QWidget *parent, const QString &windowTitle, const QString &heading,
             const QStringList &text = QStringList(), const QString &detailedText = QString(),
             QMessageBox::StandardButtons buttons = QMessageBox::Ok,
             QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton
    question(QWidget *parent, const QString &windowTitle, const QString &heading,
             const QStringList &text = QStringList(), const QString &detailedText = QString(),
             QMessageBox::StandardButtons buttons = QMessageBox::Ok | QMessageBox::Cancel,
             QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    /// Dominant confirmation case: returns true iff Ok or Yes was pressed.
    static bool confirm(QWidget *parent, const QString &windowTitle, const QString &heading,
                        const QStringList &text = QStringList(),
                        const QString &detailedText = QString(),
                        QMessageBox::Icon icon = QMessageBox::Warning);

    // ---- input --------------------------------------------------------
    static QString getText(QWidget *parent, const QString &windowTitle, const QString &label,
                           const QString &value = QString(), bool *ok = nullptr);

    static int getInt(QWidget *parent, const QString &windowTitle, const QString &label,
                      int value = 0, int minValue = -2147483647, int maxValue = 2147483647,
                      int step = 1, bool *ok = nullptr);

    static double getDouble(QWidget *parent, const QString &windowTitle, const QString &label,
                            double value = 0.0, double minValue = -1.0e9, double maxValue = 1.0e9,
                            int decimals = 3, bool *ok = nullptr);

    // ---- files -------------------------------------------------------
    static QString getOpenFileName(QWidget *parent, const QString &windowTitle,
                                   const QString &dir, const QString &filter);

    /// Appends @p defaultSuffix if the chosen name has none. If @p changeDir,
    /// QDir::setCurrent() is set to the chosen file's directory.
    static QString getSaveFileName(QWidget *parent, const QString &windowTitle,
                                   const QString &dir, const QString &filter,
                                   const QString &defaultSuffix = QString(),
                                   bool changeDir = true);

    // ---- instantiable builders ------------------------------------
    class MessageBox;
    class InputDialog;
    class ProgressDialog;

    // ---- shared styling helpers (used by the builders) --------------
    /// Strip the context-help "?" and min/max buttons from a dialog.
    static void applyWindowFlags(QWidget *dlg);
    /// fontMetrics(title).boundingRect().width() * 2 (qApp metrics if ref is null).
    static int bodyWidth(const QWidget *ref, const QString &fullTitle);
    /// Wrap heading + text rows in the fixed-width <table> used across the gui.
    static QString htmlBody(int width, const QString &heading, const QStringList &text);
};

class Dialogs::MessageBox : public QMessageBox
{
public:
    explicit MessageBox(QWidget *parent = nullptr);
    MessageBox(QMessageBox::Icon icon, const QString &windowTitle, const QString &heading,
               QMessageBox::StandardButtons buttons = NoButton, QWidget *parent = nullptr);

    /// Set the window title to Dialogs::appTitle(windowTitle).
    void setWindowSubtitle(const QString &windowTitle);
    /// Set the message text to Dialogs::htmlBody(...) sized from the window title.
    void setBody(const QString &heading, const QStringList &text = QStringList());
};

class Dialogs::InputDialog : public QInputDialog
{
public:
    explicit InputDialog(QWidget *parent = nullptr);

    /// Set the window title to Dialogs::appTitle(windowTitle).
    void setWindowSubtitle(const QString &windowTitle);

protected:
    void showEvent(QShowEvent *e) override;
};

class Dialogs::ProgressDialog : public QProgressDialog
{
public:
    ProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum,
                   int maximum, QWidget *parent = nullptr, const QString &windowTitle = QString());
};

#endif // DIALOGS_H
