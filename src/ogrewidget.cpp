#include "ogrewidget.h"
 
#define THIS OgreWidget
 
/**
 * @brief init the object
 * @author kito berg-taylor
 */
void THIS::init( std::string plugins_file,
         std::string ogre_cfg_file,
         std::string ogre_resources_file,
         std::string ogre_log )
{
  // create the main ogre object
  mOgreRoot = new Ogre::Root( plugins_file, ogre_cfg_file, ogre_log );

  // setup resources
  // Load resource paths from config file
  Ogre::ConfigFile cf;
  cf.load(ogre_resources_file);
  std::cerr << "loading resource" << std::endl;

  // Go through all sections & settings in the file
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
  {
      secName = seci.peekNextKey();
      Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
      Ogre::ConfigFile::SettingsMultiMap::iterator i;
      for (i = settings->begin(); i != settings->end(); ++i)
      {
          typeName = i->first;
          archName = i->second;

          std::cerr << "Archive " << archName << " of type " << typeName << " secName " << secName << std::endl;
          Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
              archName, typeName, secName);
      }
  }


  // setup a renderer
  const Ogre::RenderSystemList renderers = mOgreRoot->getAvailableRenderers();
  assert( !renderers.empty() ); // we need at least one renderer to do anything useful

  Ogre::RenderSystem *renderSystem;
  renderSystem = chooseRenderer(renderers);

  assert( renderSystem ); // user might pass back a null renderer, which would be bad!

  mOgreRoot->setRenderSystem( renderSystem );
  QString dimensions = QString( "%1x%2" )
                    .arg(this->width())
                    .arg(this->height());

  renderSystem->setConfigOption( "Video Mode", dimensions.toStdString() );

  // initialize without creating window
  mOgreRoot->getRenderSystem()->setConfigOption( "Full Screen", "No" );
  mOgreRoot->saveConfig();
  mOgreRoot->initialise(false); // don't create a window
}
 
/**
 * @brief setup the rendering context
 * @author Kito Berg-Taylor
 */
void THIS::initializeGL()
{
  //== Creating and Acquiring Ogre Window ==//

  // Get the parameters of the window QT created
  Ogre::String winHandle;
#ifdef WIN32
  // Windows code
  winHandle += Ogre::StringConverter::toString((unsigned long)(this->parentWidget()->winId()));
#elif MACOS
  // Mac code, tested on Mac OSX 10.6 using Qt 4.7.4 and Ogre 1.7.3
  Ogre::String winHandle  = Ogre::StringConverter::toString(winId());
#else
  // Unix code
  QX11Info info = x11Info();
  winHandle  = Ogre::StringConverter::toString((unsigned long)(info.display()));
  winHandle += ":";
  winHandle += Ogre::StringConverter::toString((unsigned int)(info.screen()));
  winHandle += ":";
  winHandle += Ogre::StringConverter::toString((unsigned long)(this->parentWidget()->winId()));
#endif


  Ogre::NameValuePairList params;
#ifndef MACOS
  // code for Windows and Linux
  params["parentWindowHandle"] = winHandle;
  mOgreWindow = mOgreRoot->createRenderWindow( "QOgreWidget_RenderWindow",
                           this->width(),
                           this->height(),
                           false,
                           &params );

  mOgreWindow->setActive(true);
  WId ogreWinId = 0x0;
  mOgreWindow->getCustomAttribute( "WINDOW", &ogreWinId );

  assert( ogreWinId );

  // bug fix, extract geometry
  QRect geo = this->frameGeometry ( );

  // create new window
  this->create( ogreWinId );

  // set geometrie infos to new window
  this->setGeometry (geo);

#else
  // code for Mac
  params["externalWindowHandle"] = winHandle;
  params["macAPI"] = "cocoa";
  params["macAPICocoaUseNSView"] = "true";
  mOgreWindow = mOgreRoot->createRenderWindow("QOgreWidget_RenderWindow",
      width(), height(), false, &params);
  mOgreWindow->setActive(true);
  makeCurrent();
#endif

  setAttribute( Qt::WA_PaintOnScreen, true );
  setAttribute( Qt::WA_NoBackground );

  //== Ogre Initialization == //
  Ogre::SceneType scene_manager_type = Ogre::ST_EXTERIOR_CLOSE;

  mSceneMgr = mOgreRoot->createSceneManager( scene_manager_type );
  mSceneMgr->setAmbientLight( Ogre::ColourValue(1,1,1) );
  // Create the camera
  mCamera = mSceneMgr->createCamera("PlayerCam");

  // Position it at 500 in Z direction
  mCamera->setPosition(Ogre::Vector3(0,0,80));
  // Look back along -Z
  mCamera->lookAt(Ogre::Vector3(0,0,-300));
  mCamera->setNearClipDistance(5);
  //mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
  // load resources
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
  // Create one viewport, entire window
  Ogre::Viewport* vp = mOgreWindow->addViewport( mCamera );
  vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

  // Alter the camera aspect ratio to match the viewport
  mCamera->setAspectRatio(
  Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
  //-------------------------------------------------------------------------------------
  // Set default mipmap level (NB some APIs ignore this)
  Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
  //-------------------------------------------------------------------------------------
  // Create any resource listeners (for loading screens)
  //createResourceListener();
  //-------------------------------------------------------------------------------------
  // load resources
  Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
  //-------------------------------------------------------------------------------------
  // Create the scene
  Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
  Ogre::Entity* ogreHead2 = mSceneMgr->createEntity("Head2", "ogrehead.mesh");

  Ogre::SceneNode* headNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode2", Ogre::Vector3(100,0,0));
  Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

  headNode2->attachObject(ogreHead2);
  headNode->attachObject(ogreHead);

  // Set ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

  // Create a light
  Ogre::Light* l = mSceneMgr->createLight("MainLight");
  l->setPosition(20,80,50);
}
/**
 * @brief render a frame
 * @author Kito Berg-Taylor
 */
void THIS::paintGL()
{
  // Be sure to call "OgreWidget->repaint();" to call paintGL
  assert( mOgreWindow );
  mOgreRoot->renderOneFrame();
}
 
/**
 * @brief resize the GL window
 * @author Kito Berg-Taylor
 */
void THIS::resizeGL( int width, int height )
{
   assert( mOgreWindow );
   mOgreWindow->reposition( this->pos().x(), 
                            this->pos().y() );    
   mOgreWindow->resize( width, height );
   paintGL();
}
 
/**
 * @brief choose the right renderer
 * @author Kito Berg-Taylor
 */
Ogre::RenderSystem* THIS::chooseRenderer(const Ogre::RenderSystemList& renderers )
{
  // It would probably be wise to do something more friendly
  // that just use the first available renderer
  return *renderers.begin();
}

