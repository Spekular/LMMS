/*
 * TimeLineWidget.cpp - class timeLine, representing a time-line with position marker
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */


#include <QDomElement>
#include <QTimer>
#include <QApplication>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QToolBar>
#include <QMenu>


#include "TimeLineWidget.h"
#include "embed.h"
#include "NStateButton.h"
#include "GuiApplication.h"
#include "TextFloat.h"
#include "SongEditor.h"


const int DRAG_ACTION_THRESHOLD = 5;

QPixmap * TimeLineWidget::s_posMarkerPixmap = NULL;

TimeLineWidget::TimeLineWidget( const int xoff, const int yoff, const float ppb,
			Song::PlayPos & pos, const TimePos & begin, Song::PlayModes mode,
							QWidget * parent ) :
	QWidget( parent ),
	m_inactiveLoopColor( 52, 63, 53, 64 ),
	m_inactiveLoopBrush( QColor( 255, 255, 255, 32 ) ),
	m_inactiveLoopInnerColor( 255, 255, 255, 32 ),
	m_activeLoopColor( 52, 63, 53, 255 ),
	m_activeLoopBrush( QColor( 55, 141, 89 ) ),
	m_activeLoopInnerColor( 74, 155, 100, 255 ),
	m_loopRectangleVerticalPadding( 1 ),
	m_barLineColor( 192, 192, 192 ),
	m_barNumberColor( m_barLineColor.darker( 120 ) ),
	m_autoScroll( AutoScrollEnabled ),
	m_loopPoints( LoopPointsDisabled ),
	m_behaviourAtStop( BackToZero ),
	m_changedPosition( true ),
	m_xOffset( xoff ),
	m_posMarkerX( 0 ),
	m_ppb( ppb ),
	m_snapSize( 1.0 ),
	m_pos( pos ),
	m_begin( begin ),
	m_mode( mode ),
	m_savedPos( -1 ),
	m_hint( NULL ),
	m_action( NoAction )
{
	m_loopPos[0] = 0;
	m_loopPos[1] = DefaultTicksPerBar;

	if( s_posMarkerPixmap == NULL )
	{
		s_posMarkerPixmap = new QPixmap( embed::getIconPixmap(
							"playpos_marker" ) );
	}

	setAttribute( Qt::WA_OpaquePaintEvent, true );
	move( 0, yoff );

	m_xOffset -= s_posMarkerPixmap->width() / 2;

	setMouseTracking(true);
	m_pos.m_timeLine = this;

	QTimer * updateTimer = new QTimer( this );
	connect( updateTimer, SIGNAL( timeout() ),
					this, SLOT( updatePosition() ) );
	updateTimer->start( 1000 / 60 );  // 60 fps
	connect( Engine::getSong(), SIGNAL( timeSignatureChanged( int,int ) ),
					this, SLOT( update() ) );

	// Required to prevent modified right click from triggering the context menu
	// Instead, manually trigger the context menu on unmodified right clicks
	setContextMenuPolicy(Qt::PreventContextMenu);
}




TimeLineWidget::~TimeLineWidget()
{
	if( gui->songEditor() )
	{
		m_pos.m_timeLine = NULL;
	}
	delete m_hint;
}



void TimeLineWidget::setXOffset(const int x)
{
	m_xOffset = x;
	if (s_posMarkerPixmap != nullptr) { m_xOffset -= s_posMarkerPixmap->width() / 2; }
}




void TimeLineWidget::addToolButtons( QToolBar * _tool_bar )
{
	NStateButton * autoScroll = new NStateButton( _tool_bar );
	autoScroll->setGeneralToolTip( tr( "Auto scrolling" ) );
	autoScroll->addState( embed::getIconPixmap( "autoscroll_on" ) );
	autoScroll->addState( embed::getIconPixmap( "autoscroll_off" ) );
	connect( autoScroll, SIGNAL( changedState( int ) ), this,
					SLOT( toggleAutoScroll( int ) ) );

	NStateButton * loopPoints = new NStateButton( _tool_bar );
	loopPoints->setGeneralToolTip( tr( "Loop points" ) );
	loopPoints->addState( embed::getIconPixmap( "loop_points_off" ) );
	loopPoints->addState( embed::getIconPixmap( "loop_points_on" ) );
	connect( loopPoints, SIGNAL( changedState( int ) ), this,
					SLOT( toggleLoopPoints( int ) ) );
	connect( this, SIGNAL( loopPointStateLoaded( int ) ), loopPoints,
					SLOT( changeState( int ) ) );

	NStateButton * behaviourAtStop = new NStateButton( _tool_bar );
	behaviourAtStop->addState( embed::getIconPixmap( "back_to_zero" ),
					tr( "After stopping go back to beginning" )
									);
	behaviourAtStop->addState( embed::getIconPixmap( "back_to_start" ),
					tr( "After stopping go back to "
						"position at which playing was "
						"started" ) );
	behaviourAtStop->addState( embed::getIconPixmap( "keep_stop_position" ),
					tr( "After stopping keep position" ) );
	connect( behaviourAtStop, SIGNAL( changedState( int ) ), this,
					SLOT( toggleBehaviourAtStop( int ) ) );
	connect( this, SIGNAL( loadBehaviourAtStop( int ) ), behaviourAtStop,
					SLOT( changeState( int ) ) );
	behaviourAtStop->changeState( BackToStart );

	_tool_bar->addWidget( autoScroll );
	_tool_bar->addWidget( loopPoints );
	_tool_bar->addWidget( behaviourAtStop );
}




