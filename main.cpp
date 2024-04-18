#include <iostream>
#include <limits>
#include <stdexcept>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#endif

constexpr int PORT = 443;
constexpr int BUFFER_SIZE = 1024;

void systemPause()
{
  std::cout << "Press Enter to finish...";
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

class TCPServer
{
public:
  TCPServer();
  ~TCPServer();

  void runServer();

private:
  void systemClear();
  void handleSocketError(std::string message);
  SOCKET createServerSocket();
  void bindServerSocket(SOCKET serverSocket, sockaddr_in &serverAddr);
  void listenServerSocket(SOCKET serverSocket);
  SOCKET acceptClient(SOCKET serverSocket);
  void receiveAndPrintData(SOCKET clientSocket);
  void cleanupSockets(SOCKET clientSocket, SOCKET serverSocket);

  SOCKET serverSocket;
};

TCPServer::TCPServer()
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
  {
    throw std::runtime_error("WSAStartup failed");
  }
#endif

  SOCKET serverSocket = createServerSocket();

  sockaddr_in serverAddr = {0};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  bindServerSocket(serverSocket, serverAddr);
  listenServerSocket(serverSocket);

  systemClear();
  std::cout << "Waiting\n\n~ ";
}

TCPServer::~TCPServer()
{
  cleanupSockets(INVALID_SOCKET, serverSocket);
}

void TCPServer::runServer()
{
  SOCKET clientSocket = acceptClient(serverSocket);

  systemClear();
  std::cout << "Connected\n\n~ ";

  receiveAndPrintData(clientSocket);
  systemPause();
  cleanupSockets(serverSocket, clientSocket);
}

void TCPServer::systemClear()
{
#ifdef _WIN32
  const char *clear_command = "cls";
#else
  const char *clear_command = "clear";
#endif
  system(clear_command);
}

void TCPServer::handleSocketError(std::string message)
{
#ifdef _WIN32
  std::cerr << "WSAError: " << WSAGetLastError() << std::endl;
#endif
  throw std::runtime_error(message);
}

SOCKET TCPServer::createServerSocket()
{
  SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == INVALID_SOCKET)
  {
    handleSocketError("Failed to create socket");
  }
  return serverSocket;
}

void TCPServer::bindServerSocket(SOCKET serverSocket, sockaddr_in &serverAddr)
{
  if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
  {
    handleSocketError("Bind failed");
  }
}

void TCPServer::listenServerSocket(SOCKET serverSocket)
{
  if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
  {
    handleSocketError("Listen failed");
  }
}

SOCKET TCPServer::acceptClient(SOCKET serverSocket)
{
  SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
  if (clientSocket == INVALID_SOCKET)
  {
    handleSocketError("Failed to accept client");
  }
  return clientSocket;
}

void TCPServer::receiveAndPrintData(SOCKET clientSocket)
{
  char buffer[BUFFER_SIZE];
  bool signal = true;

  while (true)
  {
    if (!signal)
    {
      std::cout << "~ ";
    }
    signal = false;

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived)
    {
      buffer[bytesReceived] = '\0';
      std::cout << buffer << std::flush << "\n";
      if (std::string(buffer) == "exit")
      {
        break;
      }
    }
    else
    {
      std::cout << "exit" << std::endl;
      break;
    }
  }
}

void TCPServer::cleanupSockets(SOCKET clientSocket, SOCKET serverSocket)
{
#ifdef _WIN32
  closesocket(clientSocket);
  closesocket(serverSocket);
  WSACleanup();
#else
  close(clientSocket);
  close(serverSocket);
#endif
}

int main(int argc, char **argv)
{
  try
  {
    TCPServer server;
    server.runServer();
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << "\n$ exit " << std::endl;
    systemPause();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}