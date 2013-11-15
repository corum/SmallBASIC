// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "platform/android/jni/display.h"
#include "platform/common/form_ui.h"
#include "platform/common/utils.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define SIZE_LIMIT 4
#define FONT_FACE_NAME "Envy Code R.ttf"

Viewable *widget;
Drawable *drawTarget;
Font *activeFont;
bool mouseActive;
uint16_t drawColor;

uint16_t make565(int red, int green, int blue) {
  return (uint16_t)(((red   << 8) & 0xf800) |
                    ((green << 2) & 0x03e0) |
                    ((blue  >> 3) & 0x001f));
}

//
// Drawable implementation
//
Drawable::Drawable() :
  _canvas(NULL),
  _clip(NULL) {
}

Drawable::~Drawable() {
  delete _canvas;
  delete _clip;
}

void Drawable::beginDraw() {
}

bool Drawable::create(int w, int h) {
}

void Drawable::drawImageRegion(Drawable *dst, const MAPoint2d *dstPoint, const MARect *srcRect) {
}

void Drawable::drawLine(int startX, int startY, int endX, int endY) {
}

void Drawable::drawPixel(int posX, int posY) {
}

void Drawable::drawRectFilled(int left, int top, int width, int height) {
}

void Drawable::drawText(int left, int top, const char *str) {
}

void Drawable::endDraw()  {
}

int Drawable::getPixel(int x, int y) {
}

void Drawable::resize(int w, int h)  {
}

void Drawable::setClip(int x, int y, int w, int h) {
}

//
// Viewable implementation
//
Viewable::Viewable(ANativeWindow *window) :
  _screen(NULL),
  _window(window) {
}

Viewable::~Viewable() {
  delete _screen;
}

bool Viewable::construct(String &resourcePath) {
  return true;
}

Font *Viewable::createFont(int style, int size) {

}

int Viewable::getWidth() {
  return 0;
}

int Viewable::getHeight() {
  return 0;
}

MAHandle Viewable::setDrawTarget(MAHandle maHandle) {

}

void Viewable::redraw() {
  if (_window != NULL) {
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(_window, &buffer, NULL) < 0) {
      trace("Unable to lock window buffer");
    } else {
      //memcpy(surface_buffer.bits, _buffer,  _bufferSize);
      ANativeWindow_unlockAndPost(_window);
    }
  }
}

//
// form_ui implementation
//
void form_ui::optionsBox(StringList *items) {
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
  if (maHandle != -1) {
    Font *font = (Font *) maHandle;
    if (font == activeFont) {
      activeFont = NULL;
    }
    trace("Delete font %x", font);
    delete font;
  }
  return RES_FONT_OK;
}

int maSetColor(int c) {
  int r = (c >> 16) & 0xFF;
  int g = (c >> 8) & 0xFF;
  int b = (c) & 0xFF;
  drawColor = make565(r, g, b);
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  if (drawTarget) {
    drawTarget->setClip(left, top, width, height);
  }
}

void maPlot(int posX, int posY) {
  if (drawTarget) {
    drawTarget->drawPixel(posX, posY);
  }
}

void maLine(int startX, int startY, int endX, int endY) {
  if (drawTarget) {
    drawTarget->drawLine(startX, startY, endX, endY);
  }
}

void maFillRect(int left, int top, int width, int height) {
  if (drawTarget) {
    drawTarget->drawRectFilled(left, top, width, height);
  }
}

void maDrawText(int left, int top, const char *str) {
  if (drawTarget && str && str[0]) {
    drawTarget->drawText(left, top, str);
  }
}

void maUpdateScreen(void) {
  //  Application::GetInstance()->SendUserEvent(MSG_ID_REDRAW, NULL);
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  if (activeFont && str && str[0]) {
    // Dimension dim;
    // activeFont->GetTextExtent(str, strlen(str), dim);
    // result = (MAExtent)((dim.width << 16) + dim.height);
  } else {
    result = 0;
  }
  return result;
}

MAExtent maGetScrSize(void) {
  short width = widget->getWidth();
  short height = widget->getHeight();
  return (MAExtent)((width << 16) + height);
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return (MAHandle) widget->createFont(style, size);
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  activeFont = (Font *) maHandle;
  return maHandle;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  Drawable *drawable = (Drawable *)maHandle;
  if (drawTarget != drawable) {
    drawable->drawImageRegion(drawTarget, dstPoint, srcRect);
  }
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  if (height > widget->getHeight() * SIZE_LIMIT) {
    result -= 1;
  } else {
    Drawable *canvas = (Drawable *)maHandle;
    canvas->create(width, height);
  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  MAHandle maHandle = (MAHandle) new Drawable();
  return maHandle;
}

void maDestroyPlaceholder(MAHandle maHandle) {
  Drawable *holder = (Drawable *)maHandle;
  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int scanlength) {
  Drawable *holder = (Drawable *)maHandle;
  // maGetImageData is only used for getPixel()
  *((int *)dst) = holder->getPixel(srcRect->left, srcRect->top);
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  return widget->setDrawTarget(maHandle);
}

int maGetMilliSecondCount(void) {
  struct timespec t;
  t.tv_sec = t.tv_nsec = 0;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (int)(((int64_t)t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000;
}

int maShowVirtualKeyboard(void) {
  //Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_KEYPAD, NULL);
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
//  ArrayList *args = new ArrayList();
//  args->Construct();
//  args->Add(new Tizen::Base::String(title));
//  args->Add(new Tizen::Base::String(message));
//  Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_ALERT, args);
}

