---
title: "Fencing Equipment Tester"
author: "Josh-Lambda"
description: " "
created_at: "2025-05-31"
---

## May 31st: Initial Research & Component Selection

I began by re-reading the FIE (International Fencing Federation) documentation on the material requirements of each weapon and confirm details such as signaling information and the permitted resistances of each component. Because fencing has three main weapons, there are three different signaling protocols the device must be able to test. However, much of the physical connectors and pinouts is shared between weapons and as such I am hopeful that it will be possible to limit the need for separate testing hardware through use of a microcontroller to switch inputs, etc.

The function of testing equipment will be primarily served through measurement of resistance across components leading to a pass/fail result based on comparison to the standards. However, I also intend to add an option to select a weapon and then have weapon testing including hit detection and on/off target detection. This will require additional ports and can be self contained within the unit. Finally, I am also investigating the ability to send test signals to other score machines. Because voltages are not standardised between units, this would require implementation of a set of relays to close the connections from the external unit.

### Control Requirements

The unit will require a microcontroller to register the test inputs and generate sample outputs. This will likely be an esp32 derived controller selected further on once requirements are better established.

As there is intent to extend functionality to generating test signals for external equipment a series of relays will be required. This is because fencing computers do not all operate at a standard logic level for piste signals, with significant variation between devices. However, most devices use a voltage of around 0.75V to 1V so relays and other switching components can be designed around this low range - potentially using 3.3V rated components to provide a safe margin of error.

### Hardware interface

The hardware interfaces of the device are influenced by the definitions within the FIE technical documentation. This provides a unified system for connectors and pinouts across equipment globally.

Choices regarding the interfaces are centered around trying to provide as much functionality as possible, but will change over time as the project implementation evolves.

Interfaces that are likely required for the device include:

Weapon & Fencer Interfaces

- Body wire connector - 3 Pin Banana Plug (FIE)
- Eppe weapon connector - 3 Pin Banana Plug (FIE)
- Foil/sabre weapon connector - 2 Pin Banana Plug (FIE)
- Ground tab - strip of metal to attach lame clip

User Interfaces

- Weapon Selection
- Mode Cycle
- Screen - For output of test information & manual test feedback

External Box Test Interfaces

- 3 Pin Banana Plug (FIE) output port to test external hit machines

**Total time spent: 3h**

## June 1st: Research into individual weapon checks

I continued researching how to best implement the tests on individual weapons. The key requirements of this testing mode must include:

### Validation of resistance

While fairly simple, all equipment has specific resistance ratings that it must remain under to perform properly. If there is any increase in resistance due to rust or microscopic fractures, it can lead to the incorrect detection of a hit. This often manifests through weapons registering as off target in foil or false hits in eppe. By checking these factors, it is possible to diagnose a broken/worn out tip or a poor body wire connection (often stemming from collapsed plugs).

Required maximum resistances are:

- The body wire must have a resistance of 1 ohm
- The spool must have a resistance of 3 ohms
- Weapon resistance of 2 ohms
- Cable resistance of 2.5 ohms

### Hit registration

In order to test that weapons are wired correctly, it must be possible to check continuity between the wires going to the button. This allows verification that hits will register. For foil and sabre, it is also necessary to check continuity to the jacket of the opponent. This is because foil and sabre only register 'valid' hits on a specific section of the body and all other hits must be registered as a separate off-target signal.

Within documentation, each pin of the standard connector is only referred to by its distance from the centre. I will define pinout numbers for them to simplify the creation of documentation and schematics throughout this project.

![Pinout Diagram for 3 Pin FIE Connector](images/3-Pin-Pinout.png)

#### Pin Usage

| Weapon | Pin 1 | Pin 2 | Pin 3 |
| ------ | ----- | ----- | ----- |
| Foil   | Lame | Tip | Weapon/Piste |
| Eppe   | Direct Wire | Other Wire | Weapon/Piste |
| Sabre  | Lame | Tip | Weapon/Piste |

#### Hit detection

The criteria for registering a hit differ between weapons.

In foil and sabre, pins 2 & 3 must be closed on the weapon, and Pin 2 must be connected to the opponents pin 1 (Via the opponents conductive jacket). If pins 2 & 3 are closed but the circuit through the opponent's jacket is not completed it is counted as 'off-target'. In this case no point is awarded, but the bout must still be halted and so it must be detectable and displayed. Foil and sabre have all connections normally open.

In eppe pins 1 & 2 are normally closed, and a hit is registered when the connection is opened. Because eppe does not have a target area, Pin 3 is used to detect connection to the floor or opponents weapon. If the weapon hit the floor instead of the opponent, the hit will not register.

