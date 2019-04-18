#include "http_client.hpp"

#include <vector>
#include <optional>
#include <map>
#include <functional>
#include <curl/curl.h>
#include <curl/easy.h>

#include <iostream>
#include <cstring>


http_client::http_client() noexcept :
	curl(curl_easy_init()),
	header_list(nullptr)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_READDATA, this);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_data);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
#pragma clang diagnostic pop
}

http_client::~http_client() noexcept
{
	if(curl != nullptr)
		curl_easy_cleanup(curl);
}

void http_client::set_headers(std::map<std::string, std::string> headers)
{
	if(header_list != nullptr)
		curl_slist_free_all(header_list);
	header_list = nullptr;

	if(headers.size() > 0)
	{
		for(auto const & kvp : headers)
			header_list = curl_slist_append(header_list, (kvp.first + ": " + kvp.second).c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
}

std::optional<std::vector<std::byte>> http_client::transfer(method method, std::string const & url)
{
	return transfer(method, url, [](float){});
}

std::optional<std::vector<std::byte>> http_client::transfer(method method, std::string const & url, std::function<void(float)> const & on_progress)
{
	return transfer(method, url, ro_buffer<const std::byte> { }, on_progress);
}

std::optional<std::vector<std::byte>> http_client::transfer(method method, std::string const & url, ro_buffer<const std::byte> const & data)
{
	return transfer(method, url, data, [](float){});
}

std::optional<std::vector<std::byte>> http_client::transfer(method method, std::string const & url, ro_buffer<const std::byte> const & data, std::function<void(float)> const & on_progress)
{
	upload_buffer = data;
	upload_ptr = 0;

	buffer.clear();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_INFILESIZE, long(data.size()));
	switch(method)
	{
		case get:
			curl_easy_setopt(curl, CURLOPT_POST, 0L);
			curl_easy_setopt(curl, CURLOPT_UPLOAD,  0L);
			break;
		case post:
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_UPLOAD,  0L);
			break;
		case put:
			curl_easy_setopt(curl, CURLOPT_POST, 0L);
			curl_easy_setopt(curl, CURLOPT_UPLOAD,  1L);
			break;
	}
#pragma clang diagnostic pop

	if(on_progress)
		on_progress(0.0f);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cerr << curl_easy_strerror(res) << std::endl;
		return std::nullopt;
	}

	if(on_progress)
		on_progress(100.0f);

	return buffer;
}

std::size_t http_client::read_data(void * data, size_t size, size_t nitems, void *stream)
{
	auto & http = *reinterpret_cast<http_client*>(stream);

	auto const max_length = http.upload_buffer.size() - http.upload_ptr;
	auto read_length = size * nitems;
	if(read_length > max_length)
		read_length = max_length;

	memcpy(data, http.upload_buffer.data() + http.upload_ptr, read_length);
	http.upload_ptr += read_length;

	return read_length;
}

std::size_t http_client::write_data(void * data, size_t size, size_t nmemb, void *stream)
{
	auto & http = *reinterpret_cast<http_client*>(stream);

	auto const start = http.buffer.size();
	auto const length = size * nmemb;
	http.buffer.resize(http.buffer.size() + length);
	memcpy(&http.buffer[start], data, length);

	return length;
}
