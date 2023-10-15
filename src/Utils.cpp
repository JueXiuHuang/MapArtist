#include <iomanip>
#include <string>

using namespace std;

string GetTime() {
  auto t = time(nullptr);
  auto tm = *localtime(&t);
  ostringstream oss;
  oss << "[" << put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
  return oss.str();
};