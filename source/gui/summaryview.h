#ifndef SUMMARYVIEW_H
#define SUMMARYVIEW_H

#include "mainui.h"

class QTableWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QToolButton;

class OptionWidgetMapper;
class MainUI;

class data_table;

class SummaryView : public Page
{
    Q_OBJECT
public:
    explicit SummaryView(MainUI *ui, const QString &title, QWidget *parent = nullptr);

public slots:
    void onTallyUpdate();
    void onConfigChanged();
    void onSimulationDestroyed();

private:
    MainUI *mainui_;
    QTableWidget *table_;
    QToolButton *exportBtn_;
    int minRowWidth_{ 0 };
    int minDataColWidth_{ 0 };
    bool hasData_{ false };

    enum { idxDmgEvntsTbl = 0, idxErgTbl, idxDmgParTbl, idxIonStatTbl, idxNTbls };
    std::array<data_table *, idxNTbls> tables_;

    void rebuildColumns(int atomCount, const QStringList &atomLabels);
    QString buildExportText(bool numeric) const;
    void exportCsv(bool numeric);
    void copySelection(bool numeric);
    void setHasData(bool b);
};

#endif // SUMMARYVIEW_H
