#include <iostream>  
#include <string>    
#include <cstring>   
#include <algorithm> 
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <unistd.h>      

#include <pqxx/pqxx>     

const int PORT = номер порта; //сюда вписать номер порта

int main() {

    std::string conn_str = "host=localhost port=5432 dbname=название таблицы user=название password=пароль"; //сюда вписываем имя пользователя и пароль а так же название таблицы
    pqxx::connection C(conn_str); 

    if (!C.is_open()) {
        std::cerr << "ОШИБКА: Не удалось подключиться к базе данных." << std::endl;
        return 1; 
    }
    std::cout << "Успешно подключено к базе данных: " << C.dbname() << std::endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Ошибка: Не удалось создать сокет" << std::endl;
        return 1;
    }

    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Ошибка: setsockopt" << std::endl;
        close(server_fd); 
        return 1;
    }

    sockaddr_in address; 
    std::memset(&address, 0, sizeof(address)); 
    address.sin_family = AF_INET;           
    address.sin_addr.s_addr = INADDR_ANY;   
                                            
    address.sin_port = htons(PORT);         
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Ошибка: Не удалось привязать сокет к порту " << PORT << std::endl;
        close(server_fd); 
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Ошибка: Не удалось начать прослушивание" << std::endl;
        close(server_fd); 
        return 1;
    }

    std::cout << "Сервер C++ запущен и слушает на порту " << PORT << std::endl;
    std::cout << "Ожидаю входящие соединения от Arduino..." << std::endl;

    while (true) {
        socklen_t addrlen = sizeof(address);

        int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            std::cerr << "Ошибка: Не удалось принять соединение" << std::endl;
            continue;
        }

        std::cout << "\nПолучено новое соединение!" << std::endl;


        char buffer[1024] = {0}; 
        int bytes_read = read(new_socket, buffer, 1024);
        
        if (bytes_read > 0) {
            std::string received_message(buffer, bytes_read);

            received_message.erase(std::remove(received_message.begin(), received_message.end(), '\n'), received_message.end());
            received_message.erase(std::remove(received_message.begin(), received_message.end(), '\r'), received_message.end());
            
            std::cout << "Получено сообщение:\n--- НАЧАЛО СООБЩЕНИЯ ---\n" << received_message << "\n--- КОНЕЦ СООБЩЕНИЯ ---\n" << std::endl;


            int button_code_to_insert = 1; 

            pqxx::work W(C);

            std::string sql_insert = "INSERT INTO название таблицы (button_code) VALUES (" +  //сюда вписываем название таблицы
                                     W.quote(button_code_to_insert) + ");";

            W.exec(sql_insert);

            W.commit();

            std::cout << "Запись с button_code=" << button_code_to_insert << " успешно добавлена в таблицу ." << std::endl;

        } else if (bytes_read == 0) {
            std::cout << "Соединение закрыто клиентом." << std::endl;
        } else {
            std::cerr << "Ошибка чтения из сокета." << std::endl;
        }

        close(new_socket);
    }

   	close(server_fd); 
    
    return 0; 
}