# sailpi_UPS
This is an Arduino (Moteino) sketch for providing basic UPS functionality for a Raspberry Pi computer. 
It runs on a [Moteino](http://lowpowerlab.com/moteino/) controlled [MightyBoost](http://lowpowerlab.com/mightyboost/) 
device [modified](https://lowpowerlab.com/forum/index.php/topic,1250.0.html) for a lower sleep power consumption.

Overall functionality is as follows:
* Provide 5V to RasPi and charge local LiPo battery when receiving external power
* Provide 5V to RasPi from LiPo and signal shutdown request to Raspi when external power is lost
* Cut power to RasPi once it has (signalled) shutdown safely
* Sleep with as small power consumption as possible while waiting for external power to appear

Hardware modifications to MightyBoost required:
* Cut the PCB trace from VCC to booster IC [SC4501](http://www.semtech.com/images/datasheet/sc4501.pdf) pin 3 (nSHDN)
* Connect the booster IC pin 3 to D7 Moteino digital output
* (Remove the permanent 5V LED)

TODO
* Handle bootup and shutdown failure conditions appropriately
* (Switch MightyBoost battmon resistor divider into higher impedance for lower current bleed)
