# FreeRTOS Smart Environment Monitor

A real-time embedded systems project built using STM32F103C6, FreeRTOS, and Proteus simulation.

## Overview
This project monitors environmental parameters — temperature, humidity, ambient light, and gas levels — using dedicated FreeRTOS tasks running concurrently on an STM32 microcontroller. All code is written in pure Embedded C with direct register-level access (no HAL).

## Features
- DHT11 sensor for temperature and humidity
- ADC-based LDR for light detection
- Potentiometer simulating MQ-2 gas sensor (Proteus)
- 16x2 LCD display for live sensor readings
- Automatic fan control, LED, and buzzer based on thresholds
- FreeRTOS task scheduling with queue-based data sharing

## Tech Stack
- STM32F103C6 (ARM Cortex-M3)
- FreeRTOS
- Pure Embedded C (register-level, no HAL)
- Proteus 8 for simulation
- STM32CubeIDE
