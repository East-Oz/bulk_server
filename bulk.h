#ifndef BULK_H
#define BULK_H

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <time.h>
#include <chrono>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream> 

class Observer
{
public:
	virtual void execute( std::vector<std::string>*, time_t* ) = 0;
	virtual ~Observer() = default;
};

class Executor
{
public:
	Executor(): m_fct( time( 0 ) ) {};

	std::vector<std::string> m_commands;
	time_t m_fct; // first command time
private:
	std::vector<std::shared_ptr<Observer>> m_subscribers;

public:
	void subscribe( std::shared_ptr<Observer> ptrObs )
	{
		m_subscribers.push_back( ptrObs );
	}

	void set_commands( std::vector<std::string> commands, time_t fct )
	{
		m_commands = commands;
		m_fct = fct;
		execute();
	}

	void execute()
	{
		for( auto s : m_subscribers )
		{
			s->execute( &m_commands, &m_fct );
		}
	}

};

class FileObserver: public Observer
{
public:
	FileObserver( std::shared_ptr<Executor> ptrExecutor )
	{
		auto wptr = std::shared_ptr<FileObserver>( this, []( FileObserver* ) {} );
		ptrExecutor->subscribe( wptr );
	}

	void execute( std::vector<std::string>* commands, time_t* fct ) override
	{
		if( commands->size() > 0 )
		{
			struct tm  tstruct;
			char       buf[ 80 ];
			tstruct = *localtime( fct );
			strftime( buf, sizeof( buf ), "%OH%OM%OS", &tstruct );

			auto time_now = std::chrono::system_clock::now();
			auto ms = std::chrono::duration_cast< std::chrono::milliseconds >(time_now.time_since_epoch()) % 1000;

			std::string fn = "bulk";
			fn.append( buf );
			fn.append( std::to_string( ms.count() ) );
			fn.append( ".log" );

			std::ofstream myfile;
			myfile.open( fn );
			if( myfile.is_open() )
			{
				for( size_t i = 0; i < commands->size(); ++i )
				{
					myfile << commands->at( i );
					if( i != (commands->size() - 1) )
						myfile << ", ";
					else
						myfile << std::endl;
				}
				myfile.close();
			}
		}

	}
};

class ConsoleObserver: public Observer
{
public:
	ConsoleObserver( std::shared_ptr<Executor> ptrExecutor )
	{
		auto wptr = std::shared_ptr<ConsoleObserver>( this, []( ConsoleObserver* ) {} );
		ptrExecutor->subscribe( wptr );
	}

	void execute( std::vector<std::string>* commands, time_t* ) override
	{
		if( commands->size() > 0 )
		{
			std::cout << "bulk: ";
			for( size_t i = 0; i < commands->size(); ++i )
			{
				std::cout << commands->at( i );
				if( i < (commands->size() - 1) )
					std::cout << ", ";
				else
					std::cout << std::endl;
			}
		}
	}
};

class Parser
{
public:
	Parser( std::shared_ptr<Executor> ptrExec, size_t row_count = 3 ):
		m_pExecutor( ptrExec ), m_nRowCount( row_count ), m_nCount( 1 ), m_nOpenBraces( 0 )
	{};

	void ParseStringData( std::string data_str )
	{
		std::istringstream stream( data_str );

		bool is_ready_data = false;

		m_nCount = m_Commands.size();
		time_t fct = time( 0 );
		std::string line;

		for( std::string line; std::getline( stream, line );)
		{
			if( line.empty() )
			{
				continue;
			}
			m_nCount++;
			if( m_nCount == 1 )
			{
				fct = time( 0 );
			}
			if( line.find( '{' ) != std::string::npos )
			{
				++m_nOpenBraces;
				if( m_nOpenBraces == 1 )
				{
					is_ready_data = true;
				}
			}
			else if( (line.find( '}' ) != std::string::npos) && (m_nOpenBraces > 0) )
			{
				--m_nOpenBraces;
				if( m_nOpenBraces == 0 )
				{
					is_ready_data = true;
				}
			}
			else if( (m_nCount == m_nRowCount) && (m_nOpenBraces == 0) )
			{
				m_Commands.push_back( line );
				is_ready_data = true;
			}
			else
			{
				m_Commands.push_back( line );
			}

			if( is_ready_data )
			{
				m_pExecutor->set_commands( m_Commands, fct );
				m_Commands.clear();
				is_ready_data = false;
				m_nCount = 0;
			}
		}
	}

private:
	std::shared_ptr<Executor> m_pExecutor;
	size_t m_nRowCount;
	std::vector<std::string> m_Commands;
	size_t m_nCount;
	size_t m_nOpenBraces;
};

class ParserWrapper
{
public:
	ParserWrapper( size_t bulk_size )
	{
		std::shared_ptr<Executor> ptrExec( std::make_shared<Executor>() );

		m_ptrConsoleObserver = std::shared_ptr<ConsoleObserver>( new ConsoleObserver( ptrExec ) );
		m_ptrFileObserver = std::shared_ptr<FileObserver>( new FileObserver( ptrExec ) );

		m_ptrParserWorker = std::shared_ptr<Parser>( new Parser( ptrExec, bulk_size ) );
	}
	~ParserWrapper() = default;


	void ReceiveData( const char *data, std::size_t size )
	{
		std::string data_string( data, size );
		m_ptrParserWorker->ParseStringData( data_string );
	}

private:
	std::shared_ptr<FileObserver> m_ptrFileObserver;
	std::shared_ptr<ConsoleObserver> m_ptrConsoleObserver;
	std::shared_ptr<Parser> m_ptrParserWorker;
};

#endif // BULK_H

