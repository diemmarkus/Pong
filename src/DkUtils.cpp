/*******************************************************************************************************
 
 DkUtils.cpp
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

#include "DkUtils.h"
#include "DkMath.h"
#include "DkSettings.h"

#if defined(Q_OS_LINUX) && !defined(Q_OS_OPENBSD)
#include <sys/sysinfo.h>
#endif

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QString>
#include <QFileInfo>
#include <QDate>
#include <QRegExp>
#include <QStringList>
#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QFuture>
#include <QtConcurrentRun>
#include <QDir>
#include <QComboBox>
#include <QCoreApplication>
#include <QTranslator>
#include <QUrl>
#include <QStandardPaths>
#include <QApplication>
#pragma warning(pop)		// no warnings from includes - end

#if defined(WIN32) && !defined(SOCK_STREAM)
#include <winsock2.h>	// needed since libraw 0.16
#endif

#ifdef WIN32
#include "shlwapi.h"
#pragma comment (lib, "shlwapi.lib")
#endif


namespace pong {

int DkUtils::debugLevel = DK_MODULE;

// code based on: http://stackoverflow.com/questions/8565430/complete-these-3-methods-with-linux-and-mac-code-memory-info-platform-independe
double DkMemory::getTotalMemory() {

	double mem = -1;

#ifdef WIN32

	MEMORYSTATUSEX MemoryStatus;
	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	if (GlobalMemoryStatusEx(&MemoryStatus)) {
		mem = (double)MemoryStatus.ullTotalPhys;
	}

#elif defined Q_OS_LINUX and not defined(Q_OS_OPENBSD)

	struct sysinfo info;

	if (!sysinfo(&info))
		mem = info.totalram;


#elif defined Q_OS_MAC
	// TODO: could somebody (with a mac please add the corresponding calls?
#endif

	// convert to MB
	if (mem > 0)
		mem /= (1024*1024);

	return mem;
}

double DkMemory::getFreeMemory() {

	double mem = -1;
	

#ifdef WIN32

	MEMORYSTATUSEX MemoryStatus;

	ZeroMemory(&MemoryStatus, sizeof(MEMORYSTATUSEX));
	MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

	if (GlobalMemoryStatusEx(&MemoryStatus)) {
		mem = (double)MemoryStatus.ullAvailPhys;
	}

#elif defined Q_OS_LINUX and not defined(Q_OS_OPENBSD)

	struct sysinfo info;
	
	if (!sysinfo(&info))
		mem = info.freeram;

#elif defined Q_OS_MAC

	// TODO: could somebody with a mac please add the corresponding calls?

#endif

	// convert to MB
	if (mem > 0)
		mem /= (1024*1024);

	return mem;
}

// DkUtils --------------------------------------------------------------------
#ifdef WIN32

bool DkUtils::wCompLogic(const std::wstring & lhs, const std::wstring & rhs) {
	return StrCmpLogicalW(lhs.c_str(),rhs.c_str()) < 0;
	//return true;
}

bool DkUtils::compLogicQString(const QString & lhs, const QString & rhs) {
#if QT_VERSION < 0x050000
	return wCompLogic(lhs.toStdWString(), rhs.toStdWString());
#else
	return wCompLogic((wchar_t*)lhs.utf16(), (wchar_t*)rhs.utf16());	// TODO: is this nice?
#endif
}

#else // !WIN32

bool DkUtils::compLogicQString(const QString & lhs, const QString & rhs) {
	
	return naturalCompare(lhs, rhs, Qt::CaseInsensitive);
}

#endif //!WIN32

bool DkUtils::naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs) {

	int sIdx = 0;

	// the first value is the most significant bit
	// so we try to find the first difference in the strings
	// this gives us an advantage:
	// img1 and img10 are sorted correctly since the string compare
	// does here what it should
	// img4 and img10 are also sorted correctly since 4 < 10
	// in addition we don't get into troubles with large numbers
	// as we skip identical values...
	for (; sIdx < s1.length() && sIdx < s2.length(); sIdx++)
		if (s1[sIdx] != s2[sIdx])
			break;

	// if both values start with a digit
	if (sIdx < s1.length() && sIdx < s2.length() && s1[sIdx].isDigit() && s2[sIdx].isDigit()) {

		QString prefix = "";

		// if the number has zeros we get into troubles:
		// 101 and 12 result in '01' and '2'
		// for double sort this means: 01 < 2 (though 101 > 12)
		// so we simply search the last non zero number that was equal and prepend that
		// if there is no such number (e.g. img001 vs img101) we are fine already
		// this fixes #469
		if (s1[sIdx] == '0' || s2[sIdx] == '0') {

			for (int idx = sIdx-1; idx >= 0; idx--) {
				
				if (s1[idx] != '0' && s1[idx].isDigit()) {	// find the last non-zero number (just check one string they are the same)
					prefix = s1[idx];
					break;
				}
				else if (s1[idx] != '0')
					break;
			}
		}

		QString cs1 = prefix + getLongestNumber(s1, sIdx);
		QString cs2 = prefix + getLongestNumber(s2, sIdx);

		double n1 = cs1.toDouble();
		double n2 = cs2.toDouble();

		if (n1 != n2)
			return n1 < n2;
	}

	// we're good to go with a string compare here...
	return QString::compare(s1, s2, cs) < 0;
}

QString DkUtils::getLongestNumber(const QString& str, int startIdx) {

	int idx;

	for (idx = startIdx; idx < str.length(); idx++) {

		if (!str[idx].isDigit())
			break;
	}

	return str.mid(startIdx, idx-startIdx);
}

bool DkUtils::compDateCreated(const QFileInfo& lhf, const QFileInfo& rhf) {

	return lhf.created() < rhf.created();
}

bool DkUtils::compDateCreatedInv(const QFileInfo& lhf, const QFileInfo& rhf) {

	return !compDateCreated(lhf, rhf);
}

bool DkUtils::compDateModified(const QFileInfo& lhf, const QFileInfo& rhf) {

	return lhf.lastModified() < rhf.lastModified();
}

bool DkUtils::compDateModifiedInv(const QFileInfo& lhf, const QFileInfo& rhf) {

	return !compDateModified(lhf, rhf);
}

bool DkUtils::compFilename(const QFileInfo& lhf, const QFileInfo& rhf) {

	return compLogicQString(lhf.fileName(), rhf.fileName());
}

bool DkUtils::compFilenameInv(const QFileInfo& lhf, const QFileInfo& rhf) {

	return !compFilename(lhf, rhf);
}

bool DkUtils::compRandom(const QFileInfo&, const QFileInfo&) {

	return qrand() % 2 != 0;
}

void DkUtils::addLanguages(QComboBox* langCombo, QStringList& languages) {

	QDir qmDir = qApp->applicationDirPath();
	
	// find all translations
	QStringList translationDirs = DkSettings::getTranslationDirs();
	QStringList fileNames;

	for (int idx = 0; idx < translationDirs.size(); idx++) {
		fileNames += QDir(translationDirs[idx]).entryList(QStringList("nomacs_*.qm"));
	}

	langCombo->addItem("English");
	languages << "en";

	for (int i = 0; i < fileNames.size(); ++i) {
		QString locale = fileNames[i];
		locale.remove(0, locale.indexOf('_') + 1);
		locale.chop(3);

		QTranslator translator;
		DkSettings::loadTranslation(fileNames[i], translator);

		//: this should be the name of the language in which nomacs is translated to
		QString language = translator.translate("nmc::DkGlobalSettingsWidget", "English");
		if (language.isEmpty())
			continue;

		langCombo->addItem(language);
		languages << locale;
	}
	
	langCombo->setCurrentIndex(languages.indexOf(DkSettings::global.language));
	if (langCombo->currentIndex() == -1) // set index to English if language has not been found
		langCombo->setCurrentIndex(0);

}

void DkUtils::mSleep(int ms) {

#ifdef Q_OS_WIN
	Sleep(uint(ms));
#else
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
	nanosleep(&ts, NULL);
#endif

}


bool DkUtils::exists(const QFileInfo& file, int waitMs) {

	QFuture<bool> future = QtConcurrent::run(&DkUtils::checkFile, file);

	for (int idx = 0; idx < waitMs; idx++) {
		if (future.isFinished())
			break;

		//qDebug() << "you are trying the new exists method... - you are modern!";

		mSleep(1);
	}

	//future.cancel();

	// assume file is not existing if it took longer than waitMs
	return (future.isFinished()) ? future : false;	
}

bool DkUtils::checkFile(const QFileInfo& file) {

	return file.exists();
}

QFileInfo DkUtils::urlToLocalFile(const QUrl& url) {

	QUrl lurl = QUrl::fromUserInput(url.toString());

	// try manual conversion first, this fixes the DSC#josef.jpg problems (url fragments)
	QString fString = lurl.toString();
	fString = fString.replace("file:///", "");

	QFileInfo file = QFileInfo(fString);
	if (!file.exists())	// try an alternative conversion
		file = QFileInfo(lurl.toLocalFile());

	return file;
}

/**
 * Returns if a file is supported by nomacs or not.
 * Note: this function only checks for a valid extension.
 * @param fileInfo the file info of the file to be validated.
 * @return bool true if the file format is supported.
 **/ 
