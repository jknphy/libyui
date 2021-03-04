# LibYUI - The Base Library

[![Build Status](https://travis-ci.org/libyui/libyui.svg?branch=master)](https://travis-ci.org/libyui/libyui)

Libyui is a widget abstraction library providing Qt, GTK and ncurses
frontends. Originally it was developed for [YaST](https://yast.github.io/)
but it can be used in any independent project.

### Selecting the used UI-plugin

By default LibYUI tries to load any of the available UI-plugins in this order:

* Qt:
  - if $DISPLAY is set
  - NCurses is user-selected and stdout is *not* a TTY

* Gtk:
  - if $DISPLAY is set and Qt is not available,
  - a GTK-based desktop environment is detected from the environment variable
    XDG_CURRENT_DESKTOP
  - any of the above pre-conditions are met and NCurses is user-selected, but
    stdout is *not* a TTY

* NCurses:
  - if $DISPLAY is *not* set and stdout is a TTY
  - Qt and Gtk are not available and stdout is a TTY

This can be overridden by either:

* specifing one of the switches on the command-line of the program
  - `--gtk`,
  - `--ncurses`, or
  - `--qt`

* setting the environment variable YUI_PREFERED_BACKEND to one of
  - `gtk`,
  - `ncurses`, or
  - `qt`

If a command-line switch is given to the program, the setting from the environment
variable will be overridden by the UI-plugin chosen with the switch.

If the user-selected UI-plugin is not installed on the system, an installed
UI-plugin will be chosen by the above criteria.


### Building

Libyui uses CMake. Most operations are available from the toplevel
_Makefile.repo_ of each *libyui\** subdirectory:

```
make -f Makefile.repo
cd build
make
```

or

```
make -f Makefile.repo build
```


For reproducible builds it is best to use the
[libyui-rake](https://github.com/libyui/libyui-rake)
Ruby gem like the [Jenkins CI](https://ci.opensuse.org/view/libyui/) jobs do.

It can be installed from [rubygems.org](https://rubygems.org/gems/libyui-rake/)
using this command (Ruby needs to be installed in the system):

```
gem install libyui-rake
```

Then to build the package run:

```
rake osc:build
```


### Versioning

Changing `SONAME_MAJOR` in *VERSION.cmake* currently means that you must also
change `so_version` in *libyui.spec* and also in **all** other *libyui-\*.spec*
files. You can automatically do it by executing `rake so_version:bump`.

This is because the program-libyui API is not distinct
from the libyui-plugin API.


### More info

Please, visit the documentation at [doc folder](https://github.com/libyui/libyui/tree/master/libyui/doc)
for more information about [how to branch](https://github.com/libyui/libyui/tree/master/libyui/doc/branching.md)
libyui and about [auto-tagging](https://github.com/libyui/libyui/tree/master/libyui/doc/auto-tagging.md)
new versions.
