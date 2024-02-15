#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
   
#define PORT     5200 
   

struct DzlPacketData
{
    std::array<uint8_t, 30> data;

    DzlPacketData() {
    }
};

#pragma pack(1)
struct DzlPacket
{

    uint64_t        timestamp; // время цикла
    uint64_t        number; // номер пакета
    std::array<uint8_t, 1>  data; // данные

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
        isStarted.store(true);
    }

    void stopMetr()
    {
        if (isStarted.load())
        {
            stopPoint = std::chrono::high_resolution_clock::now();
            timeCycle.store(std::chrono::duration_cast<std::chrono::microseconds>(stopPoint - startPoint));

            isStarted.store(false);
        }
    }

    std::chrono::microseconds getTimeCycle() const
    {
        return timeCycle.load();
    }
};



int main(int argc, char *argv[])
{
    int sockfd; 
    char buffer[30]; 
    const char *hello = "Hello from client"; 
    struct sockaddr_in     servaddr; 

    ClockCycler m_timeMeter;


    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    memset(&servaddr, 0, sizeof(servaddr)); 
       
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY;   



    for(;;)
    {
        if (m_timeMeter.isStarted)
        {
            m_timeMeter.stopMetr();

            printf("= %lu \n"
             , m_timeMeter.getTimeCycle().count()); // TODO / YURIY / Delete
        }
        m_timeMeter.startMetr();

        sendto(sockfd, (const char *)hello, strlen(hello), 
            0, (const struct sockaddr *) &servaddr,  
                sizeof(servaddr));

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }


    close(sockfd);

    return 0;
}



