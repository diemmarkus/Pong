/*******************************************************************************************************
 
 DkPong.cpp
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

#include "DkPong.h"

#include "DkArduinoController.h"
#include "DkSettings.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QTimer>
#include <QDebug>
#include <QVector2D>
#include <QKeyEvent>
#include <QTime>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QSound>
#include <QSqlQuery>
#include <QSqlError>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <algorithm>
#include <cmath>
#pragma warning(pop)		// no warnings from includes - end

namespace pong {


// DkPongSettings --------------------------------------------------------------------
DkPongSettings::DkPongSettings() {
	loadSettings();
}

void DkPongSettings::setField(const QRect & field) {
	mField = field;
}

QRect DkPongSettings::field() const {
	return mField;
}

void DkPongSettings::setUnit(int unit) {
	mUnit = unit;
}

int DkPongSettings::unit() const {
	return mUnit;
}

void DkPongSettings::setBackgroundColor(const QColor & col) {
	mBgCol = col;
}

QColor DkPongSettings::backgroundColor() const {
	return mBgCol;
}

void DkPongSettings::setForegroundColor(const QColor & col) {
	mFgCol = col;
}

QColor DkPongSettings::foregroundColor() const {
	return mFgCol;
}

void DkPongSettings::setTotalScore(int maxScore) {
	mTotalScore = maxScore;
}

int DkPongSettings::totalScore() const {
	return mTotalScore;
}

void DkPongSettings::writeSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("DkPong");

	settings.setValue("field", mField);
	settings.setValue("unit", mUnit);
	settings.setValue("totalScore", mTotalScore);

	settings.setValue("backgroundColor", mBgCol.name());
	settings.setValue("foregroundColor", mFgCol.name());

	settings.setValue("backgroundAlpha", mBgCol.alpha());
	settings.setValue("foregroundAlpha", mFgCol.alpha());

	settings.setValue("player1Name", mPlayer1Name);
	settings.setValue("player2Name", mPlayer2Name);
	
	settings.setValue("playerRatio", qRound(mPlayerRatio*100.0f));

	settings.setValue("player1Pin", mPlayer1Pin);
	settings.setValue("player2Pin", mPlayer2Pin);
	settings.setValue("speedPin", mSpeedPin);
	settings.setValue("pausePin", mPausePin);

	settings.setValue("player1SelectPin", mPlayer1SelectPin);
	settings.setValue("player2SelectPin", mPlayer2SelectPin);

	settings.setValue("dbName", mDBName);
	//settings.setValue("speed", mSpeed);

	settings.endGroup();

	qDebug() << "settings written...";
}

void DkPongSettings::setPlayer1Name(const QString& name) {
	mPlayer1Name = name;
}

QString DkPongSettings::player1Name() const {
	return mPlayer1Name;
}

void DkPongSettings::setPlayer2Name(const QString& name) {
	mPlayer2Name = name;
}

QString DkPongSettings::player2Name() const {
	return mPlayer2Name;
}

int DkPongSettings::player1Pin() const {
	return mPlayer1Pin;
}

int DkPongSettings::player2Pin() const {
	return mPlayer2Pin;
}

int DkPongSettings::speedPin() const {
	return mSpeedPin;
}

int DkPongSettings::pausePin() const {
	return mPausePin;
}

int DkPongSettings::player1SelectPin() const {
	return mPlayer1SelectPin;
}

int DkPongSettings::player2SelectPin() const {
	return mPlayer2SelectPin;
}

void DkPongSettings::setSpeed(float speed) {
	mSpeed = speed;
}

float DkPongSettings::speed() const {
	return mSpeed;
}

float DkPongSettings::playerRatio() const {
	return mPlayerRatio;
}

QString DkPongSettings::DBPath() const {
	return mDBName;
}

void DkPongSettings::loadSettings() {

	QSettings& settings = Settings::instance().getSettings();
	settings.beginGroup("DkPong");

	mField = settings.value("field", mField).toRect();
	mUnit = settings.value("unit", mUnit).toInt();
	mTotalScore = settings.value("totalScore", mTotalScore).toInt();

	mPlayer1Name = settings.value("player1Name", mPlayer1Name).toString();
	mPlayer2Name = settings.value("player2Name", mPlayer2Name).toString();

	mPlayerRatio = settings.value("playerRatio", qRound(mPlayerRatio*100)).toInt()/100.0f;

	mPlayer1Pin = settings.value("player1Pin", mPlayer1Pin).toInt();
	mPlayer2Pin = settings.value("player2Pin", mPlayer2Pin).toInt();
	mSpeedPin = settings.value("speedPin", mSpeedPin).toInt();
	mPausePin = settings.value("pausePin", mPausePin).toInt();
	mPlayer1SelectPin = settings.value("player1SelectPin", mPlayer1SelectPin).toInt();
	mPlayer2SelectPin = settings.value("player2SelectPin", mPlayer2SelectPin).toInt();

	mDBName = settings.value("dbName", mDBName).toString();
	//mSpeed = settings.value("speed", mSpeed).toFloat();

	int bgAlpha = settings.value("backgroundAlpha", mBgCol.alpha()).toInt();
	int fgAlpha = settings.value("foregroundAlpha", mFgCol.alpha()).toInt();

	mBgCol.setNamedColor(settings.value("backgroundColor", mBgCol.name()).toString());
	mFgCol.setNamedColor(settings.value("foregroundColor", mFgCol.name()).toString());

	mBgCol.setAlpha(bgAlpha);
	mFgCol.setAlpha(fgAlpha);

	settings.endGroup();
}

// DkPlayer --------------------------------------------------------------------
DkPongPlayer::DkPongPlayer(const QString& playerName, const QString& soundFile, QSharedPointer<DkPongSettings> settings, QObject* parent) : QObject(parent) {

	mPlayerName = playerName;
	mS = settings;
	mSpeed = 0;
	mPos = INT_MAX;
	mRect = QRect(QPoint(), QSize(settings->unit(), 2*settings->unit()));

	// sound
	mSound = new QSound(soundFile, this);

}

void DkPongPlayer::reset(const QPoint& pos) {

	// only reset if we don't have controller
	if (mControllerPos == -1)
		mRect.moveCenter(pos);
	else
		mRect.moveCenter(QPoint(pos.x(), qRound(mControllerPos)));
}

int DkPongPlayer::pos() const {
	return mPos;
}

QRect DkPongPlayer::rect() const {
	return mRect;
}

void DkPongPlayer::setHeight(int newHeight) {
	mRect.setHeight(newHeight);
}

int DkPongPlayer::velocity() const {
	return mVelocity;
}

void DkPongPlayer::sound() const {

	if (mSound)
		mSound->play();

}

void DkPongPlayer::move() {

	int oldTop = mRect.top();

	// arduino controlls
	if (mControllerPos != -1) {
		mRect.moveTop(qRound((1-mControllerPos)*(mS->field().height()-mRect.height())));
		mVelocity = oldTop - mRect.top();
		return;
	}

	if (mRect.top() + mSpeed < 0)
		mRect.moveTop(0);
	else if (mRect.bottom() + mSpeed > mS->field().height())
		mRect.moveBottom(mS->field().height());
	else
		mRect.moveTop(mRect.top() + mSpeed);

	mVelocity = oldTop - mRect.top();
}

void DkPongPlayer::setSpeed(int speed) {
	
	mSpeed = speed;

	if (speed != 0)
		mPos = mRect.center().y();
	else
		mPos = INT_MAX;
}

void DkPongPlayer::setPos(float pos) {

	mControllerPos = pos;
	move();
	emit updatePaint();
}

void DkPongPlayer::updateSize() {
	mRect.setHeight(qRound(mS->field().height()*mS->playerRatio()));
}

void DkPongPlayer::increaseScore() {
	mScore++;
}

void DkPongPlayer::resetScore() {
	mScore = 0;
}

int DkPongPlayer::score() const {
	return mScore;
}

void DkPongPlayer::setName(const QString & name) {
	mPlayerName = name;
}

QString DkPongPlayer::name() const {
	return mPlayerName;
}

// DkScoreLabel --------------------------------------------------------------------
DkScoreLabel::DkScoreLabel(Qt::Alignment align, QWidget* parent, QSharedPointer<DkPongSettings> settings) : QLabel(parent) {
	
	mS = settings;
	mAlign = align;
	setStyleSheet("QLabel{ color: #fff;}");
	setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	
	mFont = QFont("terminal", 6);
	setFont(mFont);
	qDebug() << "using:" << mFont.family();
}

void DkScoreLabel::paintEvent(QPaintEvent* /*ev*/) {

	QFontMetrics m(mFont);
	
	QPixmap buffer(m.width(text())-1, m.height());
	buffer.fill(Qt::transparent);
	//buffer.fill(Qt::red);

	// draw font
	QPen fontPen(mS->foregroundColor());

	QPainter bp(&buffer);
	bp.setPen(fontPen);
	bp.setFont(mFont);
	bp.drawText(buffer.rect(), Qt::AlignHCenter | Qt::AlignVCenter, text());
	bp.end();

	QSize bSize(size());
	bSize.setHeight(qRound(bSize.height() - mS->unit()*0.5));
	buffer = buffer.scaled(bSize, Qt::KeepAspectRatio);

	QRect r(buffer.rect());

	if (mAlign & Qt::AlignRight)
		r.moveLeft(width() - (mS->unit() * 3 + buffer.width()));
	else if (mAlign & Qt::AlignHCenter)
		r.moveLeft(qRound((width() - buffer.width())*0.5f));
	else
		r.moveLeft(mS->unit() * 3);

	if (mAlign & Qt::AlignBottom)
		r.moveBottom(height());
	else
		r.moveTop(qRound((height()-buffer.height())/2.0f));	// default: center

	QPainter p(this);
	p.drawPixmap(r, buffer);

	//QLabel::paintEvent(ev);
}

