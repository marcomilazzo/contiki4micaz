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
 * $Id: SkyByteRadio.java,v 1.4 2008/11/03 12:31:33 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.CustomDataRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.mspmote.SkyMote;
import se.sics.mspsim.chip.CC2420;
import se.sics.mspsim.chip.RFListener;
import se.sics.mspsim.chip.CC2420.RadioState;
import se.sics.mspsim.chip.CC2420.StateListener;

/**
 * CC2420 to COOJA wrapper.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("CC2420")
public class SkyByteRadio extends Radio implements CustomDataRadio {
  private static Logger logger = Logger.getLogger(SkyByteRadio.class);

  private int lastEventTime = 0;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private SkyMote mote;

  private CC2420 cc2420;

  private boolean isInterfered = false;

  private boolean isTransmitting = false;

  private boolean isReceiving = false;
//  private boolean hasFailedReception = false;

  private boolean radioOn = true;

  private CC2420RadioByte lastOutgoingByte = null;

  private CC2420RadioByte lastIncomingByte = null;

  private RadioPacket lastOutgoingPacket = null;

  private RadioPacket lastIncomingPacket = null;

//  private int mode;

  //TODO: HW on/off

  public SkyByteRadio(SkyMote mote) {
    this.mote = mote;
    this.cc2420 = mote.skyNode.radio;

    cc2420.setRFListener(new RFListener() {
      int len = 0;
      int expLen = 0;
      byte[] buffer = new byte[127 + 5];
      public void receivedByte(byte data) {
        if (len == 0) {
          lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
          lastEvent = RadioEvent.TRANSMISSION_STARTED;
          /*logger.debug("----- SKY TRANSMISSION STARTED -----");*/
          setChanged();
          notifyObservers();
        }

        /* send this byte to all nodes */
        lastOutgoingByte = new CC2420RadioByte(data);
        lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
        setChanged();
        notifyObservers();

        buffer[len++] = data;

        if (len == 6) {
//          System.out.println("## CC2420 Packet of length: " + data + " expected...");
          expLen = data + 6;
        }

        if (len == expLen) {
          /*logger.debug("----- SKY CUSTOM DATA TRANSMITTED -----");*/

          lastOutgoingPacket = CC2420RadioPacketConverter.fromCC2420ToCooja(buffer);
          lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
          lastEvent = RadioEvent.PACKET_TRANSMITTED;
          /*logger.debug("----- SKY PACKET TRANSMITTED -----");*/
          setChanged();
          notifyObservers();


//          System.out.println("## CC2420 Transmission finished...");

          lastEventTime = SkyByteRadio.this.mote.getSimulation().getSimulationTime();
          /*logger.debug("----- SKY TRANSMISSION FINISHED -----");*/
          lastEvent = RadioEvent.TRANSMISSION_FINISHED;
          setChanged();
          notifyObservers();
          len = 0;
        }
      }
    });
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return lastOutgoingPacket;
  }

  public RadioPacket getLastPacketReceived() {
    return lastIncomingPacket;
  }

  private byte[] crossBufferedData = null;

  private TimeEvent receiveCrosslevelDataEvent = new TimeEvent(0) {
    public void execute(int t) {

      if (crossBufferedData == null) {
        return;
      }

      /*logger.info("Radio is now ready to receive the incoming data");*/

      for (byte b: crossBufferedData) {
        cc2420.receivedByte(b);
      }
      crossBufferedData = null;
    }
  };

  private StateListener cc2420StateListener = new StateListener() {
    public void newState(RadioState state) {
      if (cc2420.getState() == CC2420.RadioState.RX_SFD_SEARCH) {
        cc2420.setStateListener(null);

        if (crossBufferedData == null) {
          return;
        }

        /* Receive data very soon (just wait for a radio flush) */
        mote.getSimulation().scheduleEvent(
            receiveCrosslevelDataEvent,
            mote.getSimulation().getSimulationTime()+1
        );

      }
    }
  };

  public void setReceivedPacket(RadioPacket packet) {
    lastIncomingPacket = packet;

    /* TODO Receiving all bytes at the same time ok? */
    byte[] packetData = CC2420RadioPacketConverter.fromCoojaToCC2420((COOJARadioPacket) packet);

    if (cc2420.getState() != CC2420.RadioState.RX_SFD_SEARCH) {
      /*logger.info("Radio is not currently active. Let's wait some...");*/

      crossBufferedData = packetData;
      cc2420.setStateListener(cc2420StateListener);

      /* TODO Event to remove listener and give up */
      return;
    }

    for (byte b: packetData) {
      cc2420.receivedByte(b);
    }
  }

  /* Custom data radio support */
  public Object getLastCustomDataTransmitted() {
    return lastOutgoingByte;
  }

  public Object getLastCustomDataReceived() {
    return lastIncomingByte;
  }

  public void receiveCustomData(Object data) {
    if (data instanceof CC2420RadioByte) {
      lastIncomingByte = (CC2420RadioByte) data;
      cc2420.receivedByte(lastIncomingByte.getPacketData()[0]);
    }
  }

  /* General radio support */
  public boolean isTransmitting() {
    return isTransmitting;
  }

  public boolean isReceiving() {
    return isReceiving;
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public int getChannel() {
    /* TODO XXX Enable CC2420 channel selection (when implemented) */
    //return cc2420.getActiveChannel();
    return 26;
  }

  public int getFrequency() {
    return cc2420.getActiveFrequency();
  }

  public void signalReceptionStart() {
    cc2420.setCCA(true);
//    hasFailedReception = mode == CC2420.MODE_TXRX_OFF;
    isReceiving = true;
    /* TODO cc2420.setSFD(true); */

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    /*logger.debug("----- SKY RECEPTION STARTED -----");*/
    setChanged();
    notifyObservers();
  }

  public void signalReceptionEnd() {
    /* Deliver packet data */
    isReceiving = false;
//    hasFailedReception = false;
    isInterfered = false;
    cc2420.setCCA(false);

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    /*logger.debug("----- SKY RECEPTION FINISHED -----");*/
    setChanged();
    notifyObservers();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    isInterfered = true;
    isReceiving = false;
//    hasFailedReception = false;
    lastIncomingPacket = null;
    cc2420.setCCA(true);

    lastEventTime = mote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_INTERFERED;
    /*logger.debug("----- SKY RECEPTION INTERFERED -----");*/
    setChanged();
    notifyObservers();
  }

  public double getCurrentOutputPower() {
    return cc2420.getOutputPower();
  }

  public int getCurrentOutputPowerIndicator() {
    return cc2420.getOutputPowerIndicator();
  }

  public int getOutputPowerIndicatorMax() {
    return 31;
  }

  public double getCurrentSignalStrength() {
    return cc2420.getRSSI();
  }

  public void setCurrentSignalStrength(double signalStrength) {
    cc2420.setRSSI((int) signalStrength);
  }

  public double energyConsumption() {
    return 0;
  }

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel wrapperPanel = new JPanel(new BorderLayout());
    JPanel panel = new JPanel(new GridLayout(5, 2));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("");
    final JLabel powerLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update");

    panel.add(new JLabel("STATE:"));
    panel.add(statusLabel);

    panel.add(new JLabel("LAST EVENT:"));
    panel.add(lastEventLabel);

    panel.add(new JLabel("CHANNEL:"));
    panel.add(channelLabel);

    panel.add(new JLabel("OUTPUT POWER:"));
    panel.add(powerLabel);

    panel.add(new JLabel("SIGNAL STRENGTH:"));
    JPanel smallPanel = new JPanel(new GridLayout(1, 2));
    smallPanel.add(ssLabel);
    smallPanel.add(updateButton);
    panel.add(smallPanel);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        channelLabel.setText(getChannel() + " (freq=" + getFrequency() + " MHz)");
        powerLabel.setText(getCurrentOutputPower() + " dBm (indicator=" + getCurrentOutputPowerIndicator() + "/" + getOutputPowerIndicatorMax() + ")");
        ssLabel.setText(getCurrentSignalStrength() + " dBm");
      }
    });

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("transmitting");
        } else if (isReceiving()) {
          statusLabel.setText("receiving");
        } else if (radioOn /* mode != CC2420.MODE_TXRX_OFF */) {
          statusLabel.setText("listening for traffic");
        } else {
          statusLabel.setText("HW off");
        }

        lastEventLabel.setText(lastEvent + " @ time=" + lastEventTime);

        channelLabel.setText(getChannel() + " (freq=" + getFrequency() + " MHz)");
        powerLabel.setText(getCurrentOutputPower() + " dBm (indicator=" + getCurrentOutputPowerIndicator() + "/" + getOutputPowerIndicatorMax() + ")");
        ssLabel.setText(getCurrentSignalStrength() + " dBm");
      }
    });

    observer.update(null, null);

    wrapperPanel.add(BorderLayout.NORTH, panel);

    // Saving observer reference for releaseInterfaceVisualizer
    wrapperPanel.putClientProperty("intf_obs", observer);
    return wrapperPanel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Mote getMote() {
    return mote;
  }

  public Position getPosition() {
    return mote.getInterfaces().getPosition();
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }
}
