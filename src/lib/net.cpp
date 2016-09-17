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

#include <gcore/net.h>
#include <gcore/rex.h>
#include <gcore/threads.h>
#include <gcore/time.h>

#ifdef _MSC_VER
// 4127: conditional expression is constant (in FD_SET macro)
# pragma warning(disable: 4127)
#endif

#ifndef _WIN32
# include <signal.h>
class BlockSignal
{
public:
   BlockSignal(int sig, bool noop=false)
      : mRestoreMask(!noop)
   {
      if (!noop)
      {
         sigset_t mask;
         sigemptyset(&mask);
         sigaddset(&mask, sig);
         sigprocmask(SIG_BLOCK, &mask, &mOldMask);
      }
   }
   ~BlockSignal()
   {
      if (mRestoreMask)
      {
         sigprocmask(SIG_BLOCK, &mOldMask, NULL);
      }
   }
private:
   bool mRestoreMask;
   sigset_t mOldMask;
};
#else
class BlockSignal
{
public:
   BlockSignal(int, bool=false) {}
   ~BlockSignal() {}
};
#endif


namespace gcore
{

bool NetInitialize()
{
#ifdef _WIN32
   WSADATA wsadata;
   if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
   {
      return false;
   }
   // Confirm that the WinSock DLL supports 2.2
   if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
   {
      WSACleanup();
      return false;
   }
#endif
   return true;
}

void NetUninitialize()
{
#ifdef _WIN32
   WSACleanup();
#endif
}

// ---

static Rex IPAddressRE(RAW("\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}"));

Host::Host()
{
   memset(&mAddr, 0, sizeof(struct sockaddr_in));
}

Host::Host(const String &addr, unsigned short port, Status *status)
{
   memset(&mAddr, 0, sizeof(struct sockaddr_in));
   mAddr.sin_family = AF_INET;
   mAddr.sin_addr.s_addr = INADDR_ANY;
   mAddr.sin_port = htons((u_short)port);

   if (IPAddressRE.match(addr) == true)
   {
#ifdef _WIN32
      mAddr.sin_addr.s_addr = inet_addr(addr.c_str());
#else
      inet_aton(addr.c_str(), &(mAddr.sin_addr));
#endif
      
      if (status)
      {
         status->set(true);
      }
   }
   else
   {
      struct hostent *he = gethostbyname(addr.c_str());
      if (he == NULL)
      {
         std::ostringstream oss;
         oss << "" << addr << "\"";
         if (status)
         {
            status->set(false, "[gcore::Host::Host] Could not find host: \"%s\"", addr.c_str());
         }
      }
      else
      {
         memcpy(&(mAddr.sin_addr.s_addr), he->h_addr, he->h_length);
         if (status)
         {
            status->set(true);
         }
      }
   }
}

Host::Host(const Host &rhs)
{
   memcpy(&mAddr, &(rhs.mAddr), sizeof(struct sockaddr_in));
}

Host::~Host()
{
}

Host& Host::operator=(const Host &rhs)
{
   if (this != &rhs)
   {
      memcpy(&mAddr, &(rhs.mAddr), sizeof(struct sockaddr_in));
   }
   return *this;
}

unsigned short Host::port() const
{
   return ntohs(mAddr.sin_port);
}

String Host::address() const
{
   return inet_ntoa(mAddr.sin_addr);
}

Host::operator struct sockaddr* ()
{
   return (struct sockaddr*) &mAddr;
}

Host::operator const struct sockaddr* () const
{
   return (const struct sockaddr*) &mAddr;
}

// ---

Connection::Connection()
   : mFD(NULL_SOCKET)
   , mBufferSize(0)
   , mBuffer(0)
   , mBufferOffset(0)
{
   setBufferSize(512);
}

Connection::Connection(sock_t fd)
   : mFD(fd)
   , mBufferSize(0)
   , mBuffer(0)
   , mBufferOffset(0)
{
   setBufferSize(512);
}

Connection::~Connection()
{
   if (mBuffer)
   {
      if (mBufferOffset > 0)
      {
         std::cerr << "WARNING: Deleting Connection object that have pending data" << std::endl;
      }
      delete[] mBuffer;
      mBuffer = 0;
      mBufferSize = 0;
      mBufferOffset = 0;
   }
}

char* Connection::alloc(size_t sz, char *bytes)
{
   if (bytes)
   {
      return (char*) realloc((void*)bytes, sz);
   }
   else
   {
      return (char*) malloc(sz);
   }
}

void Connection::free(char *&bytes)
{
   if (bytes)
   {
      ::free(bytes);
      bytes = 0;
   }
}

bool Connection::isValid() const
{
   return (mFD != NULL_SOCKET);
}

void Connection::invalidate()
{
   mFD = NULL_SOCKET;
}

bool Connection::isAlive() const
{
   return (mFD != NULL_SOCKET);
}

void Connection::setBufferSize(unsigned long n)
{
   if (n > mBufferSize)
   {
      if (mBuffer)
      {
         delete[] mBuffer;
         mBuffer = 0;
      }
      mBuffer = new char[n];
      memset(mBuffer, 0, n*sizeof(char));
      mBufferOffset = 0;
   }
   mBufferSize = n;
}

bool Connection::setBlocking(bool blocking)
{
   if (!isValid())
   {
      return false;
   }
#ifdef _WIN32
   ULONG arg = (blocking ? 0 : 1);
   return (::ioctlsocket(mFD, FIONBIO, &arg) == 0);
#else
   int flags = ::fcntl(mFD, F_GETFL, NULL);
   if (blocking)
   {
      flags = flags & ~O_NONBLOCK;
   }
   else
   {
      flags = flags | O_NONBLOCK;
   }
   return (::fcntl(mFD, F_SETFL, flags) != -1);
#endif
}

bool Connection::setLinger(bool onoff)
{
   if (!isValid())
   {
      return false;
   }
#ifdef SO_LINGER
   struct linger l = {0, 0};
   socklen_t optlen = sizeof(struct linger);
   if (onoff)
   {
      // get current value to preserve timeout
      if (::getsockopt(mFD, SOL_SOCKET, SO_LINGER, (char*)&l, &optlen) != 0)
      {
         return false;
      }
   }
   l.l_onoff = (onoff ? 1 : 0);
   return (::setsockopt(mFD, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(struct linger)) == 0);
#else
   return false;
#endif
}

bool Connection::read(String &s, double timeout, Status *status)
{
   char *bytes = 0;
   size_t len = 0;
   bool rv = this->read(bytes, len, timeout, status);
   if (bytes != 0)
   {
      s = bytes;
      this->free(bytes);
   }
   else
   {
      s = "";
   }
   return rv;
}

bool Connection::readUntil(const char *until, String &s, double timeout, Status *status)
{
   char *bytes = 0;
   size_t len = 0;
   bool rv = this->readUntil(until, bytes, len, timeout, status);
   if (bytes != 0)
   {
      s = bytes;
      this->free(bytes);
   }
   else
   {
      s = "";
   }
   return rv;
}

size_t Connection::write(const String &s, double timeout, Status *status)
{
    return this->write(s.c_str(), s.length(), timeout, status);
}

// ---

TCPConnection::TCPConnection()
   : Connection()
   , mSocket(0)
{
}

TCPConnection::TCPConnection(TCPSocket *socket, sock_t fd, const Host &host)
   : Connection(fd)
   , mHost(host)
   , mSocket(socket)
{
}

TCPConnection::~TCPConnection()
{
}

bool TCPConnection::isValid() const
{
   return (Connection::isValid() && mSocket != NULL && mSocket->isValid());
}

void TCPConnection::invalidate()
{
   Connection::invalidate();
   mSocket = NULL;
}

bool TCPConnection::isAlive() const
{
   if (isValid())
   {
      char c = '\0';
      // Try to read something from mFD, 0 is returned if connection is closed.
      // This assumes that the connection is not blocking.
      // Use MSG_PEEK to avoid pulling data we're not supposed to
      return (recv(mFD, &c, 1, MSG_PEEK) != 0);
   }
   else
   {
      return false;
   }
}

bool TCPConnection::shutdownRead()
{
   if (isValid())
   {
      return (::shutdown(mFD, SHUT_RD) == 0);
   }
   return false;
}

bool TCPConnection::shutdownWrite()
{
   if (isValid())
   {
      return (::shutdown(mFD, SHUT_WR) == 0);
   }
   return false;
}

bool TCPConnection::shutdown()
{
   if (isValid())
   {
      return (::shutdown(mFD, SHUT_RDWR) == 0);
   }
   return false;
}

bool TCPConnection::read(char *&bytes, size_t &len, double timeout, Status *status)
{
   return readUntil(NULL, bytes, len, timeout, status);
}

bool TCPConnection::checkUntil(const char *until, char *in, size_t inlen, char *&out, size_t &outlen)
{
   if (until != NULL && in && inlen > 0)
   {
      // if out is set, in must be a substring
      if (out && (in < out || in + inlen > out + outlen))
      {
         return false;
      }
      
      size_t ulen = strlen(until);
      char *found = strstr(in, until);
      
      if (found != NULL)
      {
         size_t sublen = found + ulen - in;
         size_t rmnlen = inlen - sublen;
         
         if (!out)
         {
            out = this->alloc(sublen+1);
            outlen = sublen;
            memcpy(out, in, sublen);
            out[sublen] = '\0';
         }
         else
         {
            found[ulen] = '\0';
            outlen -= rmnlen;
         }
         
         // Keep any bytes after until in internal buffer
         mBufferOffset = (unsigned long)rmnlen;
         for (size_t i=0; i<rmnlen; ++i)
         {
            mBuffer[i] = mBuffer[sublen+i];
         }
         mBuffer[rmnlen] = '\0';
         
         return true;
      }
   }
   
   return false;
}

bool TCPConnection::readUntil(const char *until, char *&bytes, size_t &len, double timeout, Status *status)
{
   bytes = 0;
   len = 0;
   
   size_t allocated = 0;
   size_t searchOffset = 0;
   
   if (mBufferSize == 0)
   {
      setBufferSize(512);
   }
   else if (mBufferOffset > 0)
   {
      len = mBufferOffset;
      bytes = this->alloc(len + 1);
      memcpy(bytes, mBuffer, len);
      bytes[len] = '\0';
      
      mBufferOffset = 0;
      
      allocated = len;
      searchOffset = 0;
   }
   
   if (checkUntil(until, bytes, len, bytes, len))
   {
      if (status)
      {
         status->set(true);
      }
      return true;
   }
   
   if (!isValid())
   {
      if (len > 0)
      {
         if (status)
         {
            status->set(true);
         }
         return (until ? false : true);
      }
      else
      {
         if (status)
         {
            status->set(false, sock_errno(), "[gcore::TCPConnection::readUntil] Invalid connection.");
         }
         return false;
      }
   }
   
   int n = 0;
   bool cont = false;
   TimeCounter st(TimeCounter::MilliSeconds);
   
   do
   {
      if (timeout > 0)
      {
         if (st.elapsed().value() > timeout)
         {
            if (status)
            {
               status->set(true);
            }
            return (until ? false : (len > 0));
         }
      }
      
      // If socket is not set to be non-blocking, we'll be stuck here until
      // data comes in, defeating the purpose of 'timeout'
      // Be sure to call 'setBlocking(false)' before make use of the method with timeout
      // The TCPSocket class will do it on all the TCPConnection intances it creates
      n = recv(mFD, mBuffer+mBufferOffset, mBufferSize-mBufferOffset, 0);
      
      if (n == -1)
      {
         // There's no guaranty that EWOULDBLOCK == EAGAIN
         int err = sock_errno();
#ifdef _WIN32
         bool retry = (err == WSAEWOULDBLOCK);
#else
         bool retry = (err == EAGAIN || err == EWOULDBLOCK);
#endif
         if (retry)
         {
            if (timeout == 0)
            {
               if (status)
               {
                  status->set(true);
               }
               return (until ? false : (len > 0));
            }
            else
            {
               if (timeout < 0)
               {
                  // Blocking read
                  if (len > 0)
                  {
                     // We already have data to return
                     if (status)
                     {
                        status->set(true);
                     }
                     return (until ? false : true);
                  }
                  else
                  {
                     // Sleep 50ms before trying again
                     Thread::SleepCurrent(50);
                  }
               }
               cont = true;
               continue;
            }
         }
         else
         {
            if (status)
            {
               status->set(false, sock_errno(), "[gcore::TCPConnection::readUntil] Failed to read from socket.");
            }
            return (until ? false : (len > 0));
         }
      }
      
      if (n == 0)
      {
         // Connection closed
         mSocket->close(this);
         if (status)
         {
            status->set(false, "[gcore::TCPConnection::readUntil] Connection remotely closed.");
         }
         return (until ? false : (len > 0));
      }
      
      cont = (n == int(mBufferSize - mBufferOffset));
      mBufferOffset = 0;
      
      if (bytes == 0)
      {
         len = n;
         allocated = n;
         bytes = this->alloc(allocated+1);
         memcpy(bytes, mBuffer, n);
         bytes[len] = '\0';
         searchOffset = 0;
      }
      else
      {
         if ((len + n) >= allocated)
         {
            while (allocated < (len + n))
            {
               allocated <<= 1;
            }
            bytes = this->alloc(allocated+1, bytes);
         }
         memcpy(bytes+len, mBuffer, n);
         searchOffset = len;
         len += n;
         bytes[len] = '\0';
      }
      
      if (checkUntil(until, bytes + searchOffset, size_t(n), bytes, len))
      {
         if (status)
         {
            status->set(true);
         }
         return true;
      }
      else
      {
         if (until)
         {
            cont = true;
         }
      }
      
   } while (cont);
   
   if (status)
   {
      status->set(true);
   }
   return true;
}

size_t TCPConnection::write(const char *bytes, size_t len, double timeout, Status *status)
{
   if (!isValid())
   {
      if (status)
      {
         status->set(false, "[gcore::TCPConnection::write] Invalid connection.");
      }
      return 0;
   }
   
   if (len == 0)
   {
      if (status)
      {
         status->set(true);
      }
      return 0;
   }
   
   int offset = 0;
   int remaining = int(len);
   
   TimeCounter st(TimeCounter::MilliSeconds);
   
   while (remaining > 0)
   {
      if (timeout > 0)
      {
         if (st.elapsed().value() > timeout)
         {
            if (status)
            {
               status->set(true);
            }
            return size_t(offset);
         }
      }
      
      // If connection is remotely closed, 'send' will result in a SIGPIPE signal on unix systems
      
      /*
      // Using isAlive seems to respond faster to remove connection close
      // But it can be blocking!
      if (!isAlive()) {
         mSocket->close(this);
         if (status) {
            status->set(false, "[gcore::TCPConnection::write] Connection remotely closed.");
         }
         return size_t(offset);
      }
      int n = send(mFD, bytes+offset, remaining, 0);
      */
      
      int n = -1;
      int flags = 0;
      {
#ifdef MSG_NOSIGNAL
         flags = MSG_NOSIGNAL;
#else // MSG_NOSIGNAL
# ifdef SIGPIPE
         bool noop = false;
#  ifdef SO_NOSIGPIPE
         int nosigpipe = 0;
         socklen_t optlen = sizeof(int);
         if (!getsockopt(mFD, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, &optlen))
         {
            noop = (nosigpipe != 0);
         }
#  endif // SO_SIGPIPE
         BlockSignal _sbs(SIGPIPE, noop);
# else // SIGPIPE
         // No workaround SIGPIPE signal
# endif // SIGPIPE
#endif // MSG_NOSIGNAL
         n = send(mFD, bytes+offset, remaining, flags);
      }

      if (n == -1)
      {
         int err = sock_errno();
#ifdef _WIN32
         if (err == WSAEWOULDBLOCK)
#else
         if (err == EAGAIN || err == EWOULDBLOCK)
#endif
         {
            if (timeout == 0)
            {
               if (status)
               {
                  status->set(true);
               }
               return size_t(offset);
            }
            else
            {
               if (timeout < 0)
               {
                  // Blocking write -> Sleep 50ms before trying again
                  Thread::SleepCurrent(50);
               }
               continue;
            }
         }
         else
         {
            // Should notify socket ?
#ifdef _WIN32
            if (err == WSAECONNRESET || err == WSAECONNABORTED)
#else
            // why did I check for err == 0
            //if (err == 0 || err == EPIPE) {
            if (err == EPIPE)
#endif
            {
               mSocket->close(this);
               
               if (status)
               {
                  status->set(false, "[gcore::TCPConnection::write] Connection remotely closed.");
               }
            }
            else
            {
               if (status)
               {
                  status->set(false, sock_errno(), "[gcore::TCPConnection::write] Failed to write to socket.");
               }
            }
            
            return size_t(offset);
         }
      }
      else
      {
         remaining -= n;
         offset += n;
      }
   }
   
   if (status)
   {
      status->set(true);
   }
   return size_t(offset);
}

// ---

Socket::Socket()
   : mFD(NULL_SOCKET)
{
}

Socket::Socket(unsigned short port, Status *status)
   : mFD(NULL_SOCKET)
   , mHost("localhost", port, status)
{
}

Socket::Socket(const Host &host, Status *status)
   : mFD(NULL_SOCKET)
   , mHost(host)
{
   if (status)
   {
      status->set(true);
   }
}

Socket::Socket(sock_t fd, const Host &host)
   : mFD(fd)
   , mHost(host)
{
}

Socket::Socket(const Socket &rhs)
   : mFD(NULL_SOCKET)
   , mHost(rhs.mHost)
{
}

Socket::~Socket()
{
}

bool Socket::isValid() const
{
   return (mFD != NULL_SOCKET);
}

void Socket::invalidate()
{
   mFD = NULL_SOCKET;
}

Socket& Socket::operator=(const Socket&)
{
   return *this;
}

// ---

TCPSocket::TCPSocket()
   : Socket()
   , mMaxConnections(0)
   , mDefaultBlocking(false)
   , mDefaultLinger(true)
{
}

TCPSocket::TCPSocket(unsigned short port, Status *status)
   : Socket(port, status)
   , mMaxConnections(0)
   , mDefaultBlocking(false)
   , mDefaultLinger(true)
{
   mFD = ::socket(AF_INET, SOCK_STREAM, 0);
   if (status)
   {
      if (mFD == NULL_SOCKET)
      {
         status->set(false, sock_errno(), "[gcore::TCPSocket::TCPSocket]");
      }
   }
   mCurReadConnection = mReadConnections.end();
   mCurWriteConnection = mWriteConnections.end();
}

TCPSocket::TCPSocket(const Host &host, Status *status)
   : Socket(host, status)
   , mMaxConnections(0)
   , mDefaultBlocking(false)
   , mDefaultLinger(true)
{
   mFD = ::socket(AF_INET, SOCK_STREAM, 0);
   if (status)
   {
      if (mFD == NULL_SOCKET)
      {
         status->set(false, sock_errno(), "[gcore::TCPSocket::TCPSocket]");
      }
   }
   mCurReadConnection = mReadConnections.end();
   mCurWriteConnection = mWriteConnections.end();
}

TCPSocket::TCPSocket(const TCPSocket &rhs)
   : Socket(rhs)
   , mMaxConnections(0)
   , mDefaultBlocking(false)
   , mDefaultLinger(true)
{
}

TCPSocket::~TCPSocket()
{
   close();
   // 'close' doesn't destroy Connection objects (on purpose)
   for (ConnectionList::iterator it=mConnections.begin(); it!=mConnections.end(); ++it)
   {
      delete *it;
   }
   mConnections.clear();
}

TCPSocket& TCPSocket::operator=(const TCPSocket&)
{
   return *this;
}

Status TCPSocket::bind()
{
   if (::bind(mFD, mHost, sizeof(struct sockaddr)) == 0)
   {
      return Status(true);
   }
   else
   {
      return Status(false, sock_errno(), "[gcore::TCPSocket::bind]");
   }
}

Status TCPSocket::listen(int maxConnections)
{
   if (::listen(mFD, maxConnections) == 0)
   {
      mMaxConnections = maxConnections;
      return Status(true);
   }
   else
   {
      mMaxConnections = 0;
      return Status(false, sock_errno(), "[gcore::TCPSocket::listen]");
   }
}

Status TCPSocket::bindAndListen(int maxConnections)
{
   Status stat = this->bind();
   if (stat)
   {
      stat = this->listen(maxConnections);
   }
   return stat;
}

void TCPSocket::clearEvents()
{
#ifdef _WIN32
   for (size_t i=0; i<mEvents.size(); ++i)
   {
      WSACloseEvent(mEvents[i]);
   }
   mEvents.clear();
   mEventConns.clear();
#endif
}

void TCPSocket::close()
{
   closeConnections();
   
   if (isValid())
   {
      ::shutdown(mFD, SHUT_RDWR);
      sock_close(mFD);
      invalidate();
   }
}

void TCPSocket::closeConnections()
{
   for (ConnectionList::iterator it=mConnections.begin(); it!=mConnections.end(); ++it)
   {
      close(*it);
   }
   mReadConnections.clear();
   mWriteConnections.clear();
   mCurReadConnection = mReadConnections.end();
   mCurWriteConnection = mWriteConnections.end();
   clearEvents();
}

void TCPSocket::close(TCPConnection *conn)
{
   if (conn && conn->isValid())
   {
      bool self = (conn->fd() == mFD);
      
      conn->shutdown();
      sock_close(conn->fd());
      
      conn->invalidate();
      if (self)
      {
         invalidate();
      }
   }
}

void TCPSocket::cleanup(bool flushPending)
{
   ConnectionList::iterator it1 = mConnections.begin();
   ConnectionList::iterator it2;
   
   while (it1 != mConnections.end())
   {
      TCPConnection *conn = *it1;
      
      if (!conn->isValid() && (flushPending || !conn->hasPendingData()))
      {
         // Remove from readable connections
         it2 = std::find(mReadConnections.begin(), mReadConnections.end(), conn);
         if (it2 != mReadConnections.end())
         {
            bool upd = (mCurReadConnection == it2);
            it2 = mReadConnections.erase(it2);
            if (upd)
            {
               mCurReadConnection = it2;
            }
         }
         
         // Remove from writable connections
         it2 = std::find(mWriteConnections.begin(), mWriteConnections.end(), conn);
         if (it2 != mWriteConnections.end())
         {
            bool upd = (mCurWriteConnection == it2);
            it2 = mWriteConnections.erase(it2);
            if (upd)
            {
               mCurWriteConnection = it2;
            }
         }
         
         // Remove from connections
         it1 = mConnections.erase(it1);
         
         // Delete object
         delete conn;
      }
      else
      {
         ++it1;
      }
   }
}

void TCPSocket::setDefaultBlocking(bool blocking)
{
   mDefaultBlocking = blocking;
}

void TCPSocket::setDefaultLinger(bool linger)
{
   mDefaultLinger = linger;
}

void TCPSocket::setup(TCPConnection *conn)
{
   conn->setBlocking(mDefaultBlocking);
   conn->setLinger(mDefaultLinger);
#ifdef SO_NOSIGPIPE
   int nosigpipe = 1;
   setsockopt(mFD, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(int));
#endif
}

TCPConnection* TCPSocket::connect(Status *status)
{
   cleanup();
   
   socklen_t len = sizeof(struct sockaddr);
   
   if (::connect(mFD, mHost, len) != 0)
   {
      if (status)
      {
         status->set(false, sock_errno(), "[gcore::TCPSocket::connect] Failed to establish connection.");
      }
      return 0;
   }
   
   mConnections.push_back(new TCPConnection(this, mFD, mHost));
   
   TCPConnection *conn = mConnections.back();
   setup(conn);
   
   if (status)
   {
      status->set(true);
   }
   
   return conn;
}

TCPConnection* TCPSocket::accept(Status *status)
{
   cleanup();
   
   Host h;
   
   socklen_t len = sizeof(struct sockaddr_in);
   
   sock_t fd = ::accept(mFD, h, &len);
   
   if (fd == NULL_SOCKET)
   {
      if (status)
      {
         status->set(false, sock_errno(), "[gcore::TCPSocket::accept] Failed to accept connection.");
      }
      return 0;
   }
   
   mConnections.push_back(new TCPConnection(this, fd, h));
   
   TCPConnection *conn = mConnections.back();
   setup(conn);
   
   if (status)
   {
      status->set(true);
   }
   
   return conn;
}

bool TCPSocket::toTimeval(double ms, struct timeval &tv) const
{
   if (ms < 0)
   {
      return false;
   }
   
   if (ms > 0)
   {
      double total = TimeCounter::ConvertUnits(ms, TimeCounter::MilliSeconds, TimeCounter::Seconds);
      double secs = floor(total);
      double remain = total - secs;
      double usecs = floor(0.5 + TimeCounter::ConvertUnits(remain, TimeCounter::Seconds, TimeCounter::MicroSeconds));
      
      tv.tv_sec = (long) secs;
      tv.tv_usec = (long) usecs;
   }
   else
   {
      tv.tv_sec = 0;
      tv.tv_usec = 0;
   }
   
   return true;
}

int TCPSocket::peek(bool readable, bool writable, double timeout, fd_set *readfds, fd_set *writefds)
{
   fd_set _readfds;
   fd_set _writefds;
   
   if (!readable && !writable)
   {
      return 0;
   }
   
   if (readable && !readfds)
   {
      readfds = &_readfds;
   }
   
   if (writable && !writefds)
   {
      writefds = &_writefds;
   }
   
   int ret = 0;
   
   //cleanup();
   
#ifdef _WIN32
   
   // select() on windows would just not work
   // => WSAENOBUFS error right away
   
   if (readable)
   {
      FD_ZERO(readfds);
   }
   
   if (writable)
   {
      FD_ZERO(writefds);
   }
   
   ConnectionList::const_iterator eit = mConnections.end();
   
   clearEvents();
   
   mEvents.reserve(1 + mConnections.size());
   mEventConns.reserve(1 + mConnections.size());
   
   if (readable)
   {
      mEventConns.push_back(eit);
      mEvents.push_back(WSACreateEvent());
      WSAEventSelect(mFD, mEvents.back(), FD_ACCEPT|FD_CLOSE); // FD_CONNECT
   }
   
   for (ConnectionList::const_iterator it=mConnections.begin(); it!=eit; ++it)
   {
      TCPConnection *conn = *it;
      if (conn->isValid())
      {
         long flags = FD_CLOSE;
         if (readable)
         {
            flags = flags | FD_READ;
         }
         if (writable)
         {
            flags = flags | FD_WRITE;
         }
         mEventConns.push_back(it);
         mEvents.push_back(WSACreateEvent());
         WSAEventSelect(conn->fd(), mEvents.back(), flags);
      }
   }
   
   DWORD sz = (DWORD) mEvents.size();
   if (sz == 0)
   {
      return 0;
   }
   
   DWORD to = (timeout < 0 ? WSA_INFINITE : (DWORD) floor(timeout + 0.5));
   DWORD rv = WSAWaitForMultipleEvents(sz, &mEvents[0], FALSE, to, FALSE);
   
   if (rv == WSA_WAIT_FAILED)
   {
      ret = -1;
   }
   else if (timeout >= 0 && rv == WSA_WAIT_TIMEOUT)
   {
      ret = 0;
   }
   else if (rv >= WSA_WAIT_EVENT_0 && rv < (WSA_WAIT_EVENT_0 + sz))
   {
      size_t fidx = rv - WSA_WAIT_EVENT_0;
      ConnectionList::const_iterator cit;
      WSANETWORKEVENTS nevts;
      
      for (size_t eidx=fidx; eidx<mEvents.size(); ++eidx)
      {
         rv = WSAWaitForMultipleEvents(1, &mEvents[eidx], TRUE, 0, FALSE);
         if (rv != WSA_WAIT_FAILED)
         {
            cit = mEventConns[eidx];
            sock_t fd = (cit == eit ? mFD : (*cit)->fd());
            WSAEnumNetworkEvents(fd, mEvents[eidx], &nevts);
            if (readable && (nevts.lNetworkEvents & (FD_READ|FD_ACCEPT|FD_CLOSE)) != 0)
            {
               FD_SET(fd, readfds);
               ++ret;
            }
            if (writable && (nevts.lNetworkEvents & FD_WRITE) != 0)
            {
               FD_SET(fd, writefds);
               ++ret;
            }
         }
      }
   }
   else
   {
      // unhandled error
      ret = -1;
   }
   
#else
   
   struct timeval _tv;
   struct timeval *tv = 0;
   if (toTimeval(timeout, _tv))
   {
      tv = &_tv;
   }
   
   int curfd, maxfd = -1;
   if (readable)
   {
      FD_ZERO(readfds);
      FD_SET(mFD, readfds);
      maxfd = int(mFD);
   }
   
   if (writable)
   {
      FD_ZERO(writefds);
      FD_SET(mFD, writefds);
      maxfd = int(mFD);
   }
   
   for (ConnectionList::const_iterator it=mConnections.begin(); it!=mConnections.end(); ++it)
   {
      TCPConnection *conn = *it;
      
      if (conn->isValid())
      {
         if (readable)
         {
            FD_SET(conn->fd(), readfds);
         }
         
         if (writable)
         {
            FD_SET(conn->fd(), writefds);
         }
         
         curfd = (int) conn->fd();
         
         if (curfd > maxfd)
         {
            maxfd = curfd;
         }
      }
   }
   
   ret = ::select(maxfd+1, (readable ? readfds : NULL), (writable ? writefds : NULL), NULL, tv);
   
#endif
   
   // should add connections when pending data in their internal buffer
   // even if invalid or not ready to read
   if (ret != -1 && readable)
   {
      for (ConnectionList::const_iterator it=mConnections.begin(); it!=mConnections.end(); ++it)
      {
         TCPConnection *conn = *it;
         if (conn->hasPendingData())
         {
            if (conn->isValid())
            {
               if (FD_ISSET(conn->fd(), readfds))
               {
                  // already selected
               }
               else
               {
                  FD_SET(conn->fd(), readfds);
                  ++ret;
               }
            }
            else
            {
               ++ret;
            }
         }
      }
   }
   
   return ret;
}

size_t TCPSocket::select(bool readable, bool writable, double timeout, Status *status)
{
   mReadConnections.clear();
   mWriteConnections.clear();
   mCurReadConnection = mReadConnections.end();
   mCurWriteConnection = mWriteConnections.end();
   
   if (!readable && !writable)
   {
      if (status)
      {
         status->set(true);
      }
      return 0;
   }
   
   fd_set readfds, writefds;
      
   int rv = peek(readable, writable, timeout, &readfds, &writefds);
   
   if (rv == -1)
   {
      if (status)
      {
         status->set(false, sock_errno(), "[gcore::TCPSocket::select]");
      }
      return 0;
   }
   
   if (readable)
   {
      if (FD_ISSET(mFD, &readfds))
      {
         // Note: this could fail!
         this->accept();
      }
   }
   
   if (writable)
   {
      if (FD_ISSET(mFD, &writefds))
      {
         // Nothing special to do here?
      }
   }
   
   for (ConnectionList::iterator it=mConnections.begin(); it!=mConnections.end(); ++it)
   {
      TCPConnection *conn = *it;
      if (!conn->isValid())
      {
         if (readable && conn->hasPendingData())
         {
            mReadConnections.push_back(conn);
         }
         continue;
      }
      if (readable)
      {
         if (FD_ISSET(conn->fd(), &readfds) || conn->hasPendingData()) {
            mReadConnections.push_back(conn);
         }
      }
      if (writable)
      {
         if (FD_ISSET(conn->fd(), &writefds)) {
            mWriteConnections.push_back(conn);
         }
      }
   }
   
   mCurReadConnection = mReadConnections.begin();
   mCurWriteConnection = mWriteConnections.begin();
   
   if (status)
   {
      status->set(true);
   }
   
   return (mReadConnections.size() + mWriteConnections.size());
}

TCPConnection* TCPSocket::nextReadable()
{
   TCPConnection *rv = NULL;
   while (mCurReadConnection != mReadConnections.end())
   {
      TCPConnection *conn = *mCurReadConnection;
      if (conn->isValid() || conn->hasPendingData())
      {
         rv = conn;
      }
      ++mCurReadConnection;
      if (rv)
      {
         break;
      }
   }
   return rv;
}

TCPConnection* TCPSocket::nextWritable()
{
   TCPConnection *rv = NULL;
   while (mCurWriteConnection != mWriteConnections.end())
   {
      TCPConnection *conn = *mCurWriteConnection;
      if (conn->isValid())
      {
         rv = conn;
      }
      ++mCurWriteConnection;
      if (rv)
      {
         break;
      }
   }
   return rv;
}

} // gcore

