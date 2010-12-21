#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QIcon>
#include <animation.h>
#include <playstate.h>

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
    PlayState state() { return _state; }
    void setFirstFrame(int first) { if(first<loopOut) loopIn = first; }
    void setLastFrame(int last) { if(last>loopIn) loopOut = last; }
    /** Suspend or wake the player */
    void suspend(bool suspend);
    int fps() { return animation->fps(); }
    void setFPS(int fps) { animation->setFPS(fps); }

  signals:
    void playbackFrameChanged(int currentFrame);      //(NOT)TODO: don't attach this to the resulting Animation* !!! (else you'll get signal loop)
                                                      //UPDATE: actually this is most probably obsolete signal as AnimationView listens to its Animation changes

  public slots:
    void setPlaybackFrame(int frame);
    void onAnimationChanged(Animation* animation);

protected:
    Animation* animation;
    PlayState _state;
    QTimer timer;
    bool loop;
    int loopIn;         //the class Animation has its
    int loopOut;        //own loop-in/out  TODO: think of it. UPDATE: think harder. AnimationView (and so Animation) even has stepForward,...
                                                            //        It smells like major methods of this Player may become just thin GUI shells
                                                            //        for AnimaionView that sends them further to Animation
    int playFrame();
    int totalFrames();
    void playOrPause();
    void stepForward();
    void stepBackward();
    void skipToFirst();
    void skipToLast();
    void updateLabel();
    void setButtonsEnabled(bool enabled);

  protected slots:
    void timerTimeout();

private:
    Ui::Player *ui;
    QIcon playIcon;
    QIcon pauseIcon;

private slots:
    void on_endButton_clicked();
    void on_beginButton_clicked();
    void on_nextButton_clicked();
    void on_previousButton_clicked();
    void on_playButton_clicked();
};

#endif // PLAYER_H