// DkPongPort --------------------------------------------------------------------
DkPongPort::DkPongPort(QWidget *parent, Qt::WindowFlags) : QGraphicsView(parent) {

	setAttribute(Qt::WA_TranslucentBackground, true);

	mS = QSharedPointer<DkPongSettings>(new DkPongSettings());
	mPlayerSpeed = qRound(mS->field().width()*0.007);

	mBall = DkBall(mS);
	mPlayer1 = new DkPongPlayer(mS->player1Name(), ":/pong/audio/player1-collision.wav", mS);
	mPlayer2 = new DkPongPlayer(mS->player2Name(), ":/pong/audio/player2-collision.wav",  mS);

	mP1Score = new DkScoreLabel(Qt::AlignRight, this, mS);
	mP2Score = new DkScoreLabel(Qt::AlignLeft, this, mS);
	mLargeInfo = new DkScoreLabel(Qt::AlignHCenter | Qt::AlignBottom, this, mS);
	mSmallInfo = new DkScoreLabel(Qt::AlignHCenter, this, mS);
	 
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addStretch();

	//layout->setAlignment(Qt::AlignBottom);
	
	mHighscores = new DkHighscores(this, mS);
	layout->addWidget(mHighscores);
	connect(mHighscores, &DkHighscores::playerChanged, this, &DkPongPort::playerChanged);


	mEventLoop = new QTimer(this);
	mEventLoop->setInterval(10);
	//eventLoop->start();

	mCountDownTimer = new QTimer(this);
	mCountDownTimer->setInterval(500);

	mController = new DkArduinoController(this);
	connect(mController, SIGNAL(controllerSignal(int, int)), this, SLOT(controllerUpdate(int ,int)));

	connect(mEventLoop, SIGNAL(timeout()), this, SLOT(gameLoop()));
	connect(mCountDownTimer, SIGNAL(timeout()), this, SLOT(countDown()));

	initGame();
	pauseGame();

}

