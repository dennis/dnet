#include "history.h"
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

/////////////////////////////

Tank::Tank() : active_(false)
{
}

Tank::Tank(double x, double y, double angle, double vx, double vy) :
  active_(true), x_(x), y_(y), angle_(angle), vx_(vx), vy_(vy)
{
}

/////////////////////////////

Shot::Shot() : active_(false)
{
}

Shot::Shot(double x, double y, double vx, double vy) :
  active_(true), x_(x), y_(y), vx_(vx), vy_(vy)
{
}

/////////////////////////////

Iteration::Iteration(double t) : t_(t)
{
}

void Iteration::add(int index, const Tank &tank)
{
  if (index >= int(tanks_.size()))
    tanks_.resize(index+1);
  tanks_[index] = tank;
}

void Iteration::add(int index, const Shot &shot)
{
  if (index >= int(shots_.size()))
    shots_.resize(index+1);
  shots_[index] = shot;
}

/////////////////////////////

History::History(const string &filename, int id)
{
  ifstream input(filename.c_str());
  if (!input)
    throw string("file open failed");

  if (!(input >> worldsize_ >> tankradius_ >> tanklength_ >> tankwidth_))
    throw string("read of world parameters (size and tank dimesnsions) failed");

  int iter=0;
  double t, oldt=-0.0001;
  string line;

  while (input >> t)
  {
    if (t <= oldt)
    {
      ostringstream error;
      error << "backward time step: from " << oldt << " to " << t;
      throw error.str();
    }
    oldt = t;
    iter++;

    Iteration Iterat(t);
    getline(input, line);
    while (getline(input, line) && line!="")
    {
      istringstream ls(line);
      char type; ls >> type;
      switch (type)
      {
        case 't':
          {
            int index;
            double x,y,angle,vx,vy;
            if (!(ls >> index >> x >> y >> angle >> vx >> vy))
              throw string("error while parsing tank line: ")+line;
			if( index == id )
				Iterat.add(index, Tank(x,y,angle,vx,vy));
            break;
          }

        case 's':
          {
            int index;
            double x,y,vx,vy;
            if (!(ls >> index >> x >> y >> vx >> vy))
              throw string("error while parsing shot line: ")+line;
            //Iterat.add(index, Shot(x,y,vx,vy));
            break;
          }

        default:
          throw string("unknown type: ")+type;
      }
    }

    history_.push_back(Iterat);
  }
  numiters_ = iter;
}

