# STM32-PONG

A simple implementation of Pong running on the STM32F103C8T6 (Blue Pill), built entirely from scratch as a bare-metal project.

---



### Demo Video

<a href="https://youtu.be/8k6uzGnyQ8M">
  <img 
    src="https://img.youtube.com/vi/8k6uzGnyQ8M/hqdefault.jpg" 
    width="420"
    alt="PONG Demo Video">
</a>


---

# Building
## Requirements
```
arm-none-eabi-gcc
arm-none-eabi-binutils
make
stlink
```

Build the project:

```
make
```

Flash the firmware:

```
make flash
```

Clean build files:

```
make clean
```
