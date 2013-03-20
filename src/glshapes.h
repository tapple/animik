#ifndef GLSHAPES_H
#define GLSHAPES_H

void SolidCube( GLdouble dSize );
void SolidSphere(GLdouble radius, GLint slices, GLint stacks);
void SolidCone( GLdouble base, GLdouble height, GLint slices, GLint stacks );
void SolidTorus( GLdouble dInnerRadius, GLdouble dOuterRadius, GLint nSides, GLint nRings );

#endif