CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -L.
LDLIBS = -lcommon

all: client server libcommon.so

# Клиент
client: client.o libcommon.so
	$(CC) $(CFLAGS) -o $@ client.o $(LDFLAGS) $(LDLIBS)

client.o: client.c libcommon.h
	$(CC) $(CFLAGS) -c client.c

# Сервер
server: server.o libcommon.so
	$(CC) $(CFLAGS) -o $@ server.o $(LDFLAGS) $(LDLIBS)

server.o: server.c libcommon.h
	$(CC) $(CFLAGS) -c server.c

# Общая библиотека
libcommon.so: libcommon.o
	$(CC) -shared -o $@ libcommon.o

libcommon.o: libcommon.c libcommon.h
	$(CC) $(CFLAGS) -fPIC -c libcommon.c

# Очистка
clean:
	rm -f *.o *.so client server

# Установка библиотеки (опционально)
install: libcommon.so
	sudo cp libcommon.so /usr/lib/
	sudo cp libcommon.h /usr/include/

# Тестирование
test: client server
	@echo "Creating servers file..."
	@echo "127.0.0.1:20001" > servers.txt
	@echo "127.0.0.1:20002" >> servers.txt
	@echo "Starting servers..."
	@./server --port 20001 --tnum 2 > server1.log 2>&1 & echo $$! > server1.pid
	@./server --port 20002 --tnum 2 > server2.log 2>&1 & echo $$! > server2.pid
	@sleep 2
	@echo "Running client..."
	@./client --k 100000 --mod 1000000007 --servers servers.txt
	@echo "Cleaning up..."
	@kill `cat server1.pid` 2>/dev/null || true
	@kill `cat server2.pid` 2>/dev/null || true
	@rm -f server1.pid server2.pid servers.txt

# Запуск сервера
run-server:
	@echo "Starting server on port $(PORT) with $(TNUM) threads"
	./server --port $(PORT) --tnum $(TNUM)

# Запуск клиента
run-client:
	@echo "Running client with k=$(K), mod=$(MOD), servers=$(SERVERS)"
	./client --k $(K) --mod $(MOD) --servers $(SERVERS)

.PHONY: all clean install test run-server run-client