#include "trackviewwidget.h"

#include "track3dviewport.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPolygonF>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QFontMetrics>
#include <QSpinBox>
#include <QScrollArea>
#include <QSplitter>
#include <QStyle>
#include <QTextBrowser>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QPointer>
#include <QPropertyAnimation>
#include <qprogressbar.h>
#include <qtablewidget.h>
#include <QHeaderView>
#include <QResizeEvent>
#include <QtVectorEdit/qnumberedit.h>

static float jsonNumber(const nlohmann::json &j, const char *key, float fallback)
{
    auto it = j.find(key);
    return (it != j.end() && it->is_number()) ? it->get<float>() : fallback;
}

namespace {

// Oblique cube: front square + back square offset (+6,-6), viewBox 32x32
constexpr double FX0 = 5, FY0 = 11, FX1 = 21, FY1 = 27;
constexpr double DX = 6, DY = -6;

const QPointF ftl{ FX0, FY0 }, ftr{ FX1, FY0 };
const QPointF fbl{ FX0, FY1 }, fbr{ FX1, FY1 };
const QPointF btl{ FX0 + DX, FY0 + DY }, btr{ FX1 + DX, FY0 + DY };
const QPointF bbl{ FX0 + DX, FY1 + DY }, bbr{ FX1 + DX, FY1 + DY };

QString polygonFor(Track3DViewport::View f)
{
    const QPointF *q[4] = { };
    switch (f) {
    case Track3DViewport::Front:
        q[0] = &ftl;
        q[1] = &ftr;
        q[2] = &fbr;
        q[3] = &fbl;
        break;
    case Track3DViewport::Back:
        q[0] = &btl;
        q[1] = &btr;
        q[2] = &bbr;
        q[3] = &bbl;
        break;
    case Track3DViewport::Left:
        q[0] = &ftl;
        q[1] = &btl;
        q[2] = &bbl;
        q[3] = &fbl;
        break;
    case Track3DViewport::Right:
        q[0] = &ftr;
        q[1] = &btr;
        q[2] = &bbr;
        q[3] = &fbr;
        break;
    case Track3DViewport::Top:
        q[0] = &ftl;
        q[1] = &ftr;
        q[2] = &btr;
        q[3] = &btl;
        break;
    case Track3DViewport::Bottom:
        q[0] = &fbl;
        q[1] = &fbr;
        q[2] = &bbr;
        q[3] = &bbl;
        break;
    default:
        return { };
    }
    QString pts;
    for (int i = 0; i < 4; ++i)
        pts += QString("%1,%2 ").arg(q[i]->x()).arg(q[i]->y());
    return QString("<polygon points='%1' fill='%2'/>").arg(pts.trimmed(), QStringLiteral("COL"));
}

QString edgesPath()
{
    const QPointF *e[12][2] = {
        { &ftl, &ftr }, { &ftr, &fbr }, { &fbr, &fbl }, { &fbl, &ftl }, // front
        { &btl, &btr }, { &btr, &bbr }, { &bbr, &bbl }, { &bbl, &btl }, // back
        { &ftl, &btl }, { &ftr, &btr }, { &fbl, &bbl }, { &fbr, &bbr }
    }; // connectors
    QString d;
    for (auto &seg : e)
        d += QString("M%1,%2 L%3,%4 ")
                     .arg(seg[0]->x())
                     .arg(seg[0]->y())
                     .arg(seg[1]->x())
                     .arg(seg[1]->y());
    return QString("<path d='%1' fill='none' stroke='COL' stroke-width='1.5' "
                   "stroke-linecap='round' stroke-linejoin='round'/>")
            .arg(d.trimmed());
}

QString buildSvg(Track3DViewport::View face, const QColor &color)
{
    QString svg = QStringLiteral("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'>");
    if (face != Track3DViewport::Iso)
        svg += polygonFor(face);
    svg += edgesPath();
    svg += QStringLiteral("</svg>");
    svg.replace(QLatin1String("COL"), color.name(QColor::HexRgb));
    return svg;
}

QIcon makeViewIcon(Track3DViewport::View face, const QColor &color, int size)
{
    QSvgRenderer renderer(buildSvg(face, color).toUtf8());

    QIcon icon;
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    renderer.render(&p); // scales to pixmap rect
    p.end();
    icon.addPixmap(pm);
    return icon;
}

QString buildRecordSvg()
{
    // Outer ring: identical geometry to assets/ionicons/play-circle-outline.svg
    // (viewBox 512x512, r=192 circle as a cubic-bezier path, 32px stroke).
    // Inner disc: filled red circle, r=110, concentric at (256,256).
    return QStringLiteral("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 512 512'>"
                          "<path d='M448 256c0-106-86-192-192-192S64 150 64 256s86 192 192 192 "
                          "192-86 192-192z' fill='none' stroke='black' stroke-miterlimit='10' "
                          "stroke-width='32'/>"
                          "<circle cx='256' cy='256' r='110' fill='red'/>"
                          "</svg>");
}

// QTableWidget that keeps the first column at 1/3 of the viewport width and
// lets the second column stretch to fill the rest.
class InfoTable : public QTableWidget
{
public:
    using QTableWidget::QTableWidget;

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QTableWidget::resizeEvent(e);
        setColumnWidth(0, viewport()->width() / 3);
    }
};

