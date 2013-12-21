/*
 * Copyright (c) 2013, Benichou Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Purpose: Provide read-write access to memory-mapped files on Windows and POSIX systems.
 *
 * $Id: fmstream.cpp 4 2013-09-24 14:03:10Z benichou $
 */

#ifdef _WIN32
#include <windows.h>
#else
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include "fmstream.h"

std::streamoff filemapping::offset_granularity()
{
#ifdef _WIN32
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	return static_cast<std::streamoff>(SystemInfo.dwAllocationGranularity);
#else // If not Windows, this is a POSIX system !
	return static_cast<std::streamoff>(sysconf(_SC_PAGE_SIZE));
#endif
}

filemappingbuf::filemappingbuf() :
	m_pAddress(NULL),
	m_MapLength(0),
#ifdef _WIN32
	m_pFile(INVALID_HANDLE_VALUE),
	m_pFileMapping(NULL)
#else // If not Windows, this is a POSIX system !
	m_fd(-1)
#endif
{
}

filemappingbuf::~filemappingbuf()
{
	close();
}

#ifdef _HAS_CPP11_
filemappingbuf::filemappingbuf(filemappingbuf&& rhs_buf) :
	m_pAddress(NULL),
	m_MapLength(0),
#ifdef _WIN32
	m_pFile(INVALID_HANDLE_VALUE),
	m_pFileMapping(NULL)
#else // If not Windows, this is a POSIX system !
	m_fd(-1)
#endif
{
	swap(rhs_buf);
}

filemappingbuf& filemappingbuf::operator=(filemappingbuf&& rhs_buf)
{
	if(this != &rhs_buf)
	{
		close();
		swap(rhs_buf);
	}

	return *this;
}

void filemappingbuf::swap(filemappingbuf& buf)
{
	if(this != &buf)
	{
		std::streambuf::swap(buf);
		std::swap(m_pAddress, buf.m_pAddress);
		std::swap(m_MapLength, buf.m_MapLength);
#ifdef _WIN32
		std::swap(m_pFile, buf.m_pFile);
		std::swap(m_pFileMapping, buf.m_pFileMapping);
#else // If not Windows, this is a POSIX system !
		std::swap(m_fd, buf.m_fd);
#endif
	}
}

void swap(filemappingbuf& lhs, filemappingbuf& rhs)
{
	lhs.swap(rhs);
}
#endif // _HAS_CPP11_

bool filemappingbuf::is_open() const
{
	return (m_pAddress && m_MapLength);
}

void* filemappingbuf::pubseekptr(void* ptr, std::ios_base::openmode which)
{
	if(!is_open())
		return NULL;

	return seekptr(ptr, which);
}

std::streampos filemappingbuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
	switch(way)
	{
	case std::ios_base::beg:
		break;

	case std::ios_base::cur:
		if(which & std::ios_base::in)
			off += static_cast<std::streamoff>(gptr() - m_pAddress);
		else
			off += static_cast<std::streamoff>(pptr() - m_pAddress);
		break;

	case std::ios_base::end:
		off = m_MapLength - off;
		break;

	default:
		return -1;
	}

	if(off < 0)
		return -1;

	return seekpos(off, which);
}

std::streampos filemappingbuf::seekpos(std::streampos sp, std::ios_base::openmode which)
{
	if(sp < 0 || !seekptr(m_pAddress + static_cast<ptrdiff_t>(sp), which))
		return -1;

	return sp;
}

void* filemappingbuf::seekptr(void* ptr, std::ios_base::openmode which)
{
	char* pcPtr = static_cast<char*>(ptr);

	if((which & std::ios_base::in) && pcPtr != gptr())
	{
		if(pcPtr >= m_pAddress && pcPtr < egptr())
			setg(eback(), pcPtr, egptr());
		else
			return NULL;
	}

	if((which & std::ios_base::out) && pcPtr != pptr())
	{
		if(pcPtr >= m_pAddress && pcPtr < epptr())
			setp(pcPtr, epptr());
		else
			return NULL;
	}

	return ptr;
}

