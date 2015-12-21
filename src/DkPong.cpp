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

	settings.setValue("speed", mSpeed);

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

float DkPongSettings::speed() const {
	return mSpeed;
}

float DkPongSettings::playerRatio() const {
	return mPlayerRatio;
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
	
	mSpeed = settings.value("speed", mSpeed).toFloat();

	int bgAlpha = settings.value("backgroundAlpha", mBgCol.alpha()).toInt();
	int fgAlpha = settings.value("foregroundAlpha", mFgCol.alpha()).toInt();

	mBgCol.setNamedColor(settings.value("backgroundColor", mBgCol.name()).toString());
	mFgCol.setNamedColor(settings.value("foregroundColor", mFgCol.name()).toString());

	mBgCol.setAlpha(bgAlpha);
	mFgCol.setAlpha(fgAlpha);

	settings.endGroup();
}

// DkPlayer --------------------------------------------------------------------
DkPongPlayer::DkPongPlayer(const QString& playerName, QSharedPointer<DkPongSettings> settings, QObject* parent) : QObject(parent) {

	mPlayerName = playerName;
	mS = settings;
	mSpeed = 0;
	mPos = INT_MAX;

	mRect = QRect(QPoint(), QSize(settings->unit(), 2*settings->unit()));
}

void DkPongPlayer::reset(const QPoint& pos) {

	// only reset if we don't have controller
	if (mControllerPos == -1)
		mRect.moveCenter(pos);
	else
		mRect.moveCenter(QPoint(pos.x(), mControllerPos));
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

void DkPongPlayer::move() {

	int oldTop = mRect.top();

	// arduino controlls
	if (mControllerPos != -1) {
		mRect.moveTop(qRound(mControllerPos/1023.0*(mS->field().height()-mRect.height())));
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

void DkPongPlayer::setPos(int pos) {

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
	mPlayer1 = new DkPongPlayer(mS->player1Name(), mS);
	mPlayer2 = new DkPongPlayer(mS->player2Name(), mS);

	connect(mPlayer1, SIGNAL(updatePaint()), this, SLOT(update()));
	connect(mPlayer2, SIGNAL(updatePaint()), this, SLOT(update()));

	mP1Score = new DkScoreLabel(Qt::AlignRight, this, mS);
	mP2Score = new DkScoreLabel(Qt::AlignLeft, this, mS);
	mLargeInfo = new DkScoreLabel(Qt::AlignHCenter | Qt::AlignBottom, this, mS);
	mSmallInfo = new DkScoreLabel(Qt::AlignHCenter, this, mS);
	 
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
		mP1Score->setText(mPlayer1->name());
		mP2Score->setText(mPlayer2->name());
	}
	else {
		mP1Score->setText(QString::number(mPlayer1->score()));
		mP2Score->setText(QString::number(mPlayer2->score()));
	}

	update();
}

void DkPongPort::start() {

	mP1Score->setText(mPlayer1->name());
	mP2Score->setText(mPlayer2->name());
	qDebug() << "player name" << mPlayer1->name();
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
	}

	mLargeInfo->setVisible(pause);
	mSmallInfo->setVisible(pause);
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

	if (controller == mS->player1Pin())
		mPlayer1->setPos(val);
	if (controller == mS->player2Pin())
		mPlayer2->setPos(val);
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

	mCountDownSecs = sec;
	pauseGame();
	mCountDownTimer->start();
	mLargeInfo->setText(QString::number(mCountDownSecs));
	mLargeInfo->show();
	mSmallInfo->hide();
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
		togglePause();
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
	mMaxSpeed = qRound(mS->field().width()*0.1);
	qDebug() << "maxSpeed: " << mMaxSpeed;

	mRect = QRect(QPoint(), QSize(mS->unit(), mS->unit()));

	//setDirection(DkVector(10, 10));

	reset();
}

void DkBall::reset() {
	
	//mDirection = DkVector(3, 0);// DkVector(mUnit*0.15f, mUnit*0.15f);
	mRect.moveCenter(QPoint(qRound(mS->field().width()*0.5f), qRound(mS->field().height()*0.5f)));
	mRally = 0;
	mSpeed = mS->speed();
}

void DkBall::updateSize() {
	mMinSpeed = qRound(mS->field().width()*0.005);
	mMaxSpeed = qRound(mS->field().width()*0.01);
	setDirection(DkVector((float)qrand()/RAND_MAX*10.0f-5.0f, (float)qrand()/RAND_MAX*5.0f-2.5f));
	//setDirection(DkVector(10,10));
}

QRect DkBall::rect() const {
	return mRect;
}

QPoint DkBall::direction() const {
	return mDirection.toQPointF().toPoint();
}

bool DkBall::move(DkPongPlayer* player1, DkPongPlayer* player2) {

	DkVector dir = mDirection;
	dir.normalize();
	dir *= (float)(mSpeed + qRound(mRally/10.0));
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
		mRally++;
		qDebug() << "rally speed: " << qRound(mRally/10.0);
	}
	else if (dir.x > 0 && collision(player2->rect(), nextCenter)) {
		mSpeed *= changeDirPlayer(player2, dir);
		nextCenter = DkVector(mRect.center()) + dir;
		
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
		newSpeed += 0.2f;
	else if (player->velocity()*dir.y < 0)
		newSpeed -= 0.2f;

	double nAngle = dir.angle() + DK_PI*0.5;
	double magic = (double)qrand() / RAND_MAX * 0.5 - 0.25;

	dir.rotate((nAngle*2)+magic);

	// change the angle if the ball becomes horizontal
	if (DkMath::distAngle(dir.angle(), 0.0) > 0.01)
		dir.rotate(0.7);

	fixDirection(dir);

	return newSpeed;
}

bool DkBall::collision(const QRect& player, const DkVector& nextCenter) const {

	if (player.intersects(mRect))
		return true;

	// first check if be cross the player line
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

}
