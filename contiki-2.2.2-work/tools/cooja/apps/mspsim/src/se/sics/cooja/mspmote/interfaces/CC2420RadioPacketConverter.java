/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: CC2420RadioPacketConverter.java,v 1.5 2008/11/03 10:33:15 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import org.apache.log4j.Logger;

import se.sics.cooja.COOJARadioPacket;

/**
 * Converts radio packets between X-MAC/CC24240/Sky and COOJA.
 * Handles radio driver specifics such as length header and CRC footer.
 *
 * @author Fredrik Österlind
 */
public class CC2420RadioPacketConverter {
  private static Logger logger = Logger.getLogger(CC2420RadioPacketConverter.class);

  public static final boolean WITH_PREAMBLE = true;
  public static final boolean WITH_SYNCH = true;
  public static final boolean WITH_XMAC = true;
  public static final boolean WITH_CHECKSUM = true;
  public static final boolean WITH_TIMESTAMP = true;
  public static final boolean WITH_FOOTER = true;

  public static byte[] fromCoojaToCC2420(COOJARadioPacket packet) {
    byte cc2420Data[] = new byte[6+127];
    int pos = 0;
    byte packetData[] = packet.getPacketData();
    byte len; /* total packet minus preamble(4), synch(1) and length(1) */
    short accumulatedCRC = 0;

    /* 4 bytes preamble */
    if (WITH_PREAMBLE) {
      cc2420Data[pos++] = 0;
      cc2420Data[pos++] = 0;
      cc2420Data[pos++] = 0;
      cc2420Data[pos++] = 0;
    }

    /* 1 byte synch */
    if (WITH_SYNCH) {
      cc2420Data[pos++] = 0x7A;
    }

    /* 1 byte length */
    len = (byte) packetData.length;
    if (WITH_XMAC) {
      len += 4;
    }
    if (WITH_CHECKSUM) {
      len += 2;
    }
    if (WITH_TIMESTAMP) {
      len += 3;
    }
    if (WITH_FOOTER) {
      len += 2;
    }
    cc2420Data[pos++] = len;

    /* (TODO) 4 byte X-MAC */
    if (WITH_XMAC) {
      cc2420Data[pos++] = 0;
      accumulatedCRC = CRCCoder.crc16Add((byte)0, accumulatedCRC);
      cc2420Data[pos++] = 0;
      accumulatedCRC = CRCCoder.crc16Add((byte)0, accumulatedCRC);
      cc2420Data[pos++] = 0;
      accumulatedCRC = CRCCoder.crc16Add((byte)0, accumulatedCRC);
      cc2420Data[pos++] = 0;
      accumulatedCRC = CRCCoder.crc16Add((byte)0, accumulatedCRC);
    }

    /* Payload */
    for (byte b : packetData) {
      accumulatedCRC = CRCCoder.crc16Add(b, accumulatedCRC);
    }
    System.arraycopy(packetData, 0, cc2420Data, pos, packetData.length);
    pos += packetData.length;

    /* 2 bytes checksum */
    if (WITH_CHECKSUM) {
      cc2420Data[pos++] = (byte) (accumulatedCRC & 0xff);
      cc2420Data[pos++] = (byte) ((accumulatedCRC >> 8) & 0xff);
    }

    /* (TODO) 3 bytes timestamp */
    if (WITH_TIMESTAMP) {
      cc2420Data[pos++] = 0;
      cc2420Data[pos++] = 0;
      cc2420Data[pos++] = 0;
    }

    /* (TODO) 2 bytes footer */
    if (WITH_FOOTER) {
      cc2420Data[pos++] = 0; /* RSSI */
      cc2420Data[pos++] = (byte) 0x80; /* CRC and CORRELATION */
    }

    byte cc2420DataStripped[] = new byte[pos];
    System.arraycopy(cc2420Data, 0, cc2420DataStripped, 0, pos);

    /*logger.info("Data length: " + cc2420DataStripped.length);*/
    return cc2420DataStripped;
  }

  public static COOJARadioPacket fromCC2420ToCooja(byte[] data) {
    int pos = 0;
    int len; /* Payload */

    /* Use some CC2420/MAC specific field such as X-MAC response */

    /* (IGNORED) 4 bytes preamble */
    if (WITH_PREAMBLE) {
      pos += 4;
    }

    /* (IGNORED) 1 byte synch */
    if (WITH_SYNCH) {
      pos += 1;
    }

    /* 1 byte length */
    len = data[pos];
    pos += 1;

    /* (IGNORED) 4 byte X-MAC */
    if (WITH_XMAC) {
      pos += 4;
      len -= 4;
    }

    /* (IGNORED) 2 bytes checksum */
    if (WITH_CHECKSUM) {
      len -= 2;
    }

    /* (IGNORED) 3 bytes timestamp */
    if (WITH_TIMESTAMP) {
      len -= 3;
    }

    /* (IGNORED) 2 bytes footer */
    if (WITH_FOOTER) {
      len -= 2;
    }

    /*logger.info("Payload pos: " + pos);
    logger.info("Payload length: " + len);*/

    byte coojaData[] = new byte[len];
    System.arraycopy(data, pos, coojaData, 0, len);
    return new COOJARadioPacket(coojaData);
  }

}