bool DkUtils::isValid(const QFileInfo& fileInfo) {

	printf("accepting file...\n");

	QFileInfo fInfo = fileInfo;
	if (fInfo.isSymLink())
		fInfo = fileInfo.symLinkTarget();

	if (!fInfo.exists())
		return false;

	QString fileName = fInfo.fileName();

	return hasValidSuffix(fileName);
}

bool DkUtils::hasValidSuffix(const QString& fileName) {

	for (int idx = 0; idx < DkSettings::app.fileFilters.size(); idx++) {

		QRegExp exp = QRegExp(DkSettings::app.fileFilters.at(idx), Qt::CaseInsensitive);
		exp.setPatternSyntax(QRegExp::Wildcard);
		if (exp.exactMatch(fileName))
			return true;
	}

	return false;
}

QDateTime DkUtils::getConvertableDate(const QString& date) {

	QDateTime dateCreated;
	QStringList dateSplit = date.split(QRegExp("[/: \t]"));

	if (date.count(":") != 4 /*|| date.count(QRegExp("\t")) != 1*/)
		return dateCreated;

	if (dateSplit.size() >= 3) {

		int y = dateSplit[0].toInt();
		int m = dateSplit[1].toInt();
		int d = dateSplit[2].toInt();

		if (y == 0 || m == 0 || d == 0)
			return dateCreated;

		QDate dateV = QDate(y, m, d);
		QTime time;

		if (dateSplit.size() >= 6)
			time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());

		dateCreated = QDateTime(dateV, time);
	}

	return dateCreated;
}

