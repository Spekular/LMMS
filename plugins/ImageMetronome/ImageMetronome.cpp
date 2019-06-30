/*
 * ImageMetronome.cpp - [Put a very brief description of the effect here (not more than just a few words usually)]
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

#include "ImageMetronome.h"

#include "embed.h"
#include "plugin_export.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT imagemetronome_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"ImageMetronome",
	QT_TRANSLATE_NOOP( "pluginBrowser", "Displays an image sequence synced to the beat." ),
	"Spekular <spekularr/at/gmail/dot/com>",
	0x0100,
	Plugin::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}



ImageMetronomeEffect::ImageMetronomeEffect( Model* parent, const Descriptor::SubPluginFeatures::Key* key ) :
	Effect( &imagemetronome_plugin_descriptor, parent, key ),
	m_imagemetronomeControls( this )
{
}



ImageMetronomeEffect::~ImageMetronomeEffect()
{
}



bool ImageMetronomeEffect::processAudioBuffer( sampleFrame* buf, const fpp_t frames )
{
	if( !isEnabled() || !isRunning () )
	{
		return( false );
	}

	return isRunning();
}





extern "C"
{

// necessary for getting instance out of shared lib
PLUGIN_EXPORT Plugin * lmms_plugin_main( Model* parent, void* data )
{
	return new ImageMetronomeEffect( parent, static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>( data ) );
}

}
