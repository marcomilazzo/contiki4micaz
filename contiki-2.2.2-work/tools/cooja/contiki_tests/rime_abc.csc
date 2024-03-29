<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <ticktime>1</ticktime>
    <randomseed>123456</randomseed>
    <nrticklists>1</nrticklists>
    <motedelay>5000</motedelay>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/rime/example-abc.c</source>
      <command>make example-abc.sky TARGET=sky</command>
    </motetype>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>33.53152221759984</x>
        <y>15.078801911773077</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>54.67966631314053</x>
        <y>31.0253850381183</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>313</width>
    <z>2</z>
    <height>199</height>
    <location_x>30</location_x>
    <location_y>30</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.radiomediums.UDGM$VisUDGM
    <width>300</width>
    <z>3</z>
    <height>122</height>
    <location_x>367</location_x>
    <location_y>23</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <history>256</history>
    </plugin_config>
    <width>573</width>
    <z>1</z>
    <height>242</height>
    <location_x>23</location_x>
    <location_y>235</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

