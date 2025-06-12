//
// reMarkable Printer Application
//
// Copyright 2025 Peter K. G. Williams.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.

#include <pappl/pappl.h>


static pappl_system_t *
rmpa_system_cb(
  int num_options,
  cups_option_t *options,
  void *data
) {
  pappl_system_t *sys;

  sys = papplSystemCreate(
    PAPPL_SOPTIONS_NONE,
    "reMarkable",
    8000,
    NULL,
    NULL,
    NULL,
    PAPPL_LOGLEVEL_UNSPEC,
    NULL,
    false
  );

  return sys;
}

int
main(int  argc, char *argv[])
{
    return papplMainloop(
      argc,
      argv,
      "0.1",
      "Copyright 2025 Peter K. G. Williams. Provided under the terms of the <a href=\"https://www.apache.org/licenses/LICENSE-2.0\">Apache License 2.0</a>.",
      0, // num_drivers
      NULL, // drivers
      NULL, // autoadd_cb
      NULL, // drivers_cb
      NULL, // subcmd_name
      NULL, // subcmd_cb
      rmpa_system_cb,
      NULL, // usage_cb
      NULL // data
    );
}