QDateTime DkUtils::convertDate(const QString& date, const QFileInfo& file) {

	// convert date
	QDateTime dateCreated;
	QStringList dateSplit = date.split(QRegExp("[/: \t]"));

	if (dateSplit.size() >= 3) {

		QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
		QTime time;

		if (dateSplit.size() >= 6)
			time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());

		dateCreated = QDateTime(dateV, time);
	}
	else if (file.exists())
		dateCreated = file.created();

	return dateCreated;
};

QString DkUtils::convertDateString(const QString& date, const QFileInfo& file) {

	// convert date
	QString dateConverted;
	QStringList dateSplit = date.split(QRegExp("[/: \t]"));

	if (dateSplit.size() >= 3) {

		QDate dateV = QDate(dateSplit[0].toInt(), dateSplit[1].toInt(), dateSplit[2].toInt());
		dateConverted = dateV.toString(Qt::SystemLocaleShortDate);

		if (dateSplit.size() >= 6) {
			QTime time = QTime(dateSplit[3].toInt(), dateSplit[4].toInt(), dateSplit[5].toInt());
			dateConverted += " " + time.toString(Qt::SystemLocaleShortDate);
		}
	}
	else if (file.exists()) {
		QDateTime dateCreated = file.created();
		dateConverted += dateCreated.toString(Qt::SystemLocaleShortDate);
	}
	else
		dateConverted = "unknown date";

	return dateConverted;
}