QIcon makeRecordIcon(int size)
{
    QSvgRenderer renderer(buildRecordSvg().toUtf8());

    QIcon icon;
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    renderer.render(&p); // scales to pixmap rect
    p.end();
    icon.addPixmap(pm);
    return icon;
}

struct
{
    const char *label;
    const char *unit;
    double rate;
} playback_rates[] = { { "1fs/s", "1fs", 0.001 },   { "10fs/s", "10fs", 0.01 },
                       { "100fs/s", "100fs", 0.1 }, { "1ps/s", "1ps", 1.0 },
                       { "10ps/s", "10ps", 10.0 },  { "100ps/s", "100ps", 100.0 },
                       { "1ns/s", "1ns", 1.e3 },    { "10ns/s", "10ns", 1.e4 },
                       { "100ns/s", "100ns", 1.e5 } };

} // namespace

class PendingBlinker : public QObject
{
public:
    PendingBlinker(QAction *action, QToolBar *bar, QObject *parent = nullptr)
        : QObject(parent), m_action(action), m_bar(bar)
    {
    }

    void start()
    {
        if (m_anim)
            return; // already blinking
        QWidget *w = m_bar->widgetForAction(m_action);
        if (!w)
            return; // action not in this toolbar (yet)

        auto *fx = new QGraphicsOpacityEffect(w);
        w->setGraphicsEffect(fx); // w takes ownership

        m_anim = new QPropertyAnimation(fx, "opacity", fx);
        m_anim->setDuration(1000);
        m_anim->setKeyValueAt(0.0, 1.0); // key values, not start/end,
        m_anim->setKeyValueAt(0.5, 0.2); // so the loop ping-pongs instead
        m_anim->setKeyValueAt(1.0, 1.0); // of snapping back
        m_anim->setEasingCurve(QEasingCurve::InOutSine);
        m_anim->setLoopCount(-1);
        m_anim->start();
    }

    void stop()
    {
        if (m_anim)
            m_anim->stop();
        if (QWidget *w = m_bar->widgetForAction(m_action))
            w->setGraphicsEffect(nullptr); // deletes effect + child animation
        m_anim = nullptr;
    }

private:
    QAction *m_action;
    QToolBar *m_bar;
    QPointer<QPropertyAnimation> m_anim; // QPointer: dies with the effect
};

