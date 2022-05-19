
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <list>
#include <asio.hpp>
#include <asio/ssl.hpp>
#pragma warning(disable: 4996)

using asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
    session(asio::ssl::stream<tcp::socket>&& socket, asio::io_context& io_context)
        : socket_(std::move(socket)),
        strand_(asio::make_strand(io_context))
    {
        std::stringstream make_response;
        std::string finished_content = "Hello world";
        make_response << "HTTP/1.1 200 OK\r\n";
        make_response << "Cache-Control: no-cache, private\r\n";
        make_response << "Content-Type: text/html\r\n";
        make_response << "Content-Length: " << finished_content.length() << "\r\n";
        make_response << "\r\n";
        make_response << finished_content;
        finished_response = make_response.str();
    }

    void start()
    {
        do_handshake();
    }

private:
    void do_handshake()
    {
        auto self(shared_from_this());
        socket_.async_handshake(asio::ssl::stream_base::server,
            [this, self](const std::error_code& error)
            {
                if (!error) {
                    do_read();
                }
            });
    }

    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(asio::buffer(data_),
            [this, self](const std::error_code& ec, std::size_t length) {
                if (!ec) {
                    do_write(length);
                }
            });
    }

    void handle_write(const asio::error_code& e, std::size_t size) {
        if (!e) {
            do_read();
        }
    }

    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        /*
        asio::async_write(socket_, asio::buffer(finished_response, finished_response.length()),
            asio::bind_executor(strand_,
            [this, self](const std::error_code& ec, std::size_t)
            {
                if (!ec)
                {
                    do_read();
                }
            })
        );
        */

        asio::async_write(socket_, asio::buffer(finished_response, finished_response.length()),
            asio::bind_executor(strand_, std::bind(&session::handle_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2))
        );

    }

    std::string finished_response;
    asio::ssl::stream<tcp::socket> socket_;
    asio::strand<asio::io_context::executor_type> strand_;
    char data_[1024];
};

class server
{
public:

    server(asio::io_context& io_context, unsigned short port)
        : io_context_(io_context),
          port_(port),
          acceptor_(io_context),
          context_(asio::ssl::context::sslv23)
    {
        context_.set_options(
            asio::ssl::context::default_workarounds
            | asio::ssl::context::no_sslv2
            | asio::ssl::context::single_dh_use);
        context_.set_password_callback(std::bind(&server::get_password, this));
        context_.use_certificate_chain_file("server.pem");
        context_.use_private_key_file("server.pem", asio::ssl::context::pem);
        context_.use_tmp_dh_file("dh4096.pem");

	//        auto endpoint = tcp::endpoint(tcp::v4(), asio::ip::make_address("127.0.0.1"), port_);

	auto endpoint = tcp::endpoint(asio::ip::make_address("127.0.0.1"), port_);
	
        acceptor_.open(endpoint.protocol());
	int one = 1;
	setsockopt(acceptor_.native_handle(), SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));

	//acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
	try {
	  acceptor_.bind(endpoint);
	  printf("1.5-----\n");

	  acceptor_.listen();
	}
	catch (std::exception& e) {
	  std::cerr << "Exception: " << e.what() << "\n";
	 }
	printf("2-----\n");
        do_accept();
    }

    std::string get_password() const
    {
        return "test";
    }

    void do_accept()
    {

      acceptor_.async_accept([this](const std::error_code& error, tcp::socket socket) {
            
        //acceptor_.async_accept(*socket, [&](std::error_code & error) {
	if (!error) {
	  std::make_shared<session>(asio::ssl::stream<tcp::socket>(std::move(socket), context_), io_context_)->start();
	}

	do_accept();
        
      });
    }
private:
    asio::io_context    &io_context_;
    int                 port_;
    tcp::acceptor       acceptor_;
    asio::ssl::context  context_;
    //std::optional<tcp::socket> socket;
};

int main(int argc, char* argv[])
{
    try {
        if (argc != 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }

        auto count = std::thread::hardware_concurrency();

	printf("cpu:%d\n", count);

        asio::io_context io;

        std::list<server> srv_list;
        std::vector<std::thread> threads;
        for (int i = 0; i < count/2; i++) {
            srv_list.emplace_back(io, std::atoi(argv[1]));
            threads.emplace_back([&] {
	      //printf("run id:%d\n", GetCurrentThreadId());
	      io.run();
	      //printf("after run id:%d\n", GetCurrentThreadId());
             });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