filemappingbuf* filemappingbuf::open(const char* path_name, std::ios_base::openmode mode, std::streamsize max_length, std::streamoff offset)
{
	if(is_open() || max_length < 0 || offset < 0) // Check if a file is already opened and parameters
		return NULL;

	std::streamsize FileSize = 0;

#ifdef _WIN32
	DWORD dwDesiredAccess = GENERIC_READ;
	DWORD flProtect = PAGE_READONLY;
	DWORD dwMapAccess = FILE_MAP_READ;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwFileSizeHigh = 0;
	DWORD dwFileOffsetHigh = 0;

	if(mode & std::ios_base::out)
	{
		dwDesiredAccess |= GENERIC_WRITE;
		flProtect = PAGE_READWRITE;
		dwMapAccess |= FILE_MAP_WRITE;
		dwCreationDisposition = OPEN_ALWAYS;
	}

	m_pFile = CreateFileA(path_name, dwDesiredAccess, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_pFile == INVALID_HANDLE_VALUE)
	{
		filemappingbuf::close();
		return NULL;
	}

	FileSize = GetFileSize(m_pFile, &dwFileSizeHigh);
#ifdef _WIN64
	FileSize |= static_cast<std::streamsize>(dwFileSizeHigh) << 32;
#endif

	if(max_length)
		m_MapLength = max_length;
	else
		m_MapLength = FileSize - static_cast<std::streamsize>(offset);

	if(!(mode & std::ios_base::out) && (static_cast<std::streamsize>(offset) + m_MapLength) > FileSize)
		m_MapLength = FileSize - static_cast<std::streamsize>(offset);

	if(m_MapLength <= 0)
	{
		filemappingbuf::close();
		return NULL;
	}

	std::streamsize NewFileSize = static_cast<std::streamsize>(offset) + m_MapLength;

#ifdef _WIN64
	dwFileSizeHigh = static_cast<DWORD>(NewFileSize >> 32);
	dwFileOffsetHigh = static_cast<DWORD>(offset >> 32);
#else
	dwFileSizeHigh = 0;
#endif

	m_pFileMapping = CreateFileMappingA(m_pFile, NULL, flProtect, dwFileSizeHigh, static_cast<DWORD>(NewFileSize), NULL);
	if(!m_pFileMapping)
	{
		filemappingbuf::close();
		return NULL;
	}

	m_pAddress = static_cast<char*>(MapViewOfFile(m_pFileMapping, dwMapAccess, dwFileOffsetHigh, static_cast<DWORD>(offset), static_cast<SIZE_T>(m_MapLength)));

	if (!m_pAddress)
	{
		DWORD error = GetLastError();
		filemappingbuf::close();
		return NULL;
	}

#else // If not Windows, this is a POSIX system !
	int oflag = O_RDONLY;
	int flags = PROT_READ;
	mode_t ar = 0;

	if(mode & std::ios_base::out)
	{
		oflag = O_RDWR | O_CREAT;
		flags |= PROT_WRITE;
		ar = S_IRUSR | S_IWUSR;
	}

	m_fd = ::open(path_name, oflag, ar);
	if(m_fd == -1)
		return NULL;

	struct stat statbuf;

	// Get the file size
	if(fstat(m_fd, &statbuf) != 0)
	{
		filemappingbuf::close();
		return NULL;
	}

	FileSize = statbuf.st_size;

	if(max_length)
		m_MapLength = max_length;
	else
		m_MapLength = FileSize - static_cast<std::streamsize>(offset);

	if(static_cast<std::streamsize>(offset) + m_MapLength > FileSize)
	{
		if(mode & std::ios_base::out)
		{
			/* Something needs to be written at the end of the file to
			 * have the file actually have the new size.
			 * Just writing an empty string at the current file position will do.
			 *
			 * Note:
			 *  - The current position in the file is at the end of the stretched
			 *    file due to the call to lseek().
			 *  - An empty string is actually a single '\0' character, so a zero-byte
			 *    will be written at the last byte of the file.
			 */
			if(lseek(m_fd, static_cast<std::streamsize>(offset) + m_MapLength - 1, SEEK_SET) == -1)
			{
				filemappingbuf::close();
				return NULL;
			}

			if(write(m_fd, "", 1) != 1)
			{
				filemappingbuf::close();
				return NULL;
			}
		}
		else
		{
			m_MapLength = FileSize - static_cast<std::streamsize>(offset);

			if(m_MapLength <= 0)
			{
				filemappingbuf::close();
				return NULL;
			}
		}
	}

	m_pAddress = static_cast<char*>(mmap(NULL, static_cast<size_t>(m_MapLength), flags, MAP_SHARED, m_fd, offset));
	if(m_pAddress == MAP_FAILED)
	{
		filemappingbuf::close();
		return NULL;
	}
#endif

	char* pEnd = m_pAddress + static_cast<ptrdiff_t>(m_MapLength);

	setg(m_pAddress, m_pAddress, pEnd);

	if(mode & std::ios_base::ate) // At end
		setp(m_pAddress + static_cast<ptrdiff_t>(FileSize - offset), pEnd);
	else
		setp(m_pAddress, pEnd);

	return this;
}

