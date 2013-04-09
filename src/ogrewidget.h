#ifndef __OGREWIDGET_H__
#define __OGREWIDGET_H__
 
#include <OGRE/Ogre.h>
#include <QGLWidget>
 
class OgreWidget : public QGLWidget
{
  //Q_OBJECT;
 
 public:
  OgreWidget( QWidget *parent=0 ):
    QGLWidget( parent ),
    mOgreWindow(NULL)
    {
      init( "../res/plugins.cfg", "../res/ogre.cfg","../res/resources.cfg" ,"../res/ogre.log");
    }
 
  virtual ~OgreWidget()
    {
      mOgreRoot->shutdown();
      delete mOgreRoot;
      destroy();
    }
 
 protected:
  virtual void initializeGL();
  virtual void resizeGL( int, int );
  virtual void paintGL();

  void init( std::string, std::string,std::string, std::string );

  virtual Ogre::RenderSystem* chooseRenderer(const Ogre::RenderSystemList& );

  Ogre::Root *mOgreRoot;
  Ogre::RenderWindow *mOgreWindow;
  Ogre::Camera *mCamera;
  Ogre::Viewport *mViewport;
  Ogre::SceneManager *mSceneMgr;
};
 
#endif
