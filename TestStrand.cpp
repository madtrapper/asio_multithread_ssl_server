
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <list>
#include <asio.hpp>
#include <asio/ssl.hpp>

template <typename F, typename W>
// auto submit(F&& f, asio::thread_pool &workers) -> std::future<decltype(f())>
auto submit(F&& f, W&& w) -> std::future<decltype(f())>
{
	std::promise<decltype(f())> promise;
	auto future = promise.get_future();
	asio::post(w, [promise = std::move(promise), f = std::forward<F>(f)] () mutable
	{
		promise.set_value(f());
	});
	return future;
}


std::string work1()
{
	return "work1";
}

int work2(int count)
{
	return 1+count;
}

int work3(int second)
{
	sleep(second);
	std::cout << "sleep: "<<second<< " done\n";
	return 0;
}

extern void TestStrand()
{
	printf("TestStrand Start-----------\n");
		  
	std::atomic<int> sum = 0;
	asio::thread_pool workers(10);

	auto ex = workers.executor();
	asio::strand<asio::thread_pool::executor_type> st(ex);

	for(int n = 0; n < 16; ++n) {
        asio::post(workers, [n, &sum]
        {
            sum += n;
        });
    }

	auto val = submit([]
    {
        return "Hello from the thread pool!";
    }, workers);
    std::cout <<"val: "<< val.get() << "\n";

	auto val2 = submit(work1, workers);
    std::cout <<"val2: "<< val2.get() << "\n";

	auto val3 = submit(std::bind(work2, 3), workers);
    std::cout <<"val3: "<< val3.get() << "\n";

	asio::post(st, [] { std::cout << "FIFO-1b" << '\n'; });
	
	auto val4 = submit(std::bind(work2, 3), st);
    std::cout <<"val4: "<< val4.get() << "\n";

	submit(std::bind(work3, 5), st);
	submit(std::bind(work3, 4), st);
	submit(std::bind(work3, 3), st);
	submit(std::bind(work3, 2), st);
	submit(std::bind(work3, 1), st);
	
	workers.wait();

	std::cout << "The sum is " << sum << "\n";

	printf("TestStrand End-----------\n");
}
