#include "Utils/StrOperations.h"

using namespace std;

vector<string> splitStr(const string &s, const string &delim) {
	vector<string> result;
	size_t start = 0, pos;
	while ((pos = s.find(delim, start)) != string::npos) {
		string token = s.substr(start, pos - start);
		if (!token.empty()) {
			result.emplace_back(std::move(token));
		}
		start = pos + delim.length();
	}
	string token = s.substr(start);
	if (!token.empty()) {
		result.emplace_back(std::move(token));
	}
	return result;
}

