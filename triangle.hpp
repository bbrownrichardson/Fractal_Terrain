#ifndef TRIANGLE_HPP_INCLUDED
#define TRIANGLE_HPP_INCLUDED

#include <stdlib.h>

class Triangle {
  int i[3];
  int j[3];
  Triple n;
  RGB1 rgb[3];

public:
  RGB1 color;
  Triangle();
  Triangle (int i0, int j0, RGB1 rgb0, int i1, int j1, RGB1 rgb1, int i2, int j2, RGB1 rgb2);
  int get_i (int index);
  int get_j (int index);
  void set_n(Triple my_n);
  RGB1 getRgb(int index);
  void setRgb(int index, RGB1 myColor);
  RGB1 getColor();
  void setColor(RGB1 myColor);


};

Triangle::Triangle() {
  n = Triple(0, 0, 0);
}

Triangle::Triangle (int i0, int j0, RGB1 rgb0, int i1, int j1, RGB1 rgb1, int i2, int j2, RGB1 rgb2) {
  i[0] = i0;
  i[1] = i1;
  i[2] = i2;
  j[0] = j0;
  j[1] = j1;
  j[2] = j2;
  rgb[0] = rgb0;
  rgb[1] = rgb1;
  rgb[2] = rgb2;
  n = Triple(0, 0, 0);
}

int Triangle::get_i (int index) {
  return i[index];
}

int Triangle::get_j (int index) {
  return j[index];
}

void Triangle::set_n(Triple my_n) {
  n = my_n;
}

RGB1 Triangle::getRgb(int index) {
  return rgb[index];
}

void Triangle::setRgb(int index, RGB1 myRgb) {
  rgb[index] = myRgb;
}
RGB1 Triangle::getColor() {
  return color;
}

void Triangle::setColor(RGB1 myColor) {
  color = myColor;
}


#endif // TRIANGLE_HPP_INCLUDED