TrackViewWidget::TrackViewWidget(McDriverObj *driver, QWidget *parent) : QWidget(parent)
{
    // frame with track view + controls
    view_ = new Track3DViewport(driver, this);
    TrackColorBar *colorBar = new TrackColorBar(view_);

    QFrame *frm = new QFrame;
    frm->setFrameShape(QFrame::StyledPanel);
    frm->setFrameShadow(QFrame::Sunken);
    frm->setStyleSheet("background: white;");
    QVBoxLayout *frmLay = new QVBoxLayout(frm);
    frmLay->setContentsMargins(0, 0, 0, 0);
    frmLay->setSpacing(0);
    {
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addStretch();
        hbox->addWidget(buildViewToolBar());
        hbox->addStretch();
        frmLay->addLayout(hbox);
    }
    frmLay->addWidget(view_);
    frmLay->addWidget(buildPlaybackSlider());
    {
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addStretch();
        hbox->addWidget(buildPlaybackToolBar());
        hbox->addStretch();
        frmLay->addLayout(hbox);
    }

    QWidget *leftPanel = new QWidget;
    {
        QHBoxLayout *hbox = new QHBoxLayout(leftPanel);
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->addWidget(frm, 1);
        hbox->addWidget(colorBar);
    }

    QWidget *rightPanel = buildOptionsPanel();

    QSplitter *split = new QSplitter(Qt::Horizontal);
    split->addWidget(leftPanel);
    split->addWidget(rightPanel);
    split->setCollapsible(0, false);
    split->setCollapsible(1, true);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    frm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    split->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // must come after the setSizePolicy() calls above: setStretchFactor() works
    // by writing the widget's sizePolicy stretch, so a later setSizePolicy()
    // would wipe it. Left panel takes all extra width; the options panel opens
    // at its own size hint.
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 0);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(split);

    // Connect settingsAct <-> the options/info side panel (split's second widget).
    // The action toggles the panel open/closed; dragging the handle keeps the
    // action in sync. The panel starts collapsed.
    settingsAct->setChecked(false);
    split->setSizes({ 1000, 0 });

    connect(settingsAct, &QAction::toggled, split, [split, rightPanel](bool checked) {
        QList<int> sizes = split->sizes();
        int total = sizes[0] + sizes[1];
        if (checked) {
            if (sizes[1] == 0) {
                // open at the controls' own minimum width, not a fixed fraction
                sizes[1] = qMin(rightPanel->sizeHint().width(), total / 2);
                sizes[0] = qMax(1, total - sizes[1]);
            }
        } else {
            sizes[0] = total;
            sizes[1] = 0;
        }
        split->setSizes(sizes);
    });

    connect(split, &QSplitter::splitterMoved, settingsAct, [split, this](int, int) {
        const bool open = split->sizes().value(1) > 0;
        if (open != settingsAct->isChecked()) {
            QSignalBlocker block(settingsAct); // don't let setChecked re-emit toggled()
            settingsAct->setChecked(open);
        }
    });

    // Connect cascadeRecoredr notifications
    CascadeRecorder *R = view_->cascadeRecorder();
    connect(R, &CascadeRecorder::dataChanged, this, &TrackViewWidget::updateCtrls);
    connect(R, &CascadeRecorder::stateChange, this, &TrackViewWidget::onRecorderStateChange);

    // set initial state to capture ON
    view_->cascadeRecorder()->capture(true);
}

QToolBar *TrackViewWidget::buildViewToolBar()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QToolBar *tb = new QToolBar;
    tb->setMovable(false);
    int iconSize = 22;
    tb->setIconSize(QSize(iconSize, iconSize));
    tb->setStyleSheet("background: white;");

    struct
    {
        const char *tip;
        Track3DViewport::View view;
    } views[] = { { "Home/Iso", Track3DViewport::Iso },  { "Top", Track3DViewport::Top },
                  { "Bottom", Track3DViewport::Bottom }, { "Front", Track3DViewport::Front },
                  { "Back", Track3DViewport::Back },     { "Left", Track3DViewport::Left },
                  { "Right", Track3DViewport::Right } };
    QColor iconColor = palette().color(QPalette::ButtonText);
    for (const auto &v : views) {
        QAction *a = tb->addAction(makeViewIcon(v.view, iconColor, iconSize), QString());
        a->setToolTip(tr(v.tip));
        const Track3DViewport::View vw = v.view;
        connect(a, &QAction::triggered, view_, [this, vw]() {
            if (vw == Track3DViewport::Iso)
                view_->homeView();
            else
                view_->setPresetView(vw);
        });
    }

    tb->addSeparator();

    QAction *shot = tb->addAction(QIcon(":/assets/ionicons/camera-outline.svg"), QString());
    shot->setToolTip(tr("Screenshot"));
    connect(shot, &QAction::triggered, this, &TrackViewWidget::saveScreenshot_);

    tb->addSeparator();

    QAction *hud = tb->addAction(QIcon(":/assets/ionicons/list-outline.svg"), QString());
    hud->setToolTip(tr("Show 3D track buffer info"));
    hud->setCheckable(true);
    hud->setChecked(view_->hudVisible());
    connect(hud, &QAction::toggled, view_, &Track3DViewport::setHudVisible);
    connect(view_, &Track3DViewport::hudVisibleChanged, hud, &QAction::setChecked);

    tb->addSeparator();

    QAction *help = tb->addAction(QIcon(":/assets/ionicons/help-circle-outline.svg"), QString());
    help->setToolTip(tr("Open the 3D viewer guide"));
    connect(help, &QAction::triggered, this, &TrackViewWidget::showGuide_);

    return tb;
}