void TimeLineWidget::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	_this.setAttribute( "lp0pos", (int) loopBegin() );
	_this.setAttribute( "lp1pos", (int) loopEnd() );
	_this.setAttribute( "lpstate", m_loopPoints );
	_this.setAttribute( "stopbehaviour", m_behaviourAtStop );
}




void TimeLineWidget::loadSettings( const QDomElement & _this )
{
	m_loopPos[0] = _this.attribute( "lp0pos" ).toInt();
	m_loopPos[1] = _this.attribute( "lp1pos" ).toInt();
	m_loopPoints = static_cast<LoopPointStates>(
					_this.attribute( "lpstate" ).toInt() );
	update();
	emit loopPointStateLoaded( m_loopPoints );

	if( _this.hasAttribute( "stopbehaviour" ) )
	{
		emit loadBehaviourAtStop( _this.attribute( "stopbehaviour" ).toInt() );
	}
}




void TimeLineWidget::updatePosition( const TimePos & )
{
	const int new_x = markerX( m_pos );

	if( new_x != m_posMarkerX )
	{
		m_posMarkerX = new_x;
		m_changedPosition = true;
		emit positionChanged( m_pos );
		update();
	}
}




void TimeLineWidget::toggleAutoScroll( int _n )
{
	m_autoScroll = static_cast<AutoScrollStates>( _n );
}




void TimeLineWidget::toggleLoopPoints( int _n )
{
	m_loopPoints = static_cast<LoopPointStates>( _n );
	update();
}




void TimeLineWidget::toggleBehaviourAtStop( int _n )
{
	m_behaviourAtStop = static_cast<BehaviourAtStopStates>( _n );
}




void TimeLineWidget::paintEvent( QPaintEvent * )
{
	QPainter p( this );

	// Draw background
	p.fillRect( 0, 0, width(), height(), p.background() );

	// Clip so that we only draw everything starting from the offset
	p.setClipRect( m_xOffset, 0, width() - m_xOffset, height() );

	// Draw the loop rectangle
	int const & loopRectMargin = getLoopRectangleVerticalPadding();
	int const loopRectHeight = this->height() - 2 * loopRectMargin;
	int const loopStart = markerX( loopBegin() ) + 8;
	int const loopEndR = markerX( loopEnd() ) + 9;
	int const loopRectWidth = loopEndR - loopStart;

	bool const loopPointsActive = loopPointsEnabled();

	// Draw the main rectangle (inner fill only)
	QRect outerRectangle( loopStart, loopRectMargin, loopRectWidth - 1, loopRectHeight - 1 );
	p.fillRect( outerRectangle, loopPointsActive ? getActiveLoopBrush() : getInactiveLoopBrush());

	// Draw the bar lines and numbers
	// Activate hinting on the font
	QFont font = p.font();
	font.setHintingPreference( QFont::PreferFullHinting );
	p.setFont(font);
	int const fontAscent = p.fontMetrics().ascent();
	int const fontHeight = p.fontMetrics().height();

	QColor const & barLineColor = getBarLineColor();
	QColor const & barNumberColor = getBarNumberColor();

	bar_t barNumber = m_begin.getBar();
	int const x = m_xOffset + s_posMarkerPixmap->width() / 2 -
			( ( static_cast<int>( m_begin * m_ppb ) / TimePos::ticksPerBar() ) % static_cast<int>( m_ppb ) );

	for( int i = 0; x + i * m_ppb < width(); ++i )
	{
		++barNumber;
		if( ( barNumber - 1 ) %
			qMax( 1, qRound( 1.0f / 3.0f *
				TimePos::ticksPerBar() / m_ppb ) ) == 0 )
		{
			const int cx = x + qRound( i * m_ppb );
			p.setPen( barLineColor );
			p.drawLine( cx, 5, cx, height() - 6 );

			const QString s = QString::number( barNumber );
			p.setPen( barNumberColor );
			p.drawText( cx + 5, ((height() - fontHeight) / 2) + fontAscent, s );
		}
	}

	// Draw the main rectangle (outer border)
	p.setPen( loopPointsActive ? getActiveLoopColor() : getInactiveLoopColor() );
	p.setBrush( Qt::NoBrush );
	p.drawRect( outerRectangle );

	// Draw the inner border outline (no fill)
	QRect innerRectangle = outerRectangle.adjusted( 1, 1, -1, -1 );
	p.setPen( loopPointsActive ? getActiveLoopInnerColor() : getInactiveLoopInnerColor() );
	p.setBrush( Qt::NoBrush );
	p.drawRect( innerRectangle );

	// Draw the position marker
	p.setOpacity( 0.6 );
	p.drawPixmap( m_posMarkerX, height() - s_posMarkerPixmap->height(), *s_posMarkerPixmap );
}