void DkPongPort::initGame() {
	
	mBall.reset();
	mPlayer1->reset(QPoint(mS->unit(), qRound(height()*0.5f)));
	mPlayer2->reset(QPoint(qRound(width()-mS->unit()*1.5f), qRound(height()*0.5f)));

	if (mPlayer1->score() == 0 && mPlayer2->score() == 0) {
		mP1Score->setText("0");
		mP2Score->setText("0");
	}
	else {
		mP1Score->setText(QString::number(mPlayer1->score()));
		mP2Score->setText(QString::number(mPlayer2->score()));
	}

	update();
}

void DkPongPort::start() {

	mP1Score->setText("0");
	mP2Score->setText("0");
	update();

	if (mController)
		mController->start();
}

void DkPongPort::togglePause() {

	pauseGame(mEventLoop->isActive());
}

void DkPongPort::pauseGame(bool pause) {

	if (pause) {
		mCountDownTimer->stop();
		mEventLoop->stop();
		mLargeInfo->setText(tr("PAUSED"));
		mSmallInfo->setText(tr("Press <SPACE> to start."));
		connect(mPlayer1, SIGNAL(updatePaint()), this, SLOT(update()), Qt::UniqueConnection);
		connect(mPlayer2, SIGNAL(updatePaint()), this, SLOT(update()), Qt::UniqueConnection);
	}
	else {

		mP1Score->setText(QString::number(mPlayer1->score()));
		mP2Score->setText(QString::number(mPlayer2->score()));

		if (mPlayer1->score() >= mS->totalScore() || mPlayer2->score() >= mS->totalScore()) {
			mPlayer1->resetScore();
			mPlayer2->resetScore();
			initGame();
		}

		mEventLoop->start();
		disconnect(mPlayer1, SIGNAL(updatePaint()), this, SLOT(update()));
		disconnect(mPlayer2, SIGNAL(updatePaint()), this, SLOT(update()));
	}

	mHighscores->setVisible(pause);
	mLargeInfo->setVisible(pause);
	mSmallInfo->setVisible(pause);
}

