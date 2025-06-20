// reMarkable Printer Application
//
// Copyright Peter K. G. Williams.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//
// PAPPL API docs: https://www.msweet.org/pappl/pappl.html
// CUPS: https://www.cups.org/doc/cupspm.html

#include <stdio.h>

#include <pappl/pappl.h>


typedef struct {
  pappl_system_t *system;
} rmpa_global_data_t;

static pappl_pr_driver_t drivers[] = {
  {
    "reMarkable Cloud Printing", // description
    NULL, // IEEE-1284 device_id
    NULL, // extension data pointer
    "remarkable" // driver name
 }
};

static bool
rmpa_rendjob_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device)
{
  return true;
}

static bool
rmpa_rendpage_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page)
{
  return true;
}

static bool
rmpa_rstartjob_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device)
{
  return true;
}

static bool
rmpa_rstartpage_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page)
{
  return true;
}

static bool
rmpa_rwriteline_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned y, const unsigned char *line)
{
  return true;
}


// see: https://github.com/OpenPrinting/pappl-retrofit/blob/master/pappl-retrofit/pappl-retrofit.c#L1114
// and: https://www.msweet.org/pappl/pappl.html#the-driver-callback
static bool
rmpa_driver_init_cb(
  pappl_system_t *system,
  const char *driver_name,
  const char *device_uri,
  const char *device_id,
  pappl_pr_driver_data_t *driver_data,
  ipp_t **driver_attrs,
  void *data
) {
  // if (driver_data->extension == NULL) {
  //   driver_data->extension = malloc(1);
  // }

  driver_data->printfile_cb = NULL;
  driver_data->rendjob_cb = rmpa_rendjob_cb;
  driver_data->rendpage_cb = rmpa_rendpage_cb;
  driver_data->rstartjob_cb = rmpa_rstartjob_cb;
  driver_data->rstartpage_cb = rmpa_rstartpage_cb;
  driver_data->rwriteline_cb = rmpa_rwriteline_cb;
  driver_data->status_cb = NULL;
  driver_data->format = "application/pdf";
  driver_data->orient_default = IPP_ORIENT_NONE;
  driver_data->quality_default = IPP_QUALITY_NORMAL;
  strncpy(driver_data->make_and_model, "reMarkable Cloud", sizeof(driver_data->make_and_model) - 1);
  driver_data->ppm = 100; // pages per minute
  driver_data->ppm_color = 100;
  driver_data->num_resolution  = 1;
  driver_data->x_resolution[0] = 300;
  driver_data->y_resolution[0] = 300;
  driver_data->x_default = 300;
  driver_data->y_default = 300;
  driver_data->raster_types = PAPPL_PWG_RASTER_TYPE_BLACK_1;
  driver_data->color_supported = PAPPL_COLOR_MODE_AUTO;
  driver_data->color_default = PAPPL_COLOR_MODE_AUTO;
  driver_data->num_media = 1;
  driver_data->media[0] = "na_letter_8.5x11in";
  driver_data->sides_supported = PAPPL_SIDES_ONE_SIDED;
  driver_data->sides_default   = PAPPL_SIDES_ONE_SIDED;
  driver_data->num_source = 1;
  driver_data->source[0]  = "fake-source";
  driver_data->num_type = 1;
  driver_data->type[0] = "fake-type";
  driver_data->media_ready[0].bottom_margin = driver_data->bottom_top;
  driver_data->media_ready[0].left_margin   = driver_data->left_right;
  driver_data->media_ready[0].right_margin  = driver_data->left_right;
  driver_data->media_ready[0].size_width    = 21590;
  driver_data->media_ready[0].size_length   = 27940;
  driver_data->media_ready[0].top_margin    = driver_data->bottom_top;
  strncpy(driver_data->media_ready[0].source, driver_data->source[0], sizeof(driver_data->media_ready[0].source) - 1);
  strncpy(driver_data->media_ready[0].type, driver_data->type[0],  sizeof(driver_data->media_ready[0].type) - 1);
  driver_data->media_default = driver_data->media_ready[0];
  return true;
}

static pappl_system_t *
rmpa_system_cb(
  int num_options,
  cups_option_t *options,
  void *data
) {
  rmpa_global_data_t *gdata = (rmpa_global_data_t *) data;
  pappl_system_t *sys;

  sys = papplSystemCreate(
    PAPPL_SOPTIONS_NONE,
    "reMarkable",
    8000,
    NULL, // subtypes
    NULL, // spooldir
    NULL, // logfile
    PAPPL_LOGLEVEL_UNSPEC,
    NULL, // auth_service
    false // tls_only
  );

  gdata->system = sys;

  papplSystemSetLogLevel(sys, PAPPL_LOGLEVEL_DEBUG);

  papplSystemSetPrinterDrivers(
    sys,
    1,
    drivers,
    NULL, // autoadd_cb
    NULL, // create_cb
    rmpa_driver_init_cb,
    NULL // data
  );

  return sys;
}


// This subcommand gets rmapi logged in so that we can actually print stuff!
static int
rmpa_login_subcmd_cb(
  const char *base_name,
  int num_options,
  cups_option_t *options,
  int num_files,
  char **files,
  void *data
) {
  rmpa_global_data_t *gdata = (rmpa_global_data_t *) data;
  pappl_printer_t *printer;

  if (num_files != 0) {
    fprintf(stderr, "usage error: pass no files to the `login` subcommand\n");
    return 1;
  }

  for (int i = 0; i < num_options; i++) {
    printf("option: name=%s value=%s\n", options[i].name, options[i].value);
  }

  // need to do this ourselves, it seems
  rmpa_system_cb(num_options, options, data);

  // ensure that the unique reMarkable Cloud printer is defined

  printer = papplPrinterCreate(
    gdata->system,
    1,
    "reMarkable Cloud", // printer_name
    "remarkable", // driver_name
    NULL, // device_id
    "remarkable://Printouts" // device_uri
  );

  if (printer == NULL) {
    perror("error adding printer");
    return 1;
  }

  // exec `rmapi account`
  return 0;
}

static rmpa_global_data_t the_global_data = {
  NULL // system
};

int
main(int  argc, char *argv[])
{
    return papplMainloop(
      argc,
      argv,
      "0.1",
      "Copyright Peter K. G. Williams. Provided under the terms of the <a href=\"https://www.apache.org/licenses/LICENSE-2.0\">Apache License 2.0</a>.",
      0, // num_drivers
      NULL, // drivers
      NULL, // autoadd_cb
      NULL, // drivers_cb
      "login",
      rmpa_login_subcmd_cb,
      rmpa_system_cb,
      NULL, // usage_cb
      &the_global_data // data
    );
}
