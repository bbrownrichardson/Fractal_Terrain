#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

typedef std::vector< std::vector<double> > matrix;

class FractalTerrain {

  matrix terrain;
  double roughness, min, max;
  int divisions;
  double random;
  RGB1 *blue, *green, *white;
  bool hasWater;

public:
  FractalTerrain (int lod, double roughness, bool myHasWater);
  void diamond (int x, int y, int side, double scale);
  void square (int x, int y, int side, double scale);
  double rnd ();
  double getAltitude(double i, double j);
  RGB1 getColor(double i, double j);

};

FractalTerrain::FractalTerrain (int lod, double myRoughness, bool myHasWater) {

  hasWater = myHasWater;

  blue = new RGB1(0, 0, 1);
  green = new RGB1(0, 1, 0);
  white = new RGB1(1, 1, 1);

  roughness = myRoughness;
  divisions = 1 << lod;
  terrain = matrix(divisions+1, vector<double>(divisions+1));
  // rng = rand();

  terrain[0][0] = rnd();
  terrain[0][divisions] = rnd();
  terrain[divisions][divisions] = rnd();
  terrain[divisions][0] = rnd();

  double rough = roughness;
  for (int i = 0; i < lod; i++) {
    int q = 1 << i, r = 1 << (lod - i), s = r >> 1;
    for (int j = 0; j < divisions; j += r) {
      for (int k = 0; k < divisions; k += r) {
        diamond(j, k, r, rough);
      }
    }
    if (s > 0) {
      for (int j = 0; j <= divisions; j += s) {
        for (int k = (j + s) % r; k <= divisions; k += r) {
          square(j - s, k - s, r, rough);
        }
      }
    }
    rough *= roughness;
  }

  min = max = terrain[0][0];
  for (int i = 0; i <= divisions; i++) {
    for (int j = 0; j <= divisions; j++) {
      if (terrain[i][j] < min) {
        min = terrain[i][j];
      }
      else if (terrain[i][j] > max) {
        max = terrain[i][j];
      }
    }
  }
}

void FractalTerrain::diamond (int x, int y, int side, double scale){
  int half = 0;
  float avg = 0.0;
  int num = 0;

  if (side >1) {
  	int half = side/2;
  	avg = (terrain[x][y] + terrain[x + side][y] +
          terrain[x + side][y + side] + terrain[x][y + side]) * 0.25;
    terrain[x+half][y+half] = avg+rnd()*scale;
  }
}

double FractalTerrain::rnd () {

  return  (2 * rand() - 1.0);

}

void FractalTerrain::square (int x, int y, int side, double scale) {
  int half = side / 2;
  double avg = 0.0, sum = 0.0;
  if (x >= 0) {
    avg += terrain[x][y + half];
    sum += 1.0;
  }
  if (y >= 0) {
    avg += terrain[x + half][y];
    sum += 1.0;
  }
  if (x + side <= divisions) {
    avg += terrain[x + side][y + half];
    sum += 1.0;
  }
  if (y + side <= divisions) {
    avg += terrain[x + half][y + side];
    sum += 1.0;
  }
  terrain[x + half][y + half] = avg / sum + rnd() * scale;
}

double FractalTerrain::getAltitude (double i, double j) {
  double alt = terrain[(int) (i * divisions)][(int) (j * divisions)];
  double final_alt = (alt - min) / (max - min);
  if (hasWater && final_alt < 0.3) {
    return 0.3;
  }
  else {
    return final_alt;
  }

}

RGB1 FractalTerrain::getColor (double i, double j) {
    double a = getAltitude (i, j);
    if (a < .5) {
      return blue->add (green->subtract(*blue).scale((a - 0.0) / 0.5));
    } else {
      return green->add (white->subtract(*green).scale((a - 0.5) / 0.5));
    }
  }
