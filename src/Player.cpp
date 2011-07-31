#include <QTimer>
#include "player.h"
#include "ui_player.h"
#include "playeroptionsdialog.h"



Player::Player(QWidget *parent) : QWidget(parent), ui(new Ui::Player)
{
  ui->setupUi(this);
  pauseIcon.addFile(QString::fromUtf8(":/icons/player/icons/player/PlayerPause.png"), QSize(),
                    QIcon::Normal, QIcon::Off);
  playIcon.addFile(QString::fromUtf8(":/icons/player/icons/player/PlayerPlay.png"), QSize(),
                   QIcon::Normal, QIcon::Off);

  animation = NULL;
  _state = PLAYSTATE_STOPPED;
  ui->playButton->setIcon(playIcon);
  loopIn = 0;
  loopOut = 0;
  _loop = true;

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
  if(suspend)
  {
    stateBeforeSuspend = _state;
    _state = PLAYSTATE_SUSPENDED;
    timer.stop();
  }
  else if(_state == PLAYSTATE_SUSPENDED /*&& suspend==false */)
  {
    _state = stateBeforeSuspend;
    timer.start((int)(1.0/fps()*1000.0));
  }
}

void Player::setPlaybackFrame(int frame)
{
  if(animation->getFrame() != frame)
  {
    animation->setFrame(frame);
//automatic through signal loop    updateLabel();
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

void Player::PlayOrPause()
{
  //From pause to play
  if(_state==PLAYSTATE_STOPPED)
  {
    if(playFrame()==loopOut)
      setPlaybackFrame(loopIn);

    _state = PLAYSTATE_PLAYING;
    ui->playButton->setIcon(pauseIcon);
    timer.start((int)(1.0/fps()*1000.0));

    emit playbackStarted();
  }
  else    //pause the playback
  {
    timer.stop();
    _state = PLAYSTATE_STOPPED;
    ui->playButton->setIcon(playIcon);

    emit playbackPaused();
  }
}

void Player::StepForward()
{
  if(playFrame() < loopOut)
    setPlaybackFrame(playFrame()+1);
  else
  {
    if(_loop)
      setPlaybackFrame(loopIn);
    else    //we reached the end of non-looping animation
    {
      _state = PLAYSTATE_STOPPED;
      timer.stop();
      ui->playButton->setIcon(playIcon);
    }
  }
}

void Player::StepBackward()
{
  if(playFrame() > loopIn)
    setPlaybackFrame(playFrame()-1);
  else if(_loop)
    setPlaybackFrame(loopOut);
}

void Player::SkipToFirst()
{
  if(playFrame() != loopIn)
    setPlaybackFrame(loopIn);
}

void Player::SkipToLast()
{
  if(playFrame() != loopOut)
  {
    if(!_loop)
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
  if(/*DEBUG. Uncomment when LoopIn/Out set by some form. this->animation==0 &&*/ animation!=NULL)
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
  StepForward();
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
  ui->optionsPushButton->setEnabled(enabled);
}

void Player::updateLabel()
{
  //DEBUG variables
  QLabel* debugLbl = ui->label;
  int played = playFrame();
  int total = totalFrames();

  /*ui->label*/debugLbl->setText(QString("Frame %1 of %2").arg(played+1).arg(total));
  /*ui->label*/debugLbl->repaint();
}

void Player::on_playButton_clicked()
{
  PlayOrPause();
}

void Player::on_previousButton_clicked()
{
  StepBackward();
}

void Player::on_nextButton_clicked()
{
  StepForward();
}

void Player::on_beginButton_clicked()
{
  SkipToFirst();
}

void Player::on_endButton_clicked()
{
  SkipToLast();
}

void Player::on_optionsPushButton_clicked()
{
  PlayerOptionsDialog* pod = new PlayerOptionsDialog(fps(), _loop, this);
  pod->exec();

  if(pod->result() == QDialog::Accepted && animation != NULL)
  {
    if(pod->fps() != fps() || pod->loop() != _loop)
    {
      animation->setFPS(pod->fps());
      _loop = pod->loop();         //No need to set looping on the animation
      emit playerOptionsChanged(fps());
    }
  }

  delete pod;
}
