# Liri Text
This is Liri Text, crossplatform text editor made in accordance with Material Design.  
For now it only provides very basic functionality, but will evolve into a very powerful tool some time.

## Installation
You may find a prebuilt binary for your platform on [releases page](https://github.com/liri-project/liri-text/releases).
If your platform is not yet supported or you wish to contribute to this project's development, you can compile Liri Text from source following the instructions below:

## Build
1. Install dependencies
  - [Qt 5.5 or higher](http://qt.io)

2. Build Liri Text
  ```
  git clone --recursive https://github.com/liri-project/liri-text.git
  cd liri-text
  mkdir build; cd build
  qmake ..
  make
  ```

3. Optionally install  
  ```
  sudo make install
  ```

## License
This application is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

See LICENSE for more information.
