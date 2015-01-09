/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012 Marcus Tomlinson

This file is part of DSPatch.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LGPLv3.txt included in the packaging of this
file. Please review the following information to ensure the GNU Lesser
General Public License version 3.0 requirements will be met:
http://www.gnu.org/copyleft/lgpl.html.

Other Usage
Alternatively, this file may be used in accordance with the terms and
conditions contained in a signed written agreement between you and
Marcus Tomlinson.

DSPatch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
************************************************************************/

#include "DspComponentThread.h"
#include "DspComponent.h"

//=================================================================================================

DspComponentThread::DspComponentThread( DspComponent& component )
: _component( component )
{
	Start();
}

//-------------------------------------------------------------------------------------------------

DspComponentThread::~DspComponentThread()
{
	Stop();
}

//=================================================================================================

void DspComponentThread::Start( Priority priority )
{
	_stop = false;
	_stopped = false;
	_pause = false;
	DspThread::Start( priority );
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Stop()
{
	_stop = true;

	while( _stopped != true )
	{
		_pauseCondt.WakeAll();
		_resumeCondt.WakeAll();
		MsSleep( 1 );
	}
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Pause()
{
	_pauseMutex.Lock();

	_pause = true;
	_pauseCondt.Wait( _pauseMutex );		//wait for resume
	_pause = false;

	_pauseMutex.Unlock();
}

//-------------------------------------------------------------------------------------------------

void DspComponentThread::Resume()
{
	_resumeMutex.Lock();
	_resumeCondt.WakeAll();
	_resumeMutex.Unlock();
}

//=================================================================================================

void DspComponentThread::_Run()
{
	while( !_stop )
	{
		_component.Reset();
		_component.Tick();

		if( _pause )
		{
			_resumeMutex.Lock();

			_pauseMutex.Lock();
			_pauseCondt.WakeAll();
			_pauseMutex.Unlock();

			_resumeCondt.Wait( _resumeMutex );		//wait for resume

			_resumeMutex.Unlock();
		}
	}

	_stopped = true;
}

//=================================================================================================