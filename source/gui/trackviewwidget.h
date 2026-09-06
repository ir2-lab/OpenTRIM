#ifndef TRACKVIEWWIDGET_H
#define TRACKVIEWWIDGET_H

#include <QWidget>

#include "track3dviewport.h"

class McDriverObj;
class Track3DViewport;
class QToolBar;
class PendingBlinker;
class QLabel;
class QSlider;
class QTableWidget;
class QProgressBar;

class TrackViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrackViewWidget(McDriverObj *driver, QWidget *parent = nullptr);

    Track3DViewport *viewport() const { return view_; }

private slots:
    void saveCamera_();
    void loadCamera_();
    void saveScreenshot_();
    void showGuide_();
    void updateCtrls();
    void onRecorderStateChange(CascadeRecorder::State from, CascadeRecorder::State to);

private:
    QToolBar *buildViewToolBar();
    QWidget *buildPlaybackSlider();
    QWidget *buildPlaybackToolBar();
    QWidget *buildOptionsPanel();
    QWidget *buildInfoPanel();
    QWidget *buildBufferTab();
    QWidget *buildColorTab();
    QWidget *buildCameraTab();

    Track3DViewport *view_;
    PendingBlinker *blinker_{ nullptr };
    QWidget *guide_{ nullptr };

    // rec/play actions
    QAction *recAct;
    QAction *playAct;
    QAction *settingsAct;

    // playback slider control
    QLabel *lblMin;
    QLabel *lblMax;
    QSlider *playBackSlider;

    // info panel
    QTableWidget *infoTable;
    QProgressBar *cascadeBar;
    QProgressBar *memBar;
};

#endif // TRACKVIEWWIDGET_H
