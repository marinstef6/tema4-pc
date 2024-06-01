#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "parson.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

#define HOST "34.246.184.49"
#define PORT 8080
#define LEN 5000

/* Functia care sterge caracterul "\n" */
void remove_newline(char *s) {
    // Verifica daca pointerul este NULL pentru a preveni erori
    if (s == NULL) {
        // Daca pointerul este NULL, functia se termina brusc
        return; 
    }
    // Calculeaza lungimea sirului de caractere
    size_t len = strlen(s); 
    if (len > 0 && s[len - 1] == '\n') {
    // Inlocuieste newline-ul cu terminatorul de sir daca este prezent
        s[len - 1] = '\0'; 
    }
}

/* Functie care verifica daca un string este reprezentat de un numar */
int check_number(char *s) {
    if (s == NULL) {
    // Verifica daca sirul este NULL
        return 0; 
    }
    // Calculeaza lungimea sirului o singura data
    int length = strlen(s);
    // Verifica daca sirul este gol
    if (length == 0) {
        return 0;
    }
    /* Verifica daca primul caracter este zero si sirul are mai mult de un
        caracter*/
    if (s[0] == '0' && length > 1) {
        return 0; 
    }

    /* Permite semnul minus doar daca este primul caracter si 
        sirul are mai mult de un caracter*/
    int start = (s[0] == '-' && length > 1) ? 1 : 0;
    // Verifica daca fiecare caracter este o cifra
    for (int i = start; i < length; i++) {
        if (s[i] < '0' || s[i] > '9') {
            return 0; 
        }
    }
    // Toate testele sunt trecute, sirul este un numar valid
    return 1;
}

/* Functia care elibereaza memoria */
void free_function(char *message, char *data, char *username, char *password)
{
    // Se va libera imediat dupa trimitere
    free(message);  
    free(data);
    free(username);
    free(password);
}

/* Functia de creare JSON */
char* create_json_payload(const char *username, const char *password) {
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root_obj = json_value_get_object(root_val);
    json_object_set_string(root_obj, "username", username);
    json_object_set_string(root_obj, "password", password);
    char *serialized_data = json_serialize_to_string(root_val);
    // Eliberarea memoriei obiectului JSON dupa serializare
    json_value_free(root_val);
    return serialized_data;
}

/* Functie pentru comanda login */
char *auth_login(size_t buflen, int sock_tcp, char *type) {
    char access_route[BUFLEN];
    char payload_type[BUFLEN];
    snprintf(access_route, BUFLEN, "/api/v1/tema/auth/%s", type);
    snprintf(payload_type, BUFLEN, "application/json");

    char *username = NULL, *password = NULL;
    printf("username= ");
    getline(&username, &buflen, stdin);
    remove_newline(username);
    printf("password= ");
    getline(&password, &buflen, stdin);
    remove_newline(password);

    // Generam payload-ul JSON
    char *data = create_json_payload(username, password);
    char *message = compute_post_request(HOST, access_route, payload_type, &data, strlen(data), NULL, NULL);
    
    send_to_server(sock_tcp, message);
    free_function(message, data, username, password);
    // Primeste raspunsul de la server
    return receive_from_server(sock_tcp);
}


