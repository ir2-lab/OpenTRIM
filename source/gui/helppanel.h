#ifndef HELPPANEL_H
#define HELPPANEL_H

#include <QWidget>
#include <QMap>

class QTextBrowser;
class QModelIndex;
class OptionsModel;
class OptionsItem;

struct HelpEntry{
    QString description;   
    QString tip;           
    QString units;               
};

class HelpPanel : public QWidget{
    Q_OBJECT

public:
    explicit HelpPanel(OptionsModel *model, QWidget *parent = nullptr);

    void registerWidget(QWidget *editor, const QModelIndex &index);

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void showHelpFor(const QModelIndex &index);
    void clear();
    void toggleVisibility();

private:
    QString generateHelpHtml(OptionsItem *item) const;
    QString parseOptionsListHtml(const QString &text) const;
    QString extractMeaning(const QString &text) const;
    QStringList extractOptions(const QString &text) const;
    QString typeInfoHtml(OptionsItem *item) const;
    QString booleanEffectHtml(OptionsItem *item) const;

    QString sectionHeader(const QString &title) const;
    QString codeBlock(const QString &text) const;
    QString infoBox(const QString &text, const QString &color = "#e8f0fe") const;
    QString typeBadge(const QString &typeName) const;
    QString effectRow(const QString &value, const QString &desc, const QString &color) const;

    void buildHelpDatabase();

    QTextBrowser *browser_;
    OptionsModel *model_;
    QMap<QWidget *, QPersistentModelIndex> widgetIndexMap_;
    QMap<QString, HelpEntry> helpDb_;
};

#endif 
