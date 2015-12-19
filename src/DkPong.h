/*******************************************************************************************************
 
 DkPong.h
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
#include <QMainWindow>
#include <QGraphicsView>
#include <QRect>
#include <QLabel>
#include <QSharedPointer>
#pragma warning(pop)		// no warnings from includes - end

#include "DkMath.h"
#pragma warning(disable: 4251)

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

class DkArduinoController;

class DllExport DkPongSettings {

public:
	DkPongSettings();

	void setField(const QRect& field);
	QRect field() const;

	void setUnit(int unit);
	int unit() const;

	void setBackgroundColor(const QColor& col);
	QColor backgroundColor() const;

	void setForegroundColor(const QColor& col);
	QColor foregroundColor() const;

	void setTotalScore(int maxScore);
	int totalScore() const;

	void writeSettings();

	void setPlayer1Name(const QString& name);
	QString player1Name() const;

	void setPlayer2Name(const QString& name);
	QString player2Name() const;

	float playerRatio() const;

protected:
	QRect mField;
	int mUnit = 10;
	int mTotalScore = 10;

	QColor mBgCol = QColor(0,0,0,100);
	QColor mFgCol = QColor(255,255,255);

	QString mPlayer1Name = QObject::tr("Player 1");
	QString mPlayer2Name = QObject::tr("Player 2");

	float mPlayerRatio = 0.15f;

	void loadSettings();
};

class DllExport DkPongPlayer : public QObject {
	Q_OBJECT

public:
	DkPongPlayer(const QString& playerName = QObject::tr("Anonymous"), QSharedPointer<DkPongSettings> settings = QSharedPointer<DkPongSettings>(new DkPongSettings()), QObject* parent = 0);

	void reset(const QPoint& pos);
	QRect rect() const;
	int pos() const;
	void setHeight(int newHeight);

	void move();
	void setSpeed(int speed);

	void updateSize();
	void increaseScore();

	void resetScore();
	int score() const;

	void setName(const QString& name);
	QString name() const;
	void setPos(int pos);

signals:
	void updatePaint() const;

protected:
	int mSpeed;
	int mVelocity;

	int mScore = 0;
	int mPos = INT_MAX;
	int mControllerPos = -1;

	QSharedPointer<DkPongSettings> mS;
	QRect mRect;

	QString mPlayerName;
};

class DllExport DkBall {

public:
	DkBall(QSharedPointer<DkPongSettings> settings = QSharedPointer<DkPongSettings>(new DkPongSettings()));

	void reset();
	void updateSize();

	QRect rect() const;
	QPoint direction() const;

	bool move(DkPongPlayer* player1, DkPongPlayer* player2);

protected:
	int mMinSpeed;
	int mMaxSpeed;

	DkVector mDirection;
	QRect mRect;

	QSharedPointer<DkPongSettings> mS;

	void fixAngle();
	void setDirection(const DkVector& dir);
};

class DllExport DkScoreLabel : public QLabel {
	Q_OBJECT

public:
	DkScoreLabel(Qt::Alignment align = Qt::AlignLeft, QWidget* parent = 0, QSharedPointer<DkPongSettings> settings = QSharedPointer<DkPongSettings>(new DkPongSettings()));

protected:
	void paintEvent(QPaintEvent* ev);
	QFont mFont;
	Qt::Alignment mAlign;

	QSharedPointer<DkPongSettings> mS;
};

class DllExport DkPongPort : public QGraphicsView {
	Q_OBJECT

public:
	DkPongPort(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkPongPort();

	QSharedPointer<DkPongSettings> settings() const;

	DkArduinoController* getController();
	DkPongPlayer* player1();
	DkPongPlayer* player2();

	void start();

	enum {
		player_1 = 0,
		player_2 = 3,
	};

public slots:
	void gameLoop();
	void countDown();
	void controllerUpdate(int controller, int val);
	
protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void resizeEvent(QResizeEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

	void initGame();
	void togglePause();
	void pauseGame(bool pause = true);

private:
	QTimer *mEventLoop;
	QTimer *mCountDownTimer;
	int mCountDownSecs = 3;

	int mPlayerSpeed;
	bool mPause = true;

	DkBall mBall;
	DkPongPlayer* mPlayer1 = 0;
	DkPongPlayer* mPlayer2 = 0;

	QSharedPointer<DkPongSettings> mS;
	void drawField(QPainter& p);

	DkScoreLabel* mP1Score;
	DkScoreLabel* mP2Score;

	DkScoreLabel* mLargeInfo;
	DkScoreLabel* mSmallInfo;

	DkArduinoController* mController = 0;

	void startCountDown(int sec = 3);
};

class DllExport DkPong : public QMainWindow {
	Q_OBJECT

public:
	DkPong(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	virtual ~DkPong() {};

	DkPongPort* viewport();

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent* event) override;
	
	DkPongPort* mViewport;
};


};
