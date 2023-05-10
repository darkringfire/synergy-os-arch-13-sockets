#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>

using namespace std;

int client_socket;

void clean_socket(int socket) {
  shutdown(socket, SHUT_RDWR);
  close(socket);
}

int main(int argc, char **argv) {
  string server_ip_address = "127.0.0.1";
  uint16_t port = 12345;

  int pid = getpid();
  cout << "Client PID: " << pid << endl;

  // Создание сетевого сокета
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    cerr << "Ошибка: не удалось создать Unix-сокет" << endl;
    exit(1);
  }

  // Подключение к сокету
  sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  if (inet_pton(AF_INET, server_ip_address.c_str(), &server_address.sin_addr) <= 0) {
    std::cerr << "Ошибка: некорректный адрес сервера" << std::endl;
    exit(1);
  }

  if (connect(client_socket, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
    std::cerr << "Ошибка: не удалось подключиться к сокету" << std::endl;
    exit(1);
  }
  cout << "Подключено" << endl;

  // Send PID to server
  char pid_str[16];
  snprintf(pid_str, sizeof(pid_str), "%d", pid);
  if (write(client_socket, pid_str, strlen(pid_str)) < 0) {
    std::cerr << "Ошибка: не удалось отправить данные на сервер" << std::endl;
    exit(1);
  }

  // Receive message from server
  char buffer[1024];
  int num_bytes = read(client_socket, buffer, sizeof(buffer));
  if (num_bytes < 0) {
    std::cerr << "Ошибка: не удалось прочитать данные от сервера" << std::endl;
    exit(1);
  }

  // Print received message
  std::cout << "Получено от сервера: " << std::string(buffer, num_bytes) << std::endl;

  // Close socket
  clean_socket(client_socket);

  return 0;
}