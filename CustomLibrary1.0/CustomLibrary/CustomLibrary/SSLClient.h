#pragma once

#include "ThirdParty/asio.hpp"
#include "ThirdParty/ssl.hpp"

#include <iostream>
#include <memory>

#include "Error.h"

namespace ctl
{
	using asio::ip::tcp;
	class SSL_Client
	{
	public:
		SSL_Client(asio::io_context& io, asio::ssl::context &ctx)
			: m_io(io), m_ctx(ctx), m_resolver(io) {}

		void connect(const std::string &host, const std::string &path, std::iostream &content, std::iostream &header)
		{
			//Bind output
			m_contents = &content;
			m_headers = &header;

			//Allocate new socket (REQUIRED for continued use)
			m_socket.reset(new asio::ssl::stream<tcp::socket>(m_io, m_ctx));

			//Form the request
			std::ostream request_s(&m_request);
			request_s << "GET " << path << " HTTP/1.1\r\n"
				<< "Host: " << host << "\r\n"
				<< "Accept: */*\r\n"
				//Close connection IMPORTANT
				<< "Connection: close\r\n\r\n";

			//Query and start async chain
			tcp::resolver::query q(host, "https");
			m_resolver.async_resolve(q, 
				std::bind(&SSL_Client::_resolve_, this, std::placeholders::_1, std::placeholders::_2));
		}

		void close()
		{
			if (m_socket->lowest_layer().is_open())
				m_socket->shutdown();
		}

		~SSL_Client()
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
			asio::async_connect(m_socket->lowest_layer(), endIter,
				std::bind(&SSL_Client::_connect_, this, std::placeholders::_1));
		}

		void _connect_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			//Get Handshake
			m_socket->async_handshake(asio::ssl::stream_base::client,
				std::bind(&SSL_Client::_handshake_, this, std::placeholders::_1));
		}

		//Handshake function
		void _handshake_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			asio::async_write(*m_socket, m_request,
				std::bind(&SSL_Client::_write_, this, std::placeholders::_1));
		}

		void _write_(const std::error_code &err)
		{
			if (err)
				throw Log(err.message(), Log::Severity::ERR0R);

			asio::async_read_until(*m_socket, m_response, "\r\n",
				std::bind(&SSL_Client::_readStatusLine_, this, std::placeholders::_1));
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
				throw Log("Invalid response\n");
			if (statusCode != 200)
				throw Log("Response returned with status code " + std::to_string(statusCode));

			//Read the response headers
			asio::async_read_until(*m_socket, m_response, "\r\n\r\n",
				std::bind(&SSL_Client::_readHeaders_, this, std::placeholders::_1));
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
				std::bind(&SSL_Client::_readContent_, this, std::placeholders::_1));
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
					std::bind(&SSL_Client::_readContent_, this, std::placeholders::_1));
			}
			else if (err != asio::error::eof)
				throw Log(err.message(), Log::Severity::ERR0R);
		}

		asio::io_context &m_io;
		asio::ssl::context &m_ctx;

		tcp::resolver m_resolver;
		std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> m_socket;

		asio::streambuf m_request;
		asio::streambuf m_response;

		std::iostream *m_headers;
		std::iostream *m_contents;
	};
}