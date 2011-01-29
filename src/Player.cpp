#include <QTimer>
#include "Player.h"
#include "ui_Player.h"



Player::Player(QWidget *parent) : QWidget(parent), ui(new Ui::Player)
{
  ui->setupUi(this);
  pauseIcon.addFile(QString::fromUtf8(":/icons/player/icons/player/PlayerPause.png"), QSize(),
                    QIcon::Normal, QIcon::Off);
  playIcon.addFile(QString::fromUtf8(":/icons/player/icons/player/PlayerPlay.png"), QSize(),
                   QIcon::Normal, QIcon::Off);

  animation = 0;
  _state = PLAYSTATE_STOPPED;
  ui->playButton->setIcon(playIcon);
  loopIn = 0;
  loopOut = 0;
  loop = true;

  setButtonsEnabled(false);
  updateLabel();
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

int Player::playFrame()
{
  if(animation)
    return animation->getFrame();
  else
    return 0;
}

int Player::totalFrames()
{
  if(animation)
    return animation->getNumberOfFrames();
  else
    return 0;
}

void Player::playOrPause()
{
  //From pause to play
  if(_state==PLAYSTATE_STOPPED)
  {
    if(playFrame()==loopOut)
      setPlaybackFrame(loopIn);

    _state = PLAYSTATE_PLAYING;
    ui->playButton->setIcon(pauseIcon);
    timer.start((int)(1.0/fps()*1000.0));
  }
  else    //pause the playback
  {
    timer.stop();
    _state = PLAYSTATE_STOPPED;
    ui->playButton->setIcon(playIcon);
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


void Player::onAnimationChanged(WeightedAnimation *animation)
{
  setButtonsEnabled(animation!=0);

  int oldPlayFrame = playFrame();
  //When first overall animation emerges, stretch looping to max
  if(/*DEBUG. Uncomment when LoopIn/Out set by some form. this->animation==0 &&*/ animation)
    loopOut = animation->getNumberOfFrames()-1;

//edu: this is insidous! Disconnects ALL slots tied to that object.
//      DO NOT USE THAT!  disconnect(animation, SIGNAL(currentFrame(int)), 0, 0);
  this->animation = animation;

  if(animation)
  {
    connect(animation, SIGNAL(currentFrame(int)), this, SLOT(animationFrameChanged()));

    int framesCount = animation->getNumberOfFrames();
    if(framesCount<loopIn)
      loopIn=0;
    if(framesCount<loopOut)
      loopOut=framesCount-1;    //last animation's frame
    if(oldPlayFrame<=totalFrames())
      setPlaybackFrame(oldPlayFrame);
  }
  else
    loopIn = loopOut = 0;
}


void Player::timerTimeout()
{
  stepForward();
}

void Player::animationFrameChanged()
{
  updateLabel();
}


void Player::setButtonsEnabled(bool enabled)
{
  ui->beginButton->setEnabled(enabled);
  ui->previousButton->setEnabled(enabled);
  ui->playButton->setEnabled(enabled);
  ui->nextButton->setEnabled(enabled);
  ui->endButton->setEnabled(enabled);
}

void Player::updateLabel()
{
  ui->label->setText(QString("Frame %1 of %2").arg(playFrame()+1).arg(totalFrames()));
  ui->label->repaint();
}

void Player::on_playButton_clicked()
{
  playOrPause();
}

void Player::on_previousButton_clicked()
{
  stepBackward();
}

void Player::on_nextButton_clicked()
{
  stepForward();
}

void Player::on_beginButton_clicked()
{
  skipToFirst();
}

void Player::on_endButton_clicked()
{
  skipToLast();
}
