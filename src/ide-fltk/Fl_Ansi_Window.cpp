// -*- c-file-style: "java" -*-
// $Id: Fl_Ansi_Window.cpp,v 1.5 2004-11-07 22:59:23 zeeb90au Exp $
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <fltk/Window.H>
#include <fltk/draw.H>
#include <fltk/ask.H>
#include <fltk/Image.h>
#include <fltk/layout.h>
#include <fltk/Style.h>
#include <fltk/Font.h>

#include "Fl_Ansi_Window.h"

using namespace fltk;

// uncomment for unit testing and then run:
// make Fl_Ansi_Window.exe
// #define UNIT_TEST 1

#define begin_offscreen() initOffscreen(); ImageDraw imageDraw(img);
#define end_offscreen() redraw();

Fl_Ansi_Window::Fl_Ansi_Window(int x, int y, int w, int h) : 
    Widget(x, y, w, h, 0) {
    init();
}

Fl_Ansi_Window::~Fl_Ansi_Window() {
    if (img) {
        delete img;
        img = 0;
    }
}

void Fl_Ansi_Window::init() {
    if (img) {
        delete img;
    }
    img = 0;
    curY = 0;
    curX = 0;
    tabSize = 40; // tab size in pixels (160/32 = 5)
    reset();
}

void Fl_Ansi_Window::initOffscreen() {
    // can only be called following Fl::check() or Fl::run()
    if (img == 0) {
        img = new Image(w(), h());
        ImageDraw imageDraw(img);
        setcolor(color());
        fillrect(0, 0, w(), h());
        setfont(labelfont(), labelsize());
        curY = textHeight();
    }
}

void Fl_Ansi_Window::reset() {
    curYSaved = 0;
    curXSaved = 0;
    invert = false;
    underline = false;
    bold = false;
    italic = false;
    color(WHITE); // bg
    labelcolor(BLACK); // fg
    labelfont(COURIER);
    labelsize(11);
}

void Fl_Ansi_Window::layout() {
    if (img && (layout_damage() & LAYOUT_WH)) {
        Image* old = img;
        img = 0;
        begin_offscreen();
        old->draw(0, 0, w(), h(), 0, OUTPUT);
        delete old;
        end_offscreen();
    }
    Widget::layout();
}

void Fl_Ansi_Window::draw() {
    if (img) {
        img->draw(0, 0, w(), h(), 0, OUTPUT);
    } else {
        setcolor(color());
        fillrect(0, 0, w(), h());
    }
}

void Fl_Ansi_Window::drawLine(int x1, int y1, int x2, int y2) {
    begin_offscreen();
    setcolor(labelcolor());
    drawline(x1, y1, x2, y2);
    end_offscreen();
}

void Fl_Ansi_Window::drawFGRectFilled(int x, int y, int width, int height) {
    begin_offscreen();
    setcolor(labelcolor());
    fillrect(x, y, width, height);
    end_offscreen();
}

void Fl_Ansi_Window::drawBGRectFilled(int x, int y, int width, int height) {
    begin_offscreen();
    setcolor(color());
    fillrect(x, y, width, height);
    end_offscreen();
}

void Fl_Ansi_Window::drawFGRect(int x, int y, int width, int height) {
    begin_offscreen();
    setcolor(labelcolor());
    strokerect(x, y, width, height);
    end_offscreen();
}

void Fl_Ansi_Window::drawBGRect(int x, int y, int width, int height) {
    begin_offscreen();
    setcolor(color());
    strokerect(x, y, width, height);
    end_offscreen();
}

void Fl_Ansi_Window::clearScreen() {
    init();
    begin_offscreen();
    setcolor(color());
    fillrect(0, 0, w(), h());
    end_offscreen();
}

void Fl_Ansi_Window::saveScreen() {
}

void Fl_Ansi_Window::restoreScreen() {
    redraw();
}

