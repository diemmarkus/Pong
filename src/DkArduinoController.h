/*******************************************************************************************************
 
 DkArduinoController.h
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

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QThread>
#pragma warning(pop)		// no warnings from includes - end

#include <windows.h>	// needed to read from serial

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

namespace pong {

class DllExport DkArduinoController : public QThread {
	Q_OBJECT

public:
	DkArduinoController(QWidget* widget = 0);

	void setComPort(const QString& cP) { comPort = cP; };

	void run();
	void quit() {
		stop = true;
	}
	
signals:
	void controllerSignal(int controller, int value) const;

protected:
	QWidget* parent;
	HANDLE hCOM;

	QString comPort;
	bool stop;

	void init();
	void serialValue(unsigned short val) const;
	void printComParams(const DCB& dcb) const;

};

};