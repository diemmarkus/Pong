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
#include <map>
#include <QSqlDatabase>
#include <QHBoxLayout>

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

class QSound;

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

	int player1Pin() const;
	int player2Pin() const;
	int speedPin() const;
	int pausePin() const;
	int player1SelectPin() const;
	int player2SelectPin() const;

	float playerRatio() const;

	void setSpeed(float speed);
	float speed() const;

	QString DBPath() const;

protected:
	QRect mField;
	int mUnit = 10;
	int mTotalScore = 10;

	int mPlayer1Pin = 2;
	int mPlayer2Pin = 4;
	int mSpeedPin = 1;
	int mPausePin = 7;
	int mPlayer1SelectPin = 3;
	int mPlayer2SelectPin = 4;
	float mSpeed = 30.0f;

	QColor mBgCol = QColor(0,0,0,100);
	QColor mFgCol = QColor(255,255,255);

	QString mPlayer1Name = QObject::tr("Player 1");
	QString mPlayer2Name = QObject::tr("Player 2");

	float mPlayerRatio = 0.15f;

	QString mDBName;

	void loadSettings();
};

class DllExport DkPongPlayer : public QObject {
	Q_OBJECT

public:
	DkPongPlayer(const QString& playerName = QObject::tr("Anonymous"), const QString& soundFile = ":/pong/audio/player1-collision.wav", QSharedPointer<DkPongSettings> settings = QSharedPointer<DkPongSettings>(new DkPongSettings()), QObject* parent = 0);

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

	void setPos(float pos);

	void sound() const;

	int velocity() const;

signals:
	void updatePaint() const;

protected:
	int mSpeed = 0;
	int mVelocity = 0;

	int mScore = 0;
	int mPos = INT_MAX;
	float mControllerPos = -1.0f;
	QSound* mSound = 0;

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

	void setSpeed(float val);
	float speed() const;

	void setAnalogueSpeed(float val);
	bool move(DkPongPlayer* player1, DkPongPlayer* player2);

protected:
	int mMinSpeed = 5;
	int mMaxSpeed = 50;
	float mSpeed = 3.0f;

	DkVector mDirection;
	QRect mRect;
	int mRally = 0;

	QSharedPointer<DkPongSettings> mS;

	void fixAngle(DkVector& dir) const;
	void fixDirection(DkVector& dir) const;
	void setDirection(const DkVector& dir);
	bool collision(const QRect& player, const DkVector& nextCenter) const;
	float changeDirPlayer(const DkPongPlayer* layer, DkVector& dir) const;
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

enum class Screen {
	Player1,
	Player2
};
struct Player {
	QPixmap picture;
	QPixmap pictureSelected;
	QString name;
};

class DkHighscores;

class DllExport DkPlayers : public QWidget {
	Q_OBJECT

public: 
	DkPlayers(DkHighscores* mHighscores, Qt::Alignment align);
	int selected() const;
	void setSelected(int idx);
	void create();
	static int selectedSize();
	static int size();

private:
	DkHighscores *mHighscores;
	int mSelected;
	Qt::Alignment mAlign;
	QHBoxLayout * mLayout;
	std::vector<QLabel*> mLabels;
	QScrollArea* mScrollArea = 0;
};

class DllExport DkHighscores : public QWidget {
	Q_OBJECT

public:
	DkHighscores(QWidget *parent = 0, QSharedPointer<DkPongSettings> settings = QSharedPointer<DkPongSettings>(new DkPongSettings()));
	//virtual ~DkHighscores();

	void loadDB(const QString& path);
	const std::vector<QSharedPointer<Player>>& players() const;
	
	/*!
		@brief Changes the current player in the specified screen
		@param screen - e.g. Player1, Player2
		@param player - floating point value in [0,1] to specify player
	*/
	void changePlayer(Screen screen, double player);
	
	/*!
		@brief Insert score into DB
		@param player1 - score for player1
		@param player2 - score for player2
	*/
	void commitScore(int player1, int player2);

	/*!
		@brief Returns the name of the selected player for the given screen
	*/
	QString playerName(Screen screen) const;

signals:
	/*!
		@brief Signal emitted when a new player is selected
	*/
	void playerChanged(Screen screen, const QString& name);
private:
	std::vector<QSharedPointer<Player>> mPlayers;
	QSqlDatabase mDB;
	DkPlayers* mLeft;
	DkPlayers* mRight;
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

public slots:
	void gameLoop();
	void countDown();
	void controllerUpdate(int controller, int val);
	void changeSpeed(int val);
	void playerChanged(Screen screen, const QString& player);

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
	float mLastSpeedValue = -1.0f;

	DkBall mBall;
	DkPongPlayer* mPlayer1 = 0;
	DkPongPlayer* mPlayer2 = 0;

	QSharedPointer<DkPongSettings> mS;
	void drawField(QPainter& p);

	DkScoreLabel* mP1Score;
	DkScoreLabel* mP2Score;

	DkScoreLabel* mLargeInfo;
	DkScoreLabel* mSmallInfo;

	DkHighscores* mHighscores;

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
