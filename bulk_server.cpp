#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include "bulk.h"

using namespace boost::asio;
using namespace boost::posix_time;
io_service service;

class ClientConnection;
typedef boost::shared_ptr<ClientConnection> client_ptr;
typedef std::vector<client_ptr> array;
array clients;

#ifdef _DEBUG
int gClient = 1;
#endif

int gPort = 9000;
int gBulkSize = 3;

boost::shared_ptr<ParserWrapper> gParserPtr;

class ClientConnection: public boost::enable_shared_from_this<ClientConnection>
	, boost::noncopyable
{
	typedef ClientConnection self_type;
	ClientConnection(): socket( service ), _started( false )
	{
#ifdef _DEBUG
		_client_id = gClient;
		++gClient;
#endif
	}
public:
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<ClientConnection> ptr;

	void Start()
	{
		_started = true;
		clients.push_back( shared_from_this() );
		Read();
	}
	static ptr Create()
	{
		ptr new_( new ClientConnection );
		return new_;
	}
	void Stop()
	{
		if( !_started ) return;
		_started = false;
		socket.close();

		ptr self = shared_from_this();
		array::iterator it = std::find( clients.begin(), clients.end(), self );
		clients.erase( it );
	}
	bool Started() const { return _started; }
	ip::tcp::socket & sock() { return socket; }

private:
	void OnRead( const error_code& err, size_t size )
	{
		if( err ) Stop();
		if( !Started() ) return;

		std::string _str( read_buffer, size );

#ifdef _DEBUG
		std::string _data( read_buffer, size );
		std::cout << "cl: " << _client_id << " IN: \n" << _data << std::endl;
#endif

		if( gParserPtr.get() != nullptr )
		{
			gParserPtr->ReceiveData( read_buffer, size );
		}
		Read();
	}

	void Read()
	{
		socket.async_receive( buffer( read_buffer ), boost::bind( &self_type::OnRead, shared_from_this(), _1, _2 ) );
	}

private:
	ip::tcp::socket socket;
	char read_buffer[ 1000 ];
	bool _started;
#ifdef _DEBUG
	int _client_id;
#endif
};

struct SAcceptor
{
	~SAcceptor() { if( m_acceptor ) delete m_acceptor; }
	ip::tcp::acceptor* m_acceptor = nullptr;
}tagAcceptor;


void Acceptor( ClientConnection::ptr client, const boost::system::error_code & err )
{
	client->Start();
	ClientConnection::ptr new_client = ClientConnection::Create();
	tagAcceptor.m_acceptor->async_accept( new_client->sock(), boost::bind( Acceptor, new_client, _1 ) );
}

int main( int argc, char* argv[] )
{
	if( argc > 1 )
		gPort = std::stoi( argv[ 1 ] );
	if( argc > 2 )
		gBulkSize = std::stoi( argv[ 2 ] );

	gParserPtr = boost::shared_ptr<ParserWrapper>( new ParserWrapper( gBulkSize ) );
	tagAcceptor.m_acceptor = new ip::tcp::acceptor( service, ip::tcp::endpoint( ip::tcp::v4(), gPort ) );

	ClientConnection::ptr client = ClientConnection::Create();
	tagAcceptor.m_acceptor->async_accept( client->sock(), boost::bind( Acceptor, client, _1 ) );
	service.run();
}