QString DkUtils::colorToString(const QColor& col) {

	return "rgba(" + QString::number(col.red()) + "," + QString::number(col.green()) + "," + QString::number(col.blue()) + "," + QString::number((float)col.alpha()/255.0f*100.0f) + "%)";
}

QStringList DkUtils::filterStringList(const QString& query, const QStringList& list) {

	// white space is the magic thingy
	QStringList queries = query.split(" ");
	QStringList resultList = list;

	for (int idx = 0; idx < queries.size(); idx++) {
		resultList = resultList.filter(queries[idx], Qt::CaseInsensitive);
		qDebug() << "query: " << queries[idx];
	}

	// if string match returns nothing -> try a regexp
	if (resultList.empty()) {
		QRegExp regExp(query);
		resultList = list.filter(regExp);

		if (resultList.empty()) {
			regExp.setPatternSyntax(QRegExp::Wildcard);
			resultList = list.filter(regExp);
		}
	}

	return resultList;
}

bool DkUtils::moveToTrash(const QString& filePath) {

	QFileInfo fileInfo(filePath);

	if (!fileInfo.exists()) {
		qDebug() << "Sorry, I cannot delete a non-existing file: " << filePath;
		return false;
	}

// code is based on:http://stackoverflow.com/questions/17964439/move-files-to-trash-recycle-bin-in-qt
#ifdef WIN32
	
	std::wstring winPath = (fileInfo.isSymLink()) ? qStringToStdWString(fileInfo.symLinkTarget()) : qStringToStdWString(filePath);
	winPath.append(1, L'\0');	// path string must be double nul-terminated

	SHFILEOPSTRUCTW shfos = {};
	shfos.hwnd   = nullptr;		// handle to window that will own generated windows, if applicable
	shfos.wFunc  = FO_DELETE;
	shfos.pFrom  = winPath.c_str();
	shfos.pTo    = nullptr;		// not used for deletion operations
	shfos.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT; // use the recycle bin

	const int retVal = SHFileOperationW(&shfos);
	
	return retVal == 0;		// true if no error code

//#elif Q_OS_LINUX
//	bool TrashInitialized = false;
//	QString TrashPath;
//	QString TrashPathInfo;
//	QString TrashPathFiles;
//
//	if( !TrashInitialized ) {
//		QStringList paths;
//		const char* xdg_data_home = getenv( "XDG_DATA_HOME" );
//		if( xdg_data_home ){
//			qDebug() << "XDG_DATA_HOME not yet tested";
//			QString xdgTrash( xdg_data_home );
//			paths.append( xdgTrash + "/Trash" );
//		}
//		QString home = QStandardPaths::writableLocation( QStandardPaths::HomeLocation );
//		paths.append( home + "/.local/share/Trash" );
//		paths.append( home + "/.trash" );
//		foreach( QString path, paths ){
//			if( TrashPath.isEmpty() ){
//				QDir dir( path );
//				if( dir.exists() ){
//					TrashPath = path;
//				}
//			}
//		}
//		if (TrashPath.isEmpty())
//			return false;
//		TrashPathInfo = TrashPath + "/info";
//		TrashPathFiles = TrashPath + "/files";
//		if (!QDir(TrashPathInfo).exists() || !QDir(TrashPathFiles).exists())
//			return false;
//		TrashInitialized = true;
//	}
//
//	QString info;
//	info += "[Trash Info]\nPath=";
//	info += filePath;
//	info += "\nDeletionDate=";
//	info += QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzzZ");
//	info += "\n";
//	QString trashname = fileInfo.fileName();
//	QString infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
//	QString trashPath = TrashPathFiles + "/" + trashname;
//	int nr = 1;
//	while( QFileInfo( infopath ).exists() || QFileInfo( trashPath ).exists() ){
//		nr++;
//		trashname = fileInfo.baseName() + "." + QString::number( nr );
//		if( !fileInfo.completeSuffix().isEmpty() ){
//			trashname += QString( "." ) + fileInfo.completeSuffix();
//		}
//		infopath = TrashPathInfo + "/" + trashname + ".trashinfo";
//		trashPath = TrashPathFiles + "/" + trashname;
//	}
//	QDir dir;
//	if( !dir.rename( filePath, trashPath )) {
//		return false;
//	}
//	File infofile;
//	infofile.createUtf8( infopath, info );
//	return true;
#else
	QFile fileHandle(filePath);
	return fileHandle.remove();
#endif

	return false;	// should never be hit
}

