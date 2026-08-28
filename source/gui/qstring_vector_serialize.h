#pragma once

#include <QString>

template <class T>
struct num_convert;

template <>
struct num_convert<float>
{
    static float str2num(const QString &s, bool &ok) { return s.toFloat(&ok); }
};

template <>
struct num_convert<int>
{
    static int str2num(const QString &s, bool &ok) { return s.toInt(&ok); }
};

template <class vector_t>
struct qstring_serialize
{
    typedef typename vector_t::value_type scalar_t;

    static bool fromString(const QString &S, vector_t &v)
    {
        QString t = S.trimmed();
        if (t.startsWith('['))
            t.remove(0, 1);
        else
            return false;

        if (t.endsWith(']'))
            t.chop(1);
        else
            return false;

        QStringList lst = t.split(',', Qt::SkipEmptyParts);
        if (lst.count() != v.size())
            return false;

        bool numok = true;
        int i = 0;
        while (i < v.size() && numok) {
            v[i] = num_convert<scalar_t>::str2num(lst.at(i), numok);
            i++;
        }
        if (!numok)
            return false;

        return true;
    }

    static QString toString(const vector_t &v)
    {
        int n = v.size();
        if (n < 1)
            return QString{ };
        QString S("[ ");
        S += QString::number(v[0]);
        for (int i = 1; i < n; ++i) {
            S += ", ";
            S += QString::number(v[i]);
        }
        S += "]";
        return S;
    }
};