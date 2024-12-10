#ifndef PERIODICTABLE_H
#define PERIODICTABLE_H

#include <QString>
#include <QVector>
#include <QMap>

class Element
{
public:
    struct Isotope {
        QString symbol;
        int A;
        double mass;
        double abundance;
    };

    const QString& name() const { return name_; }
    const QString& symbol() const { return symbol_; }
    int Z() const { return Z_; }
    int MAI() const { return MAI_; }
    double mass() const { return averageMass_; }
    const QVector<Isotope>& isotopes() const { return isotopes_; }

private:
    Element() {}
    QString       name_;
    QString       symbol_;
    int           Z_; // atomic & mass number
    int           MAI_;             // index of Most Abundant Isotope in isotopeMass list
    double        averageMass_;
    QVector<Isotope> isotopes_;

    friend struct PeriodicTable;
};

struct PeriodicTable
{
    static const QVector<Element>& elements() { loadTable_(); return elements_; }

    static const Element& at(int i) {
        if (i>=0 && i<loadTable_()) return elements_[i];
        else return dummyElement_;
    }

    static const Element& at(const QString& symbol) {
        if (loadTable_() && symbol2z_.contains(symbol))
            return elements_[symbol2z_[symbol]];
        else return dummyElement_;
    }

    static const Element::Isotope& closestIsotope(int Z, double M);

private:
    static QMap<QString, int> symbol2z_;
    static QVector<Element> elements_;
    static Element dummyElement_;
    static int loadTable_();
};

#endif // PERIODICTABLE_H
