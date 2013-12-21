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
 * $Id: fmstream.h 4 2013-09-24 14:03:10Z benichou $
 */

#ifndef FILE_MAPPING_STREAM_H_
#define FILE_MAPPING_STREAM_H_

#include <istream>

/**
 * At this time, move constructor and move assignment for streams are only implements in Microsoft Visual Studio 2010 and Intel C++ Compiler 12
 */
#if ((__cplusplus > 199711L) || (_HAS_CPP0X > 0)) && ((_MSC_VER >= 1600) || (__INTEL_COMPILER >= 1200))
#define _HAS_CPP11_ 1
#endif

/**
 * File mapping utility class.
 */
class filemapping
{
public:
	/**
	 * Get memory offset granularity.
	 * Return the offset granularity of the system.
	 * @return Return the offset granularity of the system.
	 * @see filemappingbuf::open()
	 * @see ifmstream::open()
	 * @see fmstream::open()
	 */
	static std::streamoff offset_granularity();
};

/**
 * File mapping stream buffer.
 * This class applies the functionality of the std::streambuf class to read and write from/to memory-mapped files.
 * By calling member open, a physical file is associated to the file buffer as its associated character sequence.
 * Depending on the mode used in this operation, the access to the controlled input sequence or the controlled output sequence may be restricted.
 * The state of the filemappingbuf object -i.e. whether a file is open or not- may be tested by calling member function is_open.
 * Internally, filemappingbuf objects operate as defined in the std::streambuf class.
 * The class overrides some virtual members inherited from streambuf to provide a specific functionality for memory-mapped files.
 */
class filemappingbuf : public std::streambuf
{
public:
	/**
	 * Construct object.
	 * A filemappingbuf object is constructed, initializing all its pointers to null pointers and initializing the object's locale.
	 */
	filemappingbuf();

	/**
	 * Destructs the filemappingbuf object.
	 */
	virtual ~filemappingbuf();

#ifdef _HAS_CPP11_
	/** @name C++11 
	 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
	 */
	///@{ 
	/**
	 * Move constructor (requires C++11). 
	 * Acquires the contents of rhs_buf, by move-assigning its members and base classes.
	 * @param rhs_buf filemappingbuf to move. rhs_buf becomes of invalid state after the operation. 
	 * @see swap()
	 * @see operator=()
	 */
	filemappingbuf(filemappingbuf&& rhs_buf);

	/**
	 * Move assignment (requires C++11).
	 * Closes the source filemappingbuf (as if member close was called), and then acquires the contents of rhs_buf.
	 * @param rhs_buf filemappingbuf to move. rhs_buf becomes of invalid state after the operation. 
	 * @return *this.
	 * @see swap()
	 */
	filemappingbuf& operator=(filemappingbuf&& rhs_buf);

	/**
	 * Swap internals (requires C++11).
	 * Exchanges the state of the filemappingbuf with those of other.
	 * @param buf filemappingbuf to exchange the state with. 
	 * @see operator=()
	 */
	void swap(filemappingbuf& buf);
	///@}
#endif // _HAS_CPP11_

	/**
	 * Check if a file is open.
	 * The function returns true if a previous call to open succeeded and there have been no calls to the member close since, 
	 * meaning that the filemappingbuf object is currently associated with a file. 
	 * @return true if a file is open, i.e. associated to this stream buffer object. false otherwise.
	 * @see open()
	 * @see close()
	 */
	bool is_open() const;
	