QString DkUtils::readableByte(float bytes) {

	if (bytes >= 1024*1024*1024) {
		return QString::number(bytes/(1024.0f*1024.0f*1024.0f), 'f', 2) + " GB";
	}
	else if (bytes >= 1024*1024) {
		return QString::number(bytes/(1024.0f*1024.0f), 'f', 2) + " MB";
	}
	else if (bytes >= 1024) {
		return QString::number(bytes/1024.0f, 'f', 2) + " KB";
	}
	else {
		return QString::number(bytes, 'f', 2) + " B";
	}

}

QString DkUtils::cleanFraction(const QString& frac) {

	QStringList sList = frac.split('/');
	QString cleanFrac = frac;

	if (sList.size() == 2) {
		int nom = sList[0].toInt();		// nominator
		int denom = sList[1].toInt();	// denominator

		// if exposure time is less than a second -> compute the gcd for nice values (1/500 instead of 2/1000)
		if (nom != 0 && denom != 0) {
			int gcd = DkMath::gcd(denom, nom);
			cleanFrac = QString::number(nom/gcd);

			// do not show fractions like 9/1 -> it is more natural to write 9 in these cases
			if (denom/gcd != 1)
				 cleanFrac += QString("/") + QString::number(denom/gcd);

			qDebug() << frac << " >> " << cleanFrac;
		}
	}
	
	return cleanFrac;
}

QString DkUtils::resolveFraction(const QString& frac) {

	QString result = frac;
	QStringList sList = frac.split('/');

	if (sList.size() == 2) {
	
		bool nok = false;
		bool dok = false;
		int nom = sList[0].toInt(&nok);
		int denom = sList[1].toInt(&dok);

		if (nok && dok && denom)
			result = QString::number((double)nom/denom);
	}

	return result;
}

// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
std::wstring DkUtils::qStringToStdWString(const QString &str) {
#ifdef _MSC_VER
	return std::wstring((const wchar_t *)str.utf16());
#else
	return str.toStdWString();
#endif
}

// code from: http://stackoverflow.com/questions/5625884/conversion-of-stdwstring-to-qstring-throws-linker-error
QString DkUtils::stdWStringToQString(const std::wstring &str) {
#ifdef _MSC_VER
	return QString::fromUtf16((const ushort *)str.c_str());
#else
	return QString::fromStdWString(str);
#endif
}

// DkConvertFileName --------------------------------------------------------------------
DkFileNameConverter::DkFileNameConverter(const QString& fileName, const QString& pattern, int cIdx) {

	this->mFileName = fileName;
	this->mPattern = pattern;
	this->mCIdx = cIdx;
}

/**
 * Converts file names with a given pattern (used for e.g. batch rename)
 * The pattern is:
 * <d:3> is replaced with the cIdx value (:3 -> zero padding up to 3 digits)
 * <c:0> int (0 = no change, 1 = to lower, 2 = to upper)
 * 
 * if it ends with .jpg we assume a fixed extension.
 * .<old> is replaced with the fileName extension.
 * 
 * So a filename could look like this:
 * some-fixed-name-<c:1><d:3>.<old>
 * @return QString
 **/ 
