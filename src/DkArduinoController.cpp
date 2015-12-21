/*******************************************************************************************************
 
 DkArduinoController.cpp
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

#include "DkArduinoController.h"

#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QSettings>
#include <QDebug>
#include <QWidget>
#pragma warning(pop)		// no warnings from includes - end

namespace pong {

// DkArduinoController --------------------------------------------------------------------
DkArduinoController::DkArduinoController(QWidget* widget) : QThread(widget) {
	parent = widget;
	init();
}

void DkArduinoController::init() {

	comPort = "COM4";
	stop = false;
}

void DkArduinoController::run() {

	qDebug() << "starting thread...";
	
	std::wstring comPortStd = DkUtils::qStringToStdWString(comPort);
	hCOM = CreateFileW((LPCWSTR)comPortStd.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	//DWORD EVDSRM = 42;

	if (hCOM == INVALID_HANDLE_VALUE) {
		
		qDebug() << "hCOM " << comPort << " is NULL....";
		return;
	}
	else {
		qDebug() << comPort << "is up and running...\n";
	}

	DCB dcb;

	FillMemory(&dcb, sizeof(dcb), 0);
	if (!GetCommState(hCOM, &dcb))     // get current DCB
		qDebug() << "error in get COM state...";
	// Error in GetCommState
	//return FALSE;

	// Update DCB rate.
	dcb.BaudRate = CBR_9600 ;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	//dcb.Parity = 0;
	//dcb.ByteSize = 8;
	//dcb.StopBits = 1;

	//emit message(QString("baud rate: ") + QString::number(dcb.BaudRate));

	//if (!BuildCommDCB(L("baud=9600 parity=0 data=8 stop=1"), &dcb))
	//	emit message("NOT building comm DCB");

	//dcb.fOutxCtsFlow = MS_CTS_ON;

	// Set new state.
	if (!SetCommState(hCOM, &dcb))
		qDebug() << "error in set com state";
	// Error in SetCommState. Possibly a problem with the communications 
	// port handle or a problem with the DCB structure itself.

	//GetCommState(hCOM, &dcb);
	printf("fRts: %d\n", dcb.fRtsControl);
	printf("fDtr: %d\n", dcb.fDtrControl);

	//if (!EscapeCommFunction(hCOM, CLRDTR))
	//	emit message("NOT clearing DTR");
	//Sleep(200);
	//if (!EscapeCommFunction(hCOM, SETDTR))
	//	emit message("NOT setting DTR");

	//printf("stat (beginning): %d\n", *modemStat);

	DWORD read;

	////////////////////////////////
	for (;;) {

		Sleep(5);
		if (stop)
			break;

		//if (!GetCommModemStatus(hCOM, modemStat))
		//	printf("error in getCommModemStatus...\n");

		byte magicByte = 0;
		ReadFile(hCOM, &magicByte, sizeof(magicByte), &read, NULL);

		if (magicByte == 42) {
			unsigned short buffer = 0;
			ReadFile(hCOM, &buffer, sizeof(buffer), &read, NULL);

			//qDebug() << "buffer" << buffer;

			serialValue(buffer);
		}
	}

}

void DkArduinoController::serialValue(unsigned short val) const {

	unsigned short value = (val & 0x03ff);
	unsigned short controller = ((val & 0xfc00) >> 10) ;

	emit controllerSignal((int)controller, (int)value);
}

}