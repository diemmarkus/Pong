/*******************************************************************************************************
 
 DkSettings.h
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

#pragma once

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QSettings>
#include <QBitArray>
#include <QColor>
#include <QDate>
#include <QSharedPointer>
#pragma warning(pop)	// no warnings from includes - end

#pragma warning(disable: 4251)	// TODO: remove

#ifndef DllExport
#ifdef DK_DLL_EXPORT
#define DllExport Q_DECL_EXPORT
#elif DK_DLL_IMPORT
#define DllExport Q_DECL_IMPORT
#else
#define DllExport
#endif
#endif

class QFileInfo;

namespace pong {

class DllExport Settings {

public:
	static Settings& instance();
	QSettings& getSettings();
	
private:
	Settings();

	QSharedPointer<QSettings> m_settings;
};

class DllExport DkSettings {

public:

	enum modes {
		mode_default = 0,
		mode_end,
	};

	struct App {
		bool closeOnEsc;
	};

	struct Display {
		QColor highlightColor;
		QColor bgColorWidget;
		QColor bgColor;
		QColor fontColor;
		QColor iconColor;
	};

	static bool isPortable();

	static QStringList scamDataDesc;
	static QStringList sdescriptionDesc;

	static App& getAppSettings();
	static Display& getDisplaySettings();

	static void load();
	static void save(bool force = false);
	static void setToDefaultSettings();

	static QFileInfo getSettingsFile();

	static App& app;
	static Display& display;

protected:
	static App app_p;
	static Display display_p;

	static App app_d;
	static Display display_d;
};

};
