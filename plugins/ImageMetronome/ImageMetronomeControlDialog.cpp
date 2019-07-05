/*
 * ImageMetronomeControlDialog.cpp
 *
 * Copyright (c) [the year] [your name] [<youremail/at/gmailormaybenotgmail/dot/com>]
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

#include <QLayout>

#include "ImageMetronomeControlDialog.h"
#include "ImageMetronomeControls.h"
#include "embed.h"
#include "ComboBox.h"
#include "PixmapButton.h"
#include "FileDialog.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Song.h"
#include <QDebug>


ImageMetronomeControlDialog::ImageMetronomeControlDialog( ImageMetronomeControls* controls ) :
	EffectControlDialog( controls ),
	m_imageDisplay( new QLabel(this) ),
	m_images( 1, embed::getIconPixmap( "folder" ) ),
	m_lookup( 1, 0 )
{
	setAutoFillBackground( true );
	QPalette pal;
	pal.setBrush( backgroundRole(), PLUGIN_NAME::getIconPixmap( "artwork" ) );
	setPalette( pal );
	setFixedSize( 800, 800 );

	ComboBox * loopLengthSelector = new ComboBox( this );
	loopLengthSelector->setFixedSize( 80, 22 );
	loopLengthSelector->move( 16, 0 );
	loopLengthSelector->setModel( &controls->m_loopLengthModel );

	PixmapButton * openImageDirButton = new PixmapButton( this );
	openImageDirButton->setActiveGraphic( embed::getIconPixmap( "folder" ) );
	openImageDirButton->setInactiveGraphic( embed::getIconPixmap( "folder" ) );
	connect( openImageDirButton, SIGNAL( clicked() ), this, SLOT( updateImageDir() ) );

	m_imageDisplay->move( 0, 16 );
	m_imageDisplay->setText("test");

	connect( gui->mainWindow(), SIGNAL( periodicUpdate() ), this, SLOT( update() ) );
}

void ImageMetronomeControlDialog::updateImageDir()
{
	//Let the user select a directory
	QDir imageDir = FileDialog::getExistingDirectory( this,
		tr( "Select the directory containing your images" ), "/" );
	//Get a list of all the png files in that directory with numbers as their filename
	//QStringList imagePaths = imageDir.entryList( QStringList("^[0-9]+$.png"), QDir::Files );
	QFileInfoList imagePaths = imageDir.entryInfoList( QStringList("*.png"), QDir::Files );
	//imagePaths.sort();

	//Make a vector of all the images in imagePaths
	std::vector<QPixmap> images;
	//And the numbers from their filenames
	std::vector<int> positions;
	for ( int i = 0; i < imagePaths.size(); i++ )
	{
		images.push_back( QPixmap( imagePaths.at(i).absoluteFilePath() ) );
		qDebug() << imagePaths.at(i).absoluteFilePath();
		positions.push_back( std::stoi( imagePaths.at(i).baseName().toStdString() ) );
	}

	m_images = images;
	qDebug() << positions.size();

	QPixmap li = images.back();
	m_imageDisplay->setPixmap( li.scaled( li.width(), li.height(), Qt::KeepAspectRatio ) );

	//Position [frame number - 1] contains an index,
	//pointing to the image in the vector images that should be dislayed
	std::vector<int> lookup;
	int currentKeyframe = 0;
	for ( int i = 0; i < positions.back(); i++ )
	{

		if ( positions.at( currentKeyframe ) == i + 1 )
		{
			currentKeyframe++;
		}

		lookup.push_back( std::max( 0, currentKeyframe - 1 ) );
	}

	m_lookup = lookup;
}

void ImageMetronomeControlDialog::update()
{
	Song* s = Engine::getSong();
	int tick = ( s->getMilliseconds() * s->getTempo() * (DefaultTicksPerTact / 4 ) ) / 60000 ;
	int beat = (int)(tick * 4 / s->ticksPerTact() );
	int frame = beat % m_lookup.size();
	if ( m_images.size() > 0 )
	{
		//qDebug() << "frame" << frame;
		//qDebug() << "index" <<  m_lookup.at(frame);
		m_images.at( m_lookup.at(frame) );
		m_imageDisplay->setPixmap( m_images.at( m_lookup.at(frame) ) );
		m_imageDisplay->resize( m_images.at( m_lookup.at(frame) ).width(),
			m_images.at( m_lookup.at(frame) ).height() );
	}
	//m_imageDisplay->setPixmap( m_images.at( m_lookup.at(frame) ) );
}
