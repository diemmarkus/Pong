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

	//qDebug() << "dcb defaults ------------------------";
	//printComParams(dcb);

	// set params for serial port communication
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	//dcb.EofChar = 0xFE;		// þ - should be that... (small letter thorn)
	//dcb.ErrorChar = 0x01;		// SOH (start of heading)
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fTXContinueOnXoff = 1;
	dcb.XoffLim = 512;
	dcb.XonLim = 2048;

	// Set new state.
	if (!SetCommState(hCOM, &dcb)) {
		qDebug() << "cannot set com state!";
	}

	if (!GetCommState(hCOM, &dcb))     // get current DCB
		qDebug() << "error in get COM state...";

	//qDebug() << "\n\ndcb our params ------------------------";
	//printComParams(dcb);

	// clear all operations that were performed _before_ we started...
	PurgeComm(hCOM, PURGE_RXCLEAR);

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
		qDebug() << "I read: " << read << "magic byte:" << magicByte;

		if (magicByte == 42) {
			unsigned short buffer = 0;
			ReadFile(hCOM, &buffer, sizeof(buffer), &read, NULL);

			//qDebug() << "buffer" << buffer;

			serialValue(buffer);
		}
	}

}

void DkArduinoController::printComParams(const DCB& dcb) const {

	qDebug() << "BaudRate" << dcb.BaudRate;
	qDebug() << "ByteSize"<< dcb.ByteSize;
	qDebug() << "DCBlength" << dcb.DCBlength;
	qDebug() << "EofChar" << dcb.EofChar;
	qDebug() << "ErrorChar" << dcb.ErrorChar;
	qDebug() << "EvtChar" << dcb.EvtChar;
	qDebug() << "fAbortOnError" << dcb.fAbortOnError;
	qDebug() << "fBinary" << dcb.fBinary;
	qDebug() << "fDsrSensitivity" << dcb.fDsrSensitivity;
	qDebug() << "fDtrControl" << dcb.fDtrControl;
	qDebug() << "fDummy2" << dcb.fDummy2;
	qDebug() << "fErrorChar" << dcb.fErrorChar;
	qDebug() << "fInX" << dcb.fInX;
	qDebug() << "fNull" << dcb.fNull;
	qDebug() << "fOutX" << dcb.fOutX;
	qDebug() << "fOutxCtsFlow" << dcb.fOutxCtsFlow;
	qDebug() << "fOutxDsrFlow" << dcb.fOutxDsrFlow;
	qDebug() << "fParity" << dcb.fParity;
	qDebug() << "fRtsControl" << dcb.fRtsControl;
	qDebug() << "fTXContinueOnXoff" << dcb.fTXContinueOnXoff;
	qDebug() << "Parity" << dcb.Parity;
	qDebug() << "StopBits" << dcb.StopBits;
	qDebug() << "wReserved" << dcb.wReserved;
	qDebug() << "wReserved1" << dcb.wReserved1;
	qDebug() << "XoffChar" << dcb.XoffChar;
	qDebug() << "XoffLim" << dcb.XoffLim;
	qDebug() << "XonChar" << dcb.XonChar;
	qDebug() << "XonLim" << dcb.XonLim;
}

void DkArduinoController::serialValue(unsigned short val) const {

	unsigned short value = (val & 0x03ff);
	unsigned short controller = ((val & 0xfc00) >> 10) ;

	emit controllerSignal((int)controller, (int)value);
}

}