#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

class RGB1{
public:
  double r, g, b;
  RGB1();
  RGB1(double my_r, double my_g, double my_b);
  RGB1 add(RGB1 rgb);
  RGB1 subtract(RGB1 rgb);
  RGB1 scale(double scale);
  int toInt(double value);
  int toRGB ();
  double getComponent(int component);
};

RGB1::RGB1() {

}

RGB1::RGB1 (double my_r, double my_g, double my_b) {
  r = my_r;
  g = my_g;
  b = my_b;
}

RGB1 RGB1::add (RGB1 rgb) {
  RGB1 myRGB(r + rgb.r, g + rgb.g, b + rgb.b);
  return myRGB;
}

RGB1 RGB1::subtract (RGB1 rgb) {
  RGB1 myRGB(r - rgb.r, g - rgb.g, b - rgb.b);
  return myRGB;
}

RGB1 RGB1::scale (double scale) {
  RGB1 myRGB(r * scale, g * scale, b * scale);
  return myRGB;
}

int RGB1::toInt (double value) {
  return (value < 0.0) ? 0 : (value > 1.0) ? 255 : (int) (value * 255.0);
}

int RGB1::toRGB () {
  return (0xff << 24) | (toInt (r) << 16) | (toInt (g) << 8) | toInt (b);
}

double RGB1::getComponent(int component) {
  switch (component) {
    case 0:
      return r;
      break;
    case 1:
      return g;
      break;
    case 2:
      return b;
      break;
    default:
      return 257;
      break;
  }
}
