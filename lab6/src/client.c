#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};

struct ThreadData {
  struct Server server;
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
  uint64_t result;
  int status; // 0 - успех, 1 - ошибка
};

// Прототипы функций из библиотеки
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);
bool ConvertStringToUI64(const char *str, uint64_t *val);

void *ConnectToServer(void *arg) {
  struct ThreadData *data = (struct ThreadData *)arg;
  
  struct hostent *hostname = gethostbyname(data->server.ip);
  if (hostname == NULL) {
    fprintf(stderr, "gethostbyname failed with %s\n", data->server.ip);
    data->status = 1;
    return NULL;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(data->server.port);
  server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

  int sck = socket(AF_INET, SOCK_STREAM, 0);
  if (sck < 0) {
    fprintf(stderr, "Socket creation failed!\n");
    data->status = 1;
    return NULL;
  }

  if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
    fprintf(stderr, "Connection failed to %s:%d\n", data->server.ip, data->server.port);
    close(sck);
    data->status = 1;
    return NULL;
  }

  char task[sizeof(uint64_t) * 3];
  memcpy(task, &data->begin, sizeof(uint64_t));
  memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
  memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

  if (send(sck, task, sizeof(task), 0) < 0) {
    fprintf(stderr, "Send failed\n");
    close(sck);
    data->status = 1;
    return NULL;
  }

  char response[sizeof(uint64_t)];
  if (recv(sck, response, sizeof(response), 0) < 0) {
    fprintf(stderr, "Receive failed\n");
    close(sck);
    data->status = 1;
    return NULL;
  }

  memcpy(&data->result, response, sizeof(uint64_t));
  data->status = 0;
  
  close(sck);
  return NULL;
}

int ReadServersFromFile(const char *filename, struct Server **servers, unsigned int *count) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "Cannot open file: %s\n", filename);
    return -1;
  }

  // Сначала подсчитаем количество серверов
  char line[512];
  *count = 0;
  while (fgets(line, sizeof(line), file)) {
    if (strlen(line) > 1) { // Не пустая строка
      (*count)++;
    }
  }

  if (*count == 0) {
    fprintf(stderr, "No servers in file\n");
    fclose(file);
    return -1;
  }

  // Выделяем память
  *servers = malloc(sizeof(struct Server) * (*count));
  if (!*servers) {
    fprintf(stderr, "Memory allocation failed\n");
    fclose(file);
    return -1;
  }

  // Читаем серверы
  rewind(file);
  unsigned int i = 0;
  while (fgets(line, sizeof(line), file) && i < *count) {
    if (strlen(line) <= 1) continue;
    
    char *ip = strtok(line, ":");
    char *port_str = strtok(NULL, "\n");
    
    if (ip && port_str) {
      strcpy((*servers)[i].ip, ip);
      (*servers)[i].port = atoi(port_str);
      i++;
    }
  }

  fclose(file);
  return 0;
}

int main(int argc, char **argv) {
  uint64_t k = 0;
  uint64_t mod = 0;
  char servers_file[255] = {'\0'};
  int k_set = 0, mod_set = 0;

  while (true) {
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        if (ConvertStringToUI64(optarg, &k)) {
          k_set = 1;
        }
        break;
      case 1:
        if (ConvertStringToUI64(optarg, &mod)) {
          mod_set = 1;
        }
        break;
      case 2:
        memcpy(servers_file, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (!k_set || !mod_set || !strlen(servers_file)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // Читаем серверы из файла
  struct Server *servers = NULL;
  unsigned int servers_num = 0;
  if (ReadServersFromFile(servers_file, &servers, &servers_num) < 0) {
    return 1;
  }

  printf("Loaded %d servers\n", servers_num);

  // Создаем потоки для параллельной работы с серверами
  pthread_t threads[servers_num];
  struct ThreadData thread_data[servers_num];

  // Разбиваем диапазон [1, k] между серверами
  uint64_t range_size = k;
  uint64_t chunk_size = range_size / servers_num;
  uint64_t remainder = range_size % servers_num;
  
  uint64_t current_begin = 1;
  
  for (unsigned int i = 0; i < servers_num; i++) {
    thread_data[i].server = servers[i];
    thread_data[i].begin = current_begin;
    thread_data[i].mod = mod;
    
    uint64_t current_end = current_begin + chunk_size - 1;
    if (i < remainder) {
      current_end++;
    }
    
    if (i == servers_num - 1) {
      thread_data[i].end = k;
    } else {
      thread_data[i].end = current_end;
    }
    
    current_begin = thread_data[i].end + 1;
    
    printf("Server %d (%s:%d) will handle [%lu, %lu]\n", 
           i, servers[i].ip, servers[i].port, 
           thread_data[i].begin, thread_data[i].end);
    
    if (pthread_create(&threads[i], NULL, ConnectToServer, &thread_data[i]) != 0) {
      fprintf(stderr, "Failed to create thread for server %d\n", i);
      free(servers);
      return 1;
    }
  }

  // Ожидаем завершения всех потоков и собираем результаты
  uint64_t answer = 1;
  int success_count = 0;
  
  for (unsigned int i = 0; i < servers_num; i++) {
    pthread_join(threads[i], NULL);
    
    if (thread_data[i].status == 0) {
      answer = MultModulo(answer, thread_data[i].result, mod);
      success_count++;
      printf("Server %d result: %lu\n", i, thread_data[i].result);
    } else {
      fprintf(stderr, "Server %d failed\n", i);
    }
  }

  if (success_count > 0) {
    printf("Final answer: %lu\n", answer);
  } else {
    fprintf(stderr, "All servers failed\n");
    free(servers);
    return 1;
  }

  free(servers);
  return 0;
}