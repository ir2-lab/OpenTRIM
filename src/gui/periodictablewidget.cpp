#include "periodictablewidget.h"

#include "periodictable.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QTextStream>
#include <QFile>
#include <QMenu>
#include <QAction>
#include <QDialogButtonBox>

#define ZMAX 103

QString ElementButton::getIsotopeDesc(int z, int i)
{
    const Element& E = PeriodicTable::at(z);
    const QVector<Element::Isotope>& I = E.isotopes();

    if (i>I.size() || i<0) return QString();

    if (i==I.size()) {
        return QString("%1 | <m> %2")
            .arg(E.symbol())
            .arg(E.mass(),7,'f',3);
    } else {
        if (I[i].abundance > 0.0)
            return QString("%1 | m %2 | a %3")
                .arg(I[i].symbol)
                .arg(I[i].mass,7,'f',3)
                .arg(I[i].abundance,7,'g');
        else
            return QString("%1 | m %2 | a -")
                .arg(I[i].symbol)
                .arg(I[i].mass,7,'f',3);
    }
}

ElementButton::ElementButton(int z, PeriodicTableWidget *pt, QWidget *parent)
    : QToolButton(parent), ptable(pt), Z_(z)
{
    const Element& Z = PeriodicTable::at(z);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setText(Z.symbol());
    setPopupMode(QToolButton::InstantPopup);
    setToolTip(QString("%1, Z=%2").arg(Z.name()).arg(Z.Z()));

    if (pt->getIsotopes_) {
        QMenu* toolMenu = new QMenu;
        int n = Z.isotopes().size();
        if (n>1) {
            if (Z.mass() > 0.0) {
                QAction* action = toolMenu->addAction(
                    getIsotopeDesc(z,n)
                    );
                action->setData(n);
                connect(action, &QAction::triggered,
                        this, &ElementButton::isotopeActionTrigered);
                toolMenu->addSeparator();
            }

            for(int i=0; i<n; i++) {
                QAction* action = toolMenu->addAction(
                    getIsotopeDesc(z,i)
                    );
                action->setData(i);
                connect(action, &QAction::triggered,
                        this, &ElementButton::isotopeActionTrigered);
            }
        } else {
            QAction* action = toolMenu->addAction(
                getIsotopeDesc(z,0)
                );
            action->setData(0);
            connect(action, &QAction::triggered,
                    this, &ElementButton::isotopeActionTrigered);
        }
        setMenu(toolMenu);
    } else {
        connect(this, &ElementButton::clicked,
                pt, &PeriodicTableWidget::singleElementSelect);
    }


}

void ElementButton::isotopeActionTrigered()
{
    QAction* a = qobject_cast<QAction*>(sender());
    ptable->_selectedElement = Z_;
    ptable->_selectedIsotope = a->data().toInt();
    ptable->isotopeSelect();
}

QSize ElementButton::sizeHint() const
{
    QSize size = QToolButton::sizeHint();
    size.rheight() += 6;
    size.rwidth() = size.height(); //qMax(size.width(), size.height());
    return size;
}


PeriodicTableWidget::PeriodicTableWidget(bool isotopes, QWidget *parent)
    : QWidget( parent ),
    cb(ZMAX+1,nullptr),
    getIsotopes_(isotopes)
{
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setSizeConstraint(QLayout::SetFixedSize);
    gridLayout->setHorizontalSpacing(3);
    gridLayout->setVerticalSpacing(3);


    for(int Z=1; Z<=ZMAX; Z++) {

        int x,y;
        // y coordinate of element on table
        if(Z<3) y = 0;
        else if(Z<11) y = 1;
        else if(Z<19) y = 2;
        else if(Z<37) y = 3;
        else if(Z<55) y = 4;
        else if(Z>56 && Z<72) y = 7;
        else if(Z<87) y = 5;
        else if(Z>88 && Z<104) y = 8;
        else if(Z>=104) y = 9;
        else y = 6;

        // x coordinate (in button number) of element on table
        switch (y) {
        case 0: x = (Z-1)*17; break;
        case 1: x = (Z- 3) + (Z>  4?10:0); break;
        case 2: x = (Z-11) + (Z> 12?10:0); break;
        case 3: x = (Z-19);                break;
        case 4: x = (Z-37);                break;
        case 5: x = (Z-55) - (Z> 71?14:0); break;
        case 6: x = (Z-87) - (Z>103?14:0); break;
        case 7: x = (Z-57)+3;              break;
        case 8: x = (Z-89)+3;              break;
        case 9: x = (Z-93)+1;              break;
        }

        cb[Z] = new ElementButton(Z,this);


        //if(singleSelect)
            // connect( cb[i], SIGNAL(clicked()), this, SLOT(singleElementSelect()));
        //else
            //connect( cb[i], SIGNAL(clicked()), this, SLOT(multiElementSelect()));

        gridLayout->addWidget(cb[Z],y,x);
    }

    setLayout(gridLayout);
}




void PeriodicTableWidget::singleElementSelect()
{
    // set selected element to pushed button
    ElementButton* bt = qobject_cast<ElementButton*>(sender());

    if (bt) {
        _selectedElement = bt->Z();
        _selectedMass = PeriodicTable::at(_selectedElement).mass();
        _selectedIonSymbol = PeriodicTable::at(_selectedElement).symbol();
        emit elementSelected();
    }
}

void PeriodicTableWidget::isotopeSelect()
{
    const Element& at = PeriodicTable::at(_selectedElement);
    if (_selectedIsotope == at.isotopes().size()) {
        _selectedMass = at.mass();
        _selectedIonSymbol = at.symbol();
    }
    else {
        const Element::Isotope& I = at.isotopes().at(_selectedIsotope);
        _selectedMass = I.mass;
        _selectedIonSymbol = I.symbol;
    }
    emit elementSelected();
}

PeriodicTableDialog::PeriodicTableDialog(bool isotopes, QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;

    ptable = new PeriodicTableWidget(isotopes);
    layout->addWidget(ptable);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Cancel, Qt::Horizontal);
    buttonBox->setCenterButtons(true);
    layout->addWidget(buttonBox);

    setLayout(layout);

    connect(ptable, &PeriodicTableWidget::elementSelected, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}