QWidget *TrackViewWidget::buildPlaybackSlider()
{
    QWidget *w = new QWidget;
    w->setStyleSheet("background: white;");
    QHBoxLayout *hbox = new QHBoxLayout;
    w->setLayout(hbox);
    int hs = hbox->spacing();
    hbox->setContentsMargins(hs, 0, hs, 0);

    lblMin = new QLabel("0.00ns");
    lblMax = new QLabel("1.00ns");
    playBackSlider = new QSlider(Qt::Horizontal);
    playBackSlider->setMaximum(1000000);
    hbox->addWidget(lblMin);
    hbox->addWidget(playBackSlider);
    hbox->addWidget(lblMax);

    CascadeRecorder *R = view_->cascadeRecorder();
    connect(playBackSlider, &QSlider::valueChanged, R, [R](int v) {
        double t0 = R->tMin();
        double w = R->tMax() - t0;
        R->setPlaybackTime(w * v / 1000000 + t0);
    });

    QFontMetrics fm = fontMetrics();
    int sz = fm.averageCharWidth() * 8;
    lblMin->setMinimumWidth(sz);
    lblMax->setMinimumWidth(sz);
    auto szPolicy = lblMin->sizePolicy();
    szPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    lblMin->setSizePolicy(szPolicy);
    lblMax->setSizePolicy(szPolicy);
    w->setSizePolicy(szPolicy);

    return w;
}

QWidget *TrackViewWidget::buildPlaybackToolBar()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QToolBar *tb = new QToolBar;
    tb->setMovable(false);
    int iconSize = 22;
    tb->setIconSize(QSize(iconSize, iconSize));

    // back
    backAct = tb->addAction(QIcon(":/assets/ionicons/play-back-circle-outline.svg"), QString());
    backAct->setToolTip("-1ps");
    connect(backAct, &QAction::triggered, this, &TrackViewWidget::onBack);

    // Record
    recAct = tb->addAction(makeRecordIcon(iconSize), QString());
    recAct->setCheckable(true);
    recAct->setChecked(true);
    recAct->setToolTip("Capture ion tracks into the 3D memory buffer.");
    connect(recAct, &QAction::toggled, R, &CascadeRecorder::capture);
    blinker_ = new PendingBlinker(recAct, tb, this);

    // Play
    playAct = tb->addAction(QIcon(":/assets/ionicons/play-circle-outline.svg"), QString());
    playAct->setCheckable(true);
    playAct->setEnabled(false);
    playAct->setToolTip("Replay stored ion tracks.");
    connect(playAct, &QAction::toggled, R, &CascadeRecorder::play);

    // forward
    forwardAct =
            tb->addAction(QIcon(":/assets/ionicons/play-forward-circle-outline.svg"), QString());
    forwardAct->setToolTip("+1ps");
    connect(forwardAct, &QAction::triggered, this, &TrackViewWidget::onForward);

    tb->addSeparator();

    // Clear
    QAction *clr = tb->addAction(QIcon(":/assets/ionicons/close-circle-outline.svg"), QString());
    clr->setToolTip("Clear ion tracks from 3D memory.");
    connect(clr, &QAction::triggered, R, &CascadeRecorder::clear);

    tb->addSeparator();

    // Playback speed combo
    {
        QComboBox *cb = new QComboBox;
        for (const auto &r : playback_rates) {
            cb->addItem(r.label, r.rate);
        }
        cb->setCurrentIndex(3);
        cb->setToolTip(tr("Cascade playback speed"));
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), R, [cb, R](int i) {
            double r = cb->itemData(i).toDouble();
            R->setPlaybackSpeed(r);
        });

        {
            auto *spacer = new QWidget;
            spacer->setFixedWidth(6); // or setFixedSize(...)
            tb->addWidget(spacer);
        }

        tb->addWidget(cb);

        {
            auto *spacer = new QWidget;
            spacer->setFixedWidth(6); // or setFixedSize(...)
            tb->addWidget(spacer);
        }
    }

    tb->addSeparator();

    settingsAct = tb->addAction(QIcon(":/assets/ionicons/options-outline.svg"), QString());
    settingsAct->setCheckable(true);
    settingsAct->setChecked(false);
    settingsAct->setToolTip("Open/close 3D settings panel.");

    return tb;
}

