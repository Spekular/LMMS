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


ImageMetronomeControlDialog::ImageMetronomeControlDialog( ImageMetronomeControls* controls ) :
	EffectControlDialog( controls )
{
	setAutoFillBackground( true );
	QPalette pal;
	pal.setBrush( backgroundRole(), PLUGIN_NAME::getIconPixmap( "artwork" ) );
	setPalette( pal );
	setFixedSize( 100, 110 );

	ComboBox * loopLengthSelector = new ComboBox(this);
	loopLengthSelector->setFixedSize(80,22);
	//loopLengthSelector -> move( 16, 10 );
	loopLengthSelector->setModel( &controls->m_loopLengthModel );

	// Knob * panKnob = new Knob( knobBright_26, this);
	// panKnob -> move( 57, 10 );
	// panKnob->setModel( &controls->m_panModel );
	// panKnob->setLabel( tr( "PAN" ) );
	// panKnob->setHintText( tr( "Panning:" ) , "" );
}
