#ifndef BLENDER_H
#define BLENDER_H


class Animation;

class Blender
{
public:
  Blender();
  ~Blender();

  /** Returns overall blended animation */
  Animation* getResultingAnimation();

protected:

};

#endif // BLENDER_H
