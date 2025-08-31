# arduino-oscilloscope
A simple, feature-rich DIY digital oscilloscope using an Arduino, an SSD1306 OLED display, and a rotary encoder. Features fast ADC, auto-scaling, and adjustable timebase.
# Arduino DIY Mini Oscilloscope

A simple, yet powerful, DIY digital oscilloscope built using an Arduino-compatible board, a common SSD1306 I2C OLED display, and a rotary encoder for control. This project is designed to be a low-cost, educational tool for visualizing simple electronic signals.

It leverages a memory-efficient display library (`U8g2`), hardware interrupts for a responsive user interface, and ADC prescaler optimization for faster waveform sampling.

![Alt Text](https://media.giphy.com/media/v1.Y2lkPTc5MGI3NjExMWY5aGMyNnE4eWIwZW43bHN6bDFodXEzZXRieWo0dW91Nmt4Y2hoeCZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/zTAVYJy7ZKJyL3gAjC/giphy.gif)

## Features

-   **Real-time Waveform Display:** Visualizes analog signals on a 128x64 OLED screen.
-   **Rotary Encoder Control:** Easily adjust the timebase (horizontal zoom) by turning the encoder knob.
-   **Adjustable Timebase:** Cycle through multiple time-per-division settings to inspect signals of different frequencies.
-   **Single Shot / Freeze Mode:** Press the encoder button to freeze the current waveform on the screen for detailed analysis.
-   **Auto Vertical Scaling:** The display automatically adjusts the vertical scale (voltage axis) to fit the signal perfectly on the screen (auto min/max).
-   **Fast ADC Sampling:** The ADC prescaler is adjusted to allow for a higher sampling rate than the default `analogRead()`, enabling visualization of faster signals.
-   **Informative UI:** Displays the current time-per-division setting and a "FROZEN" indicator directly on the screen.
-   **Lightweight and Efficient:** Uses the `U8G2_NONAME_1` buffer mode, which consumes minimal RAM on the microcontroller.

## Hardware Required

-   **Microcontroller:** Any Arduino-compatible board (e.g., Arduino Uno, Nano, Pro Mini).
-   **Display:** 0.96" 128x64 SSD1306 OLED Display (I2C version).
-   **Input Control:** KY-040 Rotary Encoder Module (or a standalone encoder with pull-up resistors).
-   **Wiring:** Breadboard and jumper wires.

## Wiring Diagram

Connect the components to your Arduino as follows. Note that the CLK pin of the encoder **must** be connected to a hardware interrupt pin (D2 or D3 on an Arduino Uno/Nano/Pro Mini).

| Component           | Component Pin | Arduino Pin |
| ------------------- | ------------- | ----------- |
| **OLED Display** | VCC           | 5V          |
|                     | GND           | GND         |
|                     | SCL           | A5          |
|                     | SDA           | A4          |
| **Rotary Encoder** | +             | 5V          |
|                     | GND           | GND         |
|                     | CLK           | D2          |
|                     | DT            | D3          |
|                     | SW            | D4          |
| **Signal Input** | (Probe Tip)   | A0          |
|                     | (Probe Ground)| GND         |

## Software Dependencies

This project requires the **U8g2lib** library. You can install it directly from the Arduino IDE:

1.  Go to **Tools > Manage Libraries...**
2.  Search for "U8g2".
3.  Find the library by **oliver** and click **Install**.

The `<Wire.h>` library is included with the Arduino IDE by default.

## How to Use

1.  **Assemble** the circuit according to the wiring diagram.
2.  **Upload** the code to your Arduino.
3.  **Power on** the device. You will see a brief splash screen.
4.  **Connect** the analog signal you want to measure to pin `A0` and `GND`.
5.  **Adjust Timebase:** Turn the rotary encoder knob to "zoom" in or out of the waveform horizontally.
    -   **Clockwise:** Zooms in (less time per division).
    -   **Counter-Clockwise:** Zooms out (more time per division).
6.  **Freeze Display:** Press the rotary encoder's built-in button once to freeze the current waveform. Press it again to resume live capture.

## Code Explanation

#### `setup()`
-   Initializes the I2C communication (`Wire.begin()`) and the OLED display (`u8g2.begin()`).
-   Configures the rotary encoder pins as inputs with internal pull-up resistors.
-   **ADC Optimization:** The line `ADCSRA |= (1 << ADPS2);` changes the ADC clock prescaler from the default of 128 to 16. This results in a significantly faster analog-to-digital conversion time (~13.5 µs instead of ~112 µs), allowing for a higher maximum sampling frequency.
-   Attaches a hardware interrupt to the encoder's CLK pin, which calls the `updateEncoder()` function on a `FALLING` edge for reliable and responsive input.

#### `loop()`
1.  **Button Check:** Non-blocking check for the encoder button press to toggle the `isFrozen` flag.
2.  **Data Acquisition:** If not frozen, the code captures 128 samples from `A0`. A `delayMicroseconds()` call, determined by the `timebaseOptions` array, is placed between each sample to control the time scale.
3.  **Auto-Scaling:** It iterates through the captured `readings` array to find the minimum and maximum values. These values are then used in the Arduino `map()` function to scale the waveform to fit the screen's height perfectly.
4.  **Drawing:**
    -   The code enters a `u8g2.firstPage() ... do...while(u8g2.nextPage())` loop to draw the content to the display's buffer.
    -   It first draws a grid for reference.
    -   It then draws the waveform by connecting each sample point to the next with a line.
    -   Finally, it renders the UI text (timebase and frozen status) with a black background box for better readability.

#### `updateEncoder()`
-   This is an Interrupt Service Routine (ISR). It's a short and fast function that runs whenever the encoder's CLK pin goes from HIGH to LOW.
-   It determines the direction of rotation by comparing the state of the DT pin to the CLK pin and safely increments or decrements the `timebaseIndex`.

## Possible Improvements

-   **Triggering:** Implement a simple rising or falling edge trigger to stabilize repetitive waveforms.
-   **Voltage Measurement:** Add code to calculate and display the peak-to-peak voltage ($V_{pp}$) and frequency of the signal.
-   **Input Protection:** Add a simple protection circuit (e.g., a Zener diode and resistor) to protect the Arduino's analog pin from overvoltage.
-   **AC/DC Coupling:** Add a switch and a capacitor to allow for AC coupling.

## License

This project is open-source. Feel free to use, modify, and distribute the code. A credit or link back to this repository is appreciated but not required.
