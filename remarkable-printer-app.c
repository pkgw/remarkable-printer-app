// reMarkable Printer Application
//
// Copyright the reMarkable Printer Application Contributors
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//
// API docs:
//   - PAPPL: https://www.msweet.org/pappl/pappl.html
//   - CUPS: https://www.cups.org/doc/cupspm.html

#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#include <pappl/pappl.h>

extern char **environ;

const int SET_DEBUG_LOG_LEVEL = 1;
const int UNIQUE_PRINTER_ID = 1;
const int DEFAULT_PORT = 8000;

typedef struct
{
  pappl_system_t *system;
  int port;
} rmpa_global_data_t;

static pappl_pr_driver_t drivers[] = {
    {
        "reMarkable Cloud Printing", // description
        NULL,                        // IEEE-1284 device_id
        NULL,                        // extension data pointer
        "remarkable"                 // driver name
    }};

// Required callbacks for our `remarkable://` device schema - need to define it
// in order to be able to create a printer that doesn't acťually try to interact
// with any hardware

static bool
rmpa_devopen_cb(pappl_device_t *device, const char *device_uri, const char *name)
{
  return true;
}

static void
rmpa_devclose_cb(pappl_device_t *device)
{
  return;
}

static ssize_t
rmpa_devread_cb(pappl_device_t *device, void *buffer, size_t bytes)
{
  return -1;
}

static ssize_t
rmpa_devwrite_cb(pappl_device_t *device, const void *buffer, size_t bytes)
{
  return -1;
}

static pappl_preason_t
rmpa_devstatus_cb(pappl_device_t *device)
{
  return PAPPL_PREASON_NONE;
}

// Raster rendering callbacks - we're required to provide these

static bool
rmpa_rendjob_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device)
{
  pappl_printer_t *printer = papplJobGetPrinter(job);
  pappl_system_t *system = papplPrinterGetSystem(printer);
  papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable: raster callback `rendjob` should never be used");
  return false;
}

static bool
rmpa_rendpage_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page)
{
  pappl_printer_t *printer = papplJobGetPrinter(job);
  pappl_system_t *system = papplPrinterGetSystem(printer);
  papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable: raster callback `rendpage` should never be used");
  return false;
}

static bool
rmpa_rstartjob_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device)
{
  pappl_printer_t *printer = papplJobGetPrinter(job);
  pappl_system_t *system = papplPrinterGetSystem(printer);
  papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable: raster callback `rstartjob` should never be used");
  return false;
}

static bool
rmpa_rstartpage_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page)
{
  pappl_printer_t *printer = papplJobGetPrinter(job);
  pappl_system_t *system = papplPrinterGetSystem(printer);
  papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable: raster callback `rstartpage` should never be used");
  return false;
}

static bool
rmpa_rwriteline_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned y, const unsigned char *line)
{
  pappl_printer_t *printer = papplJobGetPrinter(job);
  pappl_system_t *system = papplPrinterGetSystem(printer);
  papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable: raster callback `rwriteline` should never be used");
  return false;
}

// Our actual file-printing callback!

