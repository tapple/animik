#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

void SolidCube( GLdouble dSize );
void SolidSphere(GLdouble radius, GLint slices, GLint stacks);
void SolidCone( GLdouble base, GLdouble height, GLint slices, GLint stacks );
void SolidTorus( GLdouble dInnerRadius, GLdouble dOuterRadius, GLint nSides, GLint nRings );
