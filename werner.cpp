#include <cmath>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <ctime>

// ouput slabfield every log_interval ticks
const int log_interval = 50;

// random double in the range [0,1)
double randZeroToOne()
{
  return std::rand() / (RAND_MAX + 1.);
}

// coordinates indexing the slabfield
struct pos_t {
  pos_t(int x, int y) : x(x), y(y) {};
  int x;
  int y;
};


// PARAMETERS

const int neighbors = 8;
const pos_t pNZ(-1,0), pPZ(1,0), pZN(0,-1), pZP(0,1),
  pNN(-1,-1), pPN(1,-1), pNP(-1,1), pPP(1,1);
const pos_t nbhd[8] = {pNZ, pPZ, pZN, pZP, pNN, pPN, pNP, pPP};


// assumes 3:1 width:height aspect ratio, also heights are integer valued
// note that this aspect ratio also determines is_shadowed
const int repose_diff = 2;

const int wind_speed = 5;
const pos_t wind_dir(1,0);

// deposition probabilities
const double p_slab = 0.6;
const double p_floor = 0.4;


// the basic object is a Slabfield,
// which serves as a static array indexed by pos_t

class Slabfield
{
public:
  Slabfield();
  Slabfield(int n, int m, int h = 0);
  //  Slabfield(std::istream& if);
  ~Slabfield();

  int& operator()(pos_t p)      { return slabs[p.x*n + p.y]; };
  const int& operator()(pos_t p) const { return slabs[p.x*n + p.y]; };

  int width() { return n; }
  int height() { return m; }

  // both of these respect the "angle of repose" condition
  void deposit(pos_t p);
  void erode(pos_t p);

  pos_t saltate(pos_t p);
  bool is_shadowed(pos_t p);

  void tick();

  friend std::ostream& operator<<(std::ostream& os, const Slabfield& s);
  friend std::istream& operator>>(std::istream& is, Slabfield& s);

  // helpers
  pos_t add_pos(pos_t p1, pos_t p2);
  pos_t rand_cell();
  pos_t rand_slab();
  
private:
  int* slabs;
  int n,m;

  void avalanche_up(pos_t p);
  void avalanche_down(pos_t p);
};


int Slabfield_getitem(Slabfield &s, pos_t p)
{
  return s(p);
}

void Slabfield_setitem(Slabfield &s, pos_t p, int value)
{
  s(p) = value;
}

Slabfield::Slabfield(){

}

Slabfield::Slabfield(int n, int m, int h)
{
  this->n = n; this->m = m;
  slabs = new int[n*m];
  int i;
  for(i = 0; i < n*m; i++)
    slabs[i] = h;
}

Slabfield::~Slabfield()
{
  delete slabs;
}

// offsets p1 by p2, with periodic boundary conditions
pos_t Slabfield::add_pos(pos_t p1, pos_t p2) {
  pos_t p3((p1.x + p2.x) % n, (p1.y + p2.y) % m);
  return p3;
}

pos_t Slabfield::rand_cell() {
  pos_t p(std::rand() % n, std::rand() % m);
  return p;
}

// returns a random_slab, i.e. a nonempty cell
pos_t Slabfield::rand_slab() {
  // this *is* uniform sampling of slabs
  while (true) {
    pos_t p = rand_cell();
    if ((*this)(p) > 0)
      return p;
  }
}

void Slabfield::deposit(pos_t p) {
  (*this)(p) += 1;
  // we only need to correct for "angle of repose"
  // at this location
  avalanche_down(p);
}

void Slabfield::erode(pos_t p) {
  (*this)(p) -= 1;
  // we only need to correct for "angle of repose"
  // at this location
  avalanche_up(p);
}

// enforces a local "angle of repose" condition,
// for positions that have just increased in height
void Slabfield::avalanche_down(pos_t p) {
  for(int i = 0; i < neighbors; i++) {
     pos_t p2 = add_pos(p, nbhd[i]);
    if (((*this)(p) - (*this)(p2)) > repose_diff) {
      (*this)(p) -= 1;
      (*this)(p2) += 1;
      avalanche_down(p2);
      return;
    }
  }
}

