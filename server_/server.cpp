
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <array>
#include <queue>

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
	bool isStarted;// std::atomic<bool> isStarted;
	std::chrono::system_clock::time_point startPoint;
	std::chrono::system_clock::time_point stopPoint;

	std::chrono::microseconds timeCycle;// std::atomic<std::chrono::microseconds> timeCycle;

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
			// timeCycle.store(std::chrono::duration_cast<std::chrono::microseconds>(stopPoint - startPoint));

            timeCycle = (std::chrono::duration_cast<std::chrono::microseconds>(stopPoint - startPoint));

			isStarted = false;
		}
	}

	std::chrono::microseconds getTimeCycle() const
	{
		return timeCycle;//.load();
	}
};

class Server
{
private:    
    udp::socket     m_socket;
    udp::endpoint   m_endpoint;

    boost::asio::deadline_timer deadline_;
    DzlPacketData	m_packet;
	ClockCycler     m_timeMeter;
    std::queue<std::chrono::microseconds>      m_packets;

public:
    Server(boost::asio::io_service& service)
        : m_socket(service, udp::endpoint(address::from_string("127.0.0.1"), 5200))
        , deadline_(service)
    {
        m_endpoint = udp::endpoint(address::from_string("127.0.0.1"), 5200);

        deadline_.expires_at(boost::posix_time::pos_infin);

        // Start the persistent actor that checks for deadline expiry.
        check_deadline();
    }

    ~Server()
    {
        m_socket.close();

        for (size_t i = 0; i < m_packets.size(); ++i)
        {        
            printf("%s::%s() Delay = %lu microsec\n\n", typeid(*this).name(), __func__
            , m_packets.front().count()); // TODO / YURIY / Delete
            m_packets.pop();
        }
    }
    void check_deadline()
    {
        // Check whether the deadline has passed. We compare the deadline against
        // the current time since a new asynchronous operation may have moved the
        // deadline before this actor had a chance to run.
        if (deadline_.expires_at() <= boost::asio::deadline_timer::traits_type::now())
        {
          m_socket.cancel();


          for (size_t i = 0; i < m_packets.size(); ++i)
        {        
            printf("%s::%s() Delay = %lu microsec\n\n", typeid(*this).name(), __func__
            , m_packets.front().count()); // TODO / YURIY / Delete
            m_packets.pop();
        }

          deadline_.expires_at(boost::posix_time::pos_infin);
        }

        // Put the actor back to sleep.
        deadline_.async_wait(boost::bind(&Server::check_deadline, this));
      }

    void readStream()
    {       
        m_socket.async_receive_from(boost::asio::buffer(&m_packet.data, sizeof(DzlPacketData))
                                    , m_endpoint
                                    , boost::bind(&Server::handle_readStream
                                        , this                                      
                                        , boost::asio::placeholders::error                                       
                                       )                                       
                                    );
        deadline_.expires_from_now(boost::posix_time::seconds(10));
    }

    void handle_readStream(const boost::system::error_code& err)
    {
        if (err)
        {
            return;
        }

        if (m_timeMeter.isStarted)
        {
            m_timeMeter.stopMetr();
            m_packets.push(m_timeMeter.getTimeCycle());            
        }
        m_timeMeter.startMetr();              

        readStream();
    }
    
};

class Base
{
    boost::asio::io_service service;
    std::shared_ptr<Server> m_server;
    std::thread m_thread;

public:
    Base()
    {
        m_server = std::make_shared<Server>(service);
        m_thread = std::thread(&Base::runThread, this);
    }
    void runThread()
    {
        m_server->readStream();
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