### Implementation of hit detection

Hit detection will be implemented with a single 3 pin connector attached to the body wire and weapon. By raising pin 2 to high and connecting pins 1 & 3 to input ports on the microcontroller, it is possible to check which circuits are complete and thus register a hit in testing. In order to have this adapt to the different weapons, a weapon select button will be required to change the logic between foil, eppe, and sabre.

In order to correctly identify target/vs off target for foil and eppe, it will be necessary to include a metal tab that simulates the connection to the lame.

### Implementation of resistance checking

Resistance checking is a key feature in troubleshooting. However, implementing it with minimal components is challenging. Because of this, I have decided to limit resistance checking to just the body wire. I believe this is reasonable to prevent score creep and because these are the most commonly troubleshooted components. Since this gives us a very small range of resistance, it will be simple to include a voltage divider that compares if the wire is over the set legal resistance value with a comparison resistor. Having a small range of resistances that are being tested also removes the key issue of using a voltage divider which is that they get increasingly inaccurate as you move away from the known comparison value.

### Implementation of test signal output

In order to test external equipment capable of running matches with precise FIE certified timings, it is also necessary to add a signal generator. This can be managed by plugging a 3 pin into a strip and using relays to close/open the pins depending on the selected weapon. Relays are required as there is no standardised voltage that boxes operate on and some use more advanced ground separation between the fencers to prevent issues with foil. However, the relays can be rated for only a couple of volts as must boxes use fairly low logic levels.

### Hardware for the test signal output

Because of the variability of fencing boxed, the relays need to be bi-directional DC. This eliminates SSRs from being an option. Realistically, I need 3 SPST relays with a 3.3V coil level and a low voltage low current load. From this, I have moved towards using miniature mechanical relays but need to decide what pole type to use - DPDT and SPDT relays are often more common and depending on the component are cheaper than low quantity SPST.

As a result of these factors I have decided on the G6K as the relay for this project - specifically the G6K-2P-Y (through hole) as I cannot mount surface mount components at the moment and do not plan on getting PCB assembly (this may change after i select the microcontroller). This will require the addition of a transistor to prevent the risk of overloading the GPIO pins and a blowback diode. After completing a relay module the schematic looks like this:

![Pinout Diagram for 3 Pin FIE Connector](images/relayModule.png)

Adding in the connections for both foil and eppe along side a connection to the opponents lame it looks something like this:

![Pinout Diagram for 3 Pin FIE Connector](images/fullTestRelay.png)

### BOM

I have begun putting the components from the schematic so far into the BOM. I have tried my best to get accurate prices but they may be a bit off due to USD exchange rate.

The main thing I am unsure of is sourcing - particularly given the request to go through the cheapest supplier. This is a request I fully agree with but am unsure of how to best implement. Aliexpress has far cheaper prices per unit for like 100pcs but since I only need three getting it for a much worse price from a supplier within australia (who is renowned for having bad unit prices) ends up being slightly cheaper (around USD0.50) to buy 3pcs relative to the 100pcs on Aliexpress.

**Total time spent: 4h**

## June 3rd: Component research, new schematics

I continued researching suppliers for some of the components I am using. I have also continued developing the schematic for the resistance measurements. Furthermore, I worked on implementing the internal weapon tester. The internal tester will allow this to be used to check wiring of a weapon. While full competition boxes can be used for this weapon testing functionality, a unit will normally set you back a couple of thousand dollars meaning that incorporating this function into a smaller, more portable, and cost efficient package will be a valuable tool for diagnosis and performing repairs away from the piste.

### Resistance Checker

Previously, I settled on using a voltage divider based system to measure the resistance of components. This was because of the small form factor, low cost, and reduced complexity - especially when administrating a pass/fail against a set value.

The feature will focus on testing the body wire as it is the most common and exposed point of failure. This means it is a simple pass/fail to check the resistance is under one ohm. To get the result, a 1 ohm resistor will be places parallel to the wire and the proportion of voltage that takes the wire path can be used to calculate the resistance. A 1 ohm resistor is chosen for reference as the closer the reference resistor is to the value we are attempting to measure, the more accurate the reading is. This means that while this is not the most accurate way of measuring a wide range of resistances it will be more than sufficient for our purpose of giving a pass/fail with some indication of what the resistance of the body wire could be.

In a voltage divider, the output voltage can be calculated using:
*V*~out~=*V*~in~*R*~2~÷(*R*~1~+*R*~2~)

