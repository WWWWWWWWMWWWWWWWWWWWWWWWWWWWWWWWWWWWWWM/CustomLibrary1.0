#pragma once

class SSL_Client
{
public:
	SSL_Client(boost::asio::io_context& io, boost::asio::ssl::context &ctx)
		: m_io(io), m_ctx(ctx), m_resolver(io) {}

	void connect(const std::string &host, const std::string &path, std::iostream &content, std::iostream &header)
	{
		//Bind output
		m_contents = &content;
		m_headers = &header;

		//Allocate new socket (REQUIRED for continued use)
		m_socket.reset(new boost::asio::ssl::stream<tcp::socket>(m_io, m_ctx));

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
			boost::bind(&SSL_Client::_resolve_, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
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
	void _resolve_(const boost::system::error_code &err, tcp::resolver::iterator endIter)
	{
		if (err)
			throw Error(err.message());

		//Attempt a connection to each endpoint in the list
		boost::asio::async_connect(m_socket->lowest_layer(), endIter,
			boost::bind(&SSL_Client::_connect_, this, boost::asio::placeholders::error));
	}

	void _connect_(const boost::system::error_code& err)
	{
		if (err)
			throw Error(err.message());

		//Get Handshake
		m_socket->async_handshake(boost::asio::ssl::stream_base::client,
			boost::bind(&SSL_Client::_handshake_, this, boost::asio::placeholders::error));
	}

	//Handshake function
	void _handshake_(const boost::system::error_code& err)
	{
		if (err)
			throw Error(err.message());

		boost::asio::async_write(*m_socket, m_request,
			boost::bind(&SSL_Client::_write_, this, boost::asio::placeholders::error));
	}

	void _write_(const boost::system::error_code& err)
	{
		if (err)
			throw Error(err.message());

		boost::asio::async_read_until(*m_socket, m_response, "\r\n",
			boost::bind(&SSL_Client::_readStatusLine_, this, boost::asio::placeholders::error));
	}

	//Get status
	void _readStatusLine_(const boost::system::error_code &err)
	{
		if (err)
			throw Error(err.message());

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
			throw Error("Invalid response\n");
		if (statusCode != 200)
			throw Error("Response returned with status code " + std::to_string(statusCode));

		//Read the response headers
		boost::asio::async_read_until(*m_socket, m_response, "\r\n\r\n",
			boost::bind(&SSL_Client::_readHeaders_, this, boost::asio::placeholders::error));
	}

	//Get headers
	void _readHeaders_(const boost::system::error_code &err)
	{
		if (err)
			throw Error(err.message());

		//Put response headers into stream
		std::istream response_s(&m_response);

		//Write headers
		std::string header;
		while (std::getline(response_s, header) && header != "\r")
			*m_headers << header << '\n';

		//Start reading remaining data until EOF
		boost::asio::async_read(*m_socket, m_response, boost::asio::transfer_at_least(1),
			boost::bind(&SSL_Client::_readContent_, this, boost::asio::placeholders::error));
	}

	//Get content
	void _readContent_(const boost::system::error_code &err)
	{
		if (!err)
		{
			//Get data that was read so far
			*m_contents << &m_response;

			//Continue reading data until EOF
			boost::asio::async_read(*m_socket, m_response, boost::asio::transfer_at_least(1),
				boost::bind(&SSL_Client::_readContent_, this, boost::asio::placeholders::error));
		}
		else if (err != boost::asio::error::eof)
			throw Error(err.message());
	}

	boost::asio::io_context &m_io;
	boost::asio::ssl::context &m_ctx;

	tcp::resolver m_resolver;
	std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> m_socket;

	boost::asio::streambuf m_request;
	boost::asio::streambuf m_response;

	std::iostream *m_headers;
	std::iostream *m_contents;
};