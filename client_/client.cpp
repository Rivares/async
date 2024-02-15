
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
	std::array<uint8_t, 30> data;

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


#include <future> 

class Client
{
private:    
    udp::socket m_socket;
    udp::endpoint m_endpoint;

	std::array<DzlPacketData, 50>	m_packets;	

    std::atomic<bool> isStarted;
    std::chrono::system_clock::time_point startPoint;
    std::chrono::system_clock::time_point stopPoint;

    std::atomic<std::chrono::microseconds> timeCycle;

    std::future<uint64_t> func;

public:
    Client(boost::asio::io_service& service)
        : m_socket(service, udp::endpoint(udp::v4(), 0))
    {
        m_endpoint = udp::endpoint(address::from_string("127.0.0.1"), 5200);
    }

    void startSend()
    {
	    m_socket.async_send_to(boost::asio::buffer(&(m_packets[0].data), sizeof(DzlPacketData)),
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
            return;
        }

        auto th = std::thread([&](){
            if (isStarted.load())
            {
                stopPoint = std::chrono::high_resolution_clock::now();
                timeCycle.store(std::chrono::duration_cast<std::chrono::microseconds>(stopPoint - startPoint));

                isStarted.store(false);
            }
            startPoint = std::chrono::high_resolution_clock::now();
            isStarted.store(true);

            printf("= %lu \n"
            , timeCycle.load().count());
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        th.join();


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