void TimeLineWidget::contextMenuEvent(QContextMenuEvent*)
{
	QMenu contextMenu(tr("Timeline"), this);
	QAction setStartPoint(tr("Loop start (Shift + left click)"), this);
	connect(&setStartPoint, &QAction::triggered, this, [this](){
		setLoopPoint(0, m_moveStartX);
	});
	QAction setEndPoint(tr("Loop end (Shift + right click)"), this);
	connect(&setEndPoint, &QAction::triggered, this, [this](){
		setLoopPoint(1, m_moveStartX);
	});
	QAction selectLoopPoints(tr("Select between loop points"), this);
	connect(&selectLoopPoints, &QAction::triggered, this, [this](){
		emit regionSelectedFromPixels(
			m_xOffset + m_loopPos[0] * m_ppb / TimePos::ticksPerBar(),
			m_xOffset + m_loopPos[1] * m_ppb / TimePos::ticksPerBar()
		);
		emit selectionFinished();
	});
	contextMenu.addActions({&setStartPoint, &setEndPoint, &selectLoopPoints});
	contextMenu.exec(QCursor::pos());
}




void TimeLineWidget::mousePressEvent( QMouseEvent* event )
{
	if (m_action != NoAction || event->x() < m_xOffset) { return; }

	m_moveStartX = event->x();
	m_oldLoopPos[0] = m_loopPos[0];
	m_oldLoopPos[1] = m_loopPos[1];

	if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) { m_action = MovePositionMarker; }
	else { chooseMouseAction(event); }

	mouseMoveEvent(event);
}




void TimeLineWidget::mouseMoveEvent( QMouseEvent* event )
{
	parentWidget()->update(); // essential for widgets that this timeline had taken their mouse move event from.
	// Fine adjust when ctrl is held, hide ctrl hint
	bool unquantized = event->modifiers() & Qt::ControlModifier;
	if (unquantized)
	{
		delete m_hint;
		m_hint = nullptr;
	}

	// Set action when we exceed drag threshold
	if (m_action == Thresholded && abs(event->x() - m_moveStartX) > DRAG_ACTION_THRESHOLD)
	{
		chooseMouseAction(event);
	}

	switch (m_action)
	{
		case MovePositionMarker:
		{
			const TimePos t = getPositionFromX(event->x());
			m_pos.setTicks(t.getTicks());
			Engine::getSong()->setToTime(t, m_mode);
			if (!(Engine::getSong()->isPlaying()))
			{
				//Song::Mode_None is used when nothing is being played.
				Engine::getSong()->setToTime(t, Song::Mode_None);
			}
			m_pos.setCurrentFrame( 0 );
			m_pos.setJumped( true );
			updatePosition();
			positionMarkerMoved();
			break;
		}
		case MoveLoopBegin:
		{
			setLoopPoint(0, event->x(), unquantized);
			break;
		}
		case MoveLoopEnd:
		{
			setLoopPoint(1, event->x(), unquantized);
			break;
		}
		case DragLoop:
		{
			TimePos diff = (event->x() - m_moveStartX) * TimePos::ticksPerBar() / m_ppb;
			if (!unquantized){ diff = diff.quantize(m_snapSize); }
			// Stop when left side hits 0
			diff = std::max(diff.getTicks(), -m_oldLoopPos[0].getTicks());

			m_loopPos[0] = std::max(m_oldLoopPos[0] + diff, 0);
			m_loopPos[1] = std::max(m_oldLoopPos[1] + diff, 0);

			update();
			break;
		}
		case DrawLoop:
		{
			TimePos start = getPositionFromX(m_moveStartX);
			TimePos mousePos = getPositionFromX(event->x());
			TimePos end = mousePos;

			if (!unquantized)
			{
				start = start.quantize(m_snapSize);
				end = end.quantize(m_snapSize);
			}
			if (start > end) {
				qSwap(start, end);
			}
			// If the region is quantized to 0 length,
			// we make it 1 step wide in the direction the mouse has been dragged
			else if (start == end) {
				if (mousePos < start) { start = end - TimePos::ticksPerBar() * m_snapSize; }
				else { end = start + TimePos::ticksPerBar() * m_snapSize; }
			}

			m_loopPos[0] = start;
			m_loopPos[1] = end;

			update();
			break;
		}
		case SelectSongTCO:
			emit regionSelectedFromPixels(m_moveStartX , event->x());
			break;

		default:
			break;
	}
}




