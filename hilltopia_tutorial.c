/** Following tutorial starting at
 * http://xopendisplay.hilltopia.ca/2009/Jan/Xlib-tutorial-part-1----Beginnings.html
**/


/* first include the standard headers that we're likely going to need */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static XrmOptionDescRec xrmTable[] = {
    {"-bg", "*background", XrmoptionSepArg, NULL},
    {"-fg", "*foreground", XrmoptionSepArg, NULL},
    {"-bc", "*bordercolour", XrmoptionSepArg, NULL},
    {"-font", "*font", XrmoptionSepArg, NULL},
  };

int utf8toXChar2b(XChar2b *output_r, int outsize, const char *input, int inlen){
  int j, k;
  for( j = 0, k = 0; j < inlen && k < outsize; j++){
    unsigned char c = input[j];
    if ( c < 128) {
      output_r[k].byte1 = 0;
      output_r[k].byte2 = c;
      k++;
    } else if ( c < 0xC0) {
      /* we're inside a character we don't know */
      continue;
    } else switch(c&0xF0) {
      case 0xC0: case 0xD0: /* two bytes 5 + 6 = 11 bits*/
        if(inlen < j + 1) { return k; }
        output_r[k].byte1 = (c&0x1C) >> 2;
        j++;
        output_r[k].byte2 = ((c&0x3) << 6) + (input[j]&0x3F);
        k++;
        break;
      case 0xE0: /* three bytes 4+6+6 = 16 bits*/ 
        if (inlen < j+2){ return k; }
        j++;
        output_r[k].byte1 = ((c&0xF) << 4) + ((input[j]&0x3C) >> 2);
        c = input[j];
        j++;
        output_r[k].byte2 = ((c&0x30) << 6) + (input[j]&0x3F);
        k++;
        break;
      case 0xFF:
        /* the character uses more than 16 bits */
        continue;
    }
  }
  return k;
}

unsigned long getColour(Display *dpy, XrmDatabase db, char *name, char *cl, char *def) {
  XrmValue v;
  XColor col1, col2;
  Colormap cmap = DefaultColormap(dpy, DefaultScreen(dpy));
  char *type;

  if (XrmGetResource(db, name, cl, &type, &v) && XAllocNamedColor(dpy, cmap, v.addr, &col1, &col2)) {

  } else {
    
    XAllocNamedColor(dpy, cmap, def, &col1, &col2);

    if (v.addr)
      fprintf(stderr, "unable to find preferred color: %s using default\n", v.addr);
    else
      fprintf(stderr, "unable to find preferred color\n");

  }

  return col2.pixel;
}

XFontStruct *getFont(Display *dpy, XrmDatabase db, char *name, char *cl, char *def) {
  XrmValue v;
  char * type;
  XFontStruct *font = NULL;
  int nmissing;
  char **missing;
  char *def_string;

  if (XrmGetResource(db, name, cl, &type, &v)){
    if (v.addr)
      font = XLoadQueryFont(dpy, v.addr);
  }

  if (!font) {
    if (v.addr)
      fprintf(stderr, "unable to find preferred font: %s using fixed\n", v.addr);
    else
      fprintf(stderr, "unable to find preferred font\n");

      font = XLoadQueryFont(dpy, def);

  }
  XFreeStringList(missing);

  return font;
}