int main()
{
    // Scrierea datelor de la stdout direct in terminal
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int sock_tcp; 
    struct sockaddr_in serv_addr;
    size_t buflen = BUFLEN;

    // Se aloca bufferul
    memset(&serv_addr, 0, sizeof(serv_addr));

    // Buffer pentru input de la stdin
    char *stdin_buffer = malloc(LEN * sizeof(char));
    memset(stdin_buffer, 0, LEN * sizeof(char));
    char *server_response;
       
    // Buffer pentru rutele de acces si tipuri de payload
    char *access_route = malloc(BUFLEN * sizeof(char));
    memset(access_route, 0, BUFLEN * sizeof(char));

    char *payload_type = malloc(BUFLEN * sizeof(char));
    memset(payload_type, 0, BUFLEN * sizeof(char));

    // Credentialele utilizatorului
    char *username = malloc(BUFLEN * sizeof(char));
    memset(username, 0, BUFLEN * sizeof(char));

    char *password = malloc(BUFLEN * sizeof(char));
    memset(password, 0, BUFLEN * sizeof(char));

    // Buffer general folosit in diverse scopuri
    char *general_use_buffer = malloc(BUFLEN * sizeof(char));
    memset(general_use_buffer, 0, BUFLEN * sizeof(char));

    char *jwt = NULL;
    char *cookie = NULL;
    
    // Variabile pentru gestionarea sesiunii
    puts("Enter a command:");

    // While-ul in care vor fi comenzile principale
    while (1) {

        // Citirea comenzii de la stdin
        memset(stdin_buffer, 0, LEN);

        // Gestionarea erorii citirii de la tastatura 
        if (fgets(stdin_buffer, LEN, stdin) == NULL) {
            printf("error: Error reading input.\n");
            continue;  
        }

        // Inlaturarea caracterului de sfarsit de linie, daca exista
        size_t line_length = strlen(stdin_buffer);
        if (line_length > 0 && stdin_buffer[line_length - 1] == '\n') {
            stdin_buffer[line_length - 1] = '\0';
        }
        // Comanda "register"
        if (!strcmp(stdin_buffer, "register")) {
            sock_tcp = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sock_tcp == -1) {
                puts("error: Failed to connect to the server.");
                continue;
            }
            // Tratarea tuturor cazurilor si afisarea erorilor sau ok
            server_response = auth_login(buflen, sock_tcp, "register");
            if (server_response == NULL) {
                puts("error: Failed to receive a response from the server.");
                close(sock_tcp);
                continue;
            }

            char *errorIndicator = "error";
            if (strstr(server_response, errorIndicator)) {
                puts("error: Username already taken, try again.\n");
            } else {
                puts("200 - OK - User successfully registered.");
            }

            free(server_response);
            close(sock_tcp);
            // Comanda "login"
        } else if (!strcmp(stdin_buffer, "login")) {
            // Incercarea de a stabili conexiunea cu serverul
            sock_tcp = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (sock_tcp == -1) {
                puts("error: Connection to server failed.");
                continue;
            }
            // Verifica daca un utilizator este deja conectat
            if (cookie != NULL) {
                puts("error: There is a user logged in.");
                continue;
            }
            // Solicitarea autentificarii de la server
            server_response = auth_login(buflen, sock_tcp, "login");
            if (server_response == NULL) {
                puts("error: Authentication response not received.");
                close(sock_tcp);
                continue;
            }

            // Evaluarea rapunsului pentru autentificare
            if (strstr(server_response, "error")) {
                puts("error: Wrong credentials.");
            } else {
                // Procesul de extragere a cookie-ului din raspuns
                char *cookie_start = strstr(server_response, "connect.sid=");
                if (cookie_start) {
                    char *cookie_end = strstr(cookie_start, ";");
                    if (cookie_end && cookie_start < cookie_end) {
                        size_t cookie_length = cookie_end - cookie_start;
                        cookie = calloc(cookie_length + 1, sizeof(char));
                        memcpy(cookie, cookie_start, cookie_length);
                        puts("200 - OK - User successfully logged in.");
                    }
                }
            }

            // Curatarea si inchiderea resurselor
            free(server_response);
            close(sock_tcp);
        // Comanda "enter library"
        } else if (!strcmp(stdin_buffer, "enter_library")) {
            if (cookie == NULL) {
                puts("error: No user is logged in.");
                continue;
            }

            int server_socket = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (server_socket < 0) {
                puts("error: Failed to connect to the server.");
                continue;
            }

            char library_access_route[BUFLEN];
            snprintf(library_access_route, BUFLEN, "/api/v1/tema/library/access");

            char *library_request = compute_get_delete_request(HOST, library_access_route, NULL, cookie, NULL, "GET");
            send_to_server(server_socket, library_request);
            char *library_response = receive_from_server(server_socket);
            free(library_request);

            if (strstr(library_response, "error")) {
                puts("error: Couldn't reach the library.");
            } else {
                char *jwt_start = strstr(library_response, "{");
                if (jwt_start) {
                    char *jwt_end = strstr(jwt_start, "}");
                    if (jwt_end && (jwt_end > jwt_start)) {
                        int jwt_length = jwt_end - jwt_start + 1;
                        jwt = realloc(jwt, jwt_length + 1);
                        strncpy(jwt, jwt_start, jwt_length);
                        jwt[jwt_length] = '\0';

                        JSON_Value *root_value = json_parse_string(jwt);
                        if (json_value_get_type(root_value) == JSONObject) {
                            JSON_Object *root_object = json_value_get_object(root_value);
                            const char *token = json_object_get_string(root_object, "token");
                            if (token) {
                                strcpy(jwt, token);
                                puts("200 - OK - Entered library.");
                            }
                            json_value_free(root_value);
                        }
                    }
                }
            }
            free(library_response);
            close(server_socket);
            // Comanda "get books"
        } else if (!strcmp(stdin_buffer, "get_books")) {
            // Verifica daca exista o sesiune activa de utilizator
            if (cookie == NULL) {
                puts("error: No user is logged in.");
                continue;
            }

            // Conectarea la server
            int server_socket = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (server_socket == -1) {
                puts("error: Failed to connect to the server.");
                continue;
            }

            // Pregatirea si trimiterea cererii GET pentru a accesa cartile
            char *endpoint = "/api/v1/tema/library/books";
            char library_route[BUFLEN];
            snprintf(library_route, sizeof(library_route), "%s", endpoint);
            char *request = compute_get_delete_request(HOST, library_route, NULL, cookie, jwt, "GET");
            send_to_server(server_socket, request);
            char *response = receive_from_server(server_socket);
            free(request);

            // Verificarea raspunsului serverului
            if (strstr(response, "error")) {
                puts("error: You have no access to the library.");
                free(response);
                close(server_socket);
                continue;
            }

            // Analiza raspunsului JSON pentru a extrage lista de carti
            char *data_start = strstr(response, "[");
            if (data_start) {
                JSON_Value *library_data = json_parse_string(data_start);
                if (json_value_get_type(library_data) == JSONArray) {
                    JSON_Array *books = json_value_get_array(library_data);
                    //  Se stocheaza numarul de carti intr-o variabila locala
                    size_t book_count = json_array_get_count(books);

                    if (book_count == 0) {
                        puts("error: There are currently no books, try adding one.");
                    } else {
                        // Afiseaza titlurile cartilor
                        puts("ID -- TITLE");
                        for (size_t i = 0; i < book_count; i++) {
                            JSON_Object *book = json_array_get_object(books, i);
                            int book_id = (int)json_object_get_number(book, "id");
                            const char *book_title = json_object_get_string(book, "title");
                            printf("id: %d -- title: %s\n", book_id, book_title);
                        }
                    }

                    json_value_free(library_data);
                } else {
                    puts("error: Failed to parse book data.");
                    json_value_free(library_data);
                }
            } else {
                puts("error: Invalid response format received.");
            }

            free(response);
            close(server_socket);
        }
        // Comanda "get book"
        else if (!strcmp(stdin_buffer, "get_book")) {
            if (cookie == NULL) {
                puts("error: No user is logged in.");
                continue;
            }

            int connection_socket = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (connection_socket < 0) {
                puts("error: Failed to connect to the server.");
                continue;
            }

            printf("id=");
            char book_id[BUFLEN];
            if (fgets(book_id, BUFLEN, stdin) == NULL) {
                puts("error: Error reading book ID.");
                close(connection_socket);
                continue;
            }
            remove_newline(book_id);

            char request_url[BUFLEN];
            const char *base_url = "/api/v1/tema/library/books/";
            int base_url_len = strlen(base_url);
            int max_book_id_length = BUFLEN - base_url_len - 1;
            book_id[max_book_id_length] = '\0';

            snprintf(request_url, BUFLEN, "%s%s", base_url, book_id);
            char *get_request = compute_get_delete_request(HOST, request_url, NULL, cookie, jwt, "GET");
            send_to_server(connection_socket, get_request);
            char *response = receive_from_server(connection_socket);
            free(get_request);

            if (strstr(response, "error")) {
                printf("error: There is no book with id %s.\n", book_id);
            } else {
                char *json_start = strstr(response, "{");
                if (json_start != NULL) {
                    JSON_Value *book_details = json_parse_string(json_start);
                    if (json_value_get_type(book_details) == JSONObject) {
                        JSON_Object *book_data = json_value_get_object(book_details);
                        int book_id = (int)json_object_get_number(book_data, "id");
                        const char *title = json_object_get_string(book_data, "title");
                        const char *author = json_object_get_string(book_data, "author");
                        const char *publisher = json_object_get_string(book_data, "publisher");
                        const char *genre = json_object_get_string(book_data, "genre");
                        int page_count = (int)json_object_get_number(book_data, "page_count");

                        printf("id: %d\n", book_id);
                        printf("title: %s\n", title);
                        printf("author: %s\n", author);
                        printf("publisher: %s\n", publisher);
                        printf("genre: %s\n", genre);
                        printf("page_count: %d\n", page_count);

                        json_value_free(book_details);
                    } else {
                        puts("error: Failed to parse book data.");
                        json_value_free(book_details);
                    }
                } else {
                    puts("error: Invalid response format received.");
                }
            } 

            free(response);
            close(connection_socket);
        }
        // Comanda "delete book"
        else if (!strcmp(stdin_buffer, "delete_book")) {
            if (cookie == NULL) {
                puts("error: No user is logged in.");
                continue;
            }

            int server_socket = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (server_socket < 0) {
                puts("error: Failed to connect to the server.");
                continue;
            }

            printf("id=");
            char id_buffer[BUFLEN];
            // Asigurarea ca inputul nu depaseste limita calculata
            fgets(id_buffer, BUFLEN - strlen("/api/v1/tema/library/books/") - 1, stdin);
            remove_newline(id_buffer);

            char full_path[BUFLEN];
            // Se verifica daca lungimea id-ului este adecvata inainte de a forma URL-ul complet
            if (strlen(id_buffer) > BUFLEN - strlen("/api/v1/tema/library/books/") - 1) {
                puts("error: Book ID is too long.");
                close(server_socket);
                continue;
            }
            int written = snprintf(full_path, BUFLEN, "/api/v1/tema/library/books/%s", id_buffer);
            if (written >= BUFLEN) {
                puts("error: Failed to format request URL, buffer size exceeded.");
                close(server_socket);
                continue;
            }

            char *request = compute_get_delete_request(HOST, full_path, NULL, cookie, jwt, "DELETE");
            send_to_server(server_socket, request);
            char *response = receive_from_server(server_socket);
            free(request);

            if (strstr(response, "error")) {
                printf("error: There is no book with id %s.\n", id_buffer);
            } else {
                puts("200 - OK - Successfully deleted book.");
            }

            free(response);
            close(server_socket);
        }
        // Comanda "add book"
        else if (!strcmp(stdin_buffer, "add_book")) {
            if (!cookie) {
                puts("error: No user is logged in.");
                continue;
            }
            int tcp_socket = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (tcp_socket < 0) {
                puts("error: Connection failed.");
                continue;
            }

            char book_details[BUFLEN] = "/api/v1/tema/library/books";
            char content_type[] = "application/json";
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *book_data = json_value_get_object(root_value);

            printf("title=");
            fgets(general_use_buffer, BUFLEN, stdin);
            remove_newline(general_use_buffer);
            json_object_set_string(book_data, "title", general_use_buffer);

            printf("author=");
            fgets(general_use_buffer, BUFLEN, stdin);
            remove_newline(general_use_buffer);
            json_object_set_string(book_data, "author", general_use_buffer);

            printf("genre=");
            fgets(general_use_buffer, BUFLEN, stdin);
            remove_newline(general_use_buffer);
            json_object_set_string(book_data, "genre", general_use_buffer);

            printf("publisher=");
            fgets(general_use_buffer, BUFLEN, stdin);
            remove_newline(general_use_buffer);
            json_object_set_string(book_data, "publisher", general_use_buffer);

            printf("page_count=");
            fgets(username, BUFLEN, stdin);
            remove_newline(username);
            if (!check_number(username)) {
                puts("error: Wrong input for page_count, try again.");
                json_value_free(root_value);
                close(tcp_socket);
                continue;
            }
            json_object_set_number(book_data, "page_count", atoi(username));

            char *serialized_data = json_serialize_to_string(root_value);
            char *request = compute_post_request(HOST, book_details, content_type, &serialized_data, strlen(serialized_data), cookie, jwt);
            send_to_server(tcp_socket, request);
            char *response = receive_from_server(tcp_socket);
            
            if (strstr(response, "error")) {
                puts("error: Couldn't add the book, try again.");
            } else {
                puts("200 - OK - Book added successfully.");
            }

            free(serialized_data);
            free(request);
            free(response);
            json_value_free(root_value);
            close(tcp_socket);

        }
        // Comanda "logout"
        else if (!strcmp(stdin_buffer, "logout")) {
            if (!cookie) {
                puts("error: No user is logged in.");
                continue;
            }
            int tcp_socket = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            if (tcp_socket < 0) {
                puts("error: Failed to connect to the server.");
                continue;
            }

            char logout_path[BUFLEN] = "/api/v1/tema/auth/logout";
            char *logout_request = compute_get_delete_request(HOST, logout_path, NULL, cookie, NULL, "GET");
            send_to_server(tcp_socket, logout_request);
            char *logout_response = receive_from_server(tcp_socket);
            free(logout_request);

            if (strstr(logout_response, "error")) {
                puts("error: Couldn't log out.");
            } else {
                puts("200 - OK - User logged out.");
            }

            free(logout_response);
            close(tcp_socket);

            free(cookie);
            cookie = NULL;
            if (jwt) {
                free(jwt);
                jwt = NULL;
            }
        }
        // Comanda "exit"
        else if (!strcmp(stdin_buffer, "exit")) {
        // Eliberarea memoriei alocata
            free(cookie); 
            cookie = NULL;
            free(jwt); 
            jwt = NULL;
            free(stdin_buffer); 
            stdin_buffer = NULL;
            free(access_route); 
            access_route = NULL;
            free(payload_type); 
            payload_type = NULL;
            free(username); 
            username = NULL;
            free(password); 
            password = NULL;
            free(general_use_buffer); 
            general_use_buffer = NULL;
            break;
        } else {
            // Afisam un mesaj daca comanda introdusa nu este recunoscuta
            puts("error: Wrong command.");
        }
    }

    return 0;
}