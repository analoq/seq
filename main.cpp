#include "Session.hpp"
#include "Gui.hpp"

using namespace std;

int main(int argc, char *argv[]) try
{
  Session session;

  Gui gui { session };

  session.player.start();
  gui.run();
  session.player.stop();

  return 0;
}
catch(exception &e)
{
  cerr << "Exception: " << e.what() << endl;
}

