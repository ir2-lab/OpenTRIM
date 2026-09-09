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
    void onBack() { advanceTime(-1.0); }
    void onForward() { advanceTime(+1.0); }
    void onPlaybackRateChanged(int i);

private:
    QToolBar *buildViewToolBar();
    QWidget *buildPlaybackSlider();
    QWidget *buildPlaybackToolBar();
    QWidget *buildOptionsPanel();
    QWidget *buildBufferGroup();
    QWidget *buildColorGroup();
    QWidget *buildCameraGroup();
    // change world playback time by dt seconds
    void advanceTime(double dt);

    Track3DViewport *view_;
    PendingBlinker *blinker_{ nullptr };
    QWidget *guide_{ nullptr };

    // rec/play actions
    QAction *recAct;
    QAction *playAct;
    QAction *backAct;
    QAction *forwardAct;
    QAction *settingsAct;

    // playback slider control
    QLabel *lblMin;
    QLabel *lblMax;
    QSlider *playBackSlider;
};

#endif // TRACKVIEWWIDGET_H
