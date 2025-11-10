# System Monitor

## Overview
The **System Monitor** is a real-time system monitoring application built using **C++ (Qt Framework)** with an interactive **HTML, CSS, and JavaScript** dashboard interface.  
It provides live updates of CPU usage, memory consumption, system uptime, and active processes — similar to the Linux `top` command but with a modern graphical interface.

The backend (C++/Qt) communicates with the frontend via **Qt WebEngine** and **Qt WebChannel**, allowing real-time data exchange and dynamic UI updates.

---

## Features

- **Real-Time Monitoring**  
  Displays continuously updating system metrics such as CPU load, memory usage, and uptime.

- **Process Management**  
  Lists active processes with PID, name, CPU usage, and memory usage.

- **Process Termination**  
  Allows killing a process by entering its PID or using the inline “Kill” button.

- **Sortable Process Table**  
  Sort by CPU or memory usage to identify resource-heavy processes easily.

- **Live Data Refresh**  
  Data refreshes automatically every few seconds, ensuring up-to-date system information.

---

## Technology Stack

| Component | Technology Used |
|------------|----------------|
| Backend | C++ with Qt 5 / Qt 6 |
| Frontend | HTML5, CSS3, JavaScript |
| Visualization | Chart.js |
| GUI Bridge | Qt WebEngine, Qt WebChannel |
| OS Interface | Linux `/proc` filesystem |

---

## Project Structure

System-Monitor/
│
├── system-monitor.pro # Qt project configuration
├── README.md # Project documentation
│
├── src/ # Backend source files
│ └── sm.cpp # Main backend logic
│
├── web/ # Frontend assets
│ ├── dashboard.html # HTML dashboard UI
│ ├── style.css # Styling file
│ └── script.js # JavaScript for data updates and visualization
│
└── assets/ # Images and icons
└── dashboard.png # Project screenshot

---

## Dependencies

Before building, ensure you have the following installed:

## On Ubuntu / Debian:
bash
sudo apt update
sudo apt install build-essential qtbase5-dev qtwebengine5-dev libqt5webchannel5-dev qt5-qmake -y
sudo dnf install qt5-qtbase-devel qt5-qtwebengine-devel qt5-qtwebchannel-devel
sudo pacman -S qt5-base qt5-webengine qt5-webchannel


## Build and Run Instructions
Clone the repository
git clone https://github.com/taufeeq-devt/System-Moniter.git
cd System-Moniter

# Generate Makefile using qmake
qmake system-monitor.pro

# Compile the project
make

# Run the application
./sm
The application window will open with a live-updating dashboard showing CPU, memory, and process information.

## Functional Overview
Backend (src/sm.cpp)
Collects system data through:

/proc/stat for CPU usage

/proc/meminfo for memory statistics

/proc/[pid]/comm for process names

Calculates CPU and memory utilization.

Provides this data to the frontend via Monitor (QObject) over a QWebChannel.

Implements process termination using the SIGKILL signal.

## Frontend (web/)
Displays live charts and gauges using Chart.js.

Dynamically updates data every few seconds using asynchronous JS calls.

Includes a process table with inline “Kill” actions.

## System Requirements
Operating System: Linux (tested on Ubuntu 22.04)

Qt Version: 5.12+ or Qt 6 equivalent

C++ Standard: C++17 or later

RAM: Minimum 1 GB

Compiler: GCC or Clang

## Future Enhancements
Core-wise CPU and memory usage breakdown

Disk and network utilization graphs

Theme customization (dark/light mode)

Data export options (CSV / JSON)

System alerts for high resource usage

Push-based updates via Qt signals (instead of polling)
