#include <iostream>
#include "httplib.h"
#include "Utf8Ini.h"
#include "filehelper.h"
#include "stringutils.h"

using namespace std;
using namespace httplib;

#define OAUTH_RESPONSE_TEMPLATE R"(<OAuth20><access_token><token>%s</token><refresh_token>%s</refresh_token><expires_in>%s</expires_in></access_token></OAuth20>)"
#define NEX_RESPONSE_TEMPLATE R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><nex_token><host>%s</host><nex_password>%s</nex_password><pid>%s</pid><port>%s</port><token>%s</token></nex_token>)"
#define SERVICE_TOKEN_RESPONSE_TEMPLATE R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><service_token><token>%s</token></service_token>)"

static string dump_headers(const Headers &headers)
{
	string s;

	for(const auto &x : headers) {
		s += x.first;
		s += ": ";
		s += x.second;
		s += "\n";
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
	// disable buffering
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	String ini;
	if(!FileHelper::ReadAllText("Pretendo++.ini", ini))
	{
		puts("Failed to read Pretendo++.ini");
		return 1;
	}
	Utf8Ini settings;
	int errorLine = 0;
	if(!settings.Deserialize(ini, errorLine))
	{
		printf("Failed to parse Pretendo++.ini (line: %d)\n", errorLine);
		return 1;
	}
	Server svr;
	svr.Post("/v1/api/oauth20/access_token/generate", [&settings](const Request& req, Response& res)
	{
		auto response = StringUtils::sprintf(OAUTH_RESPONSE_TEMPLATE,
			settings.GetValue("OAuth20", "access_token").c_str(),
			settings.GetValue("OAuth20", "refresh_token").c_str(),
			settings.GetValue("OAuth20", "expires_in").c_str()
		);
		puts(response.c_str());
		res.set_content(response, "text/xml");
	});
	svr.Get("/v1/api/provider/nex_token/@me", [&settings](const Request& req, Response& res)
	{
		auto game_server_id = req.get_param_value("game_server_id");
		std::string response;
		if(!settings.Keys(game_server_id).empty())
		{
			response = StringUtils::sprintf(NEX_RESPONSE_TEMPLATE,
				settings.GetValue(game_server_id, "host").c_str(),
				settings.GetValue(game_server_id, "password").c_str(),
				settings.GetValue(game_server_id, "pid").c_str(),
				settings.GetValue(game_server_id, "port").c_str(),
				settings.GetValue(game_server_id, "token").c_str()
			);
		}
		else
		{
			response = StringUtils::sprintf("unknown game_server_id %s", game_server_id.c_str());
			res.status = 400;
		}
		res.set_content(response, "application/xml;charset=UTF-8");
	});
	svr.Get("/v1/api/provider/service_token/@me", [&settings](const Request& req, Response& res)
	{
		auto response = StringUtils::sprintf(SERVICE_TOKEN_RESPONSE_TEMPLATE,
			settings.GetValue("OAuth20", "service_token").c_str());
		res.set_content(response, "application/xml;charset=UTF-8");
	});
	svr.Get("/ping", [](const Request& req, Response& res)
	{
		res.set_content("pong", "text/plain");
	});
	svr.set_logger([](const Request &req, const Response &res)
	{
		printf("%s", log(req, res).c_str());
	});
	svr.Post("/post", [](const Request& req, Response& res)
	{
		if(req.has_file("file"))
		{
			MultipartFile file = req.get_file_value("file");
			std::string data = req.body.substr(file.offset, file.length);
			uint64_t now = time(0);
			char filename[64] = "";
			sprintf_s(filename, "file%llu.bin", now);
			FILE* f = nullptr;
			fopen_s(&f, filename, "wb");
			if(f)
			{
				fwrite(data.data(), data.size(), 1, f);
				fclose(f);
			}
		}
		else
		{
			res.status = 400;
		}
	});
	svr.set_base_dir("www");
	auto host = "127.0.0.1";
	auto port = 8383;
	printf("serving on %s:%d\n", host, port);
	if (!svr.listen(host, port))
	{
		puts("failed to bind");
		return 1;
	}
	return 0;
}