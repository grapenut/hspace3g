/*
 * spacecnf.c
 *
 * Contains routines specific to loading the HSpace configuration file.
 */
#include "hspace.h"

/*
 * Structure containing options that are configurable
 */
hconfig hs_options;

hconfigitem hs_conflist[] =
{
  { (char *) "wormhole_travel", HS_STRING, (void *) hs_options.wormhole_travel },
  { (char *) "landing_msg", HS_STRING, (void *) hs_options.landing_msg },
  { (char *) "begin_descent", HS_STRING, (void *) hs_options.begin_descent },
  { (char *) "lift_off", HS_STRING, (void *) hs_options.lift_off },
  { (char *) "engines_activating", HS_STRING, 
		(void *) hs_options.engines_activating },
  { (char *) "docking_msg", HS_STRING, (void *) hs_options.docking_msg },
  { (char *) "begin_docking", HS_STRING, (void *) hs_options.begin_docking },
  { (char *) "speed_increase", HS_STRING, (void *) hs_options.speed_increase },
  { (char *) "speed_decrease", HS_STRING, (void *) hs_options.speed_decrease },
  { (char *) "engine_reverse", HS_STRING, (void *) hs_options.engine_reverse },
  { (char *) "engine_forward", HS_STRING, (void *) hs_options.engine_forward },
  { (char *) "speed_halt", HS_STRING, (void *) hs_options.speed_halt },
  { (char *) "engage_afterburner", HS_STRING, (void *) hs_options.engage_afterburner },
  { (char *) "disengage_afterburner", HS_STRING, (void *) hs_options.disengage_afterburner },
  { (char *) "notify_engage", HS_STRING, (void *) hs_options.notify_engage },
  { (char *) "notify_disengage", HS_STRING, (void *) hs_options.notify_disengage },
  { (char *) "ship_is_docking", HS_STRING, (void *) hs_options.ship_is_docking },
  { (char *) "ship_is_undocking", HS_STRING, 
		(void *) hs_options.ship_is_undocking },
  { (char *) "ship_is_docked", HS_STRING, (void *) hs_options.ship_is_docked },
  { (char *) "max_missions", HS_INTEGER, (void *) &hs_options.max_missions },
  { (char *) "max_ships", HS_INTEGER, (void *) &hs_options.max_ships },
  { (char *) "max_celestials", HS_INTEGER, (void *) &hs_options.max_celestials },
  { (char *) "max_universes", HS_INTEGER, (void *) &hs_options.max_universes },
  
  { (char *) "max_comm_dist", HS_INTEGER, (void *) &hs_options.max_comm_dist },
  { (char *) "max_waypoint_dist", HS_INTEGER, (void *) &hs_options.max_waypoint_dist },
  { (char *) "max_gate_dist", HS_INTEGER, (void *) &hs_options.max_gate_dist },
  { (char *) "max_land_dist", HS_INTEGER, (void *) &hs_options.max_land_dist },
  { (char *) "max_cargo_dist", HS_INTEGER, (void *) &hs_options.max_cargo_dist },
  { (char *) "max_board_dist", HS_INTEGER, (void *) &hs_options.max_board_dist },
  { (char *) "max_dock_dist", HS_INTEGER, (void *) &hs_options.max_dock_dist },
  { (char *) "max_dock_size", HS_INTEGER, (void *) &hs_options.max_dock_size },

  { (char *) "autostart", HS_INTEGER, (void *) &hs_options.autostart },
  { (char *) "space_wiz", HS_INTEGER, (void *) &hs_options.space_wiz },
  { (char *) "forbid_puppets", HS_INTEGER, (void *) &hs_options.forbid_puppets },
  { (char *) "cycle_interval", HS_INTEGER, (void *) &hs_options.cyc_interval },
  { (char *) "idle_timeout", HS_INTEGER, (void *) &hs_options.idle_timeout },
  { (char *) "burn_multiplier", HS_DOUBLE, (void *) &hs_options.burn_multiplier },
  { NULL, 0, NULL }
};



/***********************************************************/
/* old style hspace config from flat text file */