	/**
	 * Open file.
	 * Opens a file, associating its content with the stream buffer object to perform input/output operations on it. 
	 * The operations allowed and some operating details depend on parameter mode.
	 * If the object already has a file associated (open), this function fails.
	 * If the i/o mode is input only and the file do not exist, this function fails.
	 * If the i/o mode is output and the file do not exist, the file is created with the 'offset + max_length'  size.
	 * If the size of the opened file is less than 'offset + max_length', the file growing.
	 * If the size of the opened file is greater than max_length, the file is not truncated.
	 * An attempt to map a file with a length of 0 fails.
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param mode Flags describing the requested i/o mode for the file. This is an object of type std::ios_base::openmode.
	 * It consists of a combination of the following member constants:
	 *  - std::ios_base::ate (at end) Set the stream's position indicator to the end of the file on opening.
	 *  - std::ios_base::in (input)   Allow input operations on the stream.
	 *  - std::ios_base::out (output) Allow output operations on the stream.
	 * @param max_length Maximum length of the file mapping.  If this parameter is 0, the mapping extends from the specified offset to the end of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system. 
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @return The function returns this if successful. In case of failure, close is called and a null pointer is returned.
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	filemappingbuf* open(const char* path_name, std::ios_base::openmode mode, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Close file.
	 * Closes the file currently associated with the object and disassociates it.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, the function returns this. In case of failure, a null pointer is returned.
	 * @see open()
	 * @see is_open()
	 */
	filemappingbuf* close();

	/**
	 * Set internal position pointer to absolute position.
	 * Calls protected virtual member seekptr, which sets a new position value for one or both of the internal position pointers.
	 * The parameter which determines which of the position pointers is affected: either the get pointer or the put pointer, or both.
	 * The function fails if no file is currently open (associated) with this object.
	 * @param ptr New absolute position for the position pointer.
	 * @param which Determines which of the internal position pointers shall be modified: the input pointer, the output pointer, or both. This is an object of type std::ios_base::openmode.
	 * @return In case of success, return the new position value of the modified position pointer. In case of failure, a null pointer is returned.
	 * @see std::streambuf::pubseekpos()
	 * @see std::streambuf::pubseekoff()
	 */
	void* pubseekptr(void* ptr, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	
 	/**
	 * Get the read-only base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the constant base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	const void* data() const;

	/**
	 * Get the base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	void* data();

	/**
	 * Get the maximum byte length of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the length of the mapping. In case of failure, 0 is returned.
	 * @see data()
	 */
	std::streamsize size() const;

protected:
	virtual int sync();
	virtual std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which);
	virtual std::streampos seekpos(std::streampos sp, std::ios_base::openmode which);
	virtual void* seekptr(void* ptr, std::ios_base::openmode which);

private:
	/**
	 * Copy constructor is private: this class is not copyable.
	 */
	filemappingbuf(const filemappingbuf&);

	/**
	 * Copy operator is private: this class is not copyable.
	 */
	filemappingbuf& operator=(const filemappingbuf&);

private:
	char* m_pAddress;            //!< Base address of the mapping
	std::streamsize m_MapLength; //!< Length of the mapping
#ifdef _WIN32
	void* m_pFile;              //!< Windows handle to the file mapping object
	void* m_pFileMapping;       //!< Windows handle to the opened file
#else // If not Windows, this is a POSIX system !
	int m_fd;                    //!< File descriptor to the opened file
#endif
};

/**
 * ifmstream provides an interface to read data from memory-mapped files as input streams.
 * The objects of this class maintain internally a pointer to a filemappingbuf object that can be obtained by calling member rdbuf.
 * The file to be associated with the stream can be specified either as a parameter in the constructor or by calling member open.
 * After all necessary operations on a file have been performed, it can be closed (or disassociated) by calling member close. Once closed, the same file stream object may be used to open another file.
 * The member function is_open can be used to determine whether the stream object is currently associated with a file.
 * ifmstream can be used in place of std::ifstream.
 */
class ifmstream : public std::istream
{
public:
	/**
	 * Construct object.
	 * Constructs an object of the ifstream class. 
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 */
	ifmstream();

