#include "floatlineedit.h"

#include <cmath>

bool validateChars(QStringView str, QByteArray *buff,
                   int decDigits);


FloatValidator::FloatValidator(QObject * parent)
    : FloatValidator(-1.e30, 1.e30, 1000, parent)
{
}
FloatValidator::FloatValidator(double bottom, double top, int decimals,
                                 QObject * parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
    dec = decimals;
}

FloatValidator::~FloatValidator()
{
}

//#ifndef LLONG_MAX
//#   define LLONG_MAX Q_INT64_C(0x7fffffffffffffff)
//#endif

QValidator::State FloatValidator::validate(QString & input, int &) const
{
    QByteArray buff;
    if (!validateChars(input, &buff, dec)) {
        return QValidator::Invalid;
    }

    if (buff.isEmpty())
        return QValidator::Intermediate;

    if (b >= 0 && buff.startsWith('-'))
        return QValidator::Invalid;

    if (t < 0 && buff.startsWith('+'))
        return QValidator::Invalid;

    bool ok = false;
    double i = input.toDouble(&ok); // returns 0.0 if !ok
    if (i == std::numeric_limits<double>::quiet_NaN())
        return QValidator::Invalid;
    if (!ok)
        return QValidator::Intermediate;
    if (i >= b && i <= t)
        return QValidator::Acceptable;

    return QValidator::Intermediate;
}
void FloatValidator::setRange(double minimum, double maximum, int decimals)
{
    bool rangeChanged = false;
    if (b != minimum) {
        b = minimum;
        rangeChanged = true;
        emit bottomChanged(b);
    }
    if (t != maximum) {
        t = maximum;
        rangeChanged = true;
        emit topChanged(t);
    }
    if (dec != decimals) {
        dec = decimals;
        rangeChanged = true;
        emit decimalsChanged(dec);
    }
    if (rangeChanged)
        emit changed();
}
void FloatValidator::setBottom(double bottom)
{
    setRange(bottom, top(), decimals());
}
void FloatValidator::setTop(double top)
{
    setRange(bottom(), top, decimals());
}
void FloatValidator::setDecimals(int decimals)
{
    setRange(bottom(), top(), decimals);
}

bool validateChars(QStringView str, QByteArray *buff, int decDigits)
{
    buff->clear();
    buff->reserve(str.length());
    const bool scientific = true;
    bool lastWasE = false;
    bool lastWasDigit = false;
    int eCnt = 0;
    int decPointCnt = 0;
    bool dec = false;
    int decDigitCnt = 0;
    for (qsizetype i = 0; i < str.size(); ++i) {
        char c = str.at(i).toLower().toLatin1();
        if (c >= '0' && c <= '9') {

            // If a float has too many digits after decpt, it shall be Invalid.
            if (dec && decDigits != -1 && decDigits < ++decDigitCnt)
                return false;

            // The only non-digit character after the 'e' can be '+' or '-'.
            // If a zero is directly after that, then the exponent is zero-padded.
            //            if ((number_options & QLocale::RejectLeadingZeroInExponent)
            //                && c == '0' && eCnt > 0 && !lastWasDigit) {
            //                return false;
            //            }
            lastWasDigit = true;
        } else {
            switch (c) {
            case '.':
            {
                // If a double has more than one decimal point, it shall be Invalid.
                if (++decPointCnt > 1)
                    return false;
#if 0 \
    // If a double with no decimal digits has a decimal point, it shall be \
    // Invalid.
                        if (decDigits == 0)
                            return false;
#endif                  // On second thoughts, it shall be Valid.
                dec = true;
            }
            break;
            case '+':
            case '-':
                // If num has a sign that's not at the beginning or after
                // an 'e', it shall be Invalid.
                if (i != 0 && !lastWasE)
                    return false;
                break;
            case 'e':
                // If a scientific has more than one 'e', it shall be Invalid.
                if (++eCnt > 1)
                    return false;
                dec = false;
                break;
            default:
                // If it's not a valid digit, it shall be Invalid.
                return false;
            }
            lastWasDigit = false;
        }
        lastWasE = c == 'e';
        if (c != ',')
            buff->append(c);
    }
    return true;
}

/***********************************************************************/

Vector3dValidator::Vector3dValidator(QObject * parent)
    : Vector3dValidator(-1.e30, 1.e30, parent)
{
}
Vector3dValidator::Vector3dValidator(double bottom, double top,
                                     QObject * parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
}

Vector3dValidator::~Vector3dValidator()
{
}

QValidator::State Vector3dValidator::validate(QString & input, int &) const
{
    QString s = input.trimmed();

    if (s.isEmpty())
        return QValidator::Intermediate;

    if (s.front()!='[') return QValidator::Intermediate;
    if (s.back()!=']') return QValidator::Intermediate;

    bool ok = false;
    Vector3D v = toVector3D(input, &ok); // returns 0.0 if !ok

    if (!ok)
        return QValidator::Intermediate;

    if (v.x() < b || v.x() > t)
        return QValidator::Intermediate;
    if (v.y() < b || v.y() > t)
        return QValidator::Intermediate;
    if (v.z() < b || v.z() > t)
        return QValidator::Intermediate;

    return QValidator::Acceptable;
}
void Vector3dValidator::setRange(double minimum, double maximum, int decimals)
{
    bool rangeChanged = false;
    if (b != minimum) {
        b = minimum;
        rangeChanged = true;
        emit bottomChanged(b);
    }
    if (t != maximum) {
        t = maximum;
        rangeChanged = true;
        emit topChanged(t);
    }
    if (rangeChanged)
        emit changed();
}
void Vector3dValidator::setBottom(double bottom)
{
    setRange(bottom, top());
}
void Vector3dValidator::setTop(double top)
{
    setRange(bottom(), top);
}

