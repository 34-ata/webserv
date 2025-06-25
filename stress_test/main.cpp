#include <chrono>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

std::mutex log_mutex;

// Struct to define each test case
struct TestCase
{
	std::string method;	   // GET, POST, DELETE
	std::string url;	   // Endpoint to request
	std::string post_data; // For POST method
	long expected_code;	   // Expected HTTP response code
};

// Dummy write callback (we discard response body)
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
	(void)userp;
	(void)contents;
	return size * nmemb;
}

// Perform HTTP request and return actual response code
bool run_test(const TestCase& test, long& actual_code)
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return false;

	curl_easy_setopt(curl, CURLOPT_URL, test.url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

	if (test.method == "POST")
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, test.post_data.c_str());
	}
	else if (test.method == "DELETE")
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	}

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &actual_code);
	curl_easy_cleanup(curl);
	return true;
}

// Thread worker that randomly tests cases
void test_worker(const std::vector< TestCase >& cases, int iterations,
				 int thread_id)
{
	std::ofstream log_file("test_log.txt", std::ios::app);
	if (!log_file.is_open())
	{
		std::cerr << "Failed to open log file.\n";
		return;
	}

	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<> dist(0, cases.size() - 1);

	for (int i = 0; i < iterations; ++i)
	{
		int idx				 = dist(rng);
		const TestCase& test = cases[idx];

		long actual_code = 0;
		bool success	 = run_test(test, actual_code);

		std::lock_guard< std::mutex > lock(log_mutex);
		log_file << "[Thread " << thread_id << "] " << test.method << " "
				 << test.url << " (expected: " << test.expected_code
				 << ", got: "
				 << (success ? std::to_string(actual_code) : "ERROR") << ") => "
				 << ((success && actual_code == test.expected_code) ? "PASS"
																	: "FAIL")
				 << "\n";
	}
}

int main()
{
	curl_global_init(CURL_GLOBAL_ALL);

	std::vector< TestCase > tests = {
		// GET requests
		{"GET", "http://localhost:8080/", "", 200},
		{"GET", "http://localhost:8080/unknown", "", 404},
		{"GET", "http://localhost:8080/api/private", "", 403},

		// POST requests
		{"POST", "http://localhost:8080/api/data", "id=1&value=foo", 201},
		{"POST", "http://localhost:8080/api/data", "", 400},
		{"POST", "http://localhost:8080/api/upload", "file=abc", 200},
		{"POST", "http://localhost:8080/invalid", "", 404},

		// DELETE requests
		//{"DELETE", "http://localhost:8080/api/data/1", "", 200},
		//{"DELETE", "http://localhost:8080/api/data/999", "", 404},
		//{"DELETE", "http://localhost:8080/api/readonly", "", 403},
		//{"DELETE", "http://localhost:8080/api/data/", "", 400},
	};

	int thread_count		  = 4;
	int iterations_per_thread = 20;

	std::vector< std::thread > threads;
	for (int i = 0; i < thread_count; ++i)
	{
		threads.emplace_back(test_worker, std::cref(tests),
							 iterations_per_thread, i);
	}

	for (auto& t : threads)
	{
		t.join();
	}

	curl_global_cleanup();
	std::cout << "Testing complete. Check test_log.txt\n";
	return 0;
}
