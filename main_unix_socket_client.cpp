#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>

using namespace std;

string socket_name;
int client_socket;

int main(int argc, char **argv) {
  int pid = getpid();
  cout << "Client PID: " << pid << endl;

  // Считывание имени Unix-сокета с клавиатуры
  // cout << "Введите имя Unix-сокета (или оставьте поле пустым для использования имени по умолчанию): ";
  // getline(cin, socket_name);
  // if (socket_name.empty()) {
  socket_name = "/tmp/my_socket";
  // }
  cout << "Socket: " << socket_name << endl;

  // Создание Unix-сокета
  client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_socket == -1) {
    cerr << "Ошибка: не удалось создать Unix-сокет" << endl;
    exit(1);
  }

  // Подключение к сокету
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_name.c_str(), sizeof(addr.sun_path) - 1);
  if (connect(client_socket, (sockaddr *)&addr, sizeof(addr)) < 0) {
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
  close(client_socket);

  return 0;
}