/***********************************************************************/

IntVector3dValidator::IntVector3dValidator(QObject * parent)
    : IntVector3dValidator(-2000000000, 2000000000, parent)
{
}
IntVector3dValidator::IntVector3dValidator(int bottom, int top,
                                     QObject * parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
}

IntVector3dValidator::~IntVector3dValidator()
{
}

QValidator::State IntVector3dValidator::validate(QString & input, int &) const
{
    QString s = input.trimmed();

    if (s.isEmpty())
        return QValidator::Intermediate;

    if (s.front()!='[') return QValidator::Intermediate;
    if (s.back()!=']') return QValidator::Intermediate;

    bool ok = false;
    IntVector3D v = toIntVector3D(input, &ok); // returns 0.0 if !ok

    if (!ok)
        return QValidator::Intermediate;

    if (v.x() < b || v.x() > t)
        return QValidator::Intermediate;
    if (v.y() < b || v.y() > t)
        return QValidator::Intermediate;
    if (v.z() < b || v.z() > t)
        return QValidator::Intermediate;

    return QValidator::Acceptable;
}
void IntVector3dValidator::setRange(int minimum, int maximum, int decimals)
{
    bool rangeChanged = false;
    if (b != minimum) {
        b = minimum;
        rangeChanged = true;
        emit bottomChanged(b);
    }
    if (t != maximum) {
        t = maximum;
        rangeChanged = true;
        emit topChanged(t);
    }
    if (rangeChanged)
        emit changed();
}
void IntVector3dValidator::setBottom(int bottom)
{
    setRange(bottom, top());
}
void IntVector3dValidator::setTop(int top)
{
    setRange(bottom(), top);
}

/*************************************************************/

FloatLineEdit::FloatLineEdit(QWidget* parent) :
    QLineEdit(parent)
{
    setValidator(new FloatValidator(-1.e30f,1.e30f,6));

    connect(this, &QLineEdit::textEdited,
            this, &FloatLineEdit::checkInput);
}

FloatLineEdit::FloatLineEdit(double fmin, double fmax, int decimals, QWidget* parent) :
    QLineEdit(parent)
{
    setValidator(new FloatValidator(fmin,fmax,decimals));

    connect(this, &QLineEdit::textEdited,
            this, &FloatLineEdit::checkInput);
}

void FloatLineEdit::setValue(double v)
{
    const FloatValidator* vld = dynamic_cast<const FloatValidator*>(validator());
    setText(QString::number(1.0*v,'g',vld->decimals()));
}

double FloatLineEdit::value() const
{
    return text().toDouble();
}

void FloatLineEdit::checkInput()
{
    if (!hasAcceptableInput())
        setStyleSheet("border: 2px solid darkred");
    else {
        setStyleSheet("");
    }
}

/*************************************************************/

Vector3dLineEdit::Vector3dLineEdit(QWidget* parent) :
    QLineEdit(parent)
{
    setValidator(new FloatValidator(-1.e30,1.e30,6));

    connect(this, &QLineEdit::textEdited,
            this, &Vector3dLineEdit::checkInput);
}

Vector3dLineEdit::Vector3dLineEdit(double fmin, double fmax, int decimals, QWidget* parent) :
    QLineEdit(parent)
{
    setValidator(new Vector3dValidator(fmin,fmax));

    connect(this, &QLineEdit::textEdited,
            this, &Vector3dLineEdit::checkInput);
}

void Vector3dLineEdit::setValue(const Vector3D& v)
{
    setText(toString(v));
}

Vector3D Vector3dLineEdit::value() const
{
    return toVector3D(text());
}

void Vector3dLineEdit::checkInput()
{
    if (!hasAcceptableInput())
        setStyleSheet("border: 2px solid darkred");
    else {
        setStyleSheet("");
    }
}

/*************************************************************/

IntVector3dLineEdit::IntVector3dLineEdit(QWidget* parent) :
    QLineEdit(parent)
{
    setValidator(new IntVector3dValidator(-2000000000,2000000000));

    connect(this, &QLineEdit::textEdited,
            this, &IntVector3dLineEdit::checkInput);
}

IntVector3dLineEdit::IntVector3dLineEdit(int imin, int imax, QWidget* parent) :
    QLineEdit(parent)
{
    setValidator(new IntVector3dValidator(imin,imax));

    connect(this, &QLineEdit::textEdited,
            this, &IntVector3dLineEdit::checkInput);
}

void IntVector3dLineEdit::setValue(const IntVector3D& v)
{
    setText(toString(v));
}

IntVector3D IntVector3dLineEdit::value() const
{
    return toIntVector3D(text());
}

void IntVector3dLineEdit::checkInput()
{
    if (!hasAcceptableInput())
        setStyleSheet("border: 2px solid darkred");
    else {
        setStyleSheet("");
    }
}

