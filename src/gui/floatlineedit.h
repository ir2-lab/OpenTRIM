#ifndef FLOATLINEEDIT_H
#define FLOATLINEEDIT_H

#include <QLineEdit>
#include <QValidator>
#include "geometry.h"


class FloatValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals NOTIFY decimalsChanged)
public:
    explicit FloatValidator(QObject * parent = nullptr);
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

template <class T> struct num_convert;

template <>
struct num_convert<float> {
    static float str2num(const QString& s, bool& ok) {
        return s.toFloat(&ok);
    }
};

template <>
struct num_convert<int> {
    static int str2num(const QString& s, bool& ok) {
        return s.toInt(&ok);
    }
};

template<class vector_t>
struct qstring_serialize
{
    typedef typename vector3::Scalar scalar_t;

    static vector_t fromString(const QString& S, bool *ok = nullptr)
    {
        vector_t v;
        bool myok;
        bool& ok_ = ok ? *ok : myok;
        ok_ = false;

        QString t = S.trimmed();
        if (t.startsWith('[')) t.remove(0,1);
        else return v;

        if (t.endsWith(']')) t.chop(1);
        else return v;

        QStringList lst = t.split(',',Qt::SkipEmptyParts);
        if (lst.count()!=3) return v;

        bool numok = true;
        int i =0;
        while(i<3 && numok) {
            v[i] = num_convert<scalar_t>::str2num(lst.at(i),numok);
            i++;
        }
        if (!numok) return vector_t{};

        ok_ = true;
        return v;
    }

    static QString toString(const vector_t& v)
    {
        return QString("[%1, %2, %3]")
            .arg(v[0])
            .arg(v[1])
            .arg(v[2]);
    }
};

Q_DECLARE_TYPEINFO(vector3, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(vector3)

Q_DECLARE_TYPEINFO(ivector3, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(ivector3)

class Vector3dValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
public:
    explicit Vector3dValidator(QObject *parent = nullptr);
    Vector3dValidator(float bottom, float top, QObject *parent = nullptr);
    ~Vector3dValidator();
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
    Q_DISABLE_COPY(Vector3dValidator)
    float b;
    float t;
};

class IntVector3dValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(int bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(int top READ top WRITE setTop NOTIFY topChanged)
public:
    explicit IntVector3dValidator(QObject *parent = nullptr);
    IntVector3dValidator(int bottom, int top, QObject *parent = nullptr);
    ~IntVector3dValidator();
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
    Q_DISABLE_COPY(IntVector3dValidator)
    int b;
    int t;
};

class FloatLineEdit : public QLineEdit
{
    Q_OBJECT

public:     
    explicit FloatLineEdit(QWidget* parent = nullptr);
    FloatLineEdit(float fmin, float fmax, int decimals, QWidget* parent = nullptr);

    void setValue(float v);
    float value() const;

private slots:
    void checkInput();
};

class Vector3dLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit Vector3dLineEdit(QWidget* parent = nullptr);
    Vector3dLineEdit(float fmin, float fmax, int decimals, QWidget* parent = nullptr);

    void setValue(const vector3& v);
    vector3 value() const;

private slots:
    void checkInput();
};

class IntVector3dLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit IntVector3dLineEdit(QWidget* parent = nullptr);
    IntVector3dLineEdit(int imin, int imax, QWidget* parent = nullptr);

    void setValue(const ivector3& v);
    ivector3 value() const;

private slots:
    void checkInput();
};

#endif // FLOATLINEEDIT_H
