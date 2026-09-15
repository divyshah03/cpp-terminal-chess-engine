#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include "window.h"

using namespace std;

Xwindow::Xwindow(int width, int height) : width{width}, height{height} {

  d = XOpenDisplay(NULL);
  if (d == NULL) {
    cerr << "Cannot open display" << endl;
    exit(1);
  }
  s = DefaultScreen(d);
  w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, width, height, 1,
                          BlackPixel(d, s), WhitePixel(d, s));
  XSelectInput(d, w, ExposureMask | KeyPressMask);
  XMapRaised(d, w);

  Pixmap pix = XCreatePixmap(d,w,width,
        height,DefaultDepth(d,DefaultScreen(d)));
  gc = XCreateGC(d, pix, 0,(XGCValues *)0);

  font = XLoadQueryFont(d, "-misc-fixed-*-*-*-*-20-*-*-*-*-*-*-*");
  if (!font) {
      font = XLoadQueryFont(d, "fixed"); // fallback
  }
  if (!font) {
      cerr << "Unable to load any usable font" << endl;
      exit(1);
  }

  XSetFont(d, gc, font->fid);

  XFlush(d);
  XFlush(d);

  // Set up colours.
  XColor xcolour;
  Colormap cmap;
  const char* color_vals[11] = {
    "white",        // WHITE
    "black",        // BLACK
    "black",        // NONE - mapped to black
    "gray",         // GREY
    "green",        // GREEN
    "burlywood1",   // LIGHTBROWN
    "saddle brown", // DARKBROWN
    "pink",         // PINK
    "red",          // RED
    "blue",         // BLUE
    "orange"        // ORANGE
  };

  cmap=DefaultColormap(d,DefaultScreen(d));
  for(int i=0; i < 11; ++i) {
      XParseColor(d,cmap,color_vals[i],&xcolour);
      XAllocColor(d,cmap,&xcolour);
      colours[i]=xcolour.pixel;
  }

  XSetForeground(d,gc,colours[static_cast<int>(Colour::BLACK)]);

  // Make window non-resizeable.
  XSizeHints hints;
  hints.flags = (USPosition | PSize | PMinSize | PMaxSize );
  hints.height = hints.base_height = hints.min_height = hints.max_height = height;
  hints.width = hints.base_width = hints.min_width = hints.max_width = width;
  XSetNormalHints(d, w, &hints);

  XSynchronize(d,True);

  usleep(1000);
}

Xwindow::~Xwindow() {
  // The font and the graphics context must be released before the display is closed
  if (font) XUnloadFont(d, font->fid);
  XFreeGC(d, gc);
  XCloseDisplay(d);
}

int Xwindow::getWidth() const { return width; }
int Xwindow::getHeight() const { return height; }

void Xwindow::fillRectangle(int x, int y, int width, int height, Colour colour) {
  XSetForeground(d, gc, colours[static_cast<int>(colour)]);
  XFillRectangle(d, w, gc, x, y, width, height);
  XSetForeground(d, gc, colours[static_cast<int>(Colour::BLACK)]);
}

void Xwindow::drawString(int x, int y, string msg, Colour colour) {
  XSetForeground(d, gc, colours[static_cast<int>(colour)]);
  XDrawString(d, w, gc, x, y, msg.c_str(), msg.length());
  XSetForeground(d, gc, colours[static_cast<int>(Colour::BLACK)]);
}

XFontStruct *Xwindow::getFontStruct() const {
    return font;
}
