// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_TCP_CONNECTION_HPP
#define HTTP_TCP_CONNECTION_HPP

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Connection.h"

namespace http {
namespace server {

class RequestHandler;
class Server;

/// Represents a single connection from a client.
class TcpConnection : public Connection
{
public:
  /// Construct a connection with the given io_service.
  explicit TcpConnection(asio::io_service& io_service, Server *server,
      ConnectionManager& manager, RequestHandler& handler);

  /// Get the socket associated with the connection.
  virtual asio::ip::tcp::socket& socket();

  virtual void stop();
  virtual std::string urlScheme() { return "http"; }

protected:
  virtual void startAsyncReadRequest(Buffer& buffer, int timeout);
  virtual void startAsyncReadBody(Buffer& buffer, int timeout);
  virtual void startAsyncWriteResponse
      (const std::vector<asio::const_buffer>& buffers, int timeout);

  /// Socket for the connection.
  asio::ip::tcp::socket socket_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

} // namespace server
} // namespace http

#endif // HTTP_CONNECTION_HPP
