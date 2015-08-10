#include "Session.hpp"

using namespace std;

int main(int argc, char *argv[]) try
{
  Session session;
  session.run();
  return 0;
}
catch(exception &e)
{
  cerr << "Exception: " << e.what() << endl;
}