QWidget *TrackViewWidget::buildOptionsPanel()
{
    QWidget *p = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(p);
    // vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    QFont f = p->font();
    f.setPointSizeF(f.pointSizeF() * 0.88);
    p->setFont(f); // inherited by every child

    QLabel *title = new QLabel("Visualization Options");
    // title->setFrameShape(QFrame::StyledPanel);
    // title->setFrameShadow(QFrame::Raised);
    title->setStyleSheet("font: bold;");
    vbox->addWidget(title);

    // all controls live on a single scrollable panel; the former tabs are now
    // group boxes stacked vertically
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // let the panel's size hint follow the controls instead of the hardcoded
    // 256x192 default, so the splitter can open it at its minimum width
    scroll->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    QWidget *content = new QWidget;
    QVBoxLayout *cvbox = new QVBoxLayout(content);
    cvbox->setContentsMargins(0, 0, 0, 0);

    auto wrapGroup = [](const QString &name, QWidget *inner) {
        QGroupBox *box = new QGroupBox(name);
        QVBoxLayout *l = new QVBoxLayout(box);
        l->setContentsMargins(0, 0, 0, 0);
        l->addWidget(inner);
        return box;
    };

    cvbox->addWidget(wrapGroup(tr("3D Buffer"), buildBufferGroup()));
    cvbox->addWidget(wrapGroup(tr("Color"), buildColorGroup()));
    cvbox->addWidget(wrapGroup(tr("Camera"), buildCameraGroup()));
    cvbox->addStretch(1);

    scroll->setWidget(content);
    vbox->addWidget(scroll, 1);

    return p;
}

static void compactForm(QFormLayout *f)
{
    f->setContentsMargins(6, 4, 6, 4);
    f->setHorizontalSpacing(6);
    f->setVerticalSpacing(2);
    // f->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    f->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
}

static void addSeparator(QFormLayout *form)
{
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(7);
    form->addRow(frame);
}

