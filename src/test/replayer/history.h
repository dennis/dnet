#ifndef HISTORY_h
#define HISTORY_h

#include <vector>
#include <string>

//////////////////////////////

class Tank
{
public:
  Tank();
  Tank(double x, double y, double angle, double vx, double vy);
  bool isActive() const {return active_;}
  double x() const {return x_;}
  double y() const {return y_;}
  double vx() const {return vx_;}
  double vy() const {return vy_;}
  double angle() const {return angle_;}

private:
  bool active_;
  double x_, y_, angle_, vx_, vy_;
};

//////////////////////////////

class Shot
{
public:
  Shot();
  Shot(double x, double y, double vx, double vy);
  bool isActive() const {return active_;}
  double x() const {return x_;}
  double y() const {return y_;}
  double vx() const {return vx_;}
  double vy() const {return vy_;}

private:
  bool active_;
  double x_, y_, vx_, vy_;
};


//////////////////////////////

class Iteration
{
public:
  Iteration(double t);
  void add(int i, const Tank &t);
  void add(int i, const Shot &s);
  const Tank& getTank(int i) const {return tanks_[i];}
  const Shot& getShot(int i) const {return shots_[i];}
  int numTanks() const {return tanks_.size();}
  int numShots() const {return shots_.size();}
  double t() const {return t_;}

private:
  double t_;
  std::vector<Tank> tanks_;
  std::vector<Shot> shots_;
};

//////////////////////////////

class History
{
public:
  History(const std::string &filename, int = 0);
  int iters() const {return numiters_;}
  const Iteration& iter(int it) const {return history_[it];}
  double worldSize()  const {return worldsize_;}
  double tankRadius() const {return tankradius_;}
  double tankLength() const {return tanklength_;}
  double tankWidth()  const {return tankwidth_;}

private:
  std::vector<Iteration> history_;
  double worldsize_;
  double tankradius_, tanklength_, tankwidth_;
  int numiters_;
};

#endif