static bool
rmpa_printfile_cb(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device)
{
  pappl_printer_t *printer = papplJobGetPrinter(job);
  pappl_system_t *system = papplPrinterGetSystem(printer);
  const char *filename = papplJobGetFilename(job); // Absolute path to the spooled PDF
  const char *jobname = papplJobGetName(job);      // This is the original basename of the input file
  const char *uri = papplPrinterGetDeviceURI(printer);
  char *destdir;
  pid_t pid;
  int status, result;
  bool retcode = false;

  papplLog(system, PAPPL_LOGLEVEL_INFO, "reMarkable printfile: filename=%s", filename);

  if (strncmp(uri, "remarkable://", 13))
  {
    papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable printfile: device URI `%s` has wrong prefix", uri);
    return false;
  }

  destdir = strchr(uri + 13, '/');
  if (destdir == NULL)
  {
    destdir = "/";
  }

  char *const argv[] = {
      "rmapi", "put", (char *)filename, destdir, NULL};

  papplLog(
      system,
      PAPPL_LOGLEVEL_INFO,
      "reMarkable printfile: launching: rmapi put %s %s",
      filename,
      destdir);

  // Our handling of the spawned process is mega-overkill, but the posix_spawn()
  // manpage has a nice thorough example.

  result = posix_spawnp(
      &pid,
      "rmapi",
      NULL, // file_actions
      NULL, // attr
      argv,
      environ);

  if (result != 0)
  {
    papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable printfile: spawn failed!");
    return false;
  }

  papplLog(
      system,
      PAPPL_LOGLEVEL_INFO,
      "reMarkable printfile: child PID %d",
      (int)pid);

  do
  {
    result = waitpid(pid, &status, WUNTRACED | WCONTINUED);
    if (result == -1)
    {
      papplLog(system, PAPPL_LOGLEVEL_ERROR, "reMarkable printfile: waitpid failed!");
      return false;
    }

    if (WIFEXITED(status))
    {
      papplLog(
          system,
          PAPPL_LOGLEVEL_INFO,
          "reMarkable printfile: child exited with status %d",
          WEXITSTATUS(status));

      if (WEXITSTATUS(status) == 0)
      {
        retcode = true;
      }
    }
    else if (WIFSIGNALED(status))
    {
      papplLog(
          system,
          PAPPL_LOGLEVEL_INFO,
          "reMarkable printfile: child killed by signal %d",
          WTERMSIG(status));
    }
    else if (WIFSTOPPED(status))
    {
      papplLog(
          system,
          PAPPL_LOGLEVEL_INFO,
          "reMarkable printfile: child stopped by signal %d",
          WSTOPSIG(status));
    }
    else if (WIFCONTINUED(status))
    {
      papplLog(
          system,
          PAPPL_LOGLEVEL_INFO,
          "reMarkable printfile: child continued");
    }
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

  if (retcode)
  {
    papplLog(
        system,
        PAPPL_LOGLEVEL_INFO,
        "reMarkable printfile: child process success");
  }
  else
  {
    papplLog(
        system,
        PAPPL_LOGLEVEL_ERROR,
        "reMarkable printfile: child process failure");
  }

  return retcode;
}

// The driver init callback needs to set up a whole bunch of stuff in order for
// PAPPL to be happy. We fake as much as possible.
//
// see:
// https://github.com/OpenPrinting/pappl-retrofit/blob/master/pappl-retrofit/pappl-retrofit.c#L1114
// and: https://www.msweet.org/pappl/pappl.html#the-driver-callback
static bool
rmpa_driver_init_cb(
    pappl_system_t *system,
    const char *driver_name,
    const char *device_uri,
    const char *device_id,
    pappl_pr_driver_data_t *driver_data,
    ipp_t **driver_attrs,
    void *data)
{
  driver_data->printfile_cb = rmpa_printfile_cb;
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
  driver_data->num_resolution = 1;
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
  driver_data->sides_default = PAPPL_SIDES_ONE_SIDED;
  driver_data->num_source = 1;
  driver_data->source[0] = "fake-source";
  driver_data->num_type = 1;
  driver_data->type[0] = "fake-type";
  driver_data->media_ready[0].bottom_margin = driver_data->bottom_top;
  driver_data->media_ready[0].left_margin = driver_data->left_right;
  driver_data->media_ready[0].right_margin = driver_data->left_right;
  driver_data->media_ready[0].size_width = 21590;
  driver_data->media_ready[0].size_length = 27940;
  driver_data->media_ready[0].top_margin = driver_data->bottom_top;
  strncpy(driver_data->media_ready[0].source, driver_data->source[0], sizeof(driver_data->media_ready[0].source) - 1);
  strncpy(driver_data->media_ready[0].type, driver_data->type[0], sizeof(driver_data->media_ready[0].type) - 1);
  driver_data->media_default = driver_data->media_ready[0];
  return true;
}

// The "system" object creation callback.
static pappl_system_t *
rmpa_system_cb(
    int num_options,
    cups_option_t *options,
    void *data)
{
  rmpa_global_data_t *gdata = (rmpa_global_data_t *)data;
  pappl_system_t *sys;

  // Without MULTI_QUEUE, there's no URL handler for `/` in the webserver!
  sys = papplSystemCreate(
      PAPPL_SOPTIONS_WEB_INTERFACE | PAPPL_SOPTIONS_MULTI_QUEUE,
      "reMarkable",
      gdata->port,
      NULL, // subtypes
      NULL, // spooldir
      NULL, // logfile
      PAPPL_LOGLEVEL_UNSPEC,
      NULL, // auth_service
      false // tls_only
  );

  gdata->system = sys;

  if (SET_DEBUG_LOG_LEVEL)
  {
    papplSystemSetLogLevel(sys, PAPPL_LOGLEVEL_DEBUG);
  }

  papplSystemSetPrinterDrivers(
      sys,
      1,
      drivers,
      NULL, // autoadd_cb
      NULL, // create_cb
      rmpa_driver_init_cb,
      NULL // data
  );

  // Set up to listen on all network interfaces
  papplSystemAddListeners(sys, NULL);

  return sys;
}

// Custom "login" subcommand -- this creates a default reMarkable printer and
// logs in the rmapi program so that we can actually do anything!
static int
rmpa_login_subcmd_cb(
    const char *base_name,
    int num_options,
    cups_option_t *options,
    int num_files,
    char **files,
    void *data)
{
  rmpa_global_data_t *gdata = (rmpa_global_data_t *)data;
  char *snap_common;
  char state_path[512];
  FILE *test_handle;
  pappl_printer_t *printer;

  if (num_files != 0)
  {
    fprintf(stderr, "usage error: pass no files to the `login` subcommand\n");
    return 1;
  }

  snap_common = getenv("SNAP_COMMON");
  if (!snap_common || !*snap_common)
  {
    fprintf(stderr, "environment variable $SNAP_COMMON must be defined\n");
    return 1;
  }

  for (int i = 0; i < num_options; i++)
  {
    printf("option: name=%s value=%s\n", options[i].name, options[i].value);
  }

  // Here we (intend to) construct the state path in exactly the same way as
  // done within PAPPL. We don't have access to the value it determines, so we
  // have to duplicate the logic.
  snprintf(state_path, sizeof(state_path) - 1, "%s/%s.state", snap_common, base_name);
  test_handle = fopen(state_path, "w");

  if (test_handle == NULL)
  {
    fprintf(stderr, "cannot open `%s` for writing: you probably need to run this command as root\n", state_path);
    return 1;
  }

  fclose(test_handle);

  // Needed to avoid an init error with us trying to grab the same port as a
  // running server.
  gdata->port = 0;

  // need to do this ourselves, it seems
  rmpa_system_cb(num_options, options, data);

  // ensure that the unique reMarkable Cloud printer is defined

  printer = papplPrinterCreate(
      gdata->system,
      UNIQUE_PRINTER_ID,
      "reMarkable Connect",            // printer_name
      "remarkable",                    // driver_name
      NULL,                            // device_id
      "remarkable://default/Printouts" // device_uri
  );

  if (printer == NULL)
  {
    perror("error adding printer");
    return 1;
  }

  papplSystemSetDefaultPrinterID(gdata->system, UNIQUE_PRINTER_ID);

  papplSystemSaveState(gdata->system, state_path);

  printf(
      "Now logging into reMarkable Connect account.\n"
      "After logging in, restart the `snap` printer app service:\n\n"
      "   sudo snap restart %s\n\n",
      base_name);

  execlp("rmapi", "rmapi", "account", NULL);

  // If we get here, something bad happened!
  perror("failed to execute `rmapi account`");
  return 1;
}

static rmpa_global_data_t the_global_data = {
    NULL,        // system
    DEFAULT_PORT // port
};

int main(int argc, char *argv[])
{
  papplDeviceAddScheme(
      "remarkable",
      PAPPL_DEVTYPE_CUSTOM_LOCAL,
      NULL, // list_cb
      rmpa_devopen_cb,
      rmpa_devclose_cb,
      rmpa_devread_cb,
      rmpa_devwrite_cb,
      rmpa_devstatus_cb,
      NULL // id_cb
  );

  return papplMainloop(
      argc,
      argv,
      "0.1",
      "Copyright the reMarkable Printer Application Contributors. Provided under the terms of the <a href=\"https://www.apache.org/licenses/LICENSE-2.0\">Apache License 2.0</a>.",
      0,    // num_drivers
      NULL, // drivers
      NULL, // autoadd_cb
      NULL, // drivers_cb
      "login",
      rmpa_login_subcmd_cb,
      rmpa_system_cb,
      NULL,            // usage_cb
      &the_global_data // data
  );
}
