/*
        TUIO C++ Server Demo
    Copyright (c) 2005-2016 Martin Kaltenbrunner <martin@tuio.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <Arduino.h>
#include <WiFi.h>
#include <math.h>

#include "TcpSender.h"
#include "TuioCursor.h"
#include "TuioServer.h"
#include "WebSockSender.h"
#include "osc/OscTypes.h"
#include <list>

const char *ssid = "_box";
const char *password = "Wifimima8nengwei0";

const char *host = "192.168.3.47";
const int port = 3333;

using namespace TUIO;

TuioServer *tuioServer;
std::list<TuioCursor *> stickyCursorList;
std::list<TuioCursor *> jointCursorList;
std::list<TuioCursor *> activeCursorList;

int width = 1920, height = 1080;
TuioTime frameTime = 20;
bool running;
static unsigned long begin_time = 0;
static unsigned long last_time = 0;

void mousePressed(float x, float y) {
  // printf("pressed %f %f\n",x,y);

  TuioCursor *match = NULL;
  float distance = 0.01f;
  for (std::list<TuioCursor *>::iterator iter = stickyCursorList.begin();
       iter != stickyCursorList.end(); iter++) {
    TuioCursor *tcur = (*iter);
    float test = tcur->getDistance(x, y);
    if ((test < distance) && (test < 8.0f / width)) {
      distance = test;
      match = tcur;
    }
  }

  if (match == NULL) {
    TuioCursor *cursor = tuioServer->addTuioCursor(x, y);
    activeCursorList.push_back(cursor);
  } else
    activeCursorList.push_back(match);
}

void mouseDragged(float x, float y) {
  // printf("dragged %f %f\n",x,y);

  TuioCursor *cursor = NULL;
  float distance = width;
  for (std::list<TuioCursor *>::iterator iter = activeCursorList.begin();
       iter != activeCursorList.end(); iter++) {
    TuioCursor *tcur = (*iter);
    float test = tcur->getDistance(x, y);
    if (test < distance) {
      distance = test;
      cursor = tcur;
    }
  }

  if (cursor == NULL)
    return;
  if (cursor->getTuioTime() == frameTime)
    return;

  std::list<TuioCursor *>::iterator joint =
      std::find(jointCursorList.begin(), jointCursorList.end(), cursor);
  if (joint != jointCursorList.end()) {
    float dx = x - cursor->getX();
    float dy = y - cursor->getY();
    for (std::list<TuioCursor *>::iterator iter = jointCursorList.begin();
         iter != jointCursorList.end(); iter++) {
      TuioCursor *jointCursor = (*iter);
      tuioServer->updateTuioCursor(jointCursor, jointCursor->getX() + dx,
                                   jointCursor->getY() + dy);
    }
  } else
    tuioServer->updateTuioCursor(cursor, x, y);
}

void mouseReleased(float x, float y) {
  // printf("released %f %f\n",x,y);

  TuioCursor *cursor = NULL;
  float distance = 0.01f;
  for (std::list<TuioCursor *>::iterator iter = stickyCursorList.begin();
       iter != stickyCursorList.end(); iter++) {
    TuioCursor *tcur = (*iter);
    float test = tcur->getDistance(x, y);
    if (test < distance) {
      distance = test;
      cursor = tcur;
    }
  }

  if (cursor != NULL) {
    activeCursorList.remove(cursor);
    return;
  }

  distance = 0.01f;
  for (std::list<TuioCursor *>::iterator iter = activeCursorList.begin();
       iter != activeCursorList.end(); iter++) {
    TuioCursor *tcur = (*iter);
    float test = tcur->getDistance(x, y);
    if (test < distance) {
      distance = test;
      cursor = tcur;
    }
  }

  if (cursor != NULL) {
    activeCursorList.remove(cursor);
    tuioServer->removeTuioCursor(cursor);
  }
}

void run() {
  running = true;
  begin_time = TuioTime::getSessionTime().getTotalMilliseconds();

  TuioTime::initSession();
  frameTime = TuioTime::getSessionTime();

  tuioServer->setSourceName("SimpleSimulator",
                            WiFi.localIP().toString().c_str());
  tuioServer->enableObjectProfile(false);
  tuioServer->enableBlobProfile(false);

  while (running) {
    frameTime = TuioTime::getSessionTime();
    tuioServer->initFrame(frameTime);

    // simulate 50Hz compensating the previous processing time
    int delay = 20 - (TuioTime::getSessionTime().getTotalMilliseconds() -
                      frameTime.getTotalMilliseconds());
    if (delay > 0) {
      usleep(delay * 1000);
    }

    if (frameTime.getTotalMilliseconds() > 1000 + last_time) {
      if (frameTime.getTotalMilliseconds() < 25000) {
        mousePressed(1920 / 2, 1080 / 2);
      } else if (frameTime.getTotalMilliseconds() < 27000) {
        mouseDragged(1920 / 2 + 100, 1080 / 2 + 100);

      } else if (frameTime.getTotalMilliseconds() < 30000) {
        mouseReleased(1920 / 2 - 100, 1080 / 2 - 100);
      } else {
        running = false;
      }
      last_time = frameTime.getTotalMilliseconds();
    } else {
      log_d("frameTime: %lu\n", frameTime.getTotalMilliseconds());
    }

    tuioServer->stopUntouchedMovingCursors();
    tuioServer->commitFrame();
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  TuioServer *server = new TuioServer(host, port);

  // add an additional TUIO/TCP sender
  OscSender *tcp_sender = new TcpSender(3333);
  if (tcp_sender)
    server->addOscSender(tcp_sender);

#if 1
  // add an additional TUIO/WEB sender
  OscSender *web_sender = NULL;
  try {
    web_sender = new WebSockSender(8080);
  } catch (std::exception e) {
    web_sender = NULL;
  }
  if (web_sender)
    server->addOscSender(web_sender);
#endif
  tuioServer = server;
  run();

  delete server;
}

void loop() {}