/*******************************************************************************************************
 
 Dkmath.cpp
 Created on:	19.12.2015
 
 Pong is a homage to the famous arcade game Pong with the capability of old-school controllers using an Arduino Uno board. 

 Copyright (C) 2015-2016 Markus Diem <markus@nomacs.org>

 This file is part of Pong.

 Pong is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Pong is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************************************/

#include "DkMath.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QCursor>
#include <QTransform>
#pragma warning(pop)		// no warnings from includes - end


#ifndef WITH_OPENCV
int cvRound(float num) {

	return (int) (num+0.5f);
}

int cvCeil(float num) {

	return (int) (num+1.0f);
}

int cvFloor(float num) {

	return (int) num;
}

#endif

namespace pong {

}