int filemappingbuf::sync()
{
	int nRet = -1;

#ifdef _WIN32
	if(m_pAddress && m_MapLength)
		nRet = (FlushViewOfFile(m_pAddress, static_cast<SIZE_T>(m_MapLength)) != FALSE)? 0: -1;
#else // If not Windows, this is a POSIX system !
	if(m_pAddress && m_MapLength)
		nRet = msync(m_pAddress, static_cast<size_t>(m_MapLength), MS_ASYNC);
#endif

	return nRet;
}

const void* filemappingbuf::data() const
{
	return m_pAddress;
}

void* filemappingbuf::data()
{
	return m_pAddress;
}

std::streamsize filemappingbuf::size() const
{
	return m_MapLength;
}

filemappingbuf* filemappingbuf::close()
{
#ifdef _WIN32
	if(m_pAddress)
		UnmapViewOfFile(m_pAddress);

	if(m_pFileMapping)
	{
		CloseHandle(m_pFileMapping);
		m_pFileMapping = NULL;
	}

	if(m_pFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_pFile);
		m_pFile = INVALID_HANDLE_VALUE;
	}
#else // If not Windows, this is a POSIX system !
	if(m_pAddress && m_MapLength)
		munmap(m_pAddress, static_cast<size_t>(m_MapLength));

	if(m_fd != -1)
	{
		::close(m_fd);
		m_fd = -1;
	}
#endif

	if(!is_open())
		return NULL;

	m_pAddress = NULL;
	m_MapLength = 0;

	setg(NULL, NULL, NULL);
	setp(NULL, NULL);

	return this;
}

/* ifmstream class */
ifmstream::ifmstream() : std::istream(&m_rdbuf)
{}

ifmstream::ifmstream(const char* path_name, std::streamsize max_length, std::streamoff offset) : std::istream(&m_rdbuf)
{
	open(path_name, max_length, offset);
}

#ifdef _HAS_CPP11_
ifmstream::ifmstream(ifmstream&& rhs_stream) : std::istream(&m_rdbuf)
{
	swap(rhs_stream);
}

ifmstream& ifmstream::operator=(ifmstream&& rhs_stream)
{
	if(this != &rhs_stream)
	{
		m_rdbuf.close();
		swap(rhs_stream);
	}

	return *this;
}

void ifmstream::swap(ifmstream& stream)
{
	if(this != &stream)
	{
		std::istream::swap(stream);
		m_rdbuf.swap(stream.m_rdbuf);
	}
}

void swap(ifmstream& lhs, ifmstream& rhs)
{
	lhs.swap(rhs);
}
#endif // _HAS_CPP11_

filemappingbuf* ifmstream::rdbuf() const
{
	return const_cast<filemappingbuf*>(&m_rdbuf);
}

bool ifmstream::is_open() const
{
	return m_rdbuf.is_open();
}

void ifmstream::open(const char* path_name, std::streamsize max_length, std::streamoff offset)
{
	if(m_rdbuf.open(path_name, std::ios_base::in, max_length, offset) == NULL)
		setstate(std::ios_base::failbit);
	else
		clear();
}