void DkPongPort::playerChanged(Screen screen, const QString& name)
{
	if (screen == Screen::Player1) {
		mPlayer1->setName(name);
	}
	else if (screen == Screen::Player2) {
		mPlayer2->setName(name);
	}
	
}
DkPongPort::~DkPongPort() {
}

DkArduinoController* DkPongPort::getController() {
	return mController;
}

DkPongPlayer * DkPongPort::player1() {
	return mPlayer1;
}

DkPongPlayer * DkPongPort::player2() {
	return mPlayer2;
}

QSharedPointer<DkPongSettings> DkPongPort::settings() const {
	return mS;
}

void DkPongPort::controllerUpdate(int controller, int val) {

	qDebug() << "pin" << controller << "value" << val;
	// convert value
	float minV = 0.0f;
	float maxV = 1023.0f;
	float v = (val - minV) / (maxV - minV);

	if (controller == mS->player1Pin())
		mPlayer1->setPos(v);
	else if (controller == mS->player2Pin())
		mPlayer2->setPos(v);
	else if (controller == mS->speedPin()) {
		mBall.setAnalogueSpeed(v);
		if (v < 0.1f) {
			pauseGame();
			qDebug() << "v:" << v << "v raw: " << val;
		}
		else if (v > 0.1f && !mEventLoop->isActive()) {
			qDebug() << "paused: " << !mEventLoop->isActive() << "v:" << v;
			startCountDown();
			mLastSpeedValue = v;	// update last value
		}
	} 
	else if (controller == mS->player1SelectPin() && !mEventLoop->isActive()) {
		mHighscores->changePlayer(Screen::Player1, v);
	}
	else if (controller == mS->player2SelectPin() && !mEventLoop->isActive()) {
		mHighscores->changePlayer(Screen::Player2, v);
	}
}

void DkPongPort::changeSpeed(int val) {
	mBall.setSpeed(val + mBall.speed());
}

void DkPongPort::countDown() {
	
	mCountDownSecs--;

	if (mCountDownSecs == 0) {
		mCountDownTimer->stop();
		pauseGame(false);	// start
	}
	else
		mLargeInfo->setText(QString::number(mCountDownSecs));
}

void DkPongPort::paintEvent(QPaintEvent* event) {

	// propagate
	QGraphicsView::paintEvent(event);

	QPainter p(viewport());
	p.setBackgroundMode(Qt::TransparentMode);

	p.fillRect(QRect(QPoint(), size()), mS->backgroundColor());
	drawField(p);

	p.fillRect(mBall.rect(), mS->foregroundColor());
	p.fillRect(mPlayer1->rect(), mS->foregroundColor());
	p.fillRect(mPlayer2->rect(), mS->foregroundColor());

	// clear area under text
	if (mLargeInfo->isVisible()) {
		p.fillRect(mLargeInfo->geometry(), mS->foregroundColor());
		p.setCompositionMode(QPainter::CompositionMode_SourceIn);
		p.fillRect(mLargeInfo->geometry(), mS->backgroundColor());
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	}

	// clear area under small text
	if (mSmallInfo->isVisible()) {
		p.fillRect(mSmallInfo->geometry(), mS->foregroundColor());
		p.setCompositionMode(QPainter::CompositionMode_SourceIn);
		p.fillRect(mSmallInfo->geometry(), mS->backgroundColor());
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	}

	p.end();
}

void DkPongPort::drawField(QPainter& p) {

	QPen cPen = p.pen();
	
	// set dash pattern
	QVector<qreal> dashes;
	dashes << 0.1 << 3;

	// create style
	QPen linePen;
	linePen.setColor(mS->foregroundColor());
	linePen.setWidth(qRound(mS->unit()*0.5));
	linePen.setDashPattern(dashes);
	p.setPen(linePen);

	// set line
	QLine line(QPoint(qRound(width()*0.5f), 0), QPoint(qRound(width()*0.5f),height()));
	p.drawLine(line);

	p.setPen(cPen);
}