Since we are measuring the output voltage, we can rearrange the equation to have *R*~2~ as the subject:
*R*~2~=*R*~1~((*V*~in~/*V*~out~)-1)^-1^

Within our system, all other variables are known (provided we use an analogue port to measure *V*~out~):

| Variable | Value |
| -------- | ----- |
| *V*~in~  | 3.3V  |
| *V*~out~ | DIO   |
| *R*~1~   | 1Ω    |
| *R*~2~   | ?Ω    |

To achieve this, the circuit will look something like this:
![Image of voltage divider circuit](images/voltageDivider.png)

However, one important thing to note is that R2 will be out body wire. As such it will actually be connected through a series of ports.
![Image of voltage divider circuit with body wire](images/voltageDividerWire.png)

When we chain three of them to access all of the wires:
![Image of all three voltage divider circuits with body wires and GPIO pins](images/voltageDividerGPIOx3.png)

### Weapon tester

The weapon testing portion is fairly simple. While complete fencing computers need to pay strict attention to the timing of the match and the interval between hits, for testing weapons the high precision is superfluous. In order to identify that all wires are connected correctly, we will apply a voltage to pin 2 and then read which of the other wires the voltage is fed to. It is also important to include a reference to the opponents lame to allow for full foil/sabre testing. This role can be fulfilled by a metal tab on the housing.

![Concept schematic for weapon testing](images/basicWeaponTest.png)

However, this creates a problem where we have nine ports just for testing the wire and weapon. In order to aid this situation, we can try to take advantage of the GPIO functionality of the esp32. For this, I will be targeting the ground side of the voltage divider module. This is because it will be isolated from the high side in all cases other than resistance testing.

Remember how earlier, it was decided to use DPDT relays as SPST ones are hard to find and more expensive? This is going to be of great benefit.

While there is probably a fancy way of implementing something like this using diodes, transistors, and differently configured GPIO pins on the ground side, we already have the relays so we may as well use them.

By linking the weapon port pins to the relays we now have the ability to switch each pin between two paths. This will allow the single set of connectors to be switched between the weapon testing circuit and the voltage divider circuit.

The resistance of the relays is negligible and so will have little impact on the resistance measurements especially at the precision we are dealing with. If any interference occurs, it will also cause the body wire to turn a false positive for high resistance which is teh preferable failure mode as a false negative would potentially further complicate troubleshooting.

After implementing these changes and unifying the schematics we are left with something like this (I know these are not the proper GPIO pins yet. I am leaving them generic until a microcontroller is decided upon):

![Schematic of the relays, voltage dividers, and weapon tester integrated together](images/integrated1.png)

### Plans for additional relays

I would also like to consider how the two ports used on the signal generator can be reduced. Currently I plan on taking the connector only used for the opponents lame and using an additional relay to switch it between the function generator and the high side of the voltage divider. However, it may be necessary to use two relays (4 switches) in order to prevent back feed issues into the other lines when testing. This issue is unlikely to change the results of tests BUT could damage the external box being tested, and as such I will proceed with caution. While this change would increase the PCB footprint, I think saving a full connector will be worth it as adding connectors not only increases the PCB size but also its perimeter by at least 50mm on a single side.

### BOM Update

I have continued looking into suppliers for the components from last entry. I have also added the voltage divider resistors to the BOM.

**Total time spent: 2h**

## June 5th: Connector Revisions

### Additional relays

I have added the extra relays to the schematic. This allows the auxiliary port's connections to be switched between the signal generator and the high side of the voltage dividers. This eliminates another set of connectors bringing the total down to 3 sets. While I would love to bring this down to two, adding another three relays and having to add two steps in routing signals does not currently seem worth it.

### Other changes to schematic

I have also added an espS3 symbol to the schematic. This is not the final microcontroller I plan on using and is mostly there to avoid the net errors from KiCad. Over the next couple of days, I plan on selecting the final microcontroller.

**Total time spent: 1h**

## June 15th

### Selecting Microcontroller

In order to complete the rest of the schematic including IO, displays and charging/batteries I will need to select the microcontroller. I am currently looking at an esp32 based controller, but need to define the technical requirements.

Bellow are the requirement for the project based on the current schematic and plans:

|          | Requirements  |
| -------- | ------------  |
| Wireless | None          |
| COMMs    | Serial, I^2^C |
| DIO      | 12 Pins       |
| Power    | Low Power     |

I intend on using a usb port to facilitate charging. If the selected controller supports USB, the same port will be used to program it.

The main options I am considering are the ESP32-S3, ESP32-C3, and ESP32-C6. The the S3 has greater processing power and USB at the disadvantage of significantly higher (up to 2x) power draw during sleep. This may have a significant impact on battery life. The C3 has improved power draw, but lacks inbuilt ROM. The C6 has the best battery performance of all options. However, it also has limited processing power.

Reflecting upon the needs of the system, the computational workload will be minimal - mostly setting GPIO values with no background tasks or wireless functionality/data transmission. Because of this, I am choosing to proceed using the C6. Other than processing power, it has few limitations over the C3 for this use case and because we are not overly concerned by how quickly it can tell an LCD to draw a rectangle even this point is fairly minimal. The C6 also has the advantage of a slightly smaller profile.

From the C6, it is then needed to consider using it as an SOC or a module. While an SOC provides greater customization regarding the chipset's function, the range of module options have several advantages. These include lower development time, and included ROM packaging.

Because of this, I am considering using either the ESP32-C6-MINI-1U or the TinyC6 board from unexpected maker, with the benefit of the later being built in charging circuitry/PMIC. With the MINI module, the U variant would be selected due to its decreased size from elimination of the PCB antenna - a change that does not effect this project due to its lack of wireless capabilities. However, the footprint & packaging of the TinyC6 are non-ideal as it is dependant on jumper pins for mounting which would increase the full units size even more than the bulky banana jacks already are. As such, I will continue planning around using the ESP32-S6.

**Total time spent: 3h**

## July 1st

### Building out interfaces and revisiting hardware definitions

Following the work selecting a controller previously, I am revisiting the requirements of the project to ensure that everything is still in alignment. The core functionalities of:

- Resistance testing
- Weapon testing
- External signal generation

were outline previously, and have all been addressed so far.

As mentioned at the start of the project the qualities expected for the device included

> **Weapon & Fencer Interfaces**
> - Body wire connector - 3 Pin Banana Plug (FIE)
> - Eppe weapon connector - 3 Pin Banana Plug (FIE)
> - Foil/sabre weapon connector - 2 Pin Banana Plug (FIE)
> - Ground tab - strip of metal to attach lame clip
> 
> **User Interfaces**
> - Weapon Selection
> - Mode Cycle
> - Screen - For output of test information & manual test feedback
> 
> **External Box Test Interfaces**
> - 3 Pin Banana Plug (FIE) output port to test external hit machines

So far, we have been able to reduce the 4 weapon/strip connectors down two 3 connectors by using an additional set of relays to make one of the ports multi-functional. This was determined to be worth the extra space for relays as teh extra connector's size impact was restricted by the housing perimeter not the area, and as such was more problematic. However, adding even more relays to reduce down to two connectors would go past the point of returns.

Regarding the rest of the interfaces, the weapon selection will be fulfilled by a tri-pole switch with mode functionality being served by a button. This is because while weapon is a fixed set of three options, mode may change with software versions or require additional functions such as a long hold implemented in the future. From these definitions, it will require an additional three GPIO connections to connect the button & switch peripherals (1 for the button. Two for the extreme poles of the switch with the centre pole detected by absence of both other poles).

Regarding the screen interface, I am currently targeting an I2C based display. While this will not provide teh greatest display quality, it will ensure that it allows for a small form factor and minimise its cost in GPIO pins. This will still bring the total GPIO pins up to 17. This stays within the C6 mini's capability of 22 GPIO pins, while leaving room for USB communications for programming.

### Updates to the schematic

From this, I have updated the references in the testing schematic to include the ESP32-C6-MINI-1 (I will have to make a footprint for the U variant later). I have also started making the changes to include the USB communications. I have also added the switch and button for reference.

In adding the USB connection, it powers the device from 5v. This will require a regulator to step down the voltage to 3.3v to power the esp. In order to reduce complexity I am (at least for the time being) discontinuing consideration of a battery power option to allow for the relays to be put on the 5v side of the regulator, reducing its current requirements.

For the voltage regulator, I have begun working with the TPS62840 as it meets the current requirements of the device once accounting for relays and displays while providing ample room for transitioning to battery in the future (including a disable mode that could be integrated with a power sequence). After adding the inductor and smoothing caps the schematic for it looks something like this:
![Schematic of voltage regulator module](images/voltageRegulator.png)

Hooking up the USB port, we obtain something like the image bellow. Importantly, the VCONN line is disconnected as it is only used by the host device to detect and eCable. The 5.1k resistor on the CC line should indicate that the device wants to draw more than the 100-500mA of the default spec and instead tell it to draw up to 3A at 5V. The data lines get connected to the ESP32 C6 and the 5V source gets fed into our converter.
![Image of USB-C port](images/usbPort.png)

This brings the full schematic to something like this:
![Full Schematic progress update](images/schematicUpdate.png)

**Total time spent: 4h**
