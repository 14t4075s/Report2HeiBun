#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <chrono>

std::mutex t_mutex;
char alphabet = 'a';
void processData()
{
	std::string filename;
	filename += alphabet;
	filename += ".txt";
	std::ifstream file(filename);
	std::lock_guard<std::mutex> guard(t_mutex);
}

int main()
{
	char a;
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	std::thread t[26];
	for (size_t i = 0; i < 26; i++)
	{
		t[i] = std::thread(processData);
	}
	for (size_t i = 0; i < 26; i++)
	{
		t[i].join();
	}
	//std::this_thread::sleep_for(std::chrono::seconds(2));
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_time(end - start);
	std::cout << "time spent: " << elapsed_time.count() <<"s"<< std::endl;
	std::cin >> a;
	return 0;
}