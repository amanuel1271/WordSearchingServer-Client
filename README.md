# WordSearchingServer-Client
an I/O multiplexing search engine for the course EE488 KAIST


(1) an event-based server with I/O multiplexing to handle requests, and

Bootstrapping: The server makes an inverted index of words in all documents in the folder. This is done when the server is started, and the server should store the generated indice s in the memory.
Searching: the server program handles a large number of request with a 10 threads in a thread pool, and it should respond to rel evant documents and lines by the queried words.
(2) There are 2 client programs that send multiple requests to a Web server.

# Test environment
Ubuntu 18.04 build-essential

# how to get the executables
make

# how to run the server
[server_name] [absolute_path_to_target] [port]

# how to test using client1
Thus, the command line should be “./client1 [server_ip] [server_port] [number_of_threads] [number_of_requests_per_thread] [word_to_search]”, (e.g., ./client 127.0.0.1 8080 100 1000 abcd)

# how to test using client2
./client2 [server_ip] [server_port]
