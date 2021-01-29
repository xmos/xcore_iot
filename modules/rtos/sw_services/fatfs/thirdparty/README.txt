This is ELM by ChaN's FatFs module for small embedded systems.

The latest version and full documentation is available here:
http://elm-chan.org/fsw/ff/00index_e.html

A default configuration file named ffconf.h is provided which
will be used if the application does not include its own ffconf.h.

If the application includes its own ffconf.h then it must specify
all of the config options used by FatFs. The application may
instead include a configuration file named ff_appconf.h. This can
be used to specify only those options that need to be different from
the default values.

Default diskio functions, as well as a get_fattime() function are all
provided. These work assuming the filesystem is located at address
0x100000 in a QSPI flash device. All of these functions are "weak"
to allow the application to provide its own versions.