// callback for fl_scroll
void eraseBottomLine(void* data, int x, int y, int w, int h) {
    Fl_Ansi_Window* out = (Fl_Ansi_Window*)data;
    setcolor(out->color());
    fillrect(x, y, w, h);
}

void Fl_Ansi_Window::newLine() {
    int height = h();
    int fontHeight = textHeight();

    curX = 0;
    if (curY+fontHeight+getdescent() >= height) {
        scrollrect(0, 0, w(), height, 0, -fontHeight, eraseBottomLine, this);
        // TODO: patched is_visible() in fl_scroll_area.cxx 
        // commented out OffsetRgn()
    } else {
        curY += fontHeight;
    }
}

int Fl_Ansi_Window::calcTab(int x) const {
    int c = 1;
    while (x > tabSize) {
        x -= tabSize;
        c++;
    }
    return c * tabSize;
}

Color Fl_Ansi_Window::ansiToFltk(long color) const {
    switch (color) {
    case 0: return BLACK;
    case 1: return RED;
    case 2: return GREEN;
    case 3: return YELLOW;
    case 4: return BLUE;
    case 5: return MAGENTA;
    case 6: return CYAN;
    case 7: return RED;
    case 8: return GREEN;
    case 9: return YELLOW;
    case 10: return BLUE;
    case 11: return MAGENTA;
    case 12: return CYAN;
        // TODO: fix 13,14
    case 13: return CYAN;
    case 14: return CYAN;
    default : return WHITE;
    }
}

bool Fl_Ansi_Window::setGraphicsRendition(char c, int escValue) {
    switch (c) {
    case 'K': {// \e[K - clear to eol
        int fontHeight = textHeight();
        setcolor(color());
        fillrect(curX, (int)(curY-fontHeight+getdescent()), w()-curX, fontHeight);
        }
        break;
    case 'G': // move to column
        curX = escValue;
        break;
    case 'T': // non-standard: move to n/80th of screen width
        curX = escValue*w()/80;
        break;
    case 's': // save cursor position
        curYSaved = curX;
        curXSaved = curY;
        break;
    case 'u': // restore cursor position
        curX = curYSaved;
        curY = curXSaved;
        break;
    case ';': // fallthru
    case 'm': // \e[...m  - ANSI terminal
        switch (escValue) {
        case 0:  // reset
            reset();
            break;
        case 1: // set bold on
            bold = true;
            return true;
        case 2: // set faint on
            break;
        case 3: // set italic on
            italic = true;
            return true;
        case 4: // set underline on
            underline = true;
            break;
        case 7: // reverse video on
            invert = true;
            break;
        case 21: // set bold off
            bold = false;
            return true;
        case 23:
            italic = false;
            return true;
        case 24: // set underline off
            underline = false;
            break;
        case 27: // reverse video off
            invert = false;
            break;
            // colors - 30..37 foreground, 40..47 background
        case 30: // set black fg
            labelcolor(ansiToFltk(0));
            return true;
        case 31: // set red fg
            labelcolor(ansiToFltk(4));
            return true;
        case 32: // set green fg
            labelcolor(ansiToFltk(2));
            return true;
        case 33: // set brown fg
            labelcolor(ansiToFltk(6));
            return true;
        case 34: // set blue fg
            labelcolor(ansiToFltk(1));
            return true;
        case 35: // set magenta fg
            labelcolor(ansiToFltk(5));
            return true;
        case 36: // set cyan fg
            labelcolor(ansiToFltk(3));
            return true;
        case 37: // set white fg
            labelcolor(ansiToFltk(7));
            return true;
        case 40: // set black bg
            color(ansiToFltk(0));
            return true;
        case 41: // set red bg
            color(ansiToFltk(4));
            return true;
        case 42: // set green bg
            color(ansiToFltk(2));
            return true;
        case 43: // set brown bg
            color(ansiToFltk(6));
            return true;
        case 44: // set blue bg
            color(ansiToFltk(1));
            return true;
        case 45: // set magenta bg
            color(ansiToFltk(5));
            return true;
        case 46: // set cyan bg
            color(ansiToFltk(3));
            return true;
        case 47: // set white bg
            color(ansiToFltk(7));
            return true;
        };                        
    }
    return false;
}

