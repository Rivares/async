
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <array>

using namespace boost::asio::ip;


struct DzlPacketData
{
	std::array<uint8_t, 50> data;

	DzlPacketData() {
	}
};

#pragma pack(1)
struct DzlPacket
{    
	uint64_t		timestamp; // время цикла
	uint64_t		number; // номер пакета
	std::array<uint8_t, 1>	data; // данные

	DzlPacket() {
		timestamp = 0;
		number = 0;
	}
};
#pragma pack()

struct ClockCycler
{
	std::atomic<bool> isStarted;
	std::chrono::system_clock::time_point startPoint;
	std::chrono::system_clock::time_point stopPoint;

	std::atomic<std::chrono::microseconds> timeCycle;

	ClockCycler() = default;

	void startMetr()
	{
		startPoint = std::chrono::high_resolution_clock::now();
		isStarted = true;
	}

	void stopMetr()
	{
		if (isStarted)
		{
			stopPoint = std::chrono::high_resolution_clock::now();
			timeCycle.store(std::chrono::duration_cast<std::chrono::microseconds>(stopPoint - startPoint));

			isStarted = false;
		}
	}

	std::chrono::microseconds getTimeCycle() const
	{
		return timeCycle.load();
	}
};

class Server
{
private:    
    udp::socket     m_socket;
    udp::endpoint   m_endpoint;

    DzlPacketData	m_packet;
	ClockCycler     m_timeMeter;

public:
    Server(boost::asio::io_service& service)
        : m_socket(service, udp::endpoint(address::from_string("192.168.131.4"), 5200))
    {        
        std::cout << "Server is connected: " << m_socket.is_open() << std::endl;
        
        m_endpoint = udp::endpoint(address::from_string("192.168.131.4"), 5200);
    }

    void readStream()
    {
        std::cout << "Read: " << std::endl;

        m_socket.async_receive_from(boost::asio::buffer(&m_packet.data, sizeof(DzlPacketData))
                                    , m_endpoint
                                    , boost::bind(&Server::handle_readStream
                                        , this                                      
                                        , boost::asio::placeholders::error
                                        , boost::asio::placeholders::bytes_transferred                                        
                                       )
                                       
                                    );
    }

    void handle_readStream(const boost::system::error_code& err, std::size_t size)
    {
        if (err)
        {
            std::cout << "Error in read answer: " << err << std::endl;
            return;
        }


        std::cout << "From server: " << std::endl;	

        if (m_timeMeter.isStarted)
        {
            m_timeMeter.stopMetr();

            printf("%s::%s() Delay = %lu microsec\n\n", typeid(*this).name(), __func__
            , m_timeMeter.getTimeCycle().count()); // TODO / YURIY / Delete
        }
        m_timeMeter.startMetr();


        DzlPacket* packet = (DzlPacket*)&m_packet.data;

        printf("%s::%s() number = %lu, timestamp = %lu\n\n", typeid(*this).name(), __func__
        , packet->number
        , packet->timestamp
	    ); // TODO / YURIY / Delete


        readStream();
    }
    
};

class Base
{
    boost::asio::io_service service;
    std::shared_ptr<Server> m_Server;
    std::thread m_thread;

public:
    Base()
    {
        m_Server = std::make_shared<Server>(service);
        m_thread = std::thread(&Base::runThread, this);
    }
    void runThread()
    {
        m_Server->readStream();
        service.run();
    }
    ~Base()
    {
        if (m_thread.joinable())
        {   m_thread.join();    }

        service.stop();
    }
};


int main(int argc, char *argv[])
{
    Base obj;

    return 0;
}
