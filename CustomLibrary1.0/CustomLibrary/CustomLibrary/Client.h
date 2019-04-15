#pragma once

#include <CustomLibrary/ThirdParty/asio.hpp>

#include <iostream>
#include <memory>

#include <CustomLibrary/Error.h>

namespace ctl
{
	using asio::ip::tcp;
	class Client
	{
	public:
		Client(asio::io_context &io)
			: m_resolver(io), m_io(io) {}

		//Connect socket to host at path and bind output
		void connect(const std::string &server, const std::string &path, std::iostream &content, std::iostream &header)
		{
			//Bind output
			m_contents = &content;
			m_headers = &header;

			//Allocate new socket (REQUIRED for continued use)
			m_socket.reset(new tcp::socket(m_io));

			//Form the request
			std::ostream request_s(&m_request);
			request_s << "GET " << path << " HTTP/1.0\r\n"
				<< "Host: " << server << "\r\n"
				<< "Accept: */*\r\n"
				//Close connection IMPORTANT
				<< "Connection: close\r\n\r\n";

			//Start an asynchronous resolve for endpoints
			tcp::resolver::query q(server, "http");
			m_resolver.async_resolve(q, std::bind(&Client::_resolve_, this, std::placeholders::_1, std::placeholders::_2));
		}

		//Handle closing
		void close()
		{
			//If socket is active, close it
			if (m_socket->is_open())
				m_socket->shutdown(asio::ip::tcp::socket::shutdown_both);
		}

		~Client()
		{
			close();
		}

	private:
		//Resolve connection
		void _resolve_(const std::error_code &err, tcp::resolver::iterator endIter)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			//Attempt a connection to each endpoint in the list
			asio::async_connect(*m_socket, endIter,
				std::bind(&Client::_connect_, this, std::placeholders::_1));
		}

		//Establish a connection
		void _connect_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			//Send the request
			asio::async_write(*m_socket, m_request,
				std::bind(&Client::_writeRequest_, this, std::placeholders::_1));
		}

		//Request response
		void _writeRequest_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			//Read the response status line
			asio::async_read_until(*m_socket, m_response, "\r\n",
				std::bind(&Client::_readStatusLine_, this, std::placeholders::_1));
		}

		//Get status
		void _readStatusLine_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			//Get status version and code
			std::istream response_s(&m_response);
			std::string httpVersion;
			unsigned int statusCode;
			response_s >> httpVersion >> statusCode;

			//Get status message
			std::string statusMessage;
			std::getline(response_s, statusMessage);

			//Check if it's OK
			if (!response_s || httpVersion.substr(0, 5) != "HTTP/")
				throw Log("Client: Invalid response\n", Log::Severity::ERR0R);
			if (statusCode != 200)
				throw Log("Client: Response returned with status code " + std::to_string(statusCode), Log::Severity::ERR0R);

			//Read the response headers
			asio::async_read_until(*m_socket, m_response, "\r\n\r\n",
				std::bind(&Client::_readHeaders_, this, std::placeholders::_1));
		}

		//Get headers
		void _readHeaders_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			//Put response headers into stream
			std::istream response_s(&m_response);

			//Write headers
			std::string header;
			while (std::getline(response_s, header) && header != "\r")
				*m_headers << header << '\n';

			//Start reading remaining data until EOF
			asio::async_read(*m_socket, m_response, asio::transfer_at_least(1),
				std::bind(&Client::_readContent_, this, std::placeholders::_1));
		}

		//Get content
		void _readContent_(const std::error_code &err)
		{
			if (!err)
			{
				//Get data that was read so far
				*m_contents << &m_response;

				//Continue reading data until EOF
				asio::async_read(*m_socket, m_response, asio::transfer_at_least(1),
					std::bind(&Client::_readContent_, this, std::placeholders::_1));
			}
			else if (err != asio::error::eof)
				throw Log(err.message(), Log::Severity::ERR0R);
		}

		asio::io_context &m_io;

		tcp::resolver m_resolver;
		std::unique_ptr<tcp::socket> m_socket;

		asio::streambuf m_request;
		asio::streambuf m_response;

		std::iostream *m_headers;
		std::iostream *m_contents;
	};
}