	/**
	 * Construct object and open a file.
	 * Constructs an object of the ifstream class. 
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 * The stream is associated with a physical file as if a call to the member function open with the same parameters was made.
	 * If the constructor is not successful in opening the file, the object is still created although no file is associated to the stream buffer and the stream's failbit is set (which can be checked with inherited member fail).
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system. 
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see open()
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	explicit ifmstream(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Destructs the ifmstream object.
	 */
	virtual ~ifmstream() {}

#ifdef _HAS_CPP11_
	/** @name C++11 
	 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
	 */
	///@{ 
	/**
	 * Move constructor (requires C++11). 
	 * Acquires the contents of rhs_stream, by move-assigning its members and base classes.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation. 
	 * @see swap()
	 * @see operator=()
	 */
	ifmstream(ifmstream&& rhs_stream);

	/**
	 * Move assignment (requires C++11).
	 * Closes the source stream (as if member close was called), and then acquires the contents of rhs_stream.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation. 
	 * @return *this.
	 * @see swap()
	 */
	ifmstream& operator=(ifmstream&& rhs_stream);

	/**
	 * Swap internals (requires C++11).
	 * Exchanges the state of the stream with those of other.
	 * @param stream ifmstream to exchange the state with. 
	 * @see operator=()
	 */
	void swap(ifmstream& stream);
	///@}
#endif // _HAS_CPP11_

	/**
	 * Get the associated filemappingbuf object.
	 * Returns a pointer to the filemappingbuf object associated with the stream.
	 * @return A pointer to the filemappingbuf object associated with the stream.
	 * Notice that for any successfully constructed ifmstream object this pointer is never a null pointer, even if no files have been opened or if the stream is unbuffered.
	 */
	filemappingbuf* rdbuf() const;

	/**
	 * Check if a file is open.
	 * Returns true if the stream is currently associated with a file, and false otherwise.
	 * The stream is associated with a file if either a previous call to member open succeeded or if the object was successfully constructed using the parameterized constructor, and close has not been called since.
	 * @return true if a file is open, i.e. associated to this stream object. false otherwise.
	 * @see open()
	 * @see close()
	 */
	bool is_open() const;

	/**
	 * Open file.
	 * Opens a file whose name is path_name, associating its content with the stream object to perform input/output operations on it. 
	 * The operations allowed and some operating details depend on parameter mode.
	 * If the object already has a file associated (open), this function fails.
	 * If the file do not exist, this function fails.
	 * An attempt to map a file with a length of 0 fails.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system. 
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	void open(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Close file.
	 * Closes the file currently associated with the object and disassociates it.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @see open()
	 * @see is_open()
	 */
	void close();

	/**
	 * Get position of the get pointer.
	 * The get pointer determines the next location in the input sequence to be read by the next input operation.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return Return the address of the get pointer. In case of failure, a null pointer is returned.
	 * @see pseekg()
	 * @see std::istream::tellg()
	 */
	const void* ptellg();
	
	/**
	 * Sets the position of the get pointer.
	 * The get pointer determines the next location to be read in the source associated to the stream.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param ptr New absolute position for the input pointer.
	 * @return The function returns *this.
	 * @see ptellg()
	 * @see std::istream::seekg()
	 */
	std::istream& pseekg(const void* ptr);
	
	/**
	 * Get the read-only base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the constant base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	const void* data() const;

	/**
	 * Get the maximum byte length of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the length of the mapping. In case of failure, 0 is returned.
	 * @see data()
	 */
	std::streamsize size() const;

private:
	/**
	 * Copy constructor is private: this class is not copyable.
	 */
	ifmstream(const ifmstream&);

	/**
	 * Copy operator is private: this class is not copyable.
	 */
	ifmstream& operator=(const ifmstream&);

private:
	filemappingbuf m_rdbuf; //!< filemappingbuf object
};

/**
 * fmstream provides an interface to read/write data from/to memory-mapped files as input/output streams.
 * The objects of this class maintain internally a pointer to a filemappingbuf object that can be obtained by calling member rdbuf.
 * The file to be associated with the stream can be specified either as a parameter in the constructor or by calling member open.
 * After all necessary operations on a file have been performed, it can be closed (or disassociated) by calling member close.
 * Once closed, the same file stream object may be used to open another file.
 * The member function is_open can be used to determine whether the stream object is currently associated with a file.
 * fmstream can be used in place of std::fstream.
 */