QString DkFileNameConverter::getConvertedFileName() {
	
	QString newFileName = mPattern;
	QRegExp rx("<.*>");
	rx.setMinimal(true);

	while (rx.indexIn(newFileName) != -1) {
		QString tag = rx.cap();
		QString res = "";

		if (tag.contains("<c:"))
			res = resolveFilename(tag);
		else if (tag.contains("<d:"))
			res = resolveIdx(tag);
		else if (tag.contains("<old>"))
			res = resolveExt(tag);

		// replace() replaces all matches - so if two tags are the very same, we save a little computing
		newFileName = newFileName.replace(tag, res);

	}

	return newFileName;
}

QString DkFileNameConverter::resolveFilename(const QString& tag) const {

	QString result = mFileName;
	
	// remove extension (Qt's QFileInfo.baseName() does a bad job if you have filenames with dots)
	result = result.replace("." + QFileInfo(mFileName).suffix(), "");

	int attr = getIntAttribute(tag);

	if (attr == 1)
		result = result.toLower();
	else if (attr == 2)
		result = result.toUpper();

	return result;
}

QString DkFileNameConverter::resolveIdx(const QString& tag) const {

	QString result = "";

	// append zeros
	int numZeros = getIntAttribute(tag);
	int startIdx = getIntAttribute(tag, 2);
	int fIdx = startIdx+mCIdx;

	if (numZeros > 0) {

		// if fIdx <= 0, log10 must not be evaluated
		int cNumZeros = fIdx > 0 ? qRound(numZeros - std::floor(std::log10(fIdx))) : numZeros;

		// zero padding
		for (int idx = 0; idx < cNumZeros; idx++) {
			result += "0";
		}
	}

	result += QString::number(fIdx);

	return result;
}

QString DkFileNameConverter::resolveExt(const QString&) const {

	QString result = QFileInfo(mFileName).suffix();

	return result;
}

int DkFileNameConverter::getIntAttribute(const QString& tag, int idx) const {

	int attr = 0;

	QStringList num = tag.split(":");

	if (num.length() > idx) {
		QString attrStr = num.at(idx);
		attrStr.replace(">", "");
		attr = attrStr.toInt();

		// no negative idx
		if (attr < 0)
			return 0;
	}

	return attr;
}

// TreeItem --------------------------------------------------------------------
TreeItem::TreeItem(const QVector<QVariant> &data, TreeItem *parent) {
	parentItem = parent;
	itemData = data;
}

TreeItem::~TreeItem() {
	clear();
}

void TreeItem::clear() {
	qDeleteAll(childItems);
	childItems.clear();
}

void TreeItem::appendChild(TreeItem *item) {
	childItems.append(item);
	//item->setParent(this);
}

TreeItem* TreeItem::child(int row) {

	if (row < 0 || row >= childItems.size())
		return 0;

	return childItems[row];
}

int TreeItem::childCount() const {
	return childItems.size();
}

int TreeItem::row() const {

	if (parentItem)
		return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

int TreeItem::columnCount() const {

	int columns = itemData.size();

	for (int idx = 0; idx < childItems.size(); idx++)
		columns = qMax(columns, childItems[idx]->columnCount());

	return columns;
}

QVariant TreeItem::data(int column) const {
	return itemData.value(column);
}

void TreeItem::setData(const QVariant& value, int column) {

	if (column < 0 || column >= itemData.size())
		return;

	qDebug() << "replacing: " << itemData[0] << " with: " << value;
	itemData.replace(column, value);
}

TreeItem* TreeItem::find(const QVariant& value, int column) {

	if (column < 0)
		return 0;

	if (column < itemData.size() && itemData[column] == value)
		return this;

	for (int idx = 0; idx < childItems.size(); idx++) 
		if (TreeItem* child = childItems[idx]->find(value, column))
			return child;

	return 0;
}

TreeItem* TreeItem::parent() const {
	return parentItem;
}

void TreeItem::setParent(TreeItem* parent) {
	parentItem = parent;
}


}
