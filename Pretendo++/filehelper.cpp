#include "filehelper.h"

bool FileHelper::ReadAllData(const String & fileName, std::vector<unsigned char> & content)
{
	FILE* fp = nullptr;
#ifdef _MSC_VER
	fopen_s(&fp, fileName.c_str(), "rb");
#else
	fp = fopen(fileName.c_str(), "rb");
#endif
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	auto size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	content.resize(size);
	fread(content.data(), size, 1, fp);
	fclose(fp);
	return true;
}

bool FileHelper::WriteAllData(const String & fileName, const void* data, size_t size)
{
	FILE* fp = nullptr;
#ifdef _MSC_VER
	fopen_s(&fp, fileName.c_str(), "wb");
#else
	fp = fopen(fileName.c_str(), "wb");
#endif
	fwrite(data, size, 1, fp);
	fclose(fp);
	return true;
}

bool FileHelper::ReadAllText(const String & fileName, String & content)
{
	std::vector<unsigned char> data;
	if (!ReadAllData(fileName, data))
		return false;
	data.push_back(0);
	content = String((const char*)data.data());
	return true;
}

bool FileHelper::WriteAllText(const String & fileName, const String & content)
{
	return WriteAllData(fileName, content.c_str(), content.length());
}

bool FileHelper::ReadAllLines(const String & fileName, std::vector<String> & lines, bool keepEmpty)
{
	String content;
	if (!ReadAllText(fileName, content))
		return false;
	lines.clear();
	String line;
	for (auto ch : content)
	{
		switch (ch)
		{
		case '\r':
			break;
		case '\n':
			if (line.length() || keepEmpty)
				lines.push_back(line);
			line.clear();
			break;
		default:
			line.resize(line.length() + 1);
			line[line.length() - 1] = ch;
			break;
		}
	}
	if (line.length())
		lines.push_back(line);
	return true;
}

String FileHelper::GetFileName(const String & fileName)
{
	auto last = strrchr(fileName.c_str(), '\\');
	return last ? last + 1 : fileName;
}
