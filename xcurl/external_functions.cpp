#include"external_functions.hpp"

std::string demultiply(std::string str, char x)
{
	unsigned int j = 0;
	for (unsigned int i = 0; i < str.size(); i++)
	{
		if (str[i] == x)
		{
			if (j == 0)
				j++;
			else
				str.erase(i, 1);
		}
		else
		{
			if (j != 0) j = 0;
		}
	}
	return str;
}

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
	using namespace std;
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

std::string charXchange(std::string& str, char x, char y)
{
	for (auto c : str)
	{
		if (c == x) c = y;
	}
	return str;
}