void TimeLineWidget::mouseReleaseEvent( QMouseEvent* event )
{
	delete m_hint;
	m_hint = NULL;

	if (m_action == Thresholded)
	{
		chooseMouseAction(event);
	}

	switch (m_action)
	{
		case SelectSongTCO:
			emit selectionFinished();
			break;

		case MoveLoopBegin:
		case MoveLoopEnd:
			mouseMoveEvent(event);
			break;

		case ShowContextMenu:
			contextMenuEvent(nullptr);
			break;

		default:
			break;
	}

	m_action = NoAction;
}



void TimeLineWidget::chooseMouseAction(QMouseEvent* event)
{
	struct Binding
	{
		Qt::MouseButton button;
		Qt::KeyboardModifier mod;
		QEvent::Type type;
		actions action;

		Binding(Qt::MouseButton button,	Qt::KeyboardModifier mod, QEvent::Type type, actions action) :
			button(button), mod(mod), type(type), action(action) {}
	};

	// MouseButtonPress triggers instantly (both click and drag)
	// MouseMove triggers when mouse surpasses drag threshold
	// MouseButtonRelease triggers only if MouseMove hasn't been triggered

	// TODO: Read these from a config
	static std::vector<Binding> bindings {
		// Actions that may be unquantized by holding Control
		// (the MouseMoves will show a TextFloat but the ButtonReleases won't)
		{Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseMove, MoveLoopBegin},
		{Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseButtonRelease, MoveLoopBegin},
		{Qt::RightButton, Qt::ShiftModifier, QEvent::MouseMove, MoveLoopEnd},
		{Qt::RightButton, Qt::ShiftModifier, QEvent::MouseButtonRelease, MoveLoopEnd},
		{Qt::RightButton, Qt::NoModifier, QEvent::MouseMove, DrawLoop},
		{Qt::MiddleButton, Qt::NoModifier, QEvent::MouseMove, DragLoop},
		// Other actions
		{Qt::LeftButton, Qt::ControlModifier, QEvent::MouseMove, SelectSongTCO},
		{Qt::RightButton, Qt::NoModifier, QEvent::MouseButtonRelease, ShowContextMenu},
		//{Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonRelease, ZoomLoop},
		//{Qt::RightButton, Qt::ControlModifier, QEvent::MouseButtonRelease, ZoomSong},
		//{Qt::MiddleButton, Qt::NoModifier, QEvent::MouseButtonRelease, NoAction},
	};

	// include the button being released
	auto buttons = event->button() | event->buttons();
	auto mods = event->modifiers();
	auto type = event->type();

	// default value for mouse press = wait for it to exceed drag threshold
	m_action = event->type() == QEvent::MouseButtonPress ? Thresholded : NoAction;

	for (Binding b: bindings)
	{
		if (b.button & buttons && (b.mod & mods || b.mod == Qt::NoModifier) && b.type == type)
		{
			m_action = b.action;
			break;
		}
	}

	// Notify the user if they can disable quantization
	// only show when dragging, and ctrl is not pressed
	bool isQuantized = m_action == MoveLoopBegin || m_action == MoveLoopEnd
					|| m_action == DrawLoop || m_action == DragLoop;
	if (isQuantized && event->type() == QEvent::MouseMove && !(mods & Qt::ControlModifier))
	{
		delete m_hint;
		m_hint = TextFloat::displayMessage(tr("Hint"),
			tr("Hold <%1> to disable quantization.").arg(UI_CTRL_KEY),
			embed::getIconPixmap("hint"), 0);
	}
}




TimePos TimeLineWidget::getPositionFromX(const int x) const
{
	return m_begin + std::max(x - m_xOffset - s_posMarkerPixmap->width() / 2, 0) * TimePos::ticksPerBar() / m_ppb;
}




void TimeLineWidget::setLoopPoint(bool end, int x, bool unquantized)
{
	TimePos oneSnap = TimePos::ticksPerBar() * m_snapSize;

	int i = end ? 1 : 0;
	TimePos pos = getPositionFromX(x);
	m_loopPos[i] = unquantized ? pos : pos.quantize(m_snapSize);

	if (m_loopPos[1] == 0) { m_loopPos[1] = oneSnap; }

	if (m_loopPos[0] >= m_loopPos[1])
	{
		if (end) { m_loopPos[0] = 0; }
		else { m_loopPos[1] = std::max(m_length, TimePos(m_loopPos[0] + oneSnap)); }
	}
	update();
}
