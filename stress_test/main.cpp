#include <chrono>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

std::mutex log_mutex;

void log_response(const std::string& log_line)
{
	std::lock_guard< std::mutex > lock(log_mutex);
	std::ofstream log_file("stress_test.log", std::ios::app);
	log_file << log_line << std::endl;
}

size_t dummy_write(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	return size * nmemb;
}

void perform_request(const std::string& method, const std::string& url,
					 const std::string& post_data = "")
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
					 dummy_write); // avoid stdout print

	if (method == "POST")
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
	}

	else if (method == "DELETE")
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

	long response_code = 0;
	CURLcode res	   = curl_easy_perform(curl);

	if (res == CURLE_OK)
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

	std::stringstream log;
	log << "[" << method << "] " << url << " - HTTP " << response_code;

	if (res != CURLE_OK)
		log << " - Error: " << curl_easy_strerror(res);

	log_response(log.str());

	curl_easy_cleanup(curl);
}

void stress_worker(const std::string& method, const std::vector< int >& ports,
				   int requests_per_port, const std::string& post_payload)
{
	for (int i = 0; i < requests_per_port; ++i)
	{
		for (int port : ports)
		{
			std::stringstream url;
			url << "http://localhost:" << port;
			if (method == "POST")
				perform_request(method, url.str(), post_payload);
			else
				perform_request(method, url.str());
		}
	}
}

std::string read_file(const std::string& path)
{
	std::ifstream f(path, std::ios::binary);
	if (!f)
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << f.rdbuf();
	return buffer.str();
}

int main(int argc, char* argv[])
{
	if (argc < 6)
	{
		std::cerr << "Usage: " << argv[0]
				  << " GET|POST|DELETE requests_per_port file_for_post port1 "
					 "port2 ...\n";
		return 1;
	}

	std::string method	  = argv[1];
	int requests_per_port = std::stoi(argv[2]);
	std::string file_path = argv[3];

	std::vector< int > ports;
	for (int i = 4; i < argc; ++i)
		ports.push_back(std::stoi(argv[i]));

	for (auto& port : ports)
		std::cout << port << std::endl;
	std::string post_data;
	if (method == "POST")
		post_data = read_file(file_path);

	curl_global_init(CURL_GLOBAL_ALL);

	const int thread_count = 8;
	std::vector< std::thread > threads;

	for (int i = 0; i < thread_count; ++i)
		threads.emplace_back(stress_worker, method, ports,
							 requests_per_port / thread_count, post_data);

	for (auto& t : threads)
		t.join();

	curl_global_cleanup();

	std::cout << "Stress test completed. Logs written to stress_test.log\n";
	return 0;
}
