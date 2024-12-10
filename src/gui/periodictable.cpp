#include "periodictable.h"

#include <QTextStream>
#include <QFile>

// find most abundant isotope (MAI)
int findMAI(const QVector<Element::Isotope>& isotopes) {
    double maxAbundance=isotopes[0].abundance;
    int MAI=0;
    for(int i=1;i<isotopes.size();i++) {
        if(isotopes[i].abundance > maxAbundance) {
            maxAbundance=isotopes[i].abundance;
            MAI = i;
        }
    }
    return MAI;
}

double computeAverageMass(const QVector<Element::Isotope>& isotopes)
{
    double s = 0.;
    for(auto i : isotopes) s += i.abundance*i.mass;
    return s;
}

QVector<Element> PeriodicTable::elements_;
QMap<QString, int> PeriodicTable::symbol2z_;
Element PeriodicTable::dummyElement_;

const Element::Isotope &PeriodicTable::closestIsotope(int Z, double M)
{
    const Element& elmnt = PeriodicTable::at(Z);

    int i = 0;
    double diff = std::abs(elmnt.isotopes()[i].mass - M);
    for(int j=1; j<elmnt.isotopes().size(); ++j) {
        double d = std::abs(elmnt.isotopes()[j].mass - M);
        if (d<diff) {
            i = j;
            diff = d;
        }
    }
    return elmnt.isotopes()[i];
}

int PeriodicTable::loadTable_()
{
    if (!elements_.empty()) return elements_.size();

    //QFile file(":/isotopes.dat");
    QFile file(":/isotope-data/isotopes_data.csv");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    // load, computing average mass based on abundance
    QTextStream f(&file);
    f.readLine(); // skip headers
    while(!f.atEnd()) {
        QString S = f.readLine();
        auto tokens = S.splitRef(',');
        int Z = tokens[3].toInt();
        while (elements_.size()<=Z) {
            elements_.push_back(Element());
        }
        Element& E = elements_[Z];
        E.name_ = tokens[0].toString();
        E.symbol_ = tokens[1].toString();
        E.Z_ = Z;
        Element::Isotope i;
        i.A = tokens[4].toInt();
        i.symbol = tokens[2].toString();
        i.mass = tokens[6].toDouble();
        i.abundance = tokens[5].toDouble()/100.;
        E.isotopes_.push_back(i);
        E.MAI_ = findMAI(E.isotopes());
        E.averageMass_ = computeAverageMass(E.isotopes());
    }
    file.close();

    for(int i=0; i<elements_.size(); ++i) {
        symbol2z_[elements_[i].symbol()] = i;
    }

    Element::Isotope i__;
    dummyElement_.isotopes_.push_back(i__);

    return elements_.size();
}
