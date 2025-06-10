//
// reMarkable Printer Application based on ps-printer-app, PAPPL and libpappl-retrofit.
//
// Copyright 2025 Peter K. G. Williams.
// Copyright © 2020 by Till Kamppeter.
// Copyright © 2020 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.

#include <pappl-retrofit.h>


#define SYSTEM_NAME "reMarkable Printer Application"
#define SYSTEM_PACKAGE_NAME "remarkable-printer-app"
#ifndef SYSTEM_VERSION_STR
#  define SYSTEM_VERSION_STR "1.0"
#endif
#ifndef SYSTEM_VERSION_ARR_0
#  define SYSTEM_VERSION_ARR_0 1
#endif
#ifndef SYSTEM_VERSION_ARR_1
#  define SYSTEM_VERSION_ARR_1 0
#endif
#ifndef SYSTEM_VERSION_ARR_2
#  define SYSTEM_VERSION_ARR_2 0
#endif
#ifndef SYSTEM_VERSION_ARR_3
#  define SYSTEM_VERSION_ARR_3 0
#endif
#define SYSTEM_WEB_IF_FOOTER "Copyright 2025 Peter K. G. Williams. Provided under the terms of the <a href=\"https://www.apache.org/licenses/LICENSE-2.0\">Apache License 2.0</a>."


static const char *
autoadd(
  const char *device_info,
  const char *device_uri,
  const char *device_id,
  void *data
) {
  return NULL;
}


int
main(int  argc, char *argv[])
{
  cups_array_t *spooling_conversions, *stream_formats;
  int ret;

  // Spooling conversions, most desirable first
  spooling_conversions = cupsArrayNew(NULL, NULL);
  cupsArrayAdd(spooling_conversions, (void *) &PR_CONVERT_PDF_TO_PDF);
  cupsArrayAdd(spooling_conversions, (void *) &PR_CONVERT_PS_TO_PDF);

  // Stream formats, most desirable first
  stream_formats = cupsArrayNew(NULL, NULL);
  cupsArrayAdd(stream_formats, (void *) &PR_STREAM_PDF);

  pr_printer_app_config_t printer_app_config = {
    SYSTEM_NAME,
    SYSTEM_PACKAGE_NAME,
    SYSTEM_VERSION_STR,
    {
      SYSTEM_VERSION_ARR_0,
      SYSTEM_VERSION_ARR_1,
      SYSTEM_VERSION_ARR_2,
      SYSTEM_VERSION_ARR_3
    },
    SYSTEM_WEB_IF_FOOTER,
    PR_COPTIONS_QUERY_PS_DEFAULTS |
      PR_COPTIONS_WEB_ADD_PPDS |
      PR_COPTIONS_NO_PAPPL_BACKENDS |
      PR_COPTIONS_CUPS_BACKENDS,
    autoadd,
    prIdentify,
    prTestPage,
    prSetupAddPPDFilesPage,
    prSetupDeviceSettingsPage,
    spooling_conversions,
    stream_formats,
    "",
    "snmp,dnssd,usb",
    "testpage.pdf",
    NULL,
    NULL
  };

  return prRetroFitPrinterApp(&printer_app_config, argc, argv);
}
