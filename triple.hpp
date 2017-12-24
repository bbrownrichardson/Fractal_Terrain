#ifndef TRIPLE_HPP_INCLUDED
#define TRIPLE_HPP_INCLUDED

#include <stdlib.h>
#include <iostream>
#include <cmath>

using namespace std;

class Triple {

public:
  double x, y, z;
  Triple();
  Triple(double myX, double myY, double myZ);
  Triple add(Triple t);
  Triple subtract(Triple t);
  Triple cross(Triple t);
  double dot(Triple t);
  double length2();
  Triple normalize();
  Triple scale(double scale);
  double getComponent(int index);
  void display();
  double operator[] (int index);
};

Triple::Triple() {
}

Triple::Triple(double myX, double myY, double myZ) {
  x = myX;
  y = myY;
  z = myZ;
}


Triple Triple::add(Triple t) {
  Triple myTriple(x + t.x, y + t.y, z + t.z);
  return myTriple;
}


Triple Triple::subtract(Triple t) {
  Triple myTriple(x - t.x, y - t.y, z - t.z);
  return myTriple;
}


Triple Triple::cross(Triple t) {
  Triple myTriple(y * t.z - z * t.y, z * t.x - x * t.z,
      x * t.y - y * t.x);
  return myTriple;
}


double Triple::dot(Triple t) {

  return (x * t.x) + (y * t.y) + (z * t.z);
}


double Triple::length2() {
  return dot(*this);
}


Triple Triple::normalize() {
  return scale(1.0 / std::sqrt(length2()));
}


Triple Triple::scale(double scale) {
  Triple myTriple(x * scale, y * scale, z * scale);
  return myTriple;
}

double Triple::getComponent(int index) {
  switch (index) {
    case 0:
      return x;
      break;
    case 1:
      return y;
      break;
    case 2:
      return z;
      break;
    default:
      break;
  }
}

void Triple::display() {
  cout << "X: " << x << "\n";
  cout << "Y: " << y << "\n";
  cout << "Z: " << z << "\n";
}

double Triple::operator[] (int index) {
  switch (index) {
    case 0:
      return x;
      break;
    case 1:
      return y;
      break;
    case 2:
      return z;
      break;
    default:
      break;
  }
}


#endif // TRIPLE_HPP_INCLUDED
