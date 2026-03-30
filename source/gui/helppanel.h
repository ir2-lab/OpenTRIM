#ifndef HELPPANEL_H
#define HELPPANEL_H

#include <QWidget>
#include <QMap>

class QTextBrowser;
class QModelIndex;
class OptionsModel;
class OptionsItem;
class QPushButton;

class HelpPanel : public QWidget{
    Q_OBJECT

public:
    explicit HelpPanel(OptionsModel *model, QWidget *parent = nullptr);

    void showHelpFor(const QModelIndex &index);
    void registerWidget(QWidget *editor, const QModelIndex &index);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void togglePanel();

private:
    QString generateHelpHtml(OptionsItem *item)const ;
    QString sectionHeader(const QString &title)const;
    QString detectCurrentValue(OptionsItem *item)const;
    QString BooleanHelp(OptionsItem *item)const;
    QString EnumHelp(OptionsItem *item)const;
    QString NumericHelp(OptionsItem *item)const;
    QString VectorHelp(OptionsItem *item)const;


    OptionsModel *model_;
    QTextBrowser *browser_;

    QMap<QWidget *, QPersistentModelIndex> widgetIndexMap_;
};

#endif 
