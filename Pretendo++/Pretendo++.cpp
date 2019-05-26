#include <iostream>
#include "httplib.h"

using namespace std;
using namespace httplib;

#define NEX_RESPONSE_TEMPLATE R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><nex_token><host>%s</host><nex_password>%s</nex_password><pid>%s</pid><port>%s</port><token>%s</token></nex_token>)"

#define OAUTH_RESPONSE "<OAuth20><access_token><token>1234567890abcdef1234567890abcdef</token><refresh_token>fedcba0987654321fedcba0987654321fedcba12</refresh_token><expires_in>3600</expires_in></access_token></OAuth20>"

// FRIENDSFRIENDS... (128 bytes)
#define FRIENDS_TOKEN "RlJJRU5EU0ZSSUVORFNGUklFTkRTRlJJRU5EU0ZSSUVORFNGUklFTkRTRlJJRU5EU0ZSSUVORFNGUklFTkRTRlJJRU5EU0ZSSUVORFNGUklFTkRTRlJJRU5EU0ZSSUVORFNGUklFTkRTRlJJRU5EU0ZSSUVORFNGUklFTkRTRlI="
#define FRIENDS_HOST "127.0.0.1"
#define FRIENDS_PORT "60000"
#define FRIENDS_NEX_PID "1337"
#define FRIENDS_NEX_PASSWORD "password"

// SMMSMMSMM... (128 bytes)
#define SMM_TOKEN "U01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU01NU00="
#define SMM_HOST "127.0.0.1"
#define SMM_PORT "59900"
#define SMM_NEX_PID "1337"
#define SMM_NEX_PASSWORD "password"

static string dump_headers(const Headers &headers)
{
	string s;
	char buf[BUFSIZ];

	for(const auto &x : headers) {
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

static string dump_multipart_files(const MultipartFiles &files)
{
	string s;
	char buf[BUFSIZ];

	s += "--------------------------------\n";

	for(const auto &x : files) {
		const auto &name = x.first;
		const auto &file = x.second;

		snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
		s += buf;

		snprintf(buf, sizeof(buf), "filename: %s\n", file.filename.c_str());
		s += buf;

		snprintf(buf, sizeof(buf), "content type: %s\n", file.content_type.c_str());
		s += buf;

		snprintf(buf, sizeof(buf), "text offset: %zu\n", file.offset);
		s += buf;

		snprintf(buf, sizeof(buf), "text length: %zu\n", file.length);
		s += buf;

		s += "----------------\n";
	}

	return s;
}

static string log(const Request &req, const Response &res)
{
	string s;
	char buf[BUFSIZ];

	s += "================================\n";

	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	string query;
	for(auto it = req.params.begin(); it != req.params.end(); ++it) {
		const auto &x = *it;
		snprintf(buf, sizeof(buf), "%c%s=%s",
			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
			x.second.c_str());
		query += buf;
	}
	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
	s += buf;

	s += dump_headers(req.headers);
	s += dump_multipart_files(req.files);

	s += "--------------------------------\n";

	snprintf(buf, sizeof(buf), "%d\n", res.status);
	s += buf;
	s += dump_headers(res.headers);

	return s;
}

int main()
{
	Server svr;
	svr.Post("/v1/api/oauth20/access_token/generate", [](const Request& req, Response& res)
	{
		res.set_content(OAUTH_RESPONSE, "text/xml");
	});
	svr.Get("/v1/api/provider/nex_token/@me", [](const Request& req, Response& res)
	{
		auto game_server_id = req.get_param_value("game_server_id");
		char response[2048] = "";
		if(game_server_id == "00003200") // Friends
		{
			sprintf_s(response, NEX_RESPONSE_TEMPLATE,
				FRIENDS_HOST,
				FRIENDS_NEX_PASSWORD,
				FRIENDS_NEX_PID,
				FRIENDS_PORT,
				FRIENDS_TOKEN);
		}
		else if(game_server_id == "1018DB00") // Super Mario Maker
		{
			sprintf_s(response, NEX_RESPONSE_TEMPLATE,
				SMM_HOST,
				SMM_NEX_PASSWORD,
				SMM_NEX_PID,
				SMM_PORT,
				SMM_TOKEN);
		}
		else
		{
			strcpy_s(response, "nope");
			res.status = 400;
		}
		res.set_content(response, "application/xml;charset=UTF-8");
	});
	svr.set_logger([](const Request &req, const Response &res)
	{
		cout << log(req, res);
	});
	svr.listen("0.0.0.0", 8383);
	return 0;
}