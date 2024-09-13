import socket
import threading
import queue

HOST = '127.0.0.1'
PORT = 8888
INSTANCES = [('127.0.0.1', 8889), ('127.0.0.1', 8890), ('127.0.0.1', 8891)]

def handle_instance(instance_queue):
    while True:
        conn, addr = instance_queue.get()
        with conn:
            print(f'Connected by {addr}')
            data = conn.recv(1024).decode()
            if not data:
                break
            print(f'Received: {data}')
            conn.sendall(b'World')
        instance_queue.task_done()

def start_instance(instance_addr):
    instance_queue = queue.Queue()
    threading.Thread(target=handle_instance, args=(instance_queue,), daemon=True).start()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(instance_addr)
        s.listen()

        while True:
            conn, addr = s.accept()
            instance_queue.put((conn, addr))

def start_coordinator():
    instance_threads = []
    for instance_addr in INSTANCES:
        instance_thread = threading.Thread(target=start_instance, args=(instance_addr,))
        instance_thread.start()
        instance_threads.append(instance_thread)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
 
        while True:
            conn, addr = s.accept()
            with conn:
                print(f'Connected by {addr}')
                data = conn.recv(1024).decode()
                if not data:
                    break
                print(f'Received: {data}')
                instance_addr = INSTANCES[0]
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as instance_sock:
                    instance_sock.connect(instance_addr)
                    instance_sock.sendall(data.encode())
                    response = instance_sock.recv(1024).decode()
                    conn.sendall(response.encode())

    for instance_thread in instance_threads:
        instance_thread.join()

if __name__ == '__main__':
    start_coordinator()