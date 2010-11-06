#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
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

  public slots:
    void setPlaybackFrame(int frame);
    void onAnimationChanged(Animation* animation);

protected:
    Animation* animation;
    PlayState _state;
    QTimer timer;
    bool loop;
    int loopIn;
    int loopOut;
    int playFrame() { return animation->getFrame(); }
    int totalFrames() { return animation->getNumberOfFrames(); }
    void playOrPause();
    void stepForward();
    void stepBackward();
    void skipToFirst();
    void skipToLast();
    void updateLabel();

  protected slots:
    void timerTimeout();

private:
    Ui::Player *ui;

private slots:
    void on_playButton_clicked();
};

#endif // PLAYER_H
