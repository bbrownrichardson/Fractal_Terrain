#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <windows.h>
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <vector>
#include "rgb.hpp"
#include "fractalTerrain.hpp"

using namespace std;

typedef std::vector<double> triple;
typedef std::vector< std::vector<triple > > mapPointMatrix;
typedef std::vector< std::vector<RGB1> > rgbMatrix;

int main(int argc, char **argv) {
  double exaggeration = .7;
  int lod = 5;
  int steps = 1 << lod;
  mapPointMatrix map = mapPointMatrix(steps+1, vector<triple>(steps+1, triple(3)));
  rgbMatrix colors = rgbMatrix(steps+1, vector<RGB1>(steps+1));
  FractalTerrain terrain = FractalTerrain(lod, .5);
  for (int i = 0; i <= steps; ++ i) {
    cout << "---------- i=" << i << " -----------\n";
    for (int j = 0; j <= steps; ++ j) {
      double x = 1.0 * i / steps, z = 1.0 * j / steps;
      double altitude = terrain.getAltitude (x, z);
      cout << "j: " << j << "; x: " << x << "; z: " << z << "\n";
      map[i][j][0] = x;
      map[i][j][1] =  altitude * exaggeration;
      map[i][j][2] = z;
      colors[i][j] = terrain.getColor(x, z);
    }
  }

  cout << "map[16][16][1]: " << map[16][16][1];

}
