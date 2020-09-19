###### Shack Kiosk v5
A faster and more responsive version of the shack kiosk

##### Dependencies

- qt5-qmake
- pkg-config
- libglm-dev
- libsdl2-dev
- libsdl2-image-dev
- libsdl2-ttf-dev
- libcurl4-openssl-dev

##### Building

```
git clone https://github.com/shackspace/shack_kiosk_v5.git
git submodule init
git submodule update
qmake -o build/
make -C build/
```