void DkPongPort::startCountDown(int sec) {

	if (mCountDownTimer->isActive())
		return;

	
	mCountDownSecs = sec;
	pauseGame();
	mCountDownTimer->start();
	mLargeInfo->setText(QString::number(mCountDownSecs));
	mLargeInfo->show();
	mSmallInfo->hide();
	mHighscores->hide();
}

void DkPongPort::resizeEvent(QResizeEvent *event) {

	//resize(event->size());

	mS->setField(QRect(QPoint(), event->size()));
	mPlayerSpeed = qRound(mS->field().width()*0.007);
	mPlayer1->updateSize();
	mPlayer2->updateSize();
	mBall.updateSize();

	initGame();

	// resize player scores
	QRect sR(QPoint(0, mS->unit()*3), QSize(qRound(width()*0.5), qRound(height()*0.15)));
	QRect sR1 = sR;
	QRect sR2 = sR;
	sR2.moveLeft(qRound(width()*0.5));
	mP1Score->setGeometry(sR1);
	mP2Score->setGeometry(sR2);
	
	// resize info labels
	QRect lIR(QPoint(qRound(width()*0.15),0), QSize(qRound(width()*0.7), qRound(height()*0.15)));
	lIR.moveBottom(qRound(height()*0.5 + mS->unit()));
	mLargeInfo->setGeometry(lIR);
	
	QRect sIR(QPoint(qRound(width()*0.15),0), QSize(qRound(width()*0.7), qRound(height()*0.08)));
	sIR.moveTop(qRound(height()*0.5 + mS->unit()*2));
	mSmallInfo->setGeometry(sIR);

	QWidget::resizeEvent(event);
	
}

void DkPongPort::gameLoop() {

	// logic first
	if (!mBall.move(mPlayer1, mPlayer2)) {

		initGame();

		// check if somebody won
		if (mPlayer1->score() >= mS->totalScore() || mPlayer2->score() >= mS->totalScore()) {
			pauseGame();
			mLargeInfo->setText(tr("%1 won!").arg(mPlayer1->score() > mPlayer2->score() ? mPlayer1->name() : mPlayer2->name()));
			mSmallInfo->setText(tr("Hit <SPACE> to start a new Game"));
			mHighscores->commitScore(mPlayer1->score(), mPlayer2->score());
		}
		else
			startCountDown();

		return;
	}

	mPlayer1->move();
	mPlayer2->move();

	//repaint();
	viewport()->update();
	
	//QGraphicsView::update();
}

void DkPongPort::keyPressEvent(QKeyEvent *event) {

	if (event->key() == Qt::Key_Up && !event->isAutoRepeat()) {
		mPlayer2->setSpeed(-mPlayerSpeed);
	}
	if (event->key() == Qt::Key_Down && !event->isAutoRepeat()) {
		mPlayer2->setSpeed(mPlayerSpeed);
	}
	if (event->key() == Qt::Key_W && !event->isAutoRepeat()) {
		mPlayer1->setSpeed(-mPlayerSpeed);
	}
	if (event->key() == Qt::Key_S && !event->isAutoRepeat()) {
		mPlayer1->setSpeed(mPlayerSpeed);
	}
	if (event->key() == Qt::Key_Space) {
		startCountDown();
	}
	if (event->key() == Qt::Key_Plus) {
		changeSpeed(1);
	}
	if (event->key() == Qt::Key_Minus) {
		changeSpeed(-1);
	}
	if (event->key() == Qt::Key_Left) {
		mHighscores->changePlayer(Screen::Player2, 0.2);
	}
	if (event->key() == Qt::Key_Right) {
		mHighscores->changePlayer(Screen::Player2, 0.4);
	}
	if (event->key() == Qt::Key_A) {
		mHighscores->changePlayer(Screen::Player1, 0.6);
	}
	if (event->key() == Qt::Key_D) {
		mHighscores->changePlayer(Screen::Player1, 0.8);
	}

	QWidget::keyPressEvent(event);
}

void DkPongPort::keyReleaseEvent(QKeyEvent* event) {

	if ((event->key() == Qt::Key_Up && !event->isAutoRepeat()) || (event->key() == Qt::Key_Down && !event->isAutoRepeat())) {
		mPlayer2->setSpeed(0);
	}
	if ((event->key() == Qt::Key_W && !event->isAutoRepeat()) || (event->key() == Qt::Key_S && !event->isAutoRepeat())) {
		mPlayer1->setSpeed(0);
	}

	QWidget::keyReleaseEvent(event);
}

