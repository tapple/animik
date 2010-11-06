#include <QTimer>
#include "Player.h"
#include "ui_Player.h"



Player::Player(QWidget *parent) : QWidget(parent), ui(new Ui::Player)
{
  ui->setupUi(this);

  _state = PLAYSTATE_STOPPED;
  loopIn = 0;
  loopOut = 0;
  loop = true;

  connect(&timer,SIGNAL(timeout()),this,SLOT(timerTimeout()));
}

Player::~Player()
{
  delete ui;
}


void Player::suspend(bool suspend)
{
  if(_state==PLAYSTATE_STOPPED)
    return;
  _state = suspend ? PLAYSTATE_SUSPENDED
                   : PLAYSTATE_PLAYING;
}

void Player::setPlaybackFrame(int frame)
{
  if(animation->getFrame() != frame)
  {
    animation->setFrame(frame);
    updateLabel();
    emit playbackFrameChanged(playFrame());
  }
}

void Player::playOrPause()
{
  //From pause to play
  if(_state==PLAYSTATE_STOPPED)
  {
    if(playFrame()==loopOut)
      setPlaybackFrame(loopIn);

    _state = PLAYSTATE_PLAYING;
    timer.start((int)(1.0/fps()*1000.0));
  }
  else    //pause the playback
  {
    timer.stop();
    _state = PLAYSTATE_STOPPED;
  }
}

void Player::stepForward()
{
  if(playFrame() < loopOut)
    setPlaybackFrame(playFrame()+1);
  else
  {
    if(loop)
      setPlaybackFrame(loopIn);
    else    //we reached the end of non-looping animation
    {
      _state = PLAYSTATE_STOPPED;
      timer.stop();
    }
  }
}

void Player::stepBackward()
{
  if(playFrame() > loopIn)
    setPlaybackFrame(playFrame()-1);
  else if(loop)      //TODO: is this the right place for looping?
    setPlaybackFrame(loopOut);
}

void Player::skipToFirst()
{
  if(playFrame() != loopIn)
    setPlaybackFrame(loopIn);
}

void Player::skipToLast()
{
  if(playFrame() != loopOut)
  {
    if(!loop)
    {
      _state = PLAYSTATE_STOPPED;
      timer.stop();
    }
    setPlaybackFrame(loopOut);
  }
}


void Player::onAnimationChanged(Animation *animation)
{
  this->animation = animation;
  int framesCount = animation->getNumberOfFrames();
  if(framesCount<loopIn)
    loopIn=0;
  if(framesCount<loopOut)
    loopOut=framesCount;
}

void Player::timerTimeout()
{
  stepForward();
}


void Player::updateLabel()
{
  ui->label->setText(QString("Frame %1 of %2").arg(playFrame()).arg(totalFrames()));
}

void Player::on_playButton_clicked()
{
  playOrPause();
}
