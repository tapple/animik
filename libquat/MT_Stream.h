/**
 * $Id: MT_Stream.h,v 1.5 2002/12/26 18:25:15 mein Exp $
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#ifndef GEN_STREAM_H
#define GEN_STREAM_H

#ifdef __CUSTOM_STREAM

class MT_OStream
{
public:
    inline  MT_OStream& operator<<(double);
    inline  MT_OStream& operator<<(int);
    inline  MT_OStream& operator<<(char*);
};

const char GEN_endl = '\n';

#else

#include <iostream>

typedef std::ostream MT_OStream;

inline MT_OStream& GEN_endl(MT_OStream& os) { return std::endl(os); }

#endif

#endif