void hs_print_config(dbref player, const char *which)
{
  hconfigitem *conf;
  int i;
  
  if (!which || !*which)
  {
    notify(player, "HSPACE: Listing configuration options...");
    i = 0;
    conf = &hs_conflist[i];
    do {
      if (conf && conf->name && conf->value)
      {
        if (conf->type == HS_INTEGER)
          notify_format(player, "%-30s %-15s %d", conf->name, "INTEGER", *(int*)(conf->value));
        else if (conf->type == HS_DOUBLE)
          notify_format(player, "%-30s %-15s %g", conf->name, "DOUBLE", *(double*)(conf->value));
        else if (conf->type == HS_STRING)
          notify_format(player, "%-30s %-15s %s", conf->name, "STRING", (char*) conf->value);
      }
        
      i++;
      conf = &hs_conflist[i];
    } while (conf && conf->name);
  } else {
    i = 0;
    conf = &hs_conflist[i];
    do {
      if (conf && conf->name && conf->value)
      {
        if (!strcasecmp(conf->name, which))
        {
          if (conf->type == HS_INTEGER)
            notify_format(player, "%-30s %-15s %d", conf->name, "INTEGER", *(int*)(conf->value));
          else if (conf->type == HS_DOUBLE)
            notify_format(player, "%-30s %-15s %g", conf->name, "DOUBLE", *(double*)(conf->value));
          else if (conf->type == HS_STRING)
            notify_format(player, "%-30s %-15s %s", conf->name, "STRING", (char*) conf->value);

          return;
        }
      }
        
      i++;
      conf = &hs_conflist[i];
    } while (conf && conf->name);
    notify(player, "HSPACE: Invalid configuration option.");
  }
}

/* 
 * Takes an option string loaded from the config file and
 * figures out where to put it in hs_options.
 */
int hspace_parse_option(char *option, char *value)
{
  hconfigitem  *cfptr;

  for (cfptr = hs_conflist; cfptr->name; cfptr++)
  {
    if (!strcasecmp(option, cfptr->name))
    {
      if (cfptr->type == HS_INTEGER)
      {
        *(int*)(cfptr->value) = strtol(value, NULL, 10);
      }
      else if (cfptr->type == HS_DOUBLE)
      {
        *(double*)(cfptr->value) = strtod(value, NULL);
      }
      else if (cfptr->type == HS_STRING)
      {
        strcpy((char *) cfptr->value, value);
      }
      else
        return 0;
      
      return 1;
    }
  }
  return 0;
}

/* 
 * Opens the HSpace configuration file, loading in option strings and
 * sending them to hspace_input_option().
 */
int hs_load_config(dbref player)
{
  FILE   *fp;
  char    tbuf[256];
  char    tbuf2[256];
  char    option[256];
  char    value[256];
  char   *ptr, *ptr2;

  if (player != NOTHING)
    notify(player,
           tprintf("%sSPACE%s: Loading configuration file.",
                   ANSI_HILITE, ANSI_NORMAL));

  if (player == NOTHING)
    hs_log((char *) "LOADING: HSpace configuration file.");
  else
  {
    sprintf(tbuf, "LOADING: Hspace configuration file by %s.",
            Name(player));
    hs_log(tbuf);
  }
  /*
   * Open the configuration file for reading
   */
  fp = fopen(HSPACE_CONFIG_FILE, "r");
  if (!fp)
  {
    if (player != NOTHING)
      notify(player,
             tprintf("%sERROR%s: Unable to open hspace configuration file.",
                     ANSI_HILITE, ANSI_NORMAL));
    hs_log((char *) "ERROR: Unable to open hspace configuration file.");
    return 0;
  }

  /*
   * Read the entire file in.  Parse lines that have something in them
   * and don't begin with a '#'
   */
  while (fgets(tbuf, 256, fp))
  {
    /*
     * Truncate at the newline
     */
    tbuf[strlen(tbuf) - 1] = '\0';

    /*
     * Strip leading spaces
     */
    ptr = tbuf;
    while (*ptr == ' ')
      ptr++;

    /*
     * Determine if the line is valid
     */
    if (!*ptr || *ptr == '#')
      continue;

    /*
     * Parse out the option and value
     */
    ptr2 = option;
    for (; *ptr && *ptr != ' ' && *ptr != '='; ptr++)
    {
      *ptr2 = *ptr;
      ptr2++;
    }
    *ptr2 = '\0';
    if (!*ptr)
    {
      if (player != NOTHING)
        notify(player, tprintf("%sERROR%s: Invalid configuration at option: %s",
                               ANSI_HILITE, ANSI_NORMAL, option));

      sprintf(tbuf2, "ERROR: Invalid configuration at option: %s",
              option);
      hs_log(tbuf2);
      continue;
    }
    ptr2 = value;
    while(*ptr && (*ptr == ' ' || *ptr == '='))
      ptr++;
    for (; *ptr; ptr++)
    {
      *ptr2 = *ptr;
      ptr2++;
    }
    *ptr2 = '\0';
    if (!hspace_parse_option(option, value))
    {
      sprintf(tbuf2, "ERROR: Invalid config option \"%s\"", option);
      hs_log(tbuf2);
      if (player != NOTHING)
        notify(player, tprintf("%sERROR%s: Invalid config option \"%s\"",
                               ANSI_HILITE, ANSI_NORMAL, option));
    }
  }
  fclose(fp);

  if (player != NOTHING)
    notify(player,
           tprintf("%sSPACE%s: Configuration file loaded.", 
            ANSI_HILITE, ANSI_NORMAL));
  return 1;
}

