#include <asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "logger.hpp"
#include "stream_socket_service.hpp"

typedef asio::basic_stream_socket<
  services::stream_socket_service<asio::ip::tcp> > debug_stream_socket;

char read_buffer[1024];

void read_handler(const asio::error& e,
    std::size_t bytes_transferred, debug_stream_socket* s)
{
  if (!e)
  {
    std::cout.write(read_buffer, bytes_transferred);

    s->async_read_some(asio::buffer(read_buffer),
        boost::bind(read_handler, asio::placeholders::error,
          asio::placeholders::bytes_transferred, s));
  }
}

void connect_handler(const asio::error& e, debug_stream_socket* s,
    asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  if (!e)
  {
    s->async_read_some(asio::buffer(read_buffer),
        boost::bind(read_handler, asio::placeholders::error,
          asio::placeholders::bytes_transferred, s));
  }
  else if (endpoint_iterator != asio::ip::tcp::resolver::iterator())
  {
    s->close();
    s->async_connect(*endpoint_iterator,
        boost::bind(connect_handler,
          asio::placeholders::error, s, ++endpoint_iterator));
  }
  else
  {
    std::cerr << e << std::endl;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: daytime_client <host>" << std::endl;
      return 1;
    }

    asio::io_service io_service;

    // Set the name of the file that all logger instances will use.
    services::logger logger(io_service, "");
    logger.use_file("log.txt");

    // Resolve the address corresponding to the given host.
    asio::ip::tcp::resolver resolver(io_service);
    asio::ip::tcp::resolver::query query(argv[1], "daytime");
    asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

    // Start an asynchronous connect.
    debug_stream_socket socket(io_service);
    socket.async_connect(*iterator,
        boost::bind(connect_handler,
          asio::placeholders::error, &socket, ++iterator));

    // Run the io_service until all operations have finished.
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
