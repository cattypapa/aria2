/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "BtRejectMessage.h"
#include "PeerMessageUtil.h"
#include "Util.h"
#include "DlAbortEx.h"
#include "message.h"
#include "Peer.h"
#include "RequestSlot.h"
#include "BtMessageDispatcher.h"
#include "StringFormat.h"

namespace aria2 {

BtRejectMessageHandle BtRejectMessage::create(const unsigned char* data, size_t dataLength) {
  if(dataLength != 13) {
    throw DlAbortEx
      (StringFormat(EX_INVALID_PAYLOAD_SIZE, "reject", dataLength, 13).str());
  }
  uint8_t id = PeerMessageUtil::getId(data);
  if(id != ID) {
    throw DlAbortEx
      (StringFormat(EX_INVALID_BT_MESSAGE_ID, id, "reject", ID).str());
  }
  BtRejectMessageHandle message(new BtRejectMessage());
  message->setIndex(PeerMessageUtil::getIntParam(data, 1));
  message->setBegin(PeerMessageUtil::getIntParam(data, 5));
  message->setLength(PeerMessageUtil::getIntParam(data, 9));
  return message;
}

void BtRejectMessage::doReceivedAction() {
  if(!peer->isFastExtensionEnabled()) {
    throw DlAbortEx
      (StringFormat("%s received while fast extension is disabled.",
		    toString().c_str()).str());
  }
  // TODO Current implementation does not close a connection even if
  // a request for this reject message has never sent.
  RequestSlot slot = dispatcher->getOutstandingRequest(index, begin, length);
  if(RequestSlot::isNull(slot)) {
    //throw DlAbortEx("reject recieved, but it is not in the request slots.");
  } else {
    dispatcher->removeOutstandingRequest(slot);
  }

}

const unsigned char* BtRejectMessage::getMessage() {
  if(!msg) {
    /**
     * len --- 13, 4bytes
     * id --- 16, 1byte
     * index --- index, 4bytes
     * begin --- begin, 4bytes
     * length -- length, 4bytes
     * total: 17bytes
     */
    msg = new unsigned char[MESSAGE_LENGTH];
    PeerMessageUtil::createPeerMessageString(msg, MESSAGE_LENGTH, 13, ID);
    PeerMessageUtil::setIntParam(&msg[5], index);
    PeerMessageUtil::setIntParam(&msg[9], begin);
    PeerMessageUtil::setIntParam(&msg[13], length);
  }
  return msg;
}

size_t BtRejectMessage::getMessageLength() {
  return MESSAGE_LENGTH;
}

std::string BtRejectMessage::toString() const {
  return "reject index="+Util::uitos(index)+", begin="+Util::uitos(begin)+
    ", length="+Util::uitos(length);
}

} // namespace aria2
