/* The source code in this file was copied in part from tools/client.cc and tools/server.cc from the BoringSSL project
 * which requires the following copyright notice be provided here:
 *
 * Copyright (c) 2014, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#ifndef ESB_BORING_SSL_SOCKET_H
#include <ESBBoringSSLSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <openssl/ssl.h>

namespace ESB {

#define ESB_SOCK_HANDSHAKING (1 << 2)
#define ESB_SOCK_ESTABLISHED (1 << 3)

Error BoringSSLSocket::Initialize() { return ESB_NOT_IMPLEMENTED; }

BoringSSLSocket::BoringSSLSocket(const char *namePrefix, const char *nameSuffix, bool isBlocking)
    : ConnectedSocket(namePrefix, nameSuffix, isBlocking) {}

BoringSSLSocket::BoringSSLSocket(const char *namePrefix, const char *nameSuffix, const SocketAddress &peer,
                                 bool isBlocking)
    : ConnectedSocket(namePrefix, nameSuffix, peer, isBlocking) {}

BoringSSLSocket::~BoringSSLSocket() {}

void BoringSSLSocket::close() { ConnectedSocket::close(); }

bool BoringSSLSocket::secure() { return true; }

SSize BoringSSLSocket::receive(char *buffer, Size bufferSize) { return ConnectedSocket::receive(buffer, bufferSize); }

SSize BoringSSLSocket::send(const char *buffer, Size bufferSize) { return ConnectedSocket::send(buffer, bufferSize); }

int BoringSSLSocket::bytesReadable() { return ConnectedSocket::bytesReadable(); }

}  // namespace ESB
