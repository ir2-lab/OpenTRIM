#ifndef FLOATLINEEDIT_H
#define FLOATLINEEDIT_H

#include <QLineEdit>
#include <QValidator>

class FloatValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals NOTIFY decimalsChanged)
public:
    explicit FloatValidator(QObject *parent = nullptr);
    FloatValidator(float bottom, float top, int decimals, QObject *parent = nullptr);
    ~FloatValidator();
    QValidator::State validate(QString &, int &) const override;
    virtual void setRange(float bottom, float top, int decimals = 0);
    void setBottom(float);
    void setTop(float);
    void setDecimals(int);
    float bottom() const { return b; }
    float top() const { return t; }
    int decimals() const { return dec; }
Q_SIGNALS:
    void bottomChanged(float bottom);
    void topChanged(float top);
    void decimalsChanged(int decimals);

private:
    Q_DISABLE_COPY(FloatValidator)
    float b;
    float t;
    int dec;
};

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
            return QString{};
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

class VectorValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
public:
    explicit VectorValidator(int size, QObject *parent = nullptr);
    VectorValidator(int size, float bottom, float top, QObject *parent = nullptr);
    ~VectorValidator();
    QValidator::State validate(QString &, int &) const override;
    virtual void setRange(float bottom, float top, int decimals = 0);
    void setBottom(float);
    void setTop(float);
    void setDecimals(int);
    float bottom() const { return b; }
    float top() const { return t; }
Q_SIGNALS:
    void bottomChanged(float bottom);
    void topChanged(float top);

private:
    Q_DISABLE_COPY(VectorValidator)
    float b;
    float t;
    int sz;
};

class IntVectorValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(int bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(int top READ top WRITE setTop NOTIFY topChanged)
public:
    explicit IntVectorValidator(int size, QObject *parent = nullptr);
    IntVectorValidator(int size, int bottom, int top, QObject *parent = nullptr);
    ~IntVectorValidator();
    QValidator::State validate(QString &, int &) const override;
    virtual void setRange(int bottom, int top, int decimals = 0);
    void setBottom(int);
    void setTop(int);
    void setDecimals(int);
    int bottom() const { return b; }
    int top() const { return t; }
Q_SIGNALS:
    void bottomChanged(int bottom);
    void topChanged(int top);

private:
    Q_DISABLE_COPY(IntVectorValidator)
    int b;
    int t;
    int sz;
};

class FloatLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit FloatLineEdit(QWidget *parent = nullptr);
    FloatLineEdit(float fmin, float fmax, int decimals, QWidget *parent = nullptr);

    void setValue(float v);
    float value() const;

private slots:
    void checkInput();
};

class VectorLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit VectorLineEdit(int size, QWidget *parent = nullptr);
    VectorLineEdit(int size, float fmin, float fmax, int decimals, QWidget *parent = nullptr);

    template <class vector_t>
    void setValue(const vector_t &v)
    {
        setText(qstring_serialize<vector_t>::toString(v));
    }
    template <class vector_t>
    bool getValue(vector_t &v)
    {
        return qstring_serialize<vector_t>::fromString(text(), v);
    }
    template <class vector_t>
    vector_t value()
    {
        vector_t v;
        qstring_serialize<vector_t>::fromString(text(), v);
        return v;
    }

private slots:
    void checkInput();
};

class IntVectorLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit IntVectorLineEdit(int size, QWidget *parent = nullptr);
    IntVectorLineEdit(int size, int imin, int imax, QWidget *parent = nullptr);

    template <class vector_t>
    void setValue(const vector_t &v)
    {
        setText(qstring_serialize<vector_t>::toString(v));
    }
    template <class vector_t>
    bool getValue(vector_t &v)
    {
        return qstring_serialize<vector_t>::fromString(text(), v);
    }
    template <class vector_t>
    vector_t value()
    {
        vector_t v;
        qstring_serialize<vector_t>::fromString(text(), v);
        return v;
    }

private slots:
    void checkInput();
};

#endif // FLOATLINEEDIT_H
