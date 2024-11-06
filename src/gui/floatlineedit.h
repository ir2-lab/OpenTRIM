#ifndef FLOATLINEEDIT_H
#define FLOATLINEEDIT_H

#include <QLineEdit>
#include <QValidator>
#include <QJsonValue>
#include <QJsonArray>


class FloatValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(double bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(double top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals NOTIFY decimalsChanged)
public:
    explicit FloatValidator(QObject * parent = nullptr);
    FloatValidator(double bottom, double top, int decimals, QObject *parent = nullptr);
    ~FloatValidator();
    QValidator::State validate(QString &, int &) const override;
    virtual void setRange(double bottom, double top, int decimals = 0);
    void setBottom(double);
    void setTop(double);
    void setDecimals(int);
    double bottom() const { return b; }
    double top() const { return t; }
    int decimals() const { return dec; }
Q_SIGNALS:
    void bottomChanged(double bottom);
    void topChanged(double top);
    void decimalsChanged(int decimals);
private:
    Q_DISABLE_COPY(FloatValidator)
    double b;
    double t;
    int dec;
};

template<class T>
class _Vector3D_
{
public:
    _Vector3D_() : v{T(0), T(0), T(0)} {}
    _Vector3D_(T xpos, T ypos, T zpos) : v{xpos, ypos, zpos} {}

    bool isNull() const
    { return v[0]==T(0) && v[1]==T(0) && v[2]==T(0); }

    T x() const { return v[0]; }
    T y() const { return v[1]; }
    T z() const { return v[2]; }

    void setX(T x) { v[0] = x; }
    void setY(T y) { v[1] = y; }
    void setZ(T z) { v[2] = z; }

    T &operator[](int i)
    { Q_ASSERT(uint(i) < 3u); return v[i]; }
    T operator[](int i) const
    { Q_ASSERT(uint(i) < 3u); return v[i]; }

    QJsonValue toJsonValue()
    {
        QJsonArray array;
        array.push_back(x());
        array.push_back(y());
        array.push_back(z());
        return array;
    }

    static _Vector3D_ fromJsonValue(QJsonValue jv)
    {
        _Vector3D_ v;
        if (jv.isArray()) {
            QJsonArray array = jv.toArray();
            if (array.size()==3) {
                v.setX(array.at(0).toDouble());
                v.setY(array.at(1).toDouble());
                v.setZ(array.at(2).toDouble());
            }
        }
        return v;
    }

    bool operator==(const _Vector3D_ &v1)
    {
        return v1.v[0] == v[0] && v1.v[1] == v[1] && v1.v[2] == v[2];
    }

    bool operator!=(const _Vector3D_ &v1)
    {
        return v1.v[0] != v[0] || v1.v[1] != v[1] || v1.v[2] != v[2];
    }

private:
    T v[3];
};

typedef _Vector3D_<double> Vector3D;
typedef _Vector3D_<int> IntVector3D;

Q_DECLARE_TYPEINFO(Vector3D, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(Vector3D)

Q_DECLARE_TYPEINFO(IntVector3D, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(IntVector3D)

inline
IntVector3D toIntVector3D(const QString& s, bool *ok = 0)
{
    QByteArray ba = s.toLatin1();
    int x,y,z;
    int i = sscanf(ba.constData(),"[%d, %d, %d]",&x,&y,&z);
    if (ok) *ok = (i==3);
    if (i==3)
        return IntVector3D(x,y,z);
    else
        return IntVector3D();
}

inline
    QString toString(const IntVector3D& v)
{
    return QString("[ %1, %2, %3]")
        .arg(v.x())
        .arg(v.y())
        .arg(v.z());
}

inline
Vector3D toVector3D(const QString& s, bool *ok = 0)
{
    QByteArray ba = s.toLatin1();
    float x,y,z;
    int i = sscanf(ba.constData(),"[%g, %g, %g]",&x,&y,&z);
    if (ok) *ok = (i==3);
    if (i==3)
        return Vector3D(x,y,z);
    else
        return Vector3D();
}

inline
QString toString(const Vector3D& v, char f = 'g', int digits = 6)
{
    return QString("[ %1, %2, %3]")
        .arg(v.x(),0,f,digits)
        .arg(v.y(),0,f,digits)
        .arg(v.z(),0,f,digits);
}

class Vector3dValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(double bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(double top READ top WRITE setTop NOTIFY topChanged)
public:
    explicit Vector3dValidator(QObject *parent = nullptr);
    Vector3dValidator(double bottom, double top, QObject *parent = nullptr);
    ~Vector3dValidator();
    QValidator::State validate(QString &, int &) const override;
    virtual void setRange(double bottom, double top, int decimals = 0);
    void setBottom(double);
    void setTop(double);
    void setDecimals(int);
    double bottom() const { return b; }
    double top() const { return t; }
Q_SIGNALS:
    void bottomChanged(double bottom);
    void topChanged(double top);
private:
    Q_DISABLE_COPY(Vector3dValidator)
    double b;
    double t;
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
    FloatLineEdit(double fmin, double fmax, int decimals, QWidget* parent = nullptr);

    void setValue(double v);
    double value() const;

private slots:
    void checkInput();
};

class Vector3dLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit Vector3dLineEdit(QWidget* parent = nullptr);
    Vector3dLineEdit(double fmin, double fmax, int decimals, QWidget* parent = nullptr);

    void setValue(const Vector3D& v);
    Vector3D value() const;

private slots:
    void checkInput();
};

class IntVector3dLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit IntVector3dLineEdit(QWidget* parent = nullptr);
    IntVector3dLineEdit(int imin, int imax, QWidget* parent = nullptr);

    void setValue(const IntVector3D& v);
    IntVector3D value() const;

private slots:
    void checkInput();
};

#endif // FLOATLINEEDIT_H