void ifmstream::close()
{
	if(m_rdbuf.close() == NULL)
		setstate(std::ios_base::failbit);
}

const void* ifmstream::ptellg()
{
	// Return input stream position
	if(!fail())
		return static_cast<char*>(m_rdbuf.data()) + static_cast<ptrdiff_t>(m_rdbuf.pubseekoff(0, std::ios_base::cur, std::ios_base::in));
	else
		return NULL;
}

std::istream& ifmstream::pseekg(const void* ptr)
{
	// Set input stream position to ptr
	if(!fail() && !m_rdbuf.pubseekptr(const_cast<void*>(ptr), ios_base::in))
		setstate(std::ios_base::failbit);
	return *this;
}

const void* ifmstream::data() const
{
	if(!fail())
		return m_rdbuf.data();
	else
		return NULL;
}

std::streamsize ifmstream::size() const
{
	if(!fail())
		return m_rdbuf.size();
	else
		return 0;
}

/* fmstream class */
fmstream::fmstream() : std::iostream(&m_rdbuf)
{}

fmstream::fmstream(const char* path_name, std::streamsize max_length, std::streamoff offset) : std::iostream(&m_rdbuf)
{
	open(path_name, max_length, offset);
}

#ifdef _HAS_CPP11_
fmstream::fmstream(fmstream&& rhs_stream) : std::iostream(&m_rdbuf)
{
	swap(rhs_stream);
}

fmstream& fmstream::operator=(fmstream&& rhs_stream)
{
	if(this != &rhs_stream)
	{
		m_rdbuf.close();
		swap(rhs_stream);
	}

	return *this;
}

void fmstream::swap(fmstream& stream)
{
	if(this != &stream)
	{
		std::iostream::swap(stream);
		m_rdbuf.swap(stream.m_rdbuf);
	}
}

void swap(fmstream& lhs, fmstream& rhs)
{
	lhs.swap(rhs);
}
#endif // _HAS_CPP11_

filemappingbuf* fmstream::rdbuf() const
{
	return const_cast<filemappingbuf*>(&m_rdbuf);
}

bool fmstream::is_open() const
{
	return m_rdbuf.is_open();
}

void fmstream::open(const char* path_name, std::streamsize max_length, std::streamoff offset)
{
	if(m_rdbuf.open(path_name, std::ios_base::in | std::ios_base::out, max_length, offset) == NULL)
		setstate(std::ios_base::failbit);
	else
		clear();
}

void fmstream::close()
{
	if(m_rdbuf.close() == NULL)
		setstate(std::ios_base::failbit);
}

const void* fmstream::ptellg()
{
	// Return input stream position
	if(!fail())
		return static_cast<char*>(m_rdbuf.data()) + static_cast<ptrdiff_t>(m_rdbuf.pubseekoff(0, std::ios_base::cur, std::ios_base::in));
	else
		return NULL;
}

void* fmstream::ptellp()
{
	// Return input stream position
	if(!fail())
		return static_cast<char*>(m_rdbuf.data()) + static_cast<ptrdiff_t>(m_rdbuf.pubseekoff(0, std::ios_base::cur, std::ios_base::out));
	else
		return NULL;
}

std::istream& fmstream::pseekg(const void* ptr)
{
	// Set input stream position to ptr
	if(!fail() && !m_rdbuf.pubseekptr(const_cast<void*>(ptr), std::ios_base::in))
		setstate(std::ios_base::failbit);
	return *this;
}

std::ostream& fmstream::pseekp(void* ptr)
{
	// Set output stream position to ptr
	if(!fail() && !m_rdbuf.pubseekptr(ptr, std::ios_base::out))
		setstate(std::ios_base::failbit);
	return *this;
}

const void* fmstream::data() const
{
	if(!fail())
		return m_rdbuf.data();
	else
		return NULL;
}

void* fmstream::data()
{
	if(!fail())
		return m_rdbuf.data();
	else
		return NULL;
}

std::streamsize fmstream::size() const
{
	if(!fail())
		return m_rdbuf.size();
	else
		return 0;
}
