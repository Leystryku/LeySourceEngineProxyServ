#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

inline std::vector<std::string> explode(std::string const & s, char delim)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        result.push_back(std::move(token));
    }

    return result;
}

inline int32_t VectorGetIndex(const char* string, std::vector<std::string> searchin)
{
	auto iterator = std::find(searchin.begin(), searchin.end(), string);
	int32_t index = (int32_t)std::distance(searchin.begin(), iterator);

	if(iterator == searchin.end())
		return -1;

	return index;
}