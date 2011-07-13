/* FakeNES - A portable, Open Source NES emulator.
   Copyright © 2011 Digital Carat

   This is free software. See 'License.txt' for additional copyright and
   licensing information. You must read and accept the license prior to
   any modification or use of this software. */

#include "Color.h"
#include "Local.hpp"

// Function prototypes (defined at bottom).
static real Hue_2_RGB(const real v1, const real v2, real vH);

// --------------------------------------------------------------------------------
// PUBLIC INTERFACE
// --------------------------------------------------------------------------------

void rgb_to_hsl(const int r, const int g, const int b, REAL *h, REAL *s, REAL *l)
{
   Safeguard(h);
   Safeguard(s);
   Safeguard(l);

   real H = H;  // kill warning
   real S;
   real L;

   real var_R = ((r + 1) / 256.0); // Where RGB values = 0 ÷ 255
   real var_G = ((g + 1) / 256.0);
   real var_B = ((b + 1) / 256.0);

   real var_Min = MIN3 (var_R, var_G, var_B);    //Min. value of RGB
   real var_Max = MAX3 (var_R, var_G, var_B);    //Max. value of RGB
   real del_Max = var_Max - var_Min;             //Delta RGB value

   L = (var_Max + var_Min) / 2;

   if (del_Max == 0)                     //This is a gray, no chroma...
   {
      H = 0;                                //HSL results = 0 ÷ 1
      S = 0;
   }
   else                                    //Chromatic data...
   {
      REAL del_R;
      REAL del_G;
      REAL del_B;

      if (L < 0.5)
         S = del_Max / (var_Max + var_Min);
      else
         S = del_Max / (2 - var_Max - var_Min);

      del_R = (((var_Max - var_R) / 6) + (del_Max / 2)) / del_Max;
      del_G = (((var_Max - var_G) / 6) + (del_Max / 2)) / del_Max;
      del_B = (((var_Max - var_B) / 6) + (del_Max / 2)) / del_Max;

      if (var_R == var_Max)
         H = del_B - del_G;
      else if (var_G == var_Max)
         H = (1.0 / 3) + del_R - del_B;
      else if (var_B == var_Max)
         H = (2.0 / 3) + del_G - del_R;

      if (H < 0)
         H += 1;
      if (H > 1)
         H -= 1;
  }

  *h = H;
  *s = S;
  *l = L;
}

void hsl_to_rgb(const REAL h, const REAL s, const REAL l, int* r, int* g, int* b)
{
   Safeguard(r);
   Safeguard(g);
   Safeguard(b);

   int R;
   int G;  
   int B;

   if (s == 0)                       //HSL values = 0 ÷ 1
   {
      R = l * 255;                      //RGB results = 0 ÷ 255
      G = l * 255;
      B = l * 255;
   }
   else
   {
      REAL var_1;
      REAL var_2;

      if (l < 0.5)
         var_2 = l * (1 + s);
      else
         var_2 = (l + s) - (s * l);
   
      var_1 = 2 * l - var_2;
   
      R = 255 * Hue_2_RGB (var_1, var_2, h + (1.0 / 3));
      G = 255 * Hue_2_RGB (var_1, var_2, h);
      B = 255 * Hue_2_RGB (var_1, var_2, h - (1.0 / 3));
   }

   *r = R;
   *g = G;
   *b = B;
}

// --------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// --------------------------------------------------------------------------------

static real Hue_2_RGB(const real v1, const real v2, real vH)             //Function Hue_2_RGB
{
   if (vH < 0)
      vH += 1;
   if (vH > 1)
      vH -= 1;

   if ((6 * vH) < 1)
      return (v1 + (v2 - v1) * 6 * vH);
   if ((2 * vH) < 1)
      return (v2);
   if ((3 * vH) < 2)
      return (v1 + (v2 - v1) * ((2.0 / 3) - vH) * 6);

   return (v1);
}
