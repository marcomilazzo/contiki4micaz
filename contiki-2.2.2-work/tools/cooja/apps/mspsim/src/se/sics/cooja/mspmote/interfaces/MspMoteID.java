/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: MspMoteID.java,v 1.8 2008/10/29 08:35:38 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;
import javax.swing.JLabel;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteMemory;

/**
 * Mote ID number.
 *
 * @see #GENERATE_ID_HEADER
 * @see #PERSISTENT_SET_ID
 *
 * @author Fredrik Österlind
 */
public class MspMoteID extends MoteID {
  private static Logger logger = Logger.getLogger(MspMoteID.class);

  private MspMote mote;
  private MspMoteMemory moteMem = null;

  /**
   * Write ID to flash at mote startup.
   */
  public static final boolean GENERATE_ID_HEADER = true;


  /**
   * Persistently set ID in mote memory multiple times at node startup.
   * May be used if emulator resets memory when firmware is loaded.
   */
  public static final boolean PERSISTENT_SET_ID = true;
  private int persistentSetIDCounter = 1000;

  private enum ID_LOCATION {
    VARIABLE_NODE_ID,
    VARIABLE_TOS_NODE_ID,
    VARIABLES_BOTH,
    JAVA_ONLY
  }

  private ID_LOCATION location = ID_LOCATION.JAVA_ONLY;

  private int moteID = -1; /* Only used for location IN_JAVA_ONLY */

  /**
   * Creates an interface to the mote ID at mote.
   *
   * @param mote
   *          Mote ID's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public MspMoteID(Mote mote) {
    this.mote = (MspMote) mote;
    this.moteMem = (MspMoteMemory) mote.getMemory();

    String[] variables = moteMem.getVariableNames();

    location = ID_LOCATION.JAVA_ONLY;
    for (String variable: variables) {
      if (variable.equals("TOS_NODE_ID")) {
        if (location == ID_LOCATION.VARIABLE_NODE_ID) {
          location = ID_LOCATION.VARIABLES_BOTH;
        } else {
          location = ID_LOCATION.VARIABLE_TOS_NODE_ID;
        }
      } else if (variable.equals("node_id")) {
        if (location == ID_LOCATION.VARIABLE_TOS_NODE_ID) {
          location = ID_LOCATION.VARIABLES_BOTH;
        } else {
          location = ID_LOCATION.VARIABLE_NODE_ID;
        }
      }
    }

    /*logger.debug("ID location: " + location);*/

    if (PERSISTENT_SET_ID) {
      mote.getSimulation().scheduleEvent(persistentSetIDEvent, mote.getSimulation().getSimulationTime()+1);
    }
  }

  private TimeEvent persistentSetIDEvent = new TimeEvent(0) {
    public void execute(int t) {

      if (persistentSetIDCounter-- > 0)
      {
        setMoteID(moteID);
        /*logger.debug("Persistent set ID: " + moteID);*/

        if (t < -mote.getInterfaces().getClock().getDrift()) {
          mote.getSimulation().scheduleEvent(this, -mote.getInterfaces().getClock().getDrift());
        } else {
          mote.getSimulation().scheduleEvent(this, t+1);
        }
      }
    }
  };

  public int getMoteID() {
    if (location == ID_LOCATION.VARIABLE_NODE_ID) {
      return moteMem.getIntValueOf("node_id");
    }

    if (location == ID_LOCATION.VARIABLES_BOTH) {
      return moteMem.getIntValueOf("node_id");
    }

    if (location == ID_LOCATION.VARIABLE_TOS_NODE_ID) {
      return moteMem.getIntValueOf("TOS_NODE_ID");
    }

    if (location == ID_LOCATION.JAVA_ONLY) {
      return moteID;
    }

    logger.fatal("Unknown node ID location");
    return -1;
  }

  public void setMoteID(int newID) {
    moteID = newID;

    if (location == ID_LOCATION.VARIABLE_NODE_ID) {
      if (GENERATE_ID_HEADER) {
        /* Write to external flash */
        SkyFlash flash = mote.getInterfaces().getInterfaceOfType(SkyFlash.class);
        if (flash != null) {
          flash.writeIDheader(newID);
        }
      }
      moteMem.setIntValueOf("node_id", newID);
      setChanged();
      notifyObservers();
      return;
    }

    if (location == ID_LOCATION.VARIABLES_BOTH) {
      if (GENERATE_ID_HEADER) {
        /* Write to external flash */
        SkyFlash flash = mote.getInterfaces().getInterfaceOfType(SkyFlash.class);
        if (flash != null) {
          flash.writeIDheader(newID);
        }
      }
      moteMem.setIntValueOf("node_id", newID);
      moteMem.setIntValueOf("TOS_NODE_ID", newID);
      moteMem.setIntValueOf("ActiveMessageAddressC$addr", newID);
      setChanged();
      notifyObservers();
      return;
    }

    if (location == ID_LOCATION.VARIABLE_TOS_NODE_ID) {
      moteMem.setIntValueOf("TOS_NODE_ID", newID);
      moteMem.setIntValueOf("ActiveMessageAddressC$addr", newID);
      setChanged();
      notifyObservers();
      return;
    }

    if (location == ID_LOCATION.JAVA_ONLY) {
      setChanged();
      notifyObservers();
      return;
    }

    logger.fatal("Unknown node ID location");
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JLabel idLabel = new JLabel();

    idLabel.setText("Mote ID: " + getMoteID());

    panel.add(idLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        idLabel.setText("Mote ID: " + getMoteID());
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public double energyConsumption() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Infinite boolean
    element = new Element("id");
    element.setText(Integer.toString(getMoteID()));
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("id")) {
        setMoteID(Integer.parseInt(element.getText()));
      }
    }
  }

}
