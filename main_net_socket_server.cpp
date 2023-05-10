#include <signal.h>
#include <sys/socket.h>
// #include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

using namespace std;

int server_socket;

volatile sig_atomic_t stop = 0;  // Флаг для безопасного завершения приложения

void clean_socket(int socket) {
  shutdown(socket, SHUT_RDWR);
  close(socket);
}

void handle_signal(int signal) {
  cout << "SIG! " << signal << endl;
  clean_socket(server_socket);
  exit(0);
}

const int BUFFER_SIZE = 1024;

int main(int argc, char **argv) {
  uint16_t port = 12345;

  int pid = getpid();
  cout << "Server PID: " << pid << endl;

  // Создание сетевого сокета
  int option = 1;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (server_socket == -1) {
    cerr << "Ошибка: не удалось создать сетевой сокет" << endl;
    exit(1);
  }

  // Настройка адреса сетевого сокета
  sockaddr_in sock_addr;
  memset(&sock_addr, 0, sizeof(sockaddr_in));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = htons(port);
  sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Привязка сетевого сокета к адресу
  if (bind(server_socket, (sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
    perror("bind");
    cerr << "Ошибка: не удалось привязать сетевой сокет к адресу" << endl;
    clean_socket(server_socket);
    exit(1);
  }

  // Прослушивание входящих подключений
  if (listen(server_socket, 10) == -1) {
    cerr << "Ошибка: не удалось начать прослушивание сетевого сокета" << endl;
    clean_socket(server_socket);
    exit(1);
  }

  cout << "Сервер прослушивает секет. Ctrl+C - выход." << endl;

  // Ожидание входящих соединений и чтение данных от клиентов
  while (!stop) {
    int client_socket = accept(server_socket, nullptr, nullptr);
    cout << "Клиент подключился" << endl;
    if (client_socket == -1) {
      if (stop) {
        clean_socket(server_socket);
        break;
      }
      cerr << "Ошибка: не удалось принять входящее соединение" << endl;
      continue;
    }

    // Чтение данных от клиента
    char buffer[BUFFER_SIZE];
    ssize_t received_bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (received_bytes == -1) {
      cerr << "Ошибка: не удалось прочитать данные от клиента" << endl;
      clean_socket(client_socket);
      continue;
    }
    buffer[received_bytes] = '\0';
    cout << "Получено от клиента: " << buffer << endl;

    // Send PID to client
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    if (write(client_socket, pid_str, strlen(pid_str)) < 0) {
      std::cerr << "Ошибка: не удалось отправить данные клиенту" << std::endl;
      exit(1);
    }

    // Закрытие сокета клиента
    clean_socket(client_socket);
  }

  clean_socket(server_socket);

  return 0;
}