GC setup(Display * dpy, int argc, char ** argv, int *width_r, int *height_r, XFontStruct **font_r) {
  int width, height;
  int screen_num;
  unsigned long background, border;
  Window win;
  GC pen;
  XGCValues values;

  XFontStruct* font;
  XrmDatabase db;

  
  
  // char *fontname;
  // Colormap cmap;
  // XColor xc, xc2;

  
  XrmInitialize();
  db = XrmGetDatabase(dpy);
  XrmParseCommand(&db, xrmTable, sizeof(xrmTable)/sizeof(xrmTable[0]), "xtut6", &argc, argv);

  /* these are macros that pull useful data out of the display object */
  /* we use these bits of info enough to want them in their own variables */
  screen_num = DefaultScreen(dpy);

  // cmap = DefaultColormap(dpy, screen_num);

  font = getFont(dpy, db, "xtut8.font", "xtut8.Font", "fixed");
  background = getColour(dpy, db, "xtut8.background", "xtut8.BackGround", "DarkGreen");
  border = getColour(dpy, db, "xtut8.border", "xtut8.Border", "LightGreen");


  // XAllocNamedColor(dpy, cmap, "goldenrod", &xc, &xc2);
  // background = xc.pixel;
  // XAllocNamedColor(dpy, cmap, "LightGreen", &xc, &xc2);
  // border = xc.pixel;
  // XAllocNamedColor(dpy, cmap, "PeachPuff", &xc, &xc2);
  // values.foreground = xc.pixel;

  // fontname = "-*-helvetica-*-10-*";
  // fontname = "*-bitstream-charter-*";

  // font = XLoadQueryFont(dpy, fontname);
  // if (!font) {
  //   fprintf(stderr, "unable to load preferred font: %s using fixed", fontname);
  //   font = XLoadQueryFont(dpy, "fixed");
  // }

  width = 400; /* start with a small window */
  height = 400;

  win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), /* display, parent */
          0, 0, /* x,  y: the window manager will place the window elsewhere */
          width, height, /* width, height */
          2, border, /* border width & color, unless you have a window manager */
          background); /* background color */

  Xutf8SetWMProperties(dpy, win, "XTut7", "xtut7", argv, argc, NULL, NULL, NULL);
  

  /* create the pen to draw lines with */
  values.foreground = getColour(dpy, db, "xtut8.foreground", "xtut8.ForeGround", "Red");
  values.line_width = 10;
  values.line_style = LineSolid;
  values.font = font->fid;
  pen = XCreateGC(dpy, win, GCForeground|GCLineWidth|GCLineStyle|GCFont, &values);

  /* tell the display server what kind of events we would like to see */
  XSelectInput(dpy, win, ButtonPressMask|ButtonReleaseMask|StructureNotifyMask|ExposureMask);

  /* okay, put the window on the screen, please */
  XMapWindow(dpy, win);

  *width_r = width;
  *height_r = height;
  *font_r = font;

  return pen;

};

int main_loop(Display *dpy, XFontStruct *font, GC pen, int width, int height, char *text) {
  int text_width;
  int textx, texty;
  XEvent ev;
  // int font_ascent;
  // XFontStruct **fonts;
  // char **font_names;
  // int nfonts;
  // int j;
  XChar2b *string;
  int strlength = strlen(text);

  /* may be too big, but definitely big enough */
  string = malloc(sizeof(*string) * strlen(text));
  strlength = utf8toXChar2b(string, strlength, text, strlength);
  printf("%d", strlength);

  text_width = XTextWidth16(font, string, strlength);
 
  /**
   * For using Font Sets
   * _______________
   * printf("%s:%d\n", text, strlength);
   * text_width = Xutf8TextEscapement(font, string, strlength);
   * font_ascent = 0;
   * nfonts = XFontsOfFontSet(font, &fonts, &font_names);
   * for(j=0; j < nfonts; j+=1) {
   *  if (font_ascent < fonts[j]->ascent) font_ascent = fonts[j]-> ascent;
   *    printf("Font: %s\n", font_names[j]);
   * }
  */
  

  /* as each event that we asked about occurs, we respond. */

  /** In this case we note if the windows shape changed, and exit if a button
   * is pressed inside the window */

  /*
    In this case, if a peice of window was exposed, we draw two diagonal lines
  */

  while(1) {
    XNextEvent(dpy,  &ev);
    switch(ev.type){
      case Expose:
      if (ev.xexpose.count > 0) break;
        XDrawLine(dpy, ev.xany.window, pen, 0, 0, width/2-text_width/2, height/2);
        XDrawLine(dpy, ev.xany.window, pen, width, 0, width/2 + text_width/2, height/2);
        XDrawLine(dpy, ev.xany.window, pen, 0, height, width/2 - text_width/2, height/2);
        XDrawLine(dpy, ev.xany.window, pen, width, height, width/2 + text_width/2, height/2);
        textx = (width - text_width)/2;
        texty = (height + font -> ascent)/2;
        // Xutf8DrawString(dpy, ev.xany.window, font, pen, textx, texty, text, strlen(text));
        XDrawString16(dpy, ev.xany.window, pen, textx, texty, string, strlength);

        break;

      case ConfigureNotify:
        if (width != ev.xconfigure.width
                        || height != ev.xconfigure.height) {
                          width = ev.xconfigure.width;
                          height = ev.xconfigure.height;
                          XClearWindow(dpy, ev.xany.window);
                        }
                        break;
      case ButtonRelease:
              XCloseDisplay(dpy);
              return 0;
    }
  }


}

int main(int argc, char ** argv) {
  int width, height;
  Display *dpy;
  GC pen;
  XFontStruct* font;
  char* text = "Hello World";
  // setlocale(LC_ALL, getenv("LANG"));


  /* First connect to the display server, as specified in the DISPLAY
  environment variable. */
  dpy = XOpenDisplay(NULL);
  if(!dpy)
  {
    fprintf(stderr, "unable to connect to display");
    return 7;
  }

  pen = setup(dpy, argc, argv, &width, &height, &font);

  if (argv[1] && argv[1][0]) text = argv[1];
  return main_loop(dpy, font, pen, width, height, text);
}