// DkBall --------------------------------------------------------------------
DkBall::DkBall(QSharedPointer<DkPongSettings> settings) {

	qsrand(QTime::currentTime().msec());
	mS = settings;
	
	mMinSpeed = qRound(mS->field().width()*0.005);
	mMaxSpeed = qRound(mS->field().width()*0.01);
	qDebug() << "maxSpeed: " << mMaxSpeed;

	mRect = QRect(QPoint(), QSize(mS->unit(), mS->unit()));

	//setDirection(DkVector(10, 10));

	reset();
}

void DkBall::reset() {
	
	qDebug() << "speed: " << mS->speed();

	//mDirection = DkVector(3, 0);// DkVector(mUnit*0.15f, mUnit*0.15f);
	mRect.moveCenter(QPoint(qRound(mS->field().width()*0.5f), qRound(mS->field().height()*0.5f)));
	mRally = 0;
	setSpeed(mS->speed());
}

void DkBall::updateSize() {
	mMinSpeed = qRound(mS->field().width()*0.005);
	mMaxSpeed = qRound(mS->field().width()*0.02);
	setDirection(DkVector((float)qrand()/RAND_MAX*10.0f-5.0f, (float)qrand()/RAND_MAX*5.0f-2.5f));
	//setDirection(DkVector(10,10));
}

QRect DkBall::rect() const {
	return mRect;
}

QPoint DkBall::direction() const {
	return mDirection.toQPointF().toPoint();
}

void DkBall::setSpeed(float val) {
	mSpeed = val;
	mS->setSpeed(mSpeed);	// update settings speed

	if (mSpeed < mMinSpeed)
		mSpeed = (float)mMinSpeed;
	if (mSpeed > mMaxSpeed)
		mSpeed = (float)mMaxSpeed;

	qDebug() << "speed" << mSpeed;
}

float DkBall::speed() const {
	return mSpeed;
}

void DkBall::setAnalogueSpeed(float val) {

	setSpeed(val * (mMaxSpeed - mMinSpeed) + mMinSpeed);
}

bool DkBall::move(DkPongPlayer* player1, DkPongPlayer* player2) {

	// check minimum speed 
	if (mSpeed < mMinSpeed)
		mSpeed = (float)mMinSpeed;

	DkVector dir = mDirection;
	dir.normalize();
	dir *= mSpeed;//	 (float)(mSpeed + qRound(mRally / 10.0));
	fixDirection(dir);

	// collision detection top & bottom
	if (mRect.top() <= mS->field().top() && dir.y < 0 || mRect.bottom() >= mS->field().bottom() && dir.y > 0) {
		dir.rotate(dir.angle()*2);
		//qDebug() << "collision...";
	}

	DkVector nextCenter = mRect.center() + dir.toQPointF().toPoint();

	// player collision
	if (dir.x < 0 && collision(player1->rect(), nextCenter)) {
		mSpeed *= changeDirPlayer(player1, dir);
		nextCenter = DkVector(mRect.center()) + dir;
		player1->sound();
		mRally++;
		qDebug() << "rally speed: " << qRound(mRally/10.0);
	}
	else if (dir.x > 0 && collision(player2->rect(), nextCenter)) {
		mSpeed *= changeDirPlayer(player2, dir);
		nextCenter = DkVector(mRect.center()) + dir;
		player2->sound();
		mRally++;
		qDebug() << "rally speed: " << qRound(mRally/10.0);
	}
	// collision detection left & right
	else if (mRect.left() <= mS->field().left()) {
		dir = QPointF(player2->rect().center())-mS->field().center();
		dir.normalize();
		dir *= (float)mMinSpeed;
		setDirection(dir);
		player2->increaseScore();
		return false;
	}
	else if (mRect.right() >= mS->field().right()) {
		dir = QPointF(player1->rect().center())-mS->field().center();
		dir.normalize();
		dir *= (float)mMinSpeed;
		setDirection(dir);
		player1->increaseScore();
		return false;
	}

	//qDebug() << ballDir.angle();

	setDirection(dir);
	mRect.moveCenter(nextCenter.toQPointF().toPoint());
	
	return true;
}

