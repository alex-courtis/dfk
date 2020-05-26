## MOVED

https://gitlab.com/interception/linux/plugins/dual-function-keys

## NAME

dfk - dual function keys

## DESCRIPTION

Tap for one key, hold for another.

Great for modifier keys like: hold for ctrl, tap for delete.

A hand-saver for those with restricted finger mobility.

A plugin for [interception tools](https://gitlab.com/interception/linux).

## FUNCTIONALITY

In these examples we will use the left shift key (LS).

It is configured to tap for backspace (BS) and hold for LS.

### Tap

Press and release LS within TAP\_MILLIS (default 200ms) for BS.

Until the tap is complete, we get LS.

``` 
                <---------200ms--------->     <---------200ms--------->
keyboard:       LS↓      LS↑                  LS↓                          LS↑
computer sees:  LS↓      LS↑ BS↓ BS↑          LS↓                          LS↑
```

### Double Tap

Tap then press again with DOUBLE\_TAP\_MILLIS (default 150ms) to hold BS.

``` 
                             <-------150ms------->
                <---------200ms--------->
keyboard:       LS↓         LS↑             LS↓               LS↑
computer sees:  LS↓         LS↑ BS↓ BS↑     BS↓ ..(repeats).. BS↑
```

You can continue double tapping so long as it is within the DOUBLE\_TAP\_MILLIS window.

### Consumption

Press or release another key during the TAP\_MILLIS window and the tap will not occur.

This is especially useful for modifiers, for instance a quick ctrl-C. In this example we press the a key during the window.

Double taps do not apply after consumption; you will need to tap first.

``` 
                                                               <-------150ms------->
                                                 <---------200ms--------->
                                 <-------150ms------->
                <---------200ms--------->
keyboard:       LS↓   a↓  a↑     LS↑             LS↓          LS↑           LS↓
computer sees:  LS↓              LS↑             LS↓          LS↑ BS↓ BS↑   BS↓ ..(repeats)..
```

## INSTALLATION

Arch Linux users may install from the AUR: [interception-dfk](https://aur.archlinux.org/packages/interception-dfk).

## BUILDING

See [dependencies](https://gitlab.com/interception/linux/tools#dependencies).

``` sh
git clone git@github.com:alex-courtis/dfk.git
cd dfk
make && sudo make install
```

Installation prefix defaults to `/usr/local`. This can be overridden in `config.mk`.

## CONFIGURATION

There are two parts to be configured: dfk and udevmon, which launches dfk.

See [examples](https://github.com/alex-courtis/dfk/tree/master/examples) which contains dfk and udevmon configurations.

### dfk

This yaml file can go anywhere.

You can use raw (integer) keycodes, however it is easier to use the `#define`d strings from [input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h).

``` yaml
# optional
TIMING:
    TAP_MILLISEC: <integer>
    DOUBLE_TAP_MILLISEC: <integer>

# necessary
MAPPINGS:
    - KEY: <integer | string>
      TAP: <integer | string>
      HOLD: <integer | string>
    - KEY: ...
```

Our example from the previous section looks like:

``` yaml
TIMING:
    TAP_MILLISEC: 200
    DOUBLE_TAP_MILLISEC: 150

MAPPINGS:
    - KEY: KEY_LEFTSHIFT
      TAP: KEY_DELETE
      HOLD: KEY_LEFTSHIFT
```

### udevmon

udevmon needs to be informed that we desire Dual Function Keys. See [How It Works](https://gitlab.com/interception/linux/tools#how-it-works) for the full story.

``` yaml
- JOB: "intercept -g $DEVNODE | dfk -c </path/to/dfk.yaml> | uinput -d $DEVNODE"
  DEVICE:
    NAME: <keyboard name>
```

Usually the name is sufficient to uniquely identify the keyboard, however some keyboards register many devices such as a virtal mouse. You can run dfk for all the devices, however I prefer to run it only for the actual keyboard.

See [uinput -p](https://gitlab.com/interception/linux/tools#how-it-works) for instructions on how to get this information.

My `/etc/udevmon.yml`:

``` yaml
- JOB: "intercept -g $DEVNODE | dfk -c /etc/dfk.home-row-modifiers.yaml | uinput -d $DEVNODE"
  DEVICE:
    NAME: "q.m.k HHKB mod Keyboard"
- JOB: "intercept -g $DEVNODE | dfk -c /etc/dfk.kinesis-advantage-2.yaml | uinput -d $DEVNODE"
  DEVICE:
    NAME: "Kinesis Advantage2 Keyboard"
    EVENTS:
      EV_KEY: [ KEY_LEFTSHIFT ]
```

## CAVEATS

As always, there is a caveat: dfk operates on raw *keycodes*, not *keysyms*, as seen by X11 or Wayland.

If you have anything modifying the keycode-\>keysym mapping, such as [XKB](https://www.x.org/wiki/XKB/) or [xmodmap](https://wiki.archlinux.org/index.php/Xmodmap), be mindful that dfk operates before them.

Some common XKB usages that might be found in your X11 configuration:

``` 
    Option "XkbModel" "pc105"
    Option "XKbLayout" "us"
    Option "XkbVariant" "dvp"
    Option "XkbOptions" "caps:escape"
```

## FAQ

*I have a new use case. Can you support it?*

Please raise an issue.

dfk has been built for my needs. I will be intrigued to hear your ideas and help you make them happen.

As usual, PRs are very welcome.

*I see you are using q.m.k HHKB mod Keyboard in your udevmon. It uses [QMK Firmware](https://qmk.fm/). Why not just use [Tap-Hold](https://docs.qmk.fm/#/tap_hold)?*

Good catch\! That does indeed provide the same functionality as dfk. Unfortunately there are some drawbacks:

1.  Few keyboards run QMK Firmware.
2.  There are some issues with that functionality, as noted in the documentation [Tap-Hold](https://docs.qmk.fm/).
3.  It requires a fast processor in the keyboard. My unscientific testing with an Ergodox (\~800 scans/sec) and HHKB (\~140) revealed that the slower keyboard is mushy and unuseably inaccurate.

*Why not use [xcape](https://github.com/alols/xcape)?*

Xcape only provides simple tap/hold functionality. It appears difficult (impossible?) to add the remaining functionality using its XTestFakeKeyEvent mechanisms.

## CONTRIBUTORS

Please fork this repo and submit a PR.

If you are making changes to the documentation, please edit the pandoc flavoured `dfk.md` and run `make doc`. Please ensure that this `README.md` and the man page `dfk.1` has your changes and commit all three.

As usual, please obey `.editorconfig`.

## LICENSE

<a href="https://gitlab.com/interception/linux/plugins/caps2esc/blob/master/LICENSE.md"><img src="https://upload.wikimedia.org/wikipedia/commons/thumb/0/0b/License_icon-mit-2.svg/120px-License_icon-mit-2.svg.png" alt="MIT"></a>

Copyright © 2020 Alexander Courtis
