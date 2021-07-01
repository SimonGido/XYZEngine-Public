#include "stdafx.h"
#include "StringUtils.h"

namespace XYZ::Utils {
	std::string GetFilename(const std::string& filepath)
	{
		std::vector<std::string> parts = SplitString(filepath, "/\\");

		if (parts.size() > 0)
			return parts[parts.size() - 1];

		return "";
	}

	std::string GetFilenameWithoutExtension(const std::string& filepath)
	{
		size_t offset = filepath.find_last_of("/\\") + 1;
		size_t count = filepath.find_last_of('.') - offset;
		return filepath.substr(offset, count);
	}

	std::string GetDirectoryPath(const std::string& filepath)
	{
		size_t offset = filepath.find_last_of("/\\");
		return filepath.substr(0, offset);
	}

	std::string GetExtension(const std::string& filename)
	{
		std::vector<std::string> parts = SplitString(filename, '.');

		if (parts.size() > 1)
			return parts[parts.size() - 1];

		return "";
	}

	std::string RemoveExtension(const std::string& filename)
	{
		return filename.substr(0, filename.find_last_of('.'));
	}

	bool StartsWith(const std::string& string, const std::string& start)
	{
		return string.find(start) == 0;
	}

	std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
	{
		size_t start = 0;
		size_t end = string.find_first_of(delimiters);

		std::vector<std::string> result;

		while (end <= std::string::npos)
		{
			std::string token = string.substr(start, end - start);
			if (!token.empty())
				result.push_back(token);

			if (end == std::string::npos)
				break;

			start = end + 1;
			end = string.find_first_of(delimiters, start);
		}

		return result;
	}

	std::vector<std::string> SplitString(const std::string& string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

	const char* FindToken(const char* str, const std::string& token)
	{
		const char* t = str;
		while (t = strstr(t, token.c_str()))
		{
			bool left = str == t || isspace(t[-1]);
			bool right = !t[token.size()] || isspace(t[token.size()]);
			if (left && right)
				return t;
			t += token.size();
		}
		return nullptr;
	}
}