class fmstream : public std::iostream
{
public:
	/**
	 * Construct object.
	 * Constructs an object of the fstream class. 
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 */
	fmstream();

	/**
	 * Construct object and open or create a file.
	 * Constructs an object of the fstream class. 
	 * This implies the initialization of the associated filemappingbuf object and the call to the constructor of its base class with the filemappingbuf object as parameter.
	 * The stream is associated with a physical file as if a call to the member function open with the same parameters was made.
	 * If the constructor is not successful in opening the file, the object is still created although no file is associated to the stream buffer and the stream's failbit is set (which can be checked with inherited member fail).
	 * @param path_name C-string contains the name of the file to be opened or created.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system. 
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see open()
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	explicit fmstream(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Destructs the fmstream object.
	 */
	virtual ~fmstream() {}

#ifdef _HAS_CPP11_
	/** @name C++11 
	 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
	 */
	///@{ 
	/**
	 * Move constructor (requires C++11). 
	 * Acquires the contents of rhs_stream, by move-assigning its members and base classes.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation. 
	 * @see swap()
	 * @see operator=()
	 */
	fmstream(fmstream&& rhs_stream);

	/**
	 * Move assignment (requires C++11).
	 * Closes the source stream (as if member close was called), and then acquires the contents of rhs_stream.
	 * @param rhs_stream File stream to move. rhs_stream becomes of invalid state after the operation. 
	 * @return *this.
	 * @see swap()
	 */
	fmstream& operator=(fmstream&& rhs_stream);

	/**
	 * Swap internals (requires C++11).
	 * Exchanges the state of the stream with those of other.
	 * @param stream fmstream to exchange the state with. 
	 * @see operator=()
	 */
	void swap(fmstream& stream);
	///@}
#endif // _HAS_CPP11_

	/**
	 * Get the associated filemappingbuf object.
	 * Returns a pointer to the filemappingbuf object associated with the stream.
	 * @return A pointer to the filemappingbuf object associated with the stream.
	 * Notice that for any successfully constructed fmstream object this pointer is never a null pointer, even if no files have been opened or if the stream is unbuffered.
	 */
	filemappingbuf* rdbuf() const;

	/**
	 * Check if a file is open.
	 * Returns true if the stream is currently associated with a file, and false otherwise.
	 * The stream is associated with a file if either a previous call to member open succeeded or if the object was successfully constructed using the parameterized constructor, and close has not been called since.
	 * @return true if a file is open, i.e. associated to this stream object. false otherwise.
	 * @see open()
	 * @see close()
	 */
	bool is_open() const;

	/**
	 * Open file.
	 * Opens a file whose name is path_name, associating its content with the stream object to perform input/output operations on it. 
	 * The operations allowed and some operating details depend on parameter mode.
	 * If the object already has a file associated (open), this function fails.
	 * If the file do not exist, the file is created with the 'offset + max_length' size.
	 * If the size of the opened file is less than 'offset + max_length', the file growing.
	 * If the size of the opened file is greater than 'offset + max_length', the file is not truncated.
	 * An attempt to map a file with a length of 0 fails.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param path_name C-string contains the name of the file to be opened.
	 * @param max_length Maximum length of the file mapping. If this parameter are 0, the maximum length of the file mapping object is equal to the current size of the file.
	 * @param offset File offset where the mapping begins. They must also match the memory allocation granularity of the system. 
	 * That is, the offset must be a multiple of the allocation granularity. To obtain the offset granularity of the system, use filemapping::offset_granularity().
	 * @see is_open()
	 * @see close()
	 * @see filemapping::offset_granularity()
	 */
	void open(const char* path_name, std::streamsize max_length = 0, std::streamoff offset = 0);

