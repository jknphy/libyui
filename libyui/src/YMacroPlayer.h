/**************************************************************************
Copyright (C) 2000 - 2010 Novell, Inc.
All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

**************************************************************************/


/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:		YMacroPlayer.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMacroPlayer_h
#define YMacroPlayer_h

#include <string>

/**
 * Abstract base class for macro player.
 *
 * Applications should not use this directly, but the static methods in YMacro.
 **/
class YMacroPlayer
{
    friend class YMacro;

protected:
    /**
     * Constructor
     **/
    YMacroPlayer() {}

public:
    /**
     * Destructor
     **/
    virtual ~YMacroPlayer() {}

    /**
     * Play a macro from the specified macro file.
     **/
    virtual void play( const string & macroFile ) = 0;

    /**
     * Play the next block from the current macro, if there is one playing.
     **/
    virtual void playNextBlock() = 0;

    /**
     * Return 'true' if a macro is currently being played.
     **/
    virtual bool playing() const = 0;
};

#endif // YMacroPlayer_h