QWidget *TrackViewWidget::buildBufferGroup()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);
    compactForm(form);

    QToolButton *batchBt = new QToolButton();
    batchBt->setText(tr("Batch"));
    batchBt->setCheckable(true);
    batchBt->setToolTip(tr("Capture N ion cascades and stop"));

    QToolButton *ringBt = new QToolButton();
    ringBt->setText(tr("Ring"));
    ringBt->setCheckable(true);
    ringBt->setToolTip(tr("Continuously capture ion cascades in a N-size ring buffer"));
    ringBt->setChecked(true); // matches CascadeRecorder's default Mode::Ring

    QButtonGroup *modeGroup = new QButtonGroup(w);
    modeGroup->setExclusive(true);
    modeGroup->addButton(batchBt);
    modeGroup->addButton(ringBt);
    connect(ringBt, &QPushButton::toggled, R, &CascadeRecorder::setRingMode);

    QHBoxLayout *modeLay = new QHBoxLayout;
    modeLay->setContentsMargins(0, 0, 0, 0);
    modeLay->setSpacing(0);
    modeLay->addWidget(batchBt);
    modeLay->addWidget(ringBt);
    form->addRow(tr("Buffer Mode"), modeLay);
    {
        QWidget *w = form->labelForField(modeLay);
        w->setToolTip("Set storage mode for ion tracks");
    }

    addSeparator(form);

    {
        QWidget *w = new QLabel("Buffer Size");
        form->addRow(w);
        w->setToolTip("Set the number and size of stored ion cascades.");
    }

    QSpinBox *nBox = new QSpinBox;
    nBox->setRange(1, 1000);
    nBox->setValue(R->nCascades());
    connect(nBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setNCascades);
    form->addRow(tr("Cascades"), nBox);
    {
        const char *eTip =
                "Number of ion cascades to store for display.\n"
                "In ring buffer mode, older cascades will be dropped \nwhen the buffer is full.";
        nBox->setToolTip(tr(eTip));
        form->labelForField(nBox)->setToolTip(tr(eTip));
    }

    QSpinBox *memBox = new QSpinBox;
    memBox->setRange(0, 2000);
    // memBox->setSpecialValueText(tr("off"));
    memBox->setValue(R->memCap() / 1024 / 1024);
    connect(memBox, QOverload<int>::of(&QSpinBox::valueChanged), R,
            [R](int mb) { R->setMemCap(mb * 1024 * 1024); });
    form->addRow(tr("Mem [MB]"), memBox);
    {
        const char *eTip = "Buffer size in MBs. "
                           "Ion cascades will be dropped when the buffer is full.";
        memBox->setToolTip(tr(eTip));
        form->labelForField(memBox)->setToolTip(tr(eTip));
    }

    addSeparator(form);

    {
        QWidget *w = new QLabel("Ion track thresholds");
        form->addRow(w);
        w->setToolTip("Set thresholds for ion track display.");
    }

    QNumberEdit *eThrBox = new QNumberEdit;
    eThrBox->setElementType(QNumberEdit::Real);
    eThrBox->setMinimum(0.001);
    eThrBox->setMaximum(1e9);
    eThrBox->setValue(view_->energyMin());
    connect(eThrBox, &QNumberEdit::valueChanged, R,
            [R](const QVariant &v) { R->setEnergyThreshold(v.toDouble()); });
    form->addRow(tr("E min [eV]"), eThrBox);
    {
        const char *eTip = "Minimum ion energy to display. "
                           "Ion tracks with energy below this value will be ignored.";
        eThrBox->setToolTip(tr(eTip));
        form->labelForField(eThrBox)->setToolTip(tr(eTip));
    }

    QSpinBox *genBox = new QSpinBox;
    genBox->setRange(-1, 20);
    genBox->setValue(-1);
    genBox->setSpecialValueText(tr("all"));
    connect(genBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setGenCutoff);
    form->addRow(tr("Max Recoil gen."), genBox);
    {
        const char *eTip = "Maximum recoil generation to display. "
                           "Recoil generations above this value will be ignored.";
        genBox->setToolTip(tr(eTip));
        form->labelForField(genBox)->setToolTip(tr(eTip));
    }

    return w;
}

