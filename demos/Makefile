CXX      := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wno-interference-size -pthread
SRC      := ..

# ──────────────────────────────────────────────
# Цели сборки
# ──────────────────────────────────────────────

STANDALONE := demo_4 demo_5 demo_6 demo_7 demo_8 demo_9 demo_17 demo_18 demo_19
SERVERS    := server_1 server_2 server_10 server_11 server_12

.PHONY: all clean \
        run_1 run_2 run_4 run_5 run_6 run_7 run_8 run_9 \
        run_10 run_11 run_12 run_17 run_18 run_19 run_all

all: $(STANDALONE) $(SERVERS)

# ── standalone демо ───────────────────────────
demo_4:  demo_4_semaphore.cpp  ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_5:  demo_5_thread_pool.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_6:  demo_6_spinlock.cpp   ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_7:  demo_7_mutex_futex.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_8:  demo_8_mutex_3_states.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_9:  demo_9_condvar_futex.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_17: demo_17_false_sharing.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_18: demo_18_treiber_stack.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<
demo_19: demo_19_queue.cpp ; $(CXX) $(CXXFLAGS) -o $@ $<

# ── серверы (оригинальные файлы, main уже есть) ──
server_1:  $(SRC)/1_TCP_sync_single.cpp  ; $(CXX) $(CXXFLAGS) -o $@ $<
server_2:  $(SRC)/2_TCP_sync_multy.cpp   ; $(CXX) $(CXXFLAGS) -o $@ $<
server_10: demo_10_busy_loop.cpp         ; $(CXX) $(CXXFLAGS) -o $@ $<
server_11: $(SRC)/11_epoll_server.cpp    ; $(CXX) $(CXXFLAGS) -o $@ $<
server_12: $(SRC)/12_io_uring_server.cpp ; $(CXX) $(CXXFLAGS) -o $@ $< -luring

# ──────────────────────────────────────────────
# Цели запуска
# ──────────────────────────────────────────────

# Вспомогательный макрос: запустить сервер, подождать, послать данные nc, убить.
# Использование: $(call run_server, <бинарь>, <описание>)
define run_server
	@echo; echo "=== $(2) ==="; \
	./$(1) & SRV=$$! ; \
	sleep 0.4 ; \
	printf "Привет от клиента 1\n" | nc -q 1 127.0.0.1 8080 ; \
	printf "Привет от клиента 2\n" | nc -q 1 127.0.0.1 8080 ; \
	kill $$SRV 2>/dev/null ; wait $$SRV 2>/dev/null ; \
	echo "=== Завершено ==="
endef

run_1: server_1
	$(call run_server,server_1,Тема 1 — однопоточный синхронный TCP echo-сервер)

# Тема 2: несколько клиентов одновременно — демонстрирует многопоточность
run_2: server_2
	@echo; echo "=== Тема 2 — многопоточный TCP echo-сервер (3 клиента одновременно) ==="; \
	./server_2 & SRV=$$! ; \
	sleep 0.4 ; \
	printf "Клиент A\n" | nc -q 1 127.0.0.1 8080 & \
	printf "Клиент B\n" | nc -q 1 127.0.0.1 8080 & \
	printf "Клиент C\n" | nc -q 1 127.0.0.1 8080 ; \
	wait ; \
	kill $$SRV 2>/dev/null ; wait $$SRV 2>/dev/null ; \
	echo "=== Завершено ==="

run_4: demo_4
	@echo; echo "=== Тема 4 — Семафор (реализация через condition_variable) ==="; ./demo_4

run_5: demo_5
	@echo; echo "=== Тема 5 — Thread pool с blocking queue ==="; ./demo_5

run_6: demo_6
	@echo; echo "=== Тема 6 — SpinLock (std::atomic + compare_exchange) ==="; ./demo_6

run_7: demo_7
	@echo; echo "=== Тема 7 — Mutex на futex (2 состояния) ==="; ./demo_7

run_8: demo_8
	@echo; echo "=== Тема 8 — Mutex на futex (3 состояния) ==="; ./demo_8

run_9: demo_9
	@echo; echo "=== Тема 9 — ConditionVariable на futex + spurious wakeups ==="; ./demo_9

run_10: server_10
	$(call run_server,server_10,Тема 10 — Busy-loop polling (неблокирующий сервер без epoll))

run_11: server_11
	$(call run_server,server_11,Тема 11 — epoll echo-сервер)

run_12: server_12
	$(call run_server,server_12,Тема 12 — io_uring echo-сервер)

run_17: demo_17
	@echo; echo "=== Тема 17 — False sharing: бенчмарк с cache-line padding и без ==="; ./demo_17

run_18: demo_18
	@echo; echo "=== Тема 18 — Lock-free стек Трайбера ==="; ./demo_18

run_19: demo_19
	@echo; echo "=== Тема 19 — Lock-free очередь Майкла-Скотта ==="; ./demo_19

run_all: run_4 run_5 run_6 run_7 run_8 run_9 \
         run_17 run_18 run_19 \
         run_1 run_2 run_10 run_11 run_12

clean:
	rm -f $(STANDALONE) $(SERVERS)
