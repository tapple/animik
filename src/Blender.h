#ifndef BLENDER_H
#define BLENDER_H


class TrailItem;
class WeightedAnimation;



class Blender
{
public:
  Blender();
  ~Blender();

  /** Returns overall blended animation */
  WeightedAnimation* GetResultingAnimation();

  /** Blends together weighted animations of given TrailItems. */
  WeightedAnimation* BlendTrails(TrailItem** trails);
};

#endif // BLENDER_H