QWidget *TrackViewWidget::buildColorGroup()
{
    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);
    compactForm(form);

    QComboBox *colorBox = new QComboBox;
    colorBox->addItems({ tr("Recoil Generation"), tr("Energy"), tr("Atomic Species") });
    connect(colorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), view_,
            &Track3DViewport::setColorMode);
    form->addRow(tr("Mode"), colorBox);

    QComboBox *colorMap = new QComboBox;
    connect(colorMap, QOverload<int>::of(&QComboBox::currentIndexChanged), view_,
            &Track3DViewport::setColorMap);
    form->addRow(tr("Color Map"), colorMap);

    // continuous maps for the energy scale, discrete ones for gen/species
    auto fillMaps = [colorMap](int mode) {
        QSignalBlocker block(colorMap);
        colorMap->clear();
        if (mode == Track3DViewport::Energy)
            colorMap->addItems({ tr("Rainbow"), tr("Turbo") });
        else
            colorMap->addItems({ tr("Tab10"), tr("Set1") });
    };
    fillMaps(view_->colorMode());
    connect(colorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), colorMap,
            [this, fillMaps](int mode) {
                fillMaps(mode);
                view_->setColorMap(0);
            });

    addSeparator(form);

    form->addRow(new QLabel("Energy scale"));

    QCheckBox *logBox = new QCheckBox(tr("Log E"));
    logBox->setChecked(view_->energyLog());
    connect(logBox, &QCheckBox::toggled, view_, &Track3DViewport::setEnergyLog);
    form->addRow(logBox);

    QCheckBox *autoBox = new QCheckBox(tr("Auto Scale"));
    autoBox->setChecked(view_->energyAuto());
    form->addRow(autoBox);

    QNumberEdit *escaleMin = new QNumberEdit;
    escaleMin->setElementType(QNumberEdit::Real);
    escaleMin->setMinimum(0.001);
    escaleMin->setMaximum(1e9);
    escaleMin->setValue(view_->energyMin());
    escaleMin->setEnabled(!view_->energyAuto());
    connect(escaleMin, &QNumberEdit::valueChanged, view_,
            [this](const QVariant &v) { this->view_->setEnergyUserMin(v.toDouble()); });
    form->addRow(tr("min [eV]"), escaleMin);

    QNumberEdit *escaleMax = new QNumberEdit;
    escaleMax->setElementType(QNumberEdit::Real);
    escaleMax->setMinimum(0.001);
    escaleMax->setMaximum(1e9);
    escaleMax->setValue(view_->energyMax());
    escaleMax->setEnabled(!view_->energyAuto());
    connect(escaleMax, &QNumberEdit::valueChanged, view_,
            [this](const QVariant &v) { this->view_->setEnergyUserMax(v.toDouble()); });
    form->addRow(tr("max [eV]"), escaleMax);

    connect(autoBox, &QCheckBox::toggled, view_, &Track3DViewport::setEnergyAuto);
    connect(autoBox, &QCheckBox::toggled, escaleMin, &QWidget::setDisabled);
    connect(autoBox, &QCheckBox::toggled, escaleMax, &QWidget::setDisabled);
    connect(autoBox, &QCheckBox::toggled, this, [this, escaleMin, escaleMax](bool on) {
        if (!on) {
            QSignalBlocker b1(escaleMin), b2(escaleMax);
            escaleMin->setValue(view_->energyMin());
            escaleMax->setValue(view_->energyMax());
        }
    });

    return w;
}

QWidget *TrackViewWidget::buildCameraGroup()
{
    QWidget *w = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout(w);
    hbox->setContentsMargins(6, 4, 6, 4);
    hbox->setSpacing(0);

    QPushButton *saveBt = new QPushButton(tr("Save"));
    saveBt->setToolTip(tr("Save the current state to a JSON file"));
    connect(saveBt, &QPushButton::clicked, this, &TrackViewWidget::saveCamera_);
    hbox->addWidget(saveBt);

    QPushButton *loadBt = new QPushButton(tr("Load"));
    loadBt->setToolTip(tr("Restore a saved state from a JSON file"));
    connect(loadBt, &QPushButton::clicked, this, &TrackViewWidget::loadCamera_);
    hbox->addWidget(loadBt);

    return w;
}

void TrackViewWidget::advanceTime(double dt)
{
    CascadeRecorder *R = view_->cascadeRecorder();
    double t0 = R->tMin();
    double t1 = R->tMax();
    double speed = R->playbackSpeed();
    double t = R->playbackTime() + dt * speed;
    if (t > t1)
        t = t1;
    else if (t < t0)
        t = t0;
    R->setPlaybackTime(t);
}

void TrackViewWidget::saveCamera_()
{
    const QString filter = tr("Json files [*.json](*.json);; All files (*.*)");
    QString path = QFileDialog::getSaveFileName(this, tr("Save camera"), QString(), filter);
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().toLower() != "json")
        path += ".json";

    const Track3DViewport::CameraState c = view_->camera();
    nlohmann::json j = { { "yaw", c.yaw },
                         { "pitch", c.pitch },
                         { "dist", c.dist },
                         { "center", { c.center.x(), c.center.y(), c.center.z() } } };

    std::ofstream os(path.toLatin1().constData());
    if (!os) {
        qWarning() << "TrackViewWidget: cannot write camera file:" << path;
        return;
    }
    os << j.dump(2) << std::endl;
}

