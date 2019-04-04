#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP


#include <vector>
#include <optional>
#include <map>
#include <functional>
#include <curl/curl.h>
#include <curl/easy.h>

template<typename T>
struct ro_buffer
{
	T * _pointer;
	size_t _length;

	constexpr ro_buffer() :
	  _pointer(nullptr), _length(0)
	{

	}

	constexpr ro_buffer(T * ptr, size_t len) :
	  _pointer(ptr), _length(len)
	{

	}

	T * data() {
		return _pointer;
	}

	T const * data() const {
		return _pointer;
	}

	size_t size() const {
		return _length;
	}
};

//! Simple frontend for libcurl to allow the download of HTTP files.
struct http_client
{
	enum method { get, put, post };

	http_client() noexcept;
	http_client(http_client const &) = delete;
	http_client(http_client && other) = default;
	~http_client() noexcept;

	void set_headers(std::map<std::string, std::string> headers);

	std::optional<std::vector<std::byte>> transfer(method method, std::string const & url);

	std::optional<std::vector<std::byte>> transfer(method method, std::string const & url, std::function<void(float)> const & on_progress);

	std::optional<std::vector<std::byte>> transfer(method method, std::string const & url, ro_buffer<const std::byte> const & data);

	std::optional<std::vector<std::byte>> transfer(method method, std::string const & url, ro_buffer<const std::byte> const & data, std::function<void(float)> const & on_progress);

private:
	void * curl;
	std::vector<std::byte> buffer;
	ro_buffer<const std::byte> upload_buffer;
	size_t upload_ptr;
	curl_slist * header_list;

	static std::size_t read_data(void * data, size_t size, size_t nitems, void *stream);

	static std::size_t write_data(void * data, size_t size, size_t nmemb, void *stream);
};


#endif // HTTP_CLIENT_HPP
