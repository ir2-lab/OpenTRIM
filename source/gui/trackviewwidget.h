#ifndef TRACKVIEWWIDGET_H
#define TRACKVIEWWIDGET_H

#include <QWidget>

class McDriverObj;
class Track3DViewport;
class QToolBar;
class PendingBlinker;

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

private:
    QToolBar *buildToolBar_();
    QWidget *buildCaptureTab_();
    QWidget *buildColorTab_();
    QWidget *buildCameraTab_();

    Track3DViewport *view_;
    PendingBlinker *blinker_{ nullptr };
    QWidget *guide_{ nullptr };
};

#endif // TRACKVIEWWIDGET_H