float DkBall::changeDirPlayer(const DkPongPlayer* player, DkVector& dir) const {

	float newSpeed = 1.0f;

	// if the player moves in the ball direction speed it up
	if (player->velocity()*dir.y > 0)
		newSpeed -= 0.2f;
	else if (player->velocity()*dir.y < 0)
		newSpeed += 0.2f;

	if (newSpeed != 1)
		qDebug() << "speed changed: " << newSpeed;

	double nAngle = dir.angle() + DK_PI*0.5;
	double magic = (double)qrand() / RAND_MAX * 0.5 - 0.25;

	dir.rotate((nAngle * 2)+magic);

	// change the angle if the ball becomes horizontal
	if (DkMath::distAngle(DkMath::normAngleRad(dir.angle(), 0.0, DK_PI), 0.0) > 0.01)
		dir.rotate(0.6);
	//	qDebug() << "0 angle: " << dir.angle();
	//else
	//	qDebug() << "angle: " << dir.angle();
		//dir.rotate(0.7);

	fixDirection(dir);

	return newSpeed;
}

bool DkBall::collision(const QRect& player, const DkVector& nextCenter) const {

	if (player.intersects(mRect))
		return true;

	// first check if we cross the player line
	float pc = (float)player.center().x();
	float cx = (float)mRect.center().x();

	if ((cx - pc)  * (nextCenter.x - pc) > 0)
		return false;
	
	if (qMin((float)mRect.center().y(), nextCenter.y) < player.top() || 
		qMax((float)mRect.center().y(), nextCenter.y) > player.bottom())
		return false;

	return true;
}

void DkBall::setDirection(const DkVector& dir) {

	mDirection = dir;
	fixDirection(mDirection);
}

void DkBall::fixDirection(DkVector& dir) const {
	
	// check angle
	fixAngle(dir);

	if (dir.norm() > mMaxSpeed) {
		dir.normalize();
		dir *= (float)mMaxSpeed;
	}
	else if (mDirection.norm() < mMinSpeed) {
		dir.normalize();
		dir *= (float)mMinSpeed;
	}
}

void DkBall::fixAngle(DkVector& dir) const {

	double angle = dir.angle();
	double range = DK_PI / 5.0;
	double sign = angle > 0 ? 1.0 : -1.0;
	angle = abs(angle);
	double newAngle = 0.0;

	if (angle < DK_PI*0.5 && angle > DK_PI*0.5 - range) {
		newAngle = DK_PI*0.5 - range;
	}
	else if (angle > DK_PI*0.5 && angle < DK_PI*0.5 + range) {
		newAngle = DK_PI*0.5 + range;
	}

	if (newAngle != 0.0) {
		dir.rotate(mDirection.angle() - (newAngle*sign));
		//qDebug() << "angle: " << angle << " new angle: " << newAngle;
	}
}

// DkBall --------------------------------------------------------------------
DkPong::DkPong(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {

	setStyleSheet("QWidget{background-color: rgba(0,0,0,0); border: none;}");
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);

	mViewport = new DkPongPort(this);

	QRect screenRect = QApplication::desktop()->screenGeometry();
	QRect winRect = screenRect;
	
	if (mViewport->settings()->field() == QRect())
		winRect.setSize(screenRect.size()*0.5);
	else
		winRect = mViewport->settings()->field();

	winRect.moveCenter(screenRect.center());
	setGeometry(winRect);
	
	setCentralWidget(mViewport);
	show();
}

DkPongPort * DkPong::viewport() {
	return mViewport;
}

void DkPong::keyPressEvent(QKeyEvent *event) {

	if (event->key() == Qt::Key_Escape)
		close();
}

void DkPong::closeEvent(QCloseEvent * event) {

	mViewport->settings()->writeSettings();

	QMainWindow::closeEvent(event);
}

// DkHighscores

DkPlayers::DkPlayers(DkHighscores* highscores, Qt::Alignment align)
	:	QWidget(highscores), 
		mHighscores(highscores), 
		mSelected(0), 
		mAlign(align)
{
	//setLayout(mLayout);
}

void DkPlayers::create()
{

	QWidget* dummy = new QWidget(this);
	mLayout = new QHBoxLayout(dummy);
	mLayout->setAlignment(mAlign);
	mLayout->setMargin(0);
	
	auto addPlayer = [this](QSharedPointer<Player> player) {
		QLabel* label = new QLabel(this);
		mLayout->addWidget(label);
		mLabels.push_back(label);
	};

	std::for_each(mHighscores->players().begin(), mHighscores->players().end(), addPlayer);
	setSelected(mSelected);

	mScrollArea = new QScrollArea(this);
	mScrollArea->setAlignment(mAlign);
	mScrollArea->setObjectName("playerSelection");
	mScrollArea->setFixedHeight(selectedSize() + 20);
	mScrollArea->setWidget(dummy);
	mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QHBoxLayout* l = new QHBoxLayout(this);
	l->setAlignment(mAlign);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(mScrollArea);

}

