
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
	std::array<uint8_t, 1472> data;

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

class Client
{
private:    
    udp::socket m_socket;
    udp::endpoint m_endpoint;

	std::array<DzlPacketData, 50>	m_packets;	

	bool				m_reconnect; // переустановить соединение
	
	volatile int		m_lastSaveIdx; //
	volatile int		m_lastSendIdx; //

    ClockCycler m_timeMeter;

public:
    Client(boost::asio::io_service& service)
        : m_socket(service, udp::endpoint(udp::v4(), 0))
    {        
        std::cout << "Client is connected: " << m_socket.is_open() << std::endl;
        
        m_endpoint = udp::endpoint(address::from_string("127.0.0.1"), 5200);
    }

    void startSend()
    {
        std::cout << "Send: " << std::endl;


        // ----- Make package -----
        DzlPacket* packet = (DzlPacket*)&m_packets[m_lastSaveIdx].data;
        if (!packet)
        {	std::cout << "packet is not exist!";	}

        packet->number = m_lastSaveIdx + 1;
        packet->timestamp = 1;
        // -----  -----

       printf("\n%s::%s() number = %lu, timestamp = %lu\n", typeid(*this).name(), __func__
        , packet->number
        , packet->timestamp
        );

        int index = m_lastSaveIdx + 1;
        if (index >= 50) {
            index = 0;
        }
        m_lastSaveIdx = index;


        if (m_timeMeter.isStarted)
        {
            m_timeMeter.stopMetr();

            printf("%s::%s() Delay = %lu microsec\n", typeid(*this).name(), __func__
             , m_timeMeter.getTimeCycle().count()); // TODO / YURIY / Delete
        }
        m_timeMeter.startMetr();


	    m_socket.async_send_to(boost::asio::buffer(&(m_packets[m_lastSendIdx].data), sizeof(DzlPacketData)),
							m_endpoint,
							boost::bind(&Client::handSend,
								this,
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred
									)
							);
    }

    void handSend(const boost::system::error_code& error, std::size_t size)
    {
	    if (error) {
            printf("%s::%s() Error message: %s\n\n", typeid(*this).name(), __func__
                , error.message().c_str()
                );
        }
        else if (size == m_packets[m_lastSendIdx].data.size())
        {
             m_lastSendIdx++;
            if (m_lastSendIdx >= 50) {
                m_lastSendIdx = 0;
            }
        }
        else
        {
            printf("%s::%s() Client: Error partial sending packet! (%lu)\n\n", typeid(*this).name(), __func__
            , size
            ); 
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        startSend();
    }
};

class Base
{
    boost::asio::io_service service;
    std::shared_ptr<Client> m_client;
    std::thread m_thread;

public:
    Base()
    {
        m_client = std::make_shared<Client>(service);
        m_thread = std::thread(&Base::runThread, this);
    }
    void runThread()
    {
        m_client->startSend();
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
