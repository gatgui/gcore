/*

Copyright (C) 2009~  Gaetan Guidet

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

#ifndef __gcore_net_h__
#define __gcore_net_h__

#include <gcore/config.h>
#include <gcore/platform.h>
#include <gcore/status.h>

namespace gcore {
   
  GCORE_API bool NetInitialize();
  GCORE_API void NetUninitialize();
  
  class GCORE_API NetScopeInit {
    public:
      inline NetScopeInit() {
        mInitialized = NetInitialize();
      }
      inline ~NetScopeInit() {
        if (mInitialized) {
          NetUninitialize();
        }
      }
      inline operator bool() const {
        return mInitialized;
      }
    private:
      bool mInitialized;
  };
  
  // ---
  
  class GCORE_API Host {
    public:
      
      Host();
      Host(const std::string &addr, unsigned short port, Status *status=0);
      Host(const Host &rhs);
      ~Host();

      Host& operator=(const Host &rhs);

      unsigned short port() const;
      std::string address() const;

      operator struct sockaddr* ();
      operator const struct sockaddr* () const;

    protected:
      struct sockaddr_in mAddr;
  };
  
  // ---
  
  class GCORE_API Connection {
    public:
      
      Connection();
      virtual ~Connection();
      
      
      inline sock_t fd() const { return mFD; }
      
      bool setBlocking(bool blocking);
      bool setLinger(bool linger);
      
      
      virtual bool isValid() const;
      virtual void invalidate();
      // isAlive default behaviour is identical to isValid
      virtual bool isAlive() const;
      
      
      // Arguments
      //   [out] bytes    : Pointer to read bytes
      //   [out] len      : Length of read bytes buffer
      //   [in] timeout   : Read timeout
      //                    A value < 0 will result in a 'blocking' read.
      //                    A value of 0 will make the function return immediately after the read, no matter the result.
      //                    A value > 0 will make the function try reading for at least the specified amount of time in milliseconds.
      //   [out] status   : Error status
      //
      // Return value
      //   true when anything read, even when call ends up with an error
      //
      // Note
      //   'bytes', if allocated, MUST be freed by the caller no matter the return value or error status
      virtual bool read(char *&bytes, size_t &len, double timeout=-1, Status *status=0) = 0;
      // Arguments
      //   See 'read'
      //   [in] until : character string to look for in read data (included in result)
      //                when set to NULL, 'readUntil' behaves as 'read'
      //
      // Return value
      //   true if the 'until' string was from in read bytes, false otherwise
      //
      // Note
      //   when false is returned, it doesn't mean nothing was read, be sure to check the 'bytes' buffer
      virtual bool readUntil(const char *until, char *&bytes, size_t &len, double timeout=-1, Status *status=0) = 0;
      virtual size_t write(const char* bytes, size_t len, double timeout=-1, Status *status=0) = 0;
      
      bool read(std::string &s, double timeout=-1, Status *status=0);
      bool readUntil(const char *until, std::string &s, double timeout=-1, Status *status=0);
      size_t write(const std::string &s, double timeout=-1, Status *status=0);
      
      // If bytes is non NULL, alloc will try re-allocating them
      // bytes must have been allocated by 'alloc' method
      char* alloc(size_t sz, char *bytes=0);
      // Use this method to release memory allocated by 'read', 'readUntil' or 'alloc' methods
      void free(char *&bytes);
      
      void setBufferSize(unsigned long n);
      inline unsigned long bufferSize() const { return mBufferSize; }
      inline bool hasPendingData() const { return (mBufferOffset > 0); }
      
    private:
      
      Connection(const Connection&);
      Connection& operator=(const Connection &);
      
    protected:
      
      Connection(sock_t fd);
      
      sock_t mFD;
      unsigned long mBufferSize;
      char *mBuffer;
      unsigned long mBufferOffset;
  };
  
  // ---
  
  class GCORE_API TCPSocket;
  
  class GCORE_API TCPConnection : public Connection {
    public:
      
      friend class TCPSocket;
      
    public:
      
      virtual ~TCPConnection();
      
      
      inline const Host& host() const { return mHost; }
      bool readShutdown();
      bool writeShutdown();
      bool shutdown();
      
      
      virtual bool isValid() const;
      virtual void invalidate();
      // isAlive will try to peek read on connection socket
      // If connection is blocking, so will isAlive be
      virtual bool isAlive() const;
      
      // Need to add those the std::string overrides of read, readUntil and write are
      //   available to TCPConnection class instances
      // (overrides only work in one scope at a time)
      using Connection::read;
      using Connection::readUntil;
      using Connection::write;
      
      virtual bool read(char *&bytes, size_t &len, double timeout=-1, Status *status=0);
      virtual bool readUntil(const char *until, char *&bytes, size_t &len, double timeout=-1, Status *status=0);
      virtual size_t write(const char* bytes, size_t len, double timeout=-1, Status *status=0);
      
      
    private:
      
      TCPConnection();
      TCPConnection(const TCPConnection&);
      TCPConnection& operator=(const TCPConnection&);
      
      bool checkUntil(const char *until, char *in, size_t inlen, char *&out, size_t &outlen);
      
    protected:
      
      TCPConnection(TCPSocket *socket, sock_t fd, const Host &host);
      
      Host mHost;
      TCPSocket *mSocket;
  };
  
  // class GCORE_API UDPConnection : public Connection { ...
  
  // ---
  
  class GCORE_API Socket {
    public:
      
      friend class Connection;
    
    public:
      
      Socket(unsigned short port, Status *status=0);
      Socket(const Host &host, Status *status=0);
      virtual ~Socket();
      
      bool isValid() const;
      
      inline const Host& host() const { return mHost; }
      inline sock_t fd() const { return mFD; }
  
    protected:
      
      Socket();
      Socket(const Socket&);
      Socket& operator=(const Socket&);
      
      void invalidate();
      
    protected:
      
      Socket(sock_t fd, const Host &host);
      
      sock_t mFD;
      Host mHost;
  };
  
  // ---
  
  class GCORE_API TCPSocket : public Socket {
    public:
      
      friend class TCPConnection;
    
    public:
      
      TCPSocket(unsigned short port, Status *status=0);
      TCPSocket(const Host &host, Status *status=0);
      virtual ~TCPSocket();
      
      
      void setDefaultBlocking(bool blocking);
      void setDefaultLinger(bool linger);
      
      
      Status bind();
      Status listen(int maxConnections);
      Status bindAndListen(int maxConnections);
      
      // Arguments
      //   [in] timeout : Select timeout.
      //                  A value < 0 will make the call blocking.
      //                  A value if 0 will make the select call returns immediately
      //                  A value > 0 will make the call monitor the connections for the specified amount of time in milliseconds
      //   [out] status : Error status
      // 
      // Return value
      //   Number of connections that can be processed
      inline size_t select(double timeout=-1, Status *status=0) { return this->select(true, true, timeout, status); }
      inline size_t selectReadable(double timeout=-1, Status *status=0) { return this->select(true, false, timeout, status); }
      inline size_t selectWritable(double timeout=-1, Status *status=0) { return this->select(false, true, timeout, status); }
      // Use the following to iterate over selected connections
      TCPConnection* nextReadable();
      TCPConnection* nextWritable();
      
      // Arguments
      //   As select
      // 
      // Return value
      //   -1 on error
      //   number of sockets ready to operate otherwise
      // 
      // Note
      //   The difference with select is that peek doen't populate the read/write connection sets
      inline int peek(double timeout=-1) { return this->peek(true, true, timeout); }
      inline int peekReadable(double timeout=-1) { return this->peek(true, false, timeout); }
      inline int peekWritable(double timeout=-1) { return this->peek(false, true, timeout); }
      
      TCPConnection* accept(Status *status=0);
      TCPConnection* connect(Status *status=0);
      
      // Note: The close methods won't free the Connection objects
      //       Use cleanup method to remove
      // close all connections and the socket itself
      void close();
      // close all connections only
      void closeConnections();
      // close a single connection
      void close(TCPConnection*);
      
      // Cleanup closed connections objects
      // 
      // Arguments
      //   [in] flushPending : Closed connection may still have remaining data in their internal read buffer.
      //                       Set to false, cleanup will keep suchs so that data can still be queried.
      void cleanup(bool flushPending=false);
      
      
    protected:
      
      TCPSocket();
      TCPSocket(const TCPSocket&);
      TCPSocket& operator=(const TCPSocket&);
      
      void setup(TCPConnection *conn);
      bool toTimeval(double ms, struct timeval &tv) const;
      size_t select(bool readable, bool writable, double timeout, Status *status=0);
      int peek(bool readable, bool writable, double timeout, fd_set *readfds=0, fd_set *writefds=0);
      void clearEvents();
    
    protected:
      
      int mMaxConnections;
      
      typedef std::list<TCPConnection*> ConnectionList;
      
      ConnectionList mConnections;
      ConnectionList mReadConnections;
      ConnectionList mWriteConnections;
      ConnectionList::iterator mCurReadConnection;
      ConnectionList::iterator mCurWriteConnection;
      bool mDefaultBlocking;
      bool mDefaultLinger;
      
#ifdef _WIN32
      std::vector<WSAEVENT> mEvents;
      std::vector<ConnectionList::const_iterator> mEventConns;
#endif
  };
  
  // class UDPSocket : public Socket { ...
}

#endif