bool Fl_Ansi_Window::doEscape(unsigned char* &p) {
    int escValue = 0;
    while (isdigit(*p)) {
        escValue = (escValue * 10) + (*p - '0');
        p++;
    }

    if (setGraphicsRendition(*p, escValue)) {
        Font* font = labelfont();
        if (bold) {
            font = font->bold();
        }
        if (italic) {
            font = font->italic();
        }
        setfont(font, labelsize());
    }
    
    if (*p == ';') {
        p++; // next rendition
        return true;
    }
    return false;
}

/**
 *   Supported control codes:
 *   \t      tab (20 px)
 *   \a      beep
 *   \r      return
 *   \n      next line
 *   \xC     clear screen
 *   \e[K    clear to end of line
 *   \e[0m   reset all attributes to their defaults
 *   \e[1m   set bold on
 *   \e[4m   set underline on
 *   \e[7m   reverse video
 *   \e[21m  set bold off
 *   \e[24m  set underline off
 *   \e[27m  set reverse off
 */
void Fl_Ansi_Window::print(const char *str) {
    int len = strlen(str);
    if (len <= 0) {
        return;
    }

    begin_offscreen();
    int fontHeight = textHeight();
    unsigned char *p = (unsigned char*)str;
    while (*p) {
        switch (*p) {
        case '\a':   // beep
            //fl_beep();
            break;
        case '\t':
            curX = calcTab(curX+1);
            break;
        case '\xC':
            init();
            setcolor(color());
            fillrect(0, 0, w(), h());
            break;
        case '\033':  // ESC ctrl chars
            if (*(p+1) == '[' ) {
                p += 2;
                while(true) {
                    if (!doEscape(p)) {
                        break;
                    }
                }
            }
            break;
        case '\n': // new line
            newLine();
            break;
        case '\r': // return
            curX = 0;
            setcolor(color());
            fillrect(0, (int)(curY-fontHeight+getdescent()), w(), fontHeight);
            break;
        default:
            int numChars = 1; // print minimum of one character
            int cx = (int)getwidth((const char*)p, 1);
            int width = w()-1;

            if (curX + cx >= width) {
                newLine();
            }

            // print further non-control, non-null characters 
            // up to the width of the line
            while (p[numChars] > 31) {
                cx += (int)getwidth((const char*)p+numChars, 1);
                if (curX + cx < width) {
                    numChars++;
                } else {
                    break;
                }
            }
            
            if (invert) {
                setcolor(labelcolor());
                fillrect(curX, (int)(curY-fontHeight+getdescent()), 
                         cx, fontHeight);
                setcolor(color());
                drawtext((const char*)p, numChars, curX, curY);
            } else {
                setcolor(labelcolor());
                drawtext((const char*)p, numChars, curX, curY);
            }

            if (underline) {
                drawline(curX, curY+1, curX+cx, curY+1);
            }
            
            // advance
            p += numChars-1; // allow for p++ 
            curX += cx;
        };
        
        if (*p == '\0') {
            break;
        }
        p++;
    }

    end_offscreen();
}

#ifdef UNIT_TEST
#include <fltk/run.h>
int main(int argc, char **argv) {
    int w = 210; // must be > 104
    int h = 200;
    Window window(w, h, "SmallBASIC");
    window.begin();
    Fl_Ansi_Window out(0, 0, w, h);
    window.resizable(&out);
    window.end();
    window.show(argc,argv);
    check();
    for (int i=0; i<100; i++) {
        out.print("\033[3mitalic\033[23moff\033[4munderline\033[24moff");
        out.print("\033[7minverse\033[27moff");
        out.print("\033[1mbold\033[21moff");
    }
    return run();
}
#endif