// same, but for positions that have decreased in height
void Slabfield::avalanche_up(pos_t p) {
  for(int i = 0; i < neighbors; i++) {
    pos_t p2 = add_pos(p, nbhd[i]);
    if (((*this)(p2) - (*this)(p)) > repose_diff) {
      (*this)(p) += 1;
      (*this)(p2) -= 1;
      avalanche_up(p2);
      return;
    }
  }
}

// from a grain "above" pos_t, move in the wind direction
// and return the landing position
pos_t Slabfield::saltate(pos_t p) {
  double l;
  int h;
  pos_t wind(wind_speed*wind_dir.x, wind_speed*wind_dir.y);
  
  while (true) {
    p = add_pos(p,wind);
    l = randZeroToOne();
    h = (*this)(p);
    if ((h > 0) && l < p_slab) {
      return p;
    }
    if ((h <= 0) && l < p_floor) {
      return p;
    }
    if (is_shadowed(p))
      return p;
  }
}

// the shadowing condition relies on the aspect ratio,
// which we have hardcoded at 3:1 w:h
bool Slabfield::is_shadowed(pos_t p) {
  pos_t upwind(-wind_dir.x,-wind_dir.y);
  int h = (*this)(p);
  for (int i = 0; i < 5; i++) { // we only check  up to 5 cells away
    p = add_pos(p, upwind);
    if ((*this)(p) - h > i) // hardcoded
      return true;
  }
  return false;
}

// this format is just the numpy text file format for integer arrays
// together with a header that specifies <width> <height>
std::ostream& operator<<(std::ostream& os, const Slabfield& s) {
  os << s.n << " " << s.m << "\n";
  for (int i = 0; i < s.n; i++) {
    for (int j = 0; j < s.m; j++) {
      pos_t p(i,j);
      os << s(p) << " ";
    }
    os << "\n";
  }
  return os;
}

std::istream& operator>>(std::istream& is, Slabfield& s) {
  delete s.slabs;
  
  is >> s.n;
  is >> s.m;
  
  s.slabs = new int[s.n*s.m];
  
  for(int i = 0; i < s.n; i++) {
    for(int j = 0; j < s.m; j++) {
      pos_t p(i,j);
      is >> s(p);
    }
  }
  
  return is;
}

// a "tick" moves N randomly selected slabs,
// where N is the total number of cells.
// this gives a """dimensionless""" unit of time
void Slabfield::tick() {
  for(int i = 0; i < n*m; i++) {
    pos_t p = this->rand_slab();
    this->erode(p);
    p = this->saltate(p);
    this->deposit(p);
  }
}

 
// usage: werner <num_ticks> <input_file> <output_file>
// right now we display the iteration counts
// and when DUMP is defined, the corresponding temp directory
int main(int argc, char** argv)
{ 
  std::srand( std::time(0));

  int ticks = std::stoi(argv[1]);

  std::ifstream input;
  input.open(argv[2]);
  Slabfield s;
  input >> s;
  input.close();

  char tmpdir[] = "/tmp/XXXXXX";
  mkdtemp(tmpdir);

  std::ofstream output;
  
  for(int i = 0; i < ticks; i++) {
    std::cout << "iter " << i << std::endl;
    s.tick();
#ifdef DUMP
    if ((i % log_interval) == 0) {
  std::stringstream ss;
  ss << tmpdir << "/" << std::setw(10) << std::setfill('0') << i;
  output.open(ss.str().c_str());
  output << s;
  output.close();
    }
#endif
  }

#ifdef DUMP
  std::cout << "\n\ndumped intermediate slabfields to " << tmpdir << std::endl;
#endif
  output.open(argv[3]);
  output << s;
  output.close();
}
// #include <boost/python.hpp>

// using namespace boost::python;

// BOOST_PYTHON_MODULE(werner)
// {
//   class_<pos_t>("Pos", init<int, int>())
//     .def_readwrite("x", &pos_t::x)
//     .def_readwrite("y", &pos_t::y)
//     ;

//   class_<Slabfield>("Slabfield",init<int, int, int>())
//     .def("__getitem__", &Slabfield_getitem)
//     .def("__setitem__", &Slabfield_setitem)
//     .add_property("width", &Slabfield::width)
//     .add_property("height", &Slabfield::height)
//     .def("tick", &Slabfield::tick)
//     ;
// }
