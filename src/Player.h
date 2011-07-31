#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QIcon>
#include "weightedanimation.h"
#include "playstate.h"

class QTimer;


namespace Ui {
    class Player;
}


class Player : public QWidget
{
    Q_OBJECT

  public:
    Player(QWidget *parent = 0);
    ~Player();

    /** Get current play state */
    PlayState state()              { return _state; }
    bool isPlaying() const         { return _state == PLAYSTATE_PLAYING || _state == PLAYSTATE_PLAYING; }
    void setFirstFrame(int first)  { if(first<loopOut) loopIn = first; }
    void setLastFrame(int last)    { if(last>loopIn) loopOut = last; }
    /** Suspend or wake the player */
    void suspend(bool suspend);
    int fps() const                { return animation->fps(); }
    void setFPS(int fps)           { animation->setFPS(fps); }
    bool loop() const              { return _loop; }
    void setLoop(bool looping)     { _loop = looping; }

    void PlayOrPause();
    void StepForward();
    void StepBackward();
    void SkipToFirst();
    void SkipToLast();

  signals:
    void playbackStarted();
    void playbackPaused();
    void playerOptionsChanged(int fps);

  public slots:
    void setPlaybackFrame(int frame);
    void onAnimationChanged(WeightedAnimation* animation);

  protected:
    WeightedAnimation* animation;
    QTimer timer;
    bool _loop;
    int loopIn;         //the class Animation has its
    int loopOut;        //own loop-in/out  TODO: think of it. UPDATE: think harder. AnimationView (and so Animation) even has stepForward,...
                                                            //        It smells like major methods of this Player may become just thin GUI shells
                                                            //        for AnimaionView that sends them further to Animation
                                                            //UPDATE2: well, it seems the LoopIN/LoopOUT points are useless for this Player used in BlenderTab
    int playFrame();
    int totalFrames();
    void updateLabel();
    void setButtonsEnabled(bool enabled);

  protected slots:
    void timerTimeout();
    void animationFrameChanged();

private:
    Ui::Player *ui;
    QIcon playIcon;
    QIcon pauseIcon;

    PlayState _state;
    PlayState stateBeforeSuspend;

private slots:
    void on_optionsPushButton_clicked();
    void on_endButton_clicked();
    void on_beginButton_clicked();
    void on_nextButton_clicked();
    void on_previousButton_clicked();
    void on_playButton_clicked();
};

#endif // PLAYER_H
