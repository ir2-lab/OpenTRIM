#include "floatlineedit.h"

#include <cmath>

bool validateChars(QStringView str, QByteArray *buff, int decDigits);

FloatValidator::FloatValidator(QObject *parent) : FloatValidator(-1.e30, 1.e30, 1000, parent) { }
FloatValidator::FloatValidator(float bottom, float top, int decimals, QObject *parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
    dec = decimals;
}

FloatValidator::~FloatValidator() { }

// #ifndef LLONG_MAX
// #   define LLONG_MAX Q_INT64_C(0x7fffffffffffffff)
// #endif

QValidator::State FloatValidator::validate(QString &input, int &) const
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
    float i = input.toFloat(&ok); // returns 0.0 if !ok
    if (i == std::numeric_limits<float>::quiet_NaN())
        return QValidator::Invalid;
    if (!ok)
        return QValidator::Intermediate;
    if (i >= b && i <= t)
        return QValidator::Acceptable;

    return QValidator::Intermediate;
}
void FloatValidator::setRange(float minimum, float maximum, int decimals)
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
void FloatValidator::setBottom(float bottom)
{
    setRange(bottom, top(), decimals());
}
void FloatValidator::setTop(float top)
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
            case '.': {
                // If a double has more than one decimal point, it shall be Invalid.
                if (++decPointCnt > 1)
                    return false;
#if 0
                    // If a double with no decimal digits has a decimal point, it shall be \
    // Invalid.
                        if (decDigits == 0)
                            return false;
#endif // On second thoughts, it shall be Valid.
                dec = true;
            } break;
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

VectorValidator::VectorValidator(int size, QObject *parent)
    : VectorValidator(size, -1.e30, 1.e30, parent)
{
}
VectorValidator::VectorValidator(int size, float bottom, float top, QObject *parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
    sz = size;
}

VectorValidator::~VectorValidator() { }

QValidator::State VectorValidator::validate(QString &input, int &) const
{
    QString s = input.trimmed();

    if (s.isEmpty())
        return QValidator::Intermediate;

    if (s.front() != '[')
        return QValidator::Intermediate;
    if (s.back() != ']')
        return QValidator::Intermediate;

    std::vector<float> v(sz, 0.0f);
    bool ok = qstring_serialize<std::vector<float>>::fromString(input, v); // returns 0.0 if !ok

    if (!ok)
        return QValidator::Intermediate;

    for (float x : v) {
        if (x < b || x > t)
            return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}
void VectorValidator::setRange(float minimum, float maximum, int decimals)
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
void VectorValidator::setBottom(float bottom)
{
    setRange(bottom, top());
}
void VectorValidator::setTop(float top)
{
    setRange(bottom(), top);
}

/***********************************************************************/

IntVectorValidator::IntVectorValidator(int size, QObject *parent)
    : IntVectorValidator(size, -2000000000, 2000000000, parent)
{
}
IntVectorValidator::IntVectorValidator(int size, int bottom, int top, QObject *parent)
    : QValidator(parent)
{
    b = bottom;
    t = top;
    sz = size;
}

IntVectorValidator::~IntVectorValidator() { }

QValidator::State IntVectorValidator::validate(QString &input, int &) const
{
    QString s = input.trimmed();

    if (s.isEmpty())
        return QValidator::Intermediate;

    if (s.front() != '[')
        return QValidator::Intermediate;
    if (s.back() != ']')
        return QValidator::Intermediate;

    std::vector<int> v(sz, 0);
    bool ok = qstring_serialize<std::vector<int>>::fromString(input, v); // returns 0.0 if !ok

    if (!ok)
        return QValidator::Intermediate;

    for (int x : v) {
        if (x < b || x > t)
            return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}
void IntVectorValidator::setRange(int minimum, int maximum, int decimals)
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
void IntVectorValidator::setBottom(int bottom)
{
    setRange(bottom, top());
}
void IntVectorValidator::setTop(int top)
{
    setRange(bottom(), top);
}

/*************************************************************/

FloatLineEdit::FloatLineEdit(QWidget *parent) : QLineEdit(parent)
{
    setValidator(new FloatValidator(-1.e30f, 1.e30f, 6));

    connect(this, &QLineEdit::textEdited, this, &FloatLineEdit::checkInput);
}

FloatLineEdit::FloatLineEdit(float fmin, float fmax, int decimals, QWidget *parent)
    : QLineEdit(parent)
{
    setValidator(new FloatValidator(fmin, fmax, decimals));

    connect(this, &QLineEdit::textEdited, this, &FloatLineEdit::checkInput);
}

void FloatLineEdit::setValue(float v)
{
    const FloatValidator *vld = dynamic_cast<const FloatValidator *>(validator());
    setText(QString::number(1.0 * v, 'g', vld->decimals()));
}

float FloatLineEdit::value() const
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

VectorLineEdit::VectorLineEdit(int size, QWidget *parent) : QLineEdit(parent)
{
    setValidator(new VectorValidator(size));

    connect(this, &QLineEdit::textEdited, this, &VectorLineEdit::checkInput);
}

VectorLineEdit::VectorLineEdit(int size, float fmin, float fmax, int decimals, QWidget *parent)
    : QLineEdit(parent)
{
    setValidator(new VectorValidator(size, fmin, fmax));

    connect(this, &QLineEdit::textEdited, this, &VectorLineEdit::checkInput);
}

void VectorLineEdit::checkInput()
{
    if (!hasAcceptableInput())
        setStyleSheet("border: 2px solid darkred");
    else {
        setStyleSheet("");
    }
}

/*************************************************************/

IntVectorLineEdit::IntVectorLineEdit(int size, QWidget *parent) : QLineEdit(parent)
{
    setValidator(new IntVectorValidator(size));

    connect(this, &QLineEdit::textEdited, this, &IntVectorLineEdit::checkInput);
}

IntVectorLineEdit::IntVectorLineEdit(int size, int imin, int imax, QWidget *parent)
    : QLineEdit(parent)
{
    setValidator(new IntVectorValidator(size, imin, imax));

    connect(this, &QLineEdit::textEdited, this, &IntVectorLineEdit::checkInput);
}

void IntVectorLineEdit::checkInput()
{
    if (!hasAcceptableInput())
        setStyleSheet("border: 2px solid darkred");
    else {
        setStyleSheet("");
    }
}
