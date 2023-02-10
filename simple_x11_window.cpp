/**
 * From video turorial "X11 C & C++ Tutorial - Window, Input, Drawing"
 * (https://www.youtube.com/watch?v=qZmJwk2xrJ0)
**/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include<X11/Xos.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <stdlib.h>

Display *dis;
int screen;
Window win;
GC gc;
Cursor main_cursor;
unsigned long black, white, red, blue;
int current_font = XC_X_cursor;

void init();
void close();
void draw();
unsigned long RGB(int r, int g, int b);

struct coord {
  int x, y;
} dot;

int main () {
  init();
  int screen_height = XDisplayHeight(dis, 0);
  int screen_width = XDisplayWidth(dis, 0);
  int screen_area = screen_height * screen_width;
  Window test_screen = DefaultRootWindow(dis);
  printf("Display dimentions are %d px by %d px.\n", screen_height, screen_width);
  printf("Total area is %d px^2.\n", screen_area);
  XEvent event;
  KeySym key;
  char text[255];
  while (1)
  {
    XNextEvent(dis, &event);
    if(event.type==Expose && event.xexpose.count==0) {
      draw();
    }

    if(event.type==KeyPress && XLookupString(&event.xkey, text, 255, &key,0)==1) {
      if(text[0] == 'q') {
        close();
      }

      if(text[0] == 'd') {
        current_font = (current_font + 1) % 154;
        main_cursor = XCreateFontCursor(dis, (current_font) % 154);
        XDefineCursor(dis, win, main_cursor);
      }
      else if(text[0] == 'a') {
        current_font = ((current_font - 2) < 0) ? current_font + 152 : current_font - 2;
        main_cursor = XCreateFontCursor(dis, (current_font) % 154);
        XDefineCursor(dis, win, main_cursor);
      }
      else {
        printf("You pressed the %c key!\n", text[0]);
      }

    }

    if(event.type==ButtonPress) {
      int x=event.xbutton.x, y=event.xbutton.y;
      XSetForeground(dis,gc,red);
      XDrawLine(dis,win,gc,dot.x,dot.y,x,y);
      XSetForeground(dis,gc,blue);
      strcpy(text, "Hello World");
      
      XDrawString(dis,win,gc,x,y,text,strlen(text));
      dot.x = x;
      dot.y = y;
    }
  }
  return 0;
}

void init () {
  //Font Cursors listed at https://tronche.com/gui/x/xlib/appendix/b/
  dot.x=100;
  dot.y=100;
  dis=XOpenDisplay((char *)0);
  screen=DefaultScreen(dis);
  black=BlackPixel(dis, screen);
  white=WhitePixel(dis, screen);
  red=RGB(255,0,0);
  blue=RGB(0,0,255);
  win=XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0, 300, 300, 10, red, white);
  main_cursor = XCreateFontCursor(dis, current_font);
  XDefineCursor(dis, win, main_cursor);
  XSetStandardProperties(dis, win, "Howdy", "Hi", None, NULL, 0, NULL);
  XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);
  gc=XCreateGC(dis, win, 0, 0);
  XSetBackground(dis,gc,white);
  XSetForeground(dis, gc, black);
  XClearWindow(dis, win);
  XMapRaised(dis, win);

}
void close () {
  XFreeGC(dis, gc);
  XDestroyWindow(dis, win);
  XCloseDisplay(dis);
  exit(0);
}

void draw () {
  XClearWindow(dis, win);
}

unsigned long RGB(int r, int g, int b) {
  return b + (g<<8) + (r<<16);
};
