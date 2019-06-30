/*
 * ImageMetronomeControls.h
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

#ifndef IMAGEMETRONOME_CONTROLS_H
#define IMAGEMETRONOME_CONTROLS_H

#include "EffectControls.h"
#include "ImageMetronomeControlDialog.h"

#include "ComboBoxModel.h"

class ImageMetronomeEffect;


class ImageMetronomeControls : public EffectControls
{
	Q_OBJECT
public:
	ImageMetronomeControls( ImageMetronomeEffect* effect );
	virtual ~ImageMetronomeControls()
	{
	}

	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );
	inline virtual QString nodeName() const
	{
		return "ImageMetronomeControls";
	}

	virtual int controlCount()
	{
		return 2;
	}

	virtual EffectControlDialog* createView()
	{
		return new ImageMetronomeControlDialog( this );
	}

private:
	ImageMetronomeEffect* m_effect;

	ComboBoxModel m_loopLengthModel;

	friend class ImageMetronomeControlDialog;
	friend class ImageMetronomeEffect;

} ;

#endif
