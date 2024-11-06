#ifndef PERIODICTABLEWIDGET_H
#define PERIODICTABLEWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QDialog>

class PeriodicTableWidget;

class ElementButton : public QToolButton
{
    Q_OBJECT

    PeriodicTableWidget* ptable{nullptr};
    int Z_{0};

    static QString getIsotopeDesc(int z, int i) ;

public:
    ElementButton(int Z, PeriodicTableWidget* pt, QWidget *parent = nullptr);
    QSize sizeHint() const override;

    int Z() const { return Z_; }

public slots:
    void isotopeActionTrigered();

signals:
    void isotopeSelected();
};


class PeriodicTableWidget : public QWidget {

    Q_OBJECT

private:
    void setSelected(int Z, bool sel = true);

    QVector<int> multiSelected();

    void createTableOfElements(int width, int height);
    bool loadAtoms();

    QVector<ElementButton *> cb;

    int _selectedElement{0};
    int _selectedIsotope{0};
    double _selectedMass{0.};
    QString _selectedIonSymbol;

    friend class ElementButton;

    bool getIsotopes_;

public:
    explicit PeriodicTableWidget(bool isotopes, QWidget *parent=0);
    int selectedZ() const { return _selectedElement; }
    int selectedIsotope() const { return _selectedIsotope; }
    double selectedMass() const { return _selectedMass; }
    QString selectedIonSymbol() const { return _selectedIonSymbol; }

public slots:
    void singleElementSelect();
    void isotopeSelect();

signals:
    void elementSelected();
};

class PeriodicTableDialog : public QDialog
{
    Q_OBJECT

    PeriodicTableWidget* ptable;

public:
    PeriodicTableDialog(bool isotopes = false, QWidget *parent=0);
    int selectedZ() const { return ptable->selectedZ(); }
    double selectedMass() const { return ptable->selectedMass(); }
    QString selectedIonSymbol() const { return ptable->selectedIonSymbol(); }
};

#endif // PERIODICTABLEWIDGET_H
