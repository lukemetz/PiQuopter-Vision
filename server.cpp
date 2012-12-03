//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "server.hpp"

using boost::asio::ip::tcp;

session::session(boost::asio::io_service& io_service, Controller * con)
  : socket_(io_service), controller(con)
{

}

tcp::socket& session::socket()
{
  return socket_;
}

void session::start()
{
  socket_.async_read_some(boost::asio::buffer(data_, max_length),
      boost::bind(&session::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
  if (!error)
  {
    boost::asio::async_write(socket_,
        boost::asio::buffer(data_, bytes_transferred),
        boost::bind(&session::handle_write, this,
          boost::asio::placeholders::error));
  }
  else
  {
    delete this;
  }
}

void session::handle_write(const boost::system::error_code& error)
{
  if (!error)
  {
      if (data_[0] == 'C') {
        printf("command found %s", &data_[1]);
        controller->command(data_);
      }
      printf("%s\n", data_);
      printf("ENDEDED\n");

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    delete this;
  }
}

server::server(boost::asio::io_service& io_service, short port, Controller * con)
  : io_service_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
    controller(con)
{
  start_accept();
}

void server::start_accept()
{
  session* new_session = new session(io_service_, controller);
  acceptor_.async_accept(new_session->socket(),
      boost::bind(&server::handle_accept, this, new_session,
        boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session,
    const boost::system::error_code& error)
{
  if (!error)
  {
    new_session->start();
  }
  else
  {
    delete new_session;
  }

  start_accept();
}