	/**
	 * Close file.
	 * Closes the file currently associated with the object and disassociates it.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @see open()
	 * @see is_open()
	 */
	void close();

	/**
	 * Get position of the get pointer.
	 * The get pointer determines the next location in the input sequence to be read by the next input operation.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return Return the address of the get pointer. In case of failure, a null pointer is returned.
	 * @see ptellp()
	 * @see pseekg()
	 * @see std::istream::tellg()
	 */
	const void* ptellg();

	/**
	 * Get position of the put pointer.
	 * The put pointer determines the location in the output sequence where the next output operation is going to take place.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return Return the address of the put pointer. In case of failure, a null pointer is returned.
	 * @see ptellg()
	 * @see pseekp()
	 * @see std::istream::tellp()
	 */
	void* ptellp();

	/**
	 * Sets the position of the get pointer.
	 * The get pointer determines the next location to be read in the source associated to the stream.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param ptr New absolute position for the input pointer.
	 * @return The function returns *this.
	 * @see pseekp()
	 * @see ptellg()
	 * @see std::istream::seekg()
	 */
	std::istream& pseekg(const void* ptr);

	/**
	 * Sets the position of the put pointer.
	 * The put pointer determines the location in the output sequence where the next output operation is going to take place.
	 * The function fails if no file is currently open (associated) with this object.
	 * On failure, the failbit flag is set (which can be checked with member fail), and depending on the value set with exceptions an exception may be thrown.
	 * @param ptr New absolute position for the output pointer.
	 * @return The function returns *this.
	 * @see pseekg()
	 * @see ptellp()
	 * @see std::istream::seekp()
	 */
	std::ostream& pseekp(void* ptr);

	/**
	 * Get the read-only base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the constant base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	const void* data() const;

	/**
	 * Get the base address of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the base address of the mapping. In case of failure, a null pointer is returned.
	 * @see size()
	 */
	void* data();

	/**
	 * Get the maximum byte length of the mapping.
	 * The function fails if no file is currently open (associated) with this object.
	 * @return In case of success, returns the length of the mapping. In case of failure, 0 is returned.
	 * @see data()
	 */
	std::streamsize size() const;

private:
	/**
	 * Copy constructor is private: this class is not copyable.
	 */
	fmstream(const fmstream&);

	/**
	 * Copy operator is private: this class is not copyable.
	 */
	fmstream& operator=(const fmstream&);

private:
	filemappingbuf m_rdbuf; //!< filemappingbuf object
};

#ifdef _HAS_CPP11_
/** @name C++11 
 * The following methods requires some features introduced by the latest revision of the C++ standard (2011). Older compilers may not support it.
 */
///@{ 
/**
 * Swap two filemappingbuf (requires C++11).
 * Overloads the std::swap algorithm for filemappingbuf. Exchanges the state of lhs with that of rhs. 
 * Effectively calls lhs.swap(rhs).
 * @param lhs filemappingbuf to exchange the state with. 
 * @param rhs filemappingbuf to exchange the state with. 
 */
void swap(filemappingbuf& lhs, filemappingbuf& rhs);

/**
 * Swap two ifmstream (requires C++11).
 * Overloads the std::swap algorithm for ifmstream. Exchanges the state of lhs with that of rhs. 
 * Effectively calls lhs.swap(rhs).
 * @param lhs ifmstream to exchange the state with. 
 * @param rhs ifmstream to exchange the state with. 
 */
void swap(ifmstream& lhs, ifmstream& rhs);

/**
 * Swap two fmstream (requires C++11).
 * Overloads the std::swap algorithm for fmstream. Exchanges the state of lhs with that of rhs. 
 * Effectively calls lhs.swap(rhs).
 * @param lhs fmstream to exchange the state with. 
 * @param rhs fmstream to exchange the state with. 
 */
void swap(fmstream& lhs, fmstream& rhs);
///@}
#endif // _HAS_CPP11_

#endif /* FILE_MAPPING_STREAM_H_ */
