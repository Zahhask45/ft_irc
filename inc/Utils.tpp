#include "colors.hpp"

template <typename T>
std::string toString(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template <typename T>
T extract_value(const T& line) {
    typename T::size_type start = 0;  // Find end of key
    if (start == T::npos) {
        return "";  // Key not found
    }
    while (start < line.length() && (line[start] == ' ' || line[start] == ':')) {
        start++;
    }
    typename T::size_type end = line.find_first_of(" \r\n", start);
    if (end == T::npos) {
        end = line.length();  // If no space or newline, take until the end of the line
    }
    T value = line.substr(start, end - start);

    // Remove any trailing newline character
    if (!value.empty() && value[value.length() - 1] == '\n') {
        value = value.substr(0, value.length() - 1);
    }

    return value;
}