int DkPlayers::selectedSize() {
	return 142;
}

int DkPlayers::size() {
	return 100;
}

void DkPlayers::setSelected(int idx)
{
	for (size_t i = 0; i < mLabels.size(); ++i) {
		if (idx == i) {
			mLabels[i]->setPixmap(mHighscores->players()[i]->pictureSelected);
			
			if (mScrollArea)
				mScrollArea->ensureWidgetVisible(mLabels[i]);
		}
		else {
			mLabels[i]->setPixmap(mHighscores->players()[i]->picture);
		}
	}

	mSelected = idx;
	update();
}

int DkPlayers::selected() const
{
	return mSelected;
}

DkHighscores::DkHighscores(QWidget *parent, QSharedPointer<DkPongSettings> settings) 
	:	QWidget(parent),
		mLeft(new DkPlayers(this, Qt::AlignLeft)),
		mRight(new DkPlayers(this, Qt::AlignRight))
{
	QGridLayout* grid = new QGridLayout(this);
	grid->setContentsMargins(0, 0, 0, 0);
	grid->addWidget(mLeft, 0, 0);
	grid->addWidget(mRight, 0, 1);

	setLayout(grid);

	loadDB(settings->DBPath());

	mLeft->create();
	mRight->create();
};

const std::vector<QSharedPointer<Player>>& DkHighscores::players() const
{
	return mPlayers;
}

void DkHighscores::changePlayer(Screen screen, double player)
{
	double cnt = (double)players().size();
	int idx = (int)floor(player*cnt);

	qDebug() << "new player index: " << idx << "normed:" << player;

	if (idx < 0 || idx >= players().size()) {
		qDebug() << "player selection out of bounds";
		return;
	}

	
	switch (screen) {
	case Screen::Player1: mLeft->setSelected(idx); break;
	case Screen::Player2: mRight->setSelected(idx); break;
	}

	emit playerChanged(screen, playerName(screen));
}

QString DkHighscores::playerName(Screen screen) const
{
	switch (screen) {
	case Screen::Player1: return mPlayers[mLeft->selected()]->name; break;
	case Screen::Player2: return mPlayers[mRight->selected()]->name; break;
	}

	return "";
}

void DkHighscores::loadDB(const QString& name)
{
	
	QFileInfo dbInfo(QApplication::applicationDirPath(), name);

	if (!dbInfo.exists()) {
		qDebug() << "Could not load database...";
		return;
	}
	
	mDB = QSqlDatabase::addDatabase("QSQLITE");
	mDB.setDatabaseName(dbInfo.absoluteFilePath());
	mDB.open();
	
	if (!mDB.isOpen()) {
		qDebug() << "Error opening sqlite database:\n" << mDB.lastError();
		return;
	}

	qDebug() << dbInfo.absoluteFilePath() << "is opened...";
	
	QSqlQuery query = QSqlQuery(mDB);
	// Get image data back from database
	if (!query.exec("SELECT name, picture from players"))
		qDebug() << "Error getting image from table:\n" << query.lastError();


	while (query.next()) {
		QString playerName = query.value(0).toString();
		QByteArray outByteArray = query.value(1).toByteArray();
		
		QPixmap picture;
		picture.loadFromData(outByteArray);
		picture = picture.scaledToWidth(DkPlayers::size());

		QPixmap selected;
		selected.loadFromData(outByteArray);
		selected = selected.scaledToWidth(DkPlayers::selectedSize());

		QSharedPointer<Player> player(new Player());
		player->name = playerName;
		player->picture = picture;
		player->pictureSelected = selected;
		mPlayers.push_back(player);
	}
}

void DkHighscores::commitScore(int player1, int player2)
{

	if (!mDB.isOpen()) {
		qDebug() << "database could not be found...";
		return;
	}

	QString winner; 
	QString looser;

	winner = mPlayers[mLeft->selected()]->name;
	looser = mPlayers[mRight->selected()]->name;

	if (player2 > player1) {
		swap(winner, looser);
	}
	QSqlQuery query(mDB);
	query.prepare("Insert into scores(winner_name, looser_name, winner_points, looser_points) "
				  " VALUES (:winner, :looser, :winner_score, :looser_score)"); 
	query.bindValue(":winner", winner);
	query.bindValue(":looser", looser);
	query.bindValue(":winner_score", std::max(player1, player2));
	query.bindValue(":looser_score", std::min(player1, player2));

	if (!query.exec()) {
		qDebug() << "Error inserting score in table:\n" << query.lastError();
	}
}
}