void TrackViewWidget::loadCamera_()
{
    const QString filter = tr("Json files [*.json](*.json);; All files (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("Load camera"), QString(), filter);
    if (path.isEmpty())
        return;

    std::ifstream is(path.toLatin1().constData());
    if (!is) {
        qWarning() << "TrackViewWidget: cannot read camera file:" << path;
        return;
    }
    nlohmann::json j = nlohmann::json::parse(is, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        qWarning() << "TrackViewWidget: invalid camera file:" << path;
        return;
    }

    Track3DViewport::CameraState c = view_->camera();
    c.yaw = jsonNumber(j, "yaw", c.yaw);
    c.pitch = jsonNumber(j, "pitch", c.pitch);
    c.dist = jsonNumber(j, "dist", c.dist);
    auto ctr = j.find("center");
    if (ctr != j.end() && ctr->is_array() && ctr->size() == 3 && (*ctr)[0].is_number()
        && (*ctr)[1].is_number() && (*ctr)[2].is_number())
        c.center =
                QVector3D((*ctr)[0].get<float>(), (*ctr)[1].get<float>(), (*ctr)[2].get<float>());
    view_->setCamera(c);
}

void TrackViewWidget::showGuide_()
{
    if (!guide_) {
        guide_ = new QWidget(this, Qt::Window);
        guide_->setWindowTitle(tr("OpenTRIM - 3D Viewer Guide"));
        QVBoxLayout *lay = new QVBoxLayout(guide_);
        QLabel *title = new QLabel(tr("OpenTRIM 3D Visualization of ion tracks"));
        title->setStyleSheet("font-size : 14pt; font-weight : bold;");
        lay->addWidget(title);
        QTextBrowser *browser = new QTextBrowser;
        browser->setSource(QUrl("qrc:./md/track_viewer_guide.md"));
        browser->setOpenExternalLinks(true);
        lay->addWidget(browser);
        guide_->resize(1240, 840);
    }
    guide_->show();
    guide_->raise();
}

void TrackViewWidget::updateCtrls()
{
    CascadeRecorder *R = view_->cascadeRecorder();
    double t = R->playbackTime();
    double tmin = R->tMin();
    double tmax = R->tMax();
    t = std::min(t, tmax);
    double w = tmax - tmin;
    double t1 = t - tmin;
    lblMin->setText(QString("%1ps").arg(t1, 6, 'f', 1));
    lblMax->setText(QString("%1ps").arg(w, 6, 'f', 1));
    playBackSlider->setValue(t1 / w * playBackSlider->maximum());
}

void TrackViewWidget::onRecorderStateChange(CascadeRecorder::State, CascadeRecorder::State to)
{
    QSignalBlocker block1(recAct); // don't let setChecked re-emit toggled()
    QSignalBlocker block2(playAct); // don't let setChecked re-emit toggled()

    CascadeRecorder *R = view_->cascadeRecorder();

    recAct->setEnabled(to != CascadeRecorder::Playing);
    playAct->setEnabled((to == CascadeRecorder::Idle && !R->cascade_buffer().empty())
                        || to == CascadeRecorder::Playing);
    recAct->setChecked(false);
    playAct->setChecked(false);

    switch (to) {
    case CascadeRecorder::Idle:
        this->blinker_->stop();
        break;
    case CascadeRecorder::Capturing:
        recAct->setChecked(true);
        playAct->setChecked(true);
        this->blinker_->stop();
        break;
    case CascadeRecorder::Finishing:
        recAct->setChecked(true);
        playAct->setChecked(true);
        this->blinker_->start();
        break;
    case CascadeRecorder::Paused:
        recAct->setChecked(true);
        playAct->setChecked(true);
        this->blinker_->stop();
        break;
    case CascadeRecorder::Playing:
        playAct->setChecked(true);
        this->blinker_->stop();
        break;
    }
}

void TrackViewWidget::onPlaybackRateChanged(int i)
{
    CascadeRecorder *R = view_->cascadeRecorder();
    double r = playback_rates[i].rate;
    R->setPlaybackSpeed(r);
    backAct->setToolTip(QString("-%1").arg(playback_rates[i].unit));
    forwardAct->setToolTip(QString("+%1").arg(playback_rates[i].unit));
}

void TrackViewWidget::saveScreenshot_()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save screenshot"), QString(),
                                                tr("PNG image [*.png](*.png);; All files (*.*)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += ".png";

    const QImage img = view_->grabScreenshot();
    if (!img.save(path))
        qWarning() << "TrackViewWidget: failed to save screenshot:" << path;
}
