#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "globals.h"

double hueToRgb(double p, double q, double t)
{
    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    if (t < 1 / 6.0)
        return p + (q - p) * 6 * t;
    if (t < 1 / 2.0)
        return q;
    if (t < 2 / 3.0)
        return p + (q - p) * (2 / 3.0 - t) * 6;
    return p;
}

rgb HexToRGB(const char *hex)
{
    int r, g, b;

    // Convert hex to RGB
    sscanf(hex, "#%2x%2x%2x", &r, &g, &b);

    rgb rgb;
    rgb.r = r / 255.0;
    rgb.g = g / 255.0;
    rgb.b = b / 255.0;

    return rgb;
}

hsl HexToHSL(const char *hex)
{
    rgb rgb = HexToRGB(hex);
    hsl hsl;

    // Convert RGB to HSL
    double max = fmax(rgb.r, fmax(rgb.g, rgb.b));
    double min = fmin(rgb.r, fmin(rgb.g, rgb.b));
    double diff = max - min;

    hsl.l = (max + min) / 2.0;

    if (max == 0)
    {
        hsl.h = 0; // undefined
        hsl.s = 0;
    }
    else
    {
        hsl.s = hsl.l > 0.5 ? diff / (2.0 - max - min) : diff / (max + min);

        if (max == rgb.r)
        {
            hsl.h = (rgb.g - rgb.b) / diff + (rgb.g < rgb.b ? 6 : 0);
        }
        else if (max == rgb.g)
        {
            hsl.h = (rgb.b - rgb.r) / diff + 2;
        }
        else
        {
            hsl.h = (rgb.r - rgb.g) / diff + 4;
        }

        hsl.h /= 6;
    }
    return hsl;
}

char *HSLToHex(hsl hsl, char *hex)
{
    rgb rgb;

    // Convert HSL to RGB
    double q = hsl.l < 0.5 ? hsl.l * (1 + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
    double p = 2 * hsl.l - q;

    rgb.r = hueToRgb(p, q, hsl.h + 1 / 3.0);
    rgb.g = hueToRgb(p, q, hsl.h);
    rgb.b = hueToRgb(p, q, hsl.h - 1 / 3.0);

    rgb.r *= 255;
    rgb.g *= 255;
    rgb.b *= 255;

    // Convert RGB to hex
    sprintf(hex, "#%02x%02x%02x", (int)(rgb.r + 0.5), (int)(rgb.g + 0.5), (int)(rgb.b + 0.5));

    return hex;
}

hsl rotateHue(hsl hsl, double degree)
{
    hsl.h = fmod(hsl.h + degree / 360.0, 1);
    return hsl;
}
