/*
 TUIO C++ Library
 Copyright (c) 2005-2017 Martin Kaltenbrunner <martin@tuio.org>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3.0 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library.
*/

#include "UdpSender.h"
#include <WiFi.h>

using namespace TUIO;

UdpSender::UdpSender(const char *host, int port) {
  udp.beginPacket(host, port);
  local = false;
  buffer_size = MAX_UDP_SIZE;
}

UdpSender::~UdpSender() {}

bool UdpSender::isConnected() {
  /* FIXME: */
  return true;
}

bool UdpSender::sendOscPacket(osc::OutboundPacketStream *bundle) {

  if (bundle->Size() > buffer_size)
    return false;
  if (bundle->Size() == 0)
    return false;
	udp.beginPacket();
  udp.write((const uint8_t *)bundle->Data(), bundle->Size());
	udp.endPacket();
  return true;
}
