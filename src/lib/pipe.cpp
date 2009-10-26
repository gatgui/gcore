/*

Copyright (C) 2009  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <gcore/pipe.h>
#include <gcore/platform.h>
using namespace std;

gcore::Pipe::Pipe() {
	mDesc[0] = INVALID_PIPE;
	mDesc[1] = INVALID_PIPE;
#ifdef PRIVATE_BUFFER
	memset(mReadBuffer, 0, PIPE_BUFFER_SIZE);
#endif
}

gcore::Pipe::Pipe(const gcore::Pipe &rhs) {
	mDesc[0] = rhs.mDesc[0];
	mDesc[1] = rhs.mDesc[1];
#ifdef PRIVATE_BUFFER
	memset(mReadBuffer, 0, PIPE_BUFFER_SIZE);
#endif
}

gcore::Pipe::~Pipe() {
  // NO !
  //close();
}

gcore::Pipe& gcore::Pipe::operator=(const gcore::Pipe &rhs) {
	if (this != &rhs) {
		mDesc[0] = rhs.mDesc[0];
		mDesc[1] = rhs.mDesc[1];
	}
	return *this;
}

gcore::PipeID gcore::Pipe::readId() const {
	return mDesc[0];
}

gcore::PipeID gcore::Pipe::writeId() const
{
	return mDesc[1];
}

void gcore::Pipe::close() {
  closeRead();
	closeWrite();
}

void gcore::Pipe::create() {
	close();
#ifndef _WIN32
	if (pipe(mDesc) == -1) {
#else
	SECURITY_ATTRIBUTES sattr;
	sattr.nLength = sizeof(sattr);
	sattr.lpSecurityDescriptor = NULL;
	sattr.bInheritHandle = TRUE;
	if (!CreatePipe(&mDesc[0], &mDesc[1],&sattr,0)) {
#endif
		mDesc[0] = INVALID_PIPE;
		mDesc[1] = INVALID_PIPE;
	}
}

bool gcore::Pipe::canRead() const {
	return (IsValidPipeID(mDesc[0]));
}

bool gcore::Pipe::canWrite() const {
	return (IsValidPipeID(mDesc[1]));
}

void gcore::Pipe::closeRead() {
	if (canRead()) {
#ifndef _WIN32
		::close(mDesc[0]);
#else
		CloseHandle(mDesc[0]);
#endif
		mDesc[0] = INVALID_PIPE;
	}
}

void gcore::Pipe::closeWrite() {
	if (canWrite()) {
#ifndef _WIN32
		::close(mDesc[1]);
#else 
		CloseHandle(mDesc[1]);
#endif
		mDesc[1] = INVALID_PIPE;
	}
}

int gcore::Pipe::read(string &str) const {
#ifndef PRIVATE_BUFFER
	static char rdbuf[PIPE_BUFFER_SIZE] = {0};
#endif

	if (canRead()) {
#ifndef _WIN32

#ifdef PRIVATE_BUFFER
		int bytesRead = ::read(mDesc[0], mReadBuffer, PIPE_BUFFER_SIZE);
		if (bytesRead != -1) {
			mReadBuffer[bytesRead] = '\0';
			str = mReadBuffer;
			return bytesRead;
		}
#else
		int bytesRead = ::read(mDesc[0], rdbuf, PIPE_BUFFER_SIZE);
		if (bytesRead != -1) {
			rdbuf[bytesRead] = '\0';
			str = rdbuf;
			return bytesRead;
		}
#endif

#else

		DWORD bytesRead = 0;
#ifdef PRIVATE_BUFFER
		if (ReadFile(mDesc[0],mReadBuffer,PIPE_BUFFER_SIZE,&bytesRead,NULL)) {
			mReadBuffer[bytesRead] = '\0';
			str = mReadBuffer;
			return bytesRead;
		}
#else
		if (ReadFile(mDesc[0],rdbuf,PIPE_BUFFER_SIZE,&bytesRead,NULL)) {
			rdbuf[bytesRead] = '\0';
			str = rdbuf;
			return bytesRead;
		}
#endif

#endif
	}
	str = "";
	return -1;
}

int gcore::Pipe::write(const string &str) const {
	if (canWrite()) {
#ifndef _WIN32
		int bytesToWrite = str.length();
		return ::write(mDesc[1], str.c_str(), bytesToWrite);
#else
		DWORD bytesToWrite = (DWORD)str.length();
		DWORD bytesWritten = 0;
		if (! WriteFile(mDesc[1], str.c_str(), bytesToWrite, &bytesWritten, NULL)) {
			return -1;
		}
		return bytesWritten;
#endif
	}
	return -1;
}

