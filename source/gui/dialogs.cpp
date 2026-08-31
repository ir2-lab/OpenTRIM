#include "dialogs.h"

#include <QApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QShowEvent>
#include <QSpinBox>

#include "mcdriver.h"

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

QString Dialogs::appTitle(const QString &windowTitle)
{
    QString project = QString::fromUtf8(mcdriver::version_info().project_name);
    if (windowTitle.isEmpty())
        return project;
    return QStringLiteral("%1 - %2").arg(project, windowTitle);
}

void Dialogs::applyWindowFlags(QWidget *dlg)
{
    dlg->setWindowFlags(dlg->windowFlags() & ~Qt::WindowContextHelpButtonHint
                        & ~Qt::WindowMinMaxButtonsHint);
}

int Dialogs::bodyWidth(const QWidget *ref, const QString &fullTitle)
{
    const QFontMetrics fm = ref ? ref->fontMetrics() : qApp->fontMetrics();
    return fm.boundingRect(fullTitle).width() * 2;
}

QString Dialogs::htmlBody(int width, const QString &heading, const QStringList &text)
{
    QString html = QStringLiteral("<table width='%1'>").arg(width);
    html += QStringLiteral("<tr><td><b>%1</b></td></tr>").arg(heading.toHtmlEscaped());
    for (const QString &line : text)
        html += QStringLiteral("<tr><td>%1</td></tr>").arg(line.toHtmlEscaped());
    html += QStringLiteral("</table>");
    return html;
}

// ---------------------------------------------------------------------------
// Dialogs::MessageBox
// ---------------------------------------------------------------------------

Dialogs::MessageBox::MessageBox(QWidget *parent) : QMessageBox(parent)
{
    Dialogs::applyWindowFlags(this);
    setTextFormat(Qt::RichText);
}

Dialogs::MessageBox::MessageBox(QMessageBox::Icon icon, const QString &windowTitle,
                                const QString &heading, QMessageBox::StandardButtons buttons,
                                QWidget *parent)
    : QMessageBox(icon, Dialogs::appTitle(windowTitle), QString(), buttons, parent)
{
    Dialogs::applyWindowFlags(this);
    setTextFormat(Qt::RichText);
    setBody(heading);
}

void Dialogs::MessageBox::setWindowSubtitle(const QString &windowTitle)
{
    setWindowTitle(Dialogs::appTitle(windowTitle));
}

void Dialogs::MessageBox::setBody(const QString &heading, const QStringList &text)
{
    setText(Dialogs::htmlBody(Dialogs::bodyWidth(this, windowTitle()), heading, text));
}

// ---------------------------------------------------------------------------
// Dialogs::InputDialog
// ---------------------------------------------------------------------------

Dialogs::InputDialog::InputDialog(QWidget *parent) : QInputDialog(parent)
{
    Dialogs::applyWindowFlags(this);
}

void Dialogs::InputDialog::setWindowSubtitle(const QString &windowTitle)
{
    setWindowTitle(Dialogs::appTitle(windowTitle));
}

void Dialogs::InputDialog::showEvent(QShowEvent *e)
{
    QInputDialog::showEvent(e);

    const int w = Dialogs::bodyWidth(this, windowTitle());
    if (auto *edit = findChild<QLineEdit *>())
        edit->setMinimumWidth(w);
    if (auto *spin = findChild<QSpinBox *>())
        spin->setMinimumWidth(w);
    if (auto *dspin = findChild<QDoubleSpinBox *>())
        dspin->setMinimumWidth(w);

    // show the prompt label in bold
    if (auto *label = findChild<QLabel *>()) {
        QFont f = label->font();
        f.setBold(true);
        label->setFont(f);
    }
}

// ---------------------------------------------------------------------------
// Dialogs::ProgressDialog
// ---------------------------------------------------------------------------

Dialogs::ProgressDialog::ProgressDialog(const QString &labelText, const QString &cancelButtonText,
                                        int minimum, int maximum, QWidget *parent,
                                        const QString &windowTitle)
    : QProgressDialog(labelText, cancelButtonText, minimum, maximum, parent)
{
    setWindowTitle(Dialogs::appTitle(windowTitle));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::WindowModal);
    setMinimumDuration(0);
}

// ---------------------------------------------------------------------------
// message boxes
// ---------------------------------------------------------------------------

QMessageBox::StandardButton Dialogs::message(QWidget *parent, QMessageBox::Icon icon,
                                             const QString &windowTitle, const QString &heading,
                                             const QStringList &text, const QString &detailedText,
                                             QMessageBox::StandardButtons buttons,
                                             QMessageBox::StandardButton defaultButton)
{
    Dialogs::MessageBox box(icon, windowTitle, heading, buttons, parent);
    if (!text.isEmpty())
        box.setBody(heading, text);
    if (!detailedText.isEmpty())
        box.setDetailedText(detailedText);
    if (defaultButton != QMessageBox::NoButton)
        box.setDefaultButton(defaultButton);
    box.exec();
    return box.standardButton(box.clickedButton());
}

QMessageBox::StandardButton Dialogs::information(QWidget *parent, const QString &windowTitle,
                                                 const QString &heading, const QStringList &text,
                                                 const QString &detailedText,
                                                 QMessageBox::StandardButtons buttons,
                                                 QMessageBox::StandardButton defaultButton)
{
    return message(parent, QMessageBox::Information, windowTitle, heading, text, detailedText,
                   buttons, defaultButton);
}

QMessageBox::StandardButton Dialogs::warning(QWidget *parent, const QString &windowTitle,
                                             const QString &heading, const QStringList &text,
                                             const QString &detailedText,
                                             QMessageBox::StandardButtons buttons,
                                             QMessageBox::StandardButton defaultButton)
{
    return message(parent, QMessageBox::Warning, windowTitle, heading, text, detailedText, buttons,
                   defaultButton);
}

QMessageBox::StandardButton Dialogs::critical(QWidget *parent, const QString &windowTitle,
                                              const QString &heading, const QStringList &text,
                                              const QString &detailedText,
                                              QMessageBox::StandardButtons buttons,
                                              QMessageBox::StandardButton defaultButton)
{
    return message(parent, QMessageBox::Critical, windowTitle, heading, text, detailedText, buttons,
                   defaultButton);
}

QMessageBox::StandardButton Dialogs::question(QWidget *parent, const QString &windowTitle,
                                              const QString &heading, const QStringList &text,
                                              const QString &detailedText,
                                              QMessageBox::StandardButtons buttons,
                                              QMessageBox::StandardButton defaultButton)
{
    return message(parent, QMessageBox::Question, windowTitle, heading, text, detailedText, buttons,
                   defaultButton);
}

bool Dialogs::confirm(QWidget *parent, const QString &windowTitle, const QString &heading,
                      const QStringList &text, const QString &detailedText, QMessageBox::Icon icon)
{
    QMessageBox::StandardButton r =
            message(parent, icon, windowTitle, heading, text, detailedText,
                    QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok);
    return r == QMessageBox::Ok || r == QMessageBox::Yes;
}

// ---------------------------------------------------------------------------
// input dialogs
// ---------------------------------------------------------------------------

QString Dialogs::getText(QWidget *parent, const QString &windowTitle, const QString &label,
                         const QString &value, bool *ok)
{
    Dialogs::InputDialog dlg(parent);
    dlg.setWindowSubtitle(windowTitle);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setLabelText(label);
    dlg.setTextValue(value);

    const bool accepted = dlg.exec() == QDialog::Accepted;
    if (ok)
        *ok = accepted;
    return accepted ? dlg.textValue() : QString();
}

int Dialogs::getInt(QWidget *parent, const QString &windowTitle, const QString &label, int value,
                    int minValue, int maxValue, int step, bool *ok)
{
    Dialogs::InputDialog dlg(parent);
    dlg.setWindowSubtitle(windowTitle);
    dlg.setInputMode(QInputDialog::IntInput);
    dlg.setLabelText(label);
    dlg.setIntRange(minValue, maxValue);
    dlg.setIntStep(step);
    dlg.setIntValue(value);

    const bool accepted = dlg.exec() == QDialog::Accepted;
    if (ok)
        *ok = accepted;
    return accepted ? dlg.intValue() : value;
}

double Dialogs::getDouble(QWidget *parent, const QString &windowTitle, const QString &label,
                          double value, double minValue, double maxValue, int decimals, bool *ok)
{
    Dialogs::InputDialog dlg(parent);
    dlg.setWindowSubtitle(windowTitle);
    dlg.setInputMode(QInputDialog::DoubleInput);
    dlg.setLabelText(label);
    dlg.setDoubleRange(minValue, maxValue);
    dlg.setDoubleDecimals(decimals);
    dlg.setDoubleValue(value);

    const bool accepted = dlg.exec() == QDialog::Accepted;
    if (ok)
        *ok = accepted;
    return accepted ? dlg.doubleValue() : value;
}

// ---------------------------------------------------------------------------
// file dialogs
// ---------------------------------------------------------------------------

QString Dialogs::getOpenFileName(QWidget *parent, const QString &windowTitle, const QString &dir,
                                 const QString &filter)
{
    QFileDialog dlg(parent, Dialogs::appTitle(windowTitle), dir, filter);
    Dialogs::applyWindowFlags(&dlg);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setFileMode(QFileDialog::ExistingFile);

    if (dlg.exec() != QDialog::Accepted)
        return QString();
    const QStringList files = dlg.selectedFiles();
    return files.isEmpty() ? QString() : files.first();
}

QString Dialogs::getSaveFileName(QWidget *parent, const QString &windowTitle, const QString &dir,
                                 const QString &filter, const QString &defaultSuffix, bool changeDir)
{
    QFileDialog dlg(parent, Dialogs::appTitle(windowTitle), dir, filter);
    Dialogs::applyWindowFlags(&dlg);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setFileMode(QFileDialog::AnyFile);
    if (!defaultSuffix.isEmpty())
        dlg.setDefaultSuffix(defaultSuffix);

    if (dlg.exec() != QDialog::Accepted)
        return QString();
    const QStringList files = dlg.selectedFiles();
    if (files.isEmpty())
        return QString();

    QString selected = files.first();
    QFileInfo finfo(selected);
    if (!defaultSuffix.isEmpty() && finfo.suffix().compare(defaultSuffix, Qt::CaseInsensitive) != 0)
        selected += QLatin1Char('.') + defaultSuffix;

    if (changeDir)
        QDir::setCurrent(QFileInfo(selected).dir().absolutePath());

    return selected;
}
