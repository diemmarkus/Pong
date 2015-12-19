/*******************************************************************************************************
 
 main.cpp
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


#pragma warning(push, 0)	// no warnings from includes
#include <QApplication>
#include <QCommandLineParser>
#pragma warning(pop)

#include "DkPong.h"
#include "DkSettings.h"
#include "DkArduinoController.h"

int main(int argc, char** argv) {
	
	QCoreApplication::setOrganizationName("Vienna University of Technology");
	QCoreApplication::setOrganizationDomain("http://www.nomacs.org");
	QCoreApplication::setApplicationName("Pong");

	QApplication app(argc, argv);

	// CMD parser --------------------------------------------------------------------
	QCommandLineParser parser;

	//parser.setApplicationDescription("Test helper");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("image", QObject::tr("An input image."));

	// fullscreen (-f)
	QCommandLineOption fullScreenOpt(QStringList() << "f" << "fullscreen", QObject::tr("Start in fullscreen."));
	parser.addOption(fullScreenOpt);

	// set com port
	QCommandLineOption comOpt(QStringList() << "c" << "comport",
		QObject::tr("Open <com> com port."),
		QObject::tr("com"));
	parser.addOption(comOpt);

	// set player name
	QCommandLineOption p1NameOpt(QStringList() << "p1" << "player1",
		QObject::tr("Player 1 <name>."),
		QObject::tr("<name>"));
	parser.addOption(p1NameOpt);

	// set player name
	QCommandLineOption p2NameOpt(QStringList() << "p2" << "player2",
		QObject::tr("Player 2 <name>."),
		QObject::tr("<name>"));
	parser.addOption(p2NameOpt);

	parser.process(app);
	// CMD parser --------------------------------------------------------------------


	pong::DkPong* pw = new pong::DkPong();
	
	// go to fullscreen if needed
	if (parser.isSet(fullScreenOpt))
		pw->showFullScreen();

	pong::DkArduinoController* controller = pw->viewport()->getController();

	// set COM port
	if (!parser.value(comOpt).isEmpty()) {
		controller->setComPort(parser.value(comOpt));
	}

	if (!parser.value(p1NameOpt).isEmpty()) {
		pw->viewport()->player1()->setName(parser.value(p1NameOpt));
		qDebug() << "player:" << parser.value(p1NameOpt);
	}

	if (!parser.value(p2NameOpt).isEmpty()) {
		pw->viewport()->player2()->setName(parser.value(p2NameOpt));
		qDebug() << "player:" << parser.value(p2NameOpt);
	}


	pw->viewport()->start();

	// run pong
	int rVal = app.exec();
	return rVal;

}