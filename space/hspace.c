#include "hspace.h"


  extern DESC *descriptor_list;

long hs_cycle_time;
long hs_cycle_max;

char cycling;

/* bit flag counter */
int FlagCount(int v)
{
  v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
  v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
  return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
}

void execute_trigger(dbref obj, char *which, hship *enactor)
{
  int i;
  PE_REGS *pe_regs;
  
  if (!RealGoodObject(obj))
    return;
  
  if (!which)
    return;
    
  if (!enactor)
    return;
  
  pe_regs = pe_regs_create(PE_REGS_ARG | PE_REGS_Q, "do_trigger");
  pe_regs_setenv(pe_regs, 0, unparse_dbref(enactor->objnum));
  
  queue_attribute_base(obj, which, hs_options.space_wiz, 0, pe_regs, 0);
  pe_regs_free(pe_regs);
}

/* execute a ufun */
void execute_ufun(dbref obj, char *which, hship *enactor)
{
  char rbuff[BUFFER_LEN];
  ufun_attrib ufun;
  int flags = UFUN_OBJECT;
  PE_REGS *pe_regs;
  int i;

  if (!RealGoodObject(obj))
    return;
  
  if (!which)
    return;

  if (!enactor)
    return;

  if (!fetch_ufun_attrib(which, obj, &ufun, flags))
    return;

  pe_regs = pe_regs_create(PE_REGS_ARG, "fun_ufun");
  pe_regs_setenv(pe_regs, 0, unparse_dbref(enactor->objnum));

  call_ufun(&ufun, rbuff, obj, obj, NULL, pe_regs);
  pe_regs_free(pe_regs);
}


/**************************************************/
/* prompt routines */

char *parse_prompt(hship *ship, hconsole *con, char *prompt)
{
  static char buff[BUFFER_LEN];
  char *bp;
  char *c;
  hbuff *stance;

  if (!ship)
    return NULL;

  bp = buff;
  for (c = prompt; c && *c; c++)
  {
    if (*c == '$')
    {
      c++;
      if (!*c)
        break;

      /* apply substitutions */

      /* ship, waypoint and target information
         $N - ship name
         $H - ship hull%
         $S - ship shield%
         $D - waypoint distance
         
         $n - target name
         $h - target hull%
         $s - target shield%
         $d - target distance
         $c - target cnum */
      
      /* stances
         $T - emitter stance
         $I - missile stance
         $w - gun stance
         $C - cannon stance
         $W - wiretap stance
         $B - booster stance
         $i - capacitor stance
      */

      /* system resource values, capital is max resource, lower is current
         $E/$e - engine
         $A/$a - reactor
         $U/$u - computer
         $Q/$q - sensor
      */

      /* weapon subs, capital is primary, lower is secondary
         $R/$r - reload timeleft
         $G/$g - range
         $P/$p - cur power
         $M/$m - max power
      */
      
      switch (*c)
      {
      /* special cases */
      case '\0':
        break;
      case '$':
        safe_chr('$', buff, &bp);
        break;
      
      /* ship subs */
      case 'H':
        safe_format(buff, &bp, "%3.0f%%%%", ship->hull.energy / get_stat(ship, SYS_MAX_ARMOR) * 100.0);
        break;
      case 'S':
        safe_format(buff, &bp, "%3.0f%%%%", ship->shield.energy / get_stat(ship, SYS_MAX_CAPACITY) * 100.0);
        break;
      case 'N':
        safe_format(buff, &bp, "%s", ship_name(ship));
        break;
      case 'T':
        stance = find_buff(ship, &STANCE_standard_beam);
        if (!stance || !stance->buff) break;
        
        safe_format(buff, &bp, "%s", STR(hs_stances, stance->buff));
        break;
      case 'D':
        if (!ship->wp_contact) break;

        safe_format(buff, &bp, "%.0f", ContactDistance(ship, ship->wp_contact));
        break;
      case 'E':
        safe_format(buff, &bp, "%.0f", get_stat(ship, SYS_MAX_HEAT));
        break;
      case 'e':
        safe_format(buff, &bp, "%.0f", ship->engine.energy);
        break;
      case 'A':
        safe_format(buff, &bp, "%.0f", get_stat(ship, SYS_MAX_ENERGY));
        break;
      case 'a':
        safe_format(buff, &bp, "%.0f", ship->reactor.energy);
        break;
      case 'U':
        safe_format(buff, &bp, "%.0f", get_stat(ship, SYS_MAX_MEMORY));
        break;
      case 'u':
        safe_format(buff, &bp, "%.0f", ship->computer.energy);
        break;
      case 'Q':
        safe_format(buff, &bp, "%.0f", get_stat(ship, SYS_MAX_FOCUS));
        break;
      case 'q':
        safe_format(buff, &bp, "%.0f", ship->sensor.energy);
        break;
      case 'R':
        safe_format(buff, &bp, "%2d", ship->primary.loading);
        break;
      case 'r':
        safe_format(buff, &bp, "%2d", ship->secondary.loading);
        break;
      case 'G':
        safe_format(buff, &bp, "%3.0f", get_wstat(ship, con, HS_PRIMARY | HS_RANGE));
        break;
      case 'g':
        safe_format(buff, &bp, "%3.0f", get_wstat(ship, con, HS_SECONDARY | HS_RANGE));
        break;
      case 'P':
        safe_format(buff, &bp, "%2.0f", get_wstat(ship, con, HS_PRIMARY | HS_CUR_POWER));
        break;
      case 'p':
        safe_format(buff, &bp, "%2.0f", get_wstat(ship, con, HS_SECONDARY | HS_CUR_POWER));
        break;
      case 'M':
        safe_format(buff, &bp, "%2.0f", get_wstat(ship, con, HS_PRIMARY | HS_MAX_POWER));
        break;
      case 'm':
        safe_format(buff, &bp, "%2.0f", get_wstat(ship, con, HS_SECONDARY | HS_MAX_POWER));
        break;
      
      /* target subs */
      case 'h':
        if (!ship->lock || !ship->lock->contact) break;
        
        safe_format(buff, &bp, "%3.0f%%%%", ContactShip(ship->lock)->hull.energy / get_stat(ContactShip(ship->lock), SYS_MAX_ARMOR) * 100.0);
        break;
      case 's':
        if (!ship->lock || !ship->lock->contact) break;
        
        safe_format(buff, &bp, "%3.0f%%%%", ContactShip(ship->lock)->shield.energy / get_stat(ContactShip(ship->lock), SYS_MAX_CAPACITY) * 100.0);
        break;
      case 'n':
        if (!ship->lock || !ship->lock->contact) break;
        
        safe_format(buff, &bp, "%s", ship_name(ContactShip(ship->lock)));
        break;
      case 'd':
        if (!ship->lock || !ship->lock->contact) break;

        safe_format(buff, &bp, "%.0f", ship_distance(ship, ContactShip(ship->lock)));
        break;
      case 'c':
        if (!ship->lock) break;
        
        safe_format(buff, &bp, "%4d", ship->lock->cnum);
        break;
      default:
        c--;
        safe_chr(*c, buff, &bp);
        break;
      }
    } else {
      safe_chr(*c, buff, &bp);
    }
  }
  *bp = '\0';
  
  return buff;
}

void hs_prompt(dbref console)
{
  DESC *d, *match;
  char buff[BUFFER_LEN];
  char *bp;
  hship *ship;
  hconsole *con;
  hcontact *lock;
  hweapon *pri, *sec;
//  ATTR *a;
  dbref player;
//  char *prompt;
//  const char *sp;
  double t;
  int type;
  char prompt = 0x0;
  
  if (IsShip(console))
  {
    ship = find_ship_by_nav(console);
    con = NULL;
    
    if (!ship)
      return;
      
    lock = ship->lock;
    pri = &ship->primary;
    sec = &ship->secondary;
    type = HS_NAV;
    prompt = ship->prompt;
  }
  else if (IsConsole(console))
  {
    ship = find_ship(console);
    if (!ship)
      return;
    
    con = find_console(console);
    if (!con)
      return;
    
    lock = con->lock;
    pri = &con->primary;
    sec = &con->secondary;
    type = con->type;
    prompt = con->prompt;
  } else {
    return;
  }

  if (!HasFlag(prompt, HS_PROMPT_ALWAYS | HS_PROMPT_COMBAT | HS_PROMPT_SPACE))
    return;
  
  /* only send prompts to players */
  player = get_user(console);
  if (!IsPlayer(player))
    return;
  
  /* that are connected */
  match = NULL;
  for(d = descriptor_list; d; d = d->next)
  {
    if (d->connected && (d->player == player) &&
      (!match || (d->last_time > match->last_time)))
    match = d;
  }

  if (!match)
    return;
    
  /* and not idle by more than the idle_timeout */
  if (difftime(mudtime, match->last_time) > hs_options.idle_timeout)
    return;
  
/*
  a = atr_get(player, "HSPROMPT");
  if (!a)
    return;
  
  prompt = safe_atr_value(a);
  sp = parse_prompt(ship, con, prompt);
  free(prompt);
  
  bp = buff;
  process_expression(buff, &bp, &sp, player, player, player, PE_DEFAULT, PT_DEFAULT, NULL);
  *bp = '\0';
*/  

  bp = buff;
  
  safe_format(buff, &bp, "%s%s<%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);

  /* health */
  t = ship->hull.energy / get_stat(ship, SYS_MAX_ARMOR) * 100.0;
  if (t > 70.0)
    safe_format(buff, &bp, "%s%s%3.0f%%%s ", ANSI_HILITE, ANSI_GREEN, t, ANSI_NORMAL);
  else if (t > 30.0)
    safe_format(buff, &bp, "%s%s%3.0f%%%s ", ANSI_HILITE, ANSI_YELLOW, t, ANSI_NORMAL);
  else
    safe_format(buff, &bp, "%s%s%3.0f%%%s ", ANSI_HILITE, ANSI_RED, t, ANSI_NORMAL);

  t = ship->shield.energy / get_stat(ship, SYS_MAX_CAPACITY) * 100.0;
  if (t > 70.0)
    safe_format(buff, &bp, "%s%s%3.0f%%%s", ANSI_HILITE, ANSI_GREEN, t, ANSI_NORMAL);
  else if (t > 30.0)
    safe_format(buff, &bp, "%s%s%3.0f%%%s", ANSI_HILITE, ANSI_YELLOW, t, ANSI_NORMAL);
  else
    safe_format(buff, &bp, "%s%s%3.0f%%%s", ANSI_HILITE, ANSI_RED, t, ANSI_NORMAL);

  if (HasFlag(type, HS_ENG))
  {
    safe_format(buff, &bp, "%s%s|%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);

    if (RealGoodObject(pri->objnum))
    {
      if (HasFlag(pri->type, HS_CAPACITOR))
      {
        safe_format(buff, &bp, "%2.0f/%2.0f%s%s|%s", pri->curpower, get_wstat(ship, con, HS_PRIMARY | HS_MAX_POWER),
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      }
    }

    if (RealGoodObject(sec->objnum))
    {
      if (HasFlag(sec->type, HS_CAPACITOR))
      {
        safe_format(buff, &bp, "%2.0f/%2.0f%s%s|%s", sec->curpower, get_wstat(ship, con, HS_PRIMARY | HS_MAX_POWER),
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      }
    }

    t = ship->reactor.energy / get_stat(ship, SYS_MAX_ENERGY) * 100.0;
    if (t > 70.0)
      safe_format(buff, &bp, "R%s%s%3.0f%%%s|%s", ANSI_HILITE, ANSI_GREEN, t, ANSI_BLUE, ANSI_NORMAL);
    else if (t > 30.0)
      safe_format(buff, &bp, "R%s%s%3.0f%%%s|%s", ANSI_HILITE, ANSI_YELLOW, t, ANSI_BLUE, ANSI_NORMAL);
    else
      safe_format(buff, &bp, "R%s%s%3.0f%%%s|%s", ANSI_HILITE, ANSI_RED, t, ANSI_BLUE, ANSI_NORMAL);

    t = ship->engine.energy / get_stat(ship, SYS_MAX_HEAT) * 100.0;
    if (t < 30.0)
      safe_format(buff, &bp, "E%s%s%3.0f%%%s|%s", ANSI_HILITE, ANSI_GREEN, t, ANSI_BLUE, ANSI_NORMAL);
    else if (t < 70.0)
      safe_format(buff, &bp, "E%s%s%3.0f%%%s|%s", ANSI_HILITE, ANSI_YELLOW, t, ANSI_BLUE, ANSI_NORMAL);
    else
      safe_format(buff, &bp, "E%s%s%3.0f%%%s|%s", ANSI_HILITE, ANSI_RED, t, ANSI_BLUE, ANSI_NORMAL);

    t = ship->sensor.energy / get_stat(ship, SYS_MAX_FOCUS) * 100.0;
    if (t > 70.0)
      safe_format(buff, &bp, "S%s%s%3.0f%%%s", ANSI_HILITE, ANSI_GREEN, t, ANSI_NORMAL);
    else if (t > 30.0)
      safe_format(buff, &bp, "S%s%s%3.0f%%%s", ANSI_HILITE, ANSI_YELLOW, t, ANSI_NORMAL);
    else
      safe_format(buff, &bp, "S%s%s%3.0f%%%s", ANSI_HILITE, ANSI_RED, t, ANSI_NORMAL);

/*    t = ship->computer.energy / get_stat(ship, SYS_MAX_MEMORY) * 100.0;
    if (t < 30.0)
      safe_format(buff, &bp, "C%s%s%3.0f%%%s", ANSI_HILITE, ANSI_GREEN, t, ANSI_NORMAL);
    else if (t < 70.0)
      safe_format(buff, &bp, "C%s%s%3.0f%%%s", ANSI_HILITE, ANSI_YELLOW, t, ANSI_NORMAL);
    else
      safe_format(buff, &bp, "C%s%s%3.0f%%%s", ANSI_HILITE, ANSI_RED, t, ANSI_NORMAL);
*/
  }
  else
  {
    if (lock && lock->contact && pri && sec)
    {
      safe_format(buff, &bp, "%s%s|%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      
      /* weapon information */
      /* primary */
      if (RealGoodObject(pri->objnum))
      {
        if (HasFlag(pri->type, HS_EMITTER | HS_CANNON | HS_CAPACITOR))
        {
          safe_format(buff, &bp, "%2.0f/%2.0f", pri->curpower, get_wstat(ship, con, HS_PRIMARY | HS_MAX_POWER));
        } else {
          safe_format(buff, &bp, "%2ds", pri->loading);
        }
        
        if (RealGoodObject(sec->objnum))
          safe_chr(' ', buff, &bp);
      }

      /* secondary */
      if (RealGoodObject(sec->objnum))
      {
        if (HasFlag(sec->type, HS_EMITTER | HS_CANNON | HS_CAPACITOR))
        {
          safe_format(buff, &bp, "%2.0f/%2.0f", sec->curpower, get_wstat(ship, con, HS_SECONDARY | HS_MAX_POWER));
        } else {
          safe_format(buff, &bp, "%2ds", sec->loading);
        }
      }
      
      safe_format(buff, &bp, "%s%s|%s%3.0f ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ContactDistance(ship, lock));

      /* shield health */
      t = ContactShip(lock)->hull.energy / get_stat(ContactShip(lock), SYS_MAX_ARMOR) * 100.0;
      if (t > 70.0)
        safe_format(buff, &bp, "%s%s%3.0f%%%s ", ANSI_HILITE, ANSI_GREEN, t, ANSI_NORMAL);
      else if (t > 30.0)
        safe_format(buff, &bp, "%s%s%3.0f%%%s ", ANSI_HILITE, ANSI_YELLOW, t, ANSI_NORMAL);
      else
        safe_format(buff, &bp, "%s%s%3.0f%%%s ", ANSI_HILITE, ANSI_RED, t, ANSI_NORMAL);

      t = ContactShip(lock)->shield.energy / get_stat(ContactShip(lock), SYS_MAX_CAPACITY) * 100.0;
      if (t > 70.0)
        safe_format(buff, &bp, "%s%s%3.0f%%%s", ANSI_HILITE, ANSI_GREEN, t, ANSI_NORMAL);
      else if (t > 30.0)
        safe_format(buff, &bp, "%s%s%3.0f%%%s", ANSI_HILITE, ANSI_YELLOW, t, ANSI_NORMAL);
      else
        safe_format(buff, &bp, "%s%s%3.0f%%%s", ANSI_HILITE, ANSI_RED, t, ANSI_NORMAL);
      
    }
    else if (ship->wp_contact && ship->wp_contact->contact)
    {
      safe_format(buff, &bp, "%s%s|%s@%s %.0f", ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, ContactDistance(ship, ship->wp_contact), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
  }
  
  safe_format(buff, &bp, "%s%s>%s ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  *bp = '\0';

  notify_flags(player, buff, NA_PROMPT);

  return;
}

/************************************************************/
/* logging and player notification routines */

FILE *spacelog_fp;

void hs_log(char *str)
{
  time_t  tt;
  struct tm *ttm;
  char    timebuf[256];

  if (!spacelog_fp) return;

  tt = time(NULL);
  ttm = localtime(&tt);

  sprintf(timebuf, "%d%d/%d%d %d%d:%d%d:%d%d",
	  (((ttm->tm_mon) + 1) / 10), (((ttm->tm_mon) + 1) % 10),
	  (ttm->tm_mday / 10), (ttm->tm_mday % 10),
	  (ttm->tm_hour / 10), (ttm->tm_hour % 10),
	  (ttm->tm_min / 10), (ttm->tm_min % 10),
	  (ttm->tm_sec / 10), (ttm->tm_sec % 10));

  fprintf(spacelog_fp, "%s: %s\n", timebuf, str);
  fflush(spacelog_fp);
}

/* format a standard notice */
void hs_std_error(dbref console, char *msg)
{
  notify_format(get_user(console), "%s%s*%s %s%s%s %s%s*%s", ANSI_HILITE, ANSI_RED, ANSI_NORMAL,
	ANSI_HILITE, msg, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
}

/* format a standard warning */
/* level controls color */
/* 0 - blue, 1 - green, 2 - yellow, 3 - red */
void hs_std_warning(dbref console, char *msg, int level)
{
  switch (level)
  {
  case 0:
    notify_format(get_user(console), "%s(%s!%s%s) %s***%s %sWARNING: %s %s***%s %s(%s!%s%s)%s",
		ANSI_BLUE, ANSI_HILITE, ANSI_NORMAL, ANSI_BLUE, ANSI_HILITE,
	   ANSI_NORMAL, ANSI_HILITE, msg, ANSI_BLUE, ANSI_NORMAL, ANSI_BLUE,
		   ANSI_HILITE, ANSI_NORMAL, ANSI_BLUE, ANSI_NORMAL);
    break;
  case 1:
    notify_format(get_user(console), "%s(%s!%s%s) %s***%s %sWARNING: %s %s***%s %s(%s!%s%s)%s",
	      ANSI_GREEN, ANSI_HILITE, ANSI_NORMAL, ANSI_GREEN, ANSI_HILITE,
	 ANSI_NORMAL, ANSI_HILITE, msg, ANSI_GREEN, ANSI_NORMAL, ANSI_GREEN,
		   ANSI_HILITE, ANSI_NORMAL, ANSI_GREEN, ANSI_NORMAL);
    break;
  case 2:
    notify_format(get_user(console), "%s(%s!%s%s) %s***%s %sWARNING: %s %s***%s %s(%s!%s%s)%s",
	    ANSI_YELLOW, ANSI_HILITE, ANSI_NORMAL, ANSI_YELLOW, ANSI_HILITE,
       ANSI_NORMAL, ANSI_HILITE, msg, ANSI_YELLOW, ANSI_NORMAL, ANSI_YELLOW,
		   ANSI_HILITE, ANSI_NORMAL, ANSI_YELLOW, ANSI_NORMAL);
    break;
  default:
    notify_format(get_user(console), "%s(%s%s!%s%s) %s***%s %sALERT: %s %s***%s %s(%s%s!%s%s)%s",
      ANSI_RED, ANSI_HILITE, ANSI_BLINK, ANSI_NORMAL, ANSI_RED, ANSI_HILITE,
	     ANSI_NORMAL, ANSI_HILITE, msg, ANSI_RED, ANSI_NORMAL, ANSI_RED,
	      ANSI_HILITE, ANSI_BLINK, ANSI_NORMAL, ANSI_RED, ANSI_NORMAL);
    break;
  }
}

/* format a standard message */
void hs_std_notice(dbref console, char *msg)
{
  notify_format(get_user(console), "%s%s-%s %s", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, msg);
}

/* send a message to all ship rooms */
void notify_srooms(hship *tship, dbref except, char *mesg)
{
  ATTR *a;
  char *s;
  char *buff;
  char *r;
  dbref obj;
  
  if (!tship || !mesg)
    return;
  
  if (!RealGoodObject(tship->objnum))
    return;
  
  if ((a = atr_get(tship->objnum, "ROOMS")) == NULL)
    return;
  buff = safe_atr_value(a);
  s = buff;
  
  r = split_token(&s, ' ');
  obj = parse_dbref(r);
  if (RealGoodObject(obj))
    notify_except(tship->objnum, obj, except, mesg, 0);
  
  while (s)
  {
    r = split_token(&s, ' ');
    obj = parse_dbref(r);
    if (RealGoodObject(obj))
      notify_except(tship->objnum, obj, except, mesg, 0);
  }
  
  free(buff);

}

/* send a message to the console user */
void notify_console(dbref console, char *mesg)
{
  dbref   user;

  user = get_user(console);
  if (user != NOTHING)
    notify(user, mesg);

  return;
}

/* send a message to all ship consoles */
void notify_consoles(hship *ship, char *mesg)
{
  int i;
 
  if (!ship)
    return;

  notify_console(ship->objnum, mesg);
  for (i = 0; i < ship->nconsoles; i++)
    notify_console(ship->console[i].objnum, mesg);
  
  return;
}

/* format a standard sensor message */
void hs_std_sensor(hship *ship, hcontact *q, char *mesg)
{
  char tbuf[BUFFER_LEN];
  char *cbuf;
  int i;
  
  if (!ship)
    return;

  if (!q)
  {
    sprintf(tbuf, "%s%s[%s----%s%s]%s - %s", ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, mesg);
  } else {
    cbuf = contact_colorstring(ship, q);
    sprintf(tbuf, "%s[%s%4d%s]%s - %s", cbuf, ANSI_NORMAL, q->cnum, cbuf, ANSI_NORMAL, mesg);
  }

  notify_console(ship->objnum, tbuf);
  for (i = 0; i < ship->nconsoles; i++)
    notify_console(ship->console[i].objnum, tbuf);
}

/* send a message to any ships which have you as a contact */
void notify_contacts(hship *ship, hship *except, char *mesg)
{
  hship *sptr;
  hcontact *q;
  
  if (!ship || !ship->uid || !ship->head_contact || !ship->head_contact->contact)
    return;
  
  for (sptr = ship->uid->head_ship; sptr; sptr = sptr->next)
  {
    if (sptr == except) continue;
    
    q = find_ship_contact(sptr, ship);
    if (q && q->contact)
    {
      hs_std_sensor(sptr, q, mesg);
    }
  }

  for (sptr = ship->uid->head_drone; sptr; sptr = sptr->next)
  {
    if (sptr == except) continue;
    
    q = find_ship_contact(sptr, ship);
    if (q && q->contact)
    {
      hs_std_sensor(sptr, q, mesg);
    }
  }
}


/**************************************************/
/* console management utilities */

/* internal function used to set a console's user */
/* also sets the user to be manning that console */
void set_user(dbref console, dbref user)
{
  boolexp key;
  char tmp[32];
  dbref manned, manning;
  
  if (!IsConsole(console) && !IsShip(console)) return;
  
  if (RealGoodObject(user))
  {
    strcpy(tmp, unparse_dbref(console));
    atr_add(user, "MANNING", tmp, hs_options.space_wiz, 0);
    strcpy(tmp, unparse_dbref(user));
    atr_add(console, "MANNED", tmp, hs_options.space_wiz, 0);
  }
  else
  {
    strcpy(tmp, unparse_dbref(console));
    manned = atr_parse_dbref(console, "MANNED");
    atr_clr(console, "MANNED", hs_options.space_wiz);
    
    if (RealGoodObject(manned))
    {
      manning = atr_parse_dbref(manned, "MANNING");
      if (manning == console)
        atr_clr(manned, "MANNING", hs_options.space_wiz);
    }
  }
  
  key = parse_boolexp(console, tmp, Use_Lock);
  add_lock(console, console, Use_Lock, key, LF_DEFAULT);
}

/* gets the user of a console */
/* consistency check to make sure they didn't man another console */
dbref get_user(dbref console)
{
  dbref   user;
  
  if (IsDrone(console))
    return console;

  if (!IsConsole(console) && !IsShip(console))
    return NOTHING;

  user = atr_parse_dbref(console, "MANNED");
    
  if (!RealGoodObject(user)) return NOTHING;

  if (user == console)
    return NOTHING;

  if (Location(user) != Location(console) || atr_parse_dbref(user, "MANNING") != console)
  {
    set_user(console, NOTHING);
    return NOTHING;
  }

  return user;
}

/* get a player's console, if they are manning one */
dbref get_console(dbref user)
{
  dbref console;
  
  if (!IsPlayer(user)) return NOTHING;
  
  console = atr_parse_dbref(user, "MANNING");
  
  if (!IsConsole(console) && !IsShip(console))
    return NOTHING;
  
  if (atr_parse_dbref(console, "MANNED") != user)
  {
    atr_clr(user, "MANNING", hs_options.space_wiz);
    return NOTHING;
  }
  
  if (Location(user) != Location(console))
  {
    set_user(console, NOTHING);
    return NOTHING;
  }
  
  return console;
}

/* why not put this here? where's waldo function */
int get_bay_capacity(hship *ship)
{
  int i, capacity;
  
  if (!ship)
    return 0;
  
  capacity = 0;
  for (i = 0; i < hs_num_ships; i++)
  {
    if (hs_ships[i].docked == ship)
    {
      capacity += hs_ships[i].nconsoles + 1;
    }
  }
  
  return capacity;
}

/************************************************************/
/* space cycle update routines  */

/* update ship heading and calculate new heading vector */
void update_heading(hship *ship)
{
  double rate;
  double dist;
  double dxy, dxy2;
  double dz, dz2;
  double zfac;
  
  /* turn rate is acceleration divided by 3 */
  rate = get_stat(ship, SYS_THRUST) / 3.0;
  if (rate < 10)
    rate = 10;

  dxy = ship->desired_xyhead - ship->xyhead;
  dz = ship->desired_zhead - ship->zhead;
  
  if (dxy > 180)
    dxy -= 360;
  
  dxy2 = dxy*dxy;
  dz2 = dz*dz;
  
  if (dxy2 < 0.25 && dz2 < 0.25)
    return;

  dist = rate / sqrt(dxy2 + dz2);
  
  if (dist >= 1.0)
  {
    ship->xyhead = ship->desired_xyhead;
    ship->zhead = ship->desired_zhead;
  } else {
    ship->xyhead = ship->xyhead + (int)(dxy*dist);
    ship->zhead = ship->zhead + (int)(dz*dist);
  
    if (ship->xyhead > 359) ship->xyhead -= 360;
    if (ship->xyhead < 0) ship->xyhead += 360;
  }
  
  zfac = cos(ship->zhead * DEG2RAD);
  ship->vx = zfac * cos((450-ship->xyhead) * DEG2RAD);
  ship->vy = zfac * sin((450-ship->xyhead) * DEG2RAD);
  ship->vz = sin(ship->zhead * DEG2RAD);

  return;
}

/* update ship velocity */
void update_speed(hship *ship)
{
  double  cur = 0, des = 0;
  double  max;
  double rate;

  cur = ship->speed;
  des = ship->desired_speed;
  max = get_stat(ship, SYS_VELOCITY);
  
  rate = get_stat(ship, SYS_THRUST);

  if (cur == 0.0 && des == 0.0)
    return;

  if (des > max)
  {
    ship->desired_speed = max;

    des = max;
  }

  if (cur < des)
  {
    if ((cur + rate) > max)
      ship->speed = max;
    else if ((cur + rate) > des)
      ship->speed = des;
    else
      ship->speed += rate;
  }
  else if (cur > des)
  {
    if ((cur - rate) < des)
      ship->speed = des;
    else
      ship->speed -= rate;
  }
  
  if (ship->speed == 0 && ship->desired_speed == 0)
    notify_srooms(ship, NOTHING, hs_options.speed_halt);

}

/* weapons regeneration/reloading */
void update_weapon(hship *ship, hconsole *con, hweapon *gun)
{
  double maxpower;
  dbref obj;
  
  if (!ship || !gun)
    return;
  
  if (!con)
    obj = ship->objnum;
  else
    obj = con->objnum;
  
  if (gun->loading > 1)
    gun->loading--;
  else if (gun->loading == 1)
  {
    if (HasFlag(gun->type, HS_PRIMARY))
      hs_std_notice(obj, "Primary weapon reloaded.");
    else if (HasFlag(gun->type, HS_SECONDARY))
      hs_std_notice(obj, "Secondary weapon reloaded.");
    
    gun->loading = 0;
  }
  
  /* discharge reactor energy to power weapons */
  maxpower = get_wstat(ship, con, gun->type | HS_MAX_POWER);
  if (gun->curpower < maxpower)
    gun->curpower += get_stat(ship, SYS_DISCHARGE);

  if (gun->curpower > maxpower)
    gun->curpower = maxpower;
}

/* normal shield regeneration */
void update_shield(hship *ship)
{
  double maxpower;
  
  maxpower = get_stat(ship, SYS_MAX_CAPACITY);
  if (ship->shield.energy < maxpower)
    ship->shield.energy += get_stat(ship, SYS_REGENERATION);

  if (ship->shield.energy > maxpower)
    ship->shield.energy = maxpower;
}

/* out of combat hull regeneration */
void update_hull(hship *ship)
{
  double maxpower;
  
  if (HasFlag(ship->type, HS_COMBAT))
    return;
  
  maxpower = get_stat(ship, SYS_MAX_ARMOR);
  if (ship->hull.energy < maxpower)
    ship->hull.energy += get_stat(ship, SYS_REPAIR);

  if (ship->hull.energy > maxpower)
    ship->hull.energy = maxpower;
}

/* restore reactor power */
void update_reactor(hship *ship)
{
  double maxpower;
  
  maxpower = get_stat(ship, SYS_MAX_ENERGY);
  if (ship->reactor.energy < maxpower)
    ship->reactor.energy += get_stat(ship, SYS_RECHARGE);

  if (ship->reactor.energy > maxpower)
    ship->reactor.energy = maxpower;
}

/* dissipate engine heat */
void update_engine(hship *ship)
{
  double maxpower;
  
  if (ship->engine.energy > 0.0)
    ship->engine.energy -= get_stat(ship, SYS_DISSIPATION);

  if (ship->engine.energy < 0.0)
    ship->engine.energy = 0.0;
}

/* restore sensor focus */
void update_sensor(hship *ship)
{
  double maxpower;
  
  maxpower = get_stat(ship, SYS_MAX_FOCUS);
  if (ship->sensor.energy < maxpower)
    ship->sensor.energy += get_stat(ship, SYS_TRACKING);

  if (ship->sensor.energy > maxpower)
    ship->sensor.energy = maxpower;
}

/* don't do anything for the computer */
/* memory is cleared by wiretaps */
void update_computer(hship *ship)
{
  return;
}

/* update all of a ships systems */
void update_systems(hship *ship)
{
  int i;
  
  update_hull(ship);
  update_shield(ship);
  update_reactor(ship);
  update_engine(ship);
  update_sensor(ship);
  update_computer(ship);

  update_weapon(ship, NULL, &(ship->primary));
  update_weapon(ship, NULL, &(ship->secondary));
  
  for (i = 0; i < ship->nconsoles; i++)
  {
    update_weapon(ship, &(ship->console[i]), &(ship->console[i].primary));
    update_weapon(ship, &(ship->console[i]), &(ship->console[i].secondary));
  }
}

/* check for space hazards */
void update_proximity(hship *ship)
{
  hcelestial *cel;
  double dist, damage;
  hcontact *q;
  int ret;
  
  if (!ship || !ship->uid)
    return;
  
  for (cel = ship->uid->head_celestial; cel; cel = cel->next)
  {
    if (!HasFlag(cel->type, HS_ANOMALY | HS_ASTEROID | HS_STAR))
      continue;

    q = find_celestial_contact(ship, cel);
    if (!q || !q->contact)
      continue;
    
    dist = ship_celestial_distance(ship, cel);
    if (dist < cel->radius)
    {
      /* we're inside, update tick */
      if (!HasFlag(q->flags, HS_INSIDE))
      {
        hs_std_sensor(ship, q, tprintf("Entering the proximity of %s.", celestial_name(cel)));
        FlagOn(q->flags, HS_INSIDE);
        TriggerEvent(update_prox,ship,q);
        execute_trigger(cel->objnum, "TR_ENTER", ship);
        execute_ufun(cel->objnum, "FN_ENTER", ship);
      }
    
      switch (HasFlag(cel->type, HS_OBJECT_FLAGS))
      {
      case HS_ASTEROID:
        if (!HasFlag(ship->type, HS_MODE_SLOW))
          FlagOn(ship->type, HS_MODE_SLOW);
        
        if (HasFlag(cel->type, HS_SENSOR_JAM) && !HasFlag(ship->type, HS_SENSOR_JAM))
          FlagOn(ship->type, HS_SENSOR_JAM);
      
      /* apply effects here */
      /* none yet */
        if (ship->speed > 200.0 || HasFlag(ship->type, HS_AFTERBURNING))
        {
          damage = ship->speed * cel->mass / 10.0;
          if (HasFlag(ship->type, HS_AFTERBURNING))
            damage *= hs_options.burn_multiplier;

          ret = do_damage(ship, damage, damage, 0.0);
      
          if (ret == HS_CRITHULL || ret == HS_MINHULL)
          {
            notify_consoles(ship, tprintf("%s%s!!%s Asteroids slam into the ships hull!", ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
          }
        }
      case HS_ANOMALY:
        if (HasFlag(cel->type, HS_SENSOR_JAM) && !HasFlag(ship->type, HS_SENSOR_JAM))
        {
          FlagOn(ship->type, HS_SENSOR_JAM);
        }
        
        execute_trigger(cel->objnum, "TRIGGER", ship);
        execute_ufun(cel->objnum, "UFUN", ship);
        break;
      case HS_STAR:
        damage = cel->mass * 30.0;
        ret = do_damage(ship, damage, damage, 0.0);
        
        if (ret == HS_CRITHULL || ret == HS_MINHULL)
        {
          notify_consoles(ship, tprintf("%s%s!!%s Solar radiation has penetrated the shields!", ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
        }
 
        break;
      default:
        break;
      }
    }
    else if (dist > cel->radius && HasFlag(q->flags, HS_INSIDE))
    {
      hs_std_sensor(ship, q, tprintf("Leaving the proximity of %s.", celestial_name(cel)));
      FlagOff(q->flags, HS_INSIDE); 

      if (HasFlag(q->type, HS_ASTEROID))
        FlagOff(ship->type, HS_MODE_SLOW);
      
      if (HasFlag(q->type, HS_SENSOR_JAM))
        FlagOff(ship->type, HS_SENSOR_JAM);
  
      TriggerEvent(update_prox,ship,q);
      execute_trigger(cel->objnum, "TR_LEAVE", ship);
      execute_ufun(cel->objnum, "FN_LEAVE", ship);
    }
  }
}

/* update the ship's position using the heading vector and speed */
void update_position(hship *ship)
{
  double  speed, diff;
  
  if (!ship)
    return;
   
  if (ship->speed != 0.0)
  {
    speed = get_stat(ship, SYS_VELOCITY);

    if (ship->linked)
    {
      diff = (ship->linked->nconsoles + 1.0) - (ship->nconsoles + 1.0);

      if (diff == 0.0)
        speed *= 0.5;
        
      if (diff > 0.0)
        speed /= pow(10.0, diff);
    }
    
    if (ship->speed > speed)
    {
      ship->speed = speed;
    }
    
    if (ship->speed < -0.5*speed)
    {
      ship->speed = -0.5 * speed;
    }
    
    speed = ship->speed;
    if (HasFlag(ship->type, HS_AFTERBURNING))
      speed *= hs_options.burn_multiplier;
    
    if (ship->linked)
    {
      ship->linked->x += speed / 1000.0 * ship->vx;
      ship->linked->y += speed / 1000.0 * ship->vy;
      ship->linked->z += speed / 1000.0 * ship->vz;
    }

    ship->x += speed / 1000.0 * ship->vx;
    ship->y += speed / 1000.0 * ship->vy;
    ship->z += speed / 1000.0 * ship->vz;
  }
  
  return;
}

/* update process of landing, docking or taking off */
void update_location(hship *ship)
{
  dbref obj;
  
  if (!ship)
    return;

  if (ship->landed && ship->landing)
  {
    ship->landing--;
    
    if (ship->landing <= 0)
    {
      ship->landing = 0;
      obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
      notify_except(obj, ship->dropto, obj, tprintf("%s glides gently to the ground.", Name(obj)), 0);
      moveto(obj, ship->dropto, hs_options.space_wiz, NULL);
      do_view(ship->objnum);
      notify_srooms(ship, NOTHING, hs_options.landing_msg);
      return;
    }
    else if (ship->landing == 3)
    {
      notify_consoles(ship, tprintf("%s%s-%s Final clearance granted...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
    else if (ship->landing == 6)
    {
      notify_srooms(ship, NOTHING, hs_options.begin_descent);
      return;
    }
    else if (ship->landing == 9)
    {
      notify_consoles(ship, tprintf("%s%s-%s Awaiting landing clearance...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
  }
  else if (ship->landed && ship->launching)
  {
    ship->launching--;
    
    if (ship->launching <= 0)
    {
      ship->launching = 0;
      obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
      notify_except(obj, Location(obj), obj, tprintf("%s roars as the engines ignite and it lifts off.", Name(obj)), 0);
      move_ship(ship, ship->landed->uid, ship->landed->x, ship->landed->y, ship->landed->z);
      notify_srooms(ship, NOTHING, hs_options.lift_off);
      return;
    }
    else if (ship->launching == 3)
    {
      notify_consoles(ship, tprintf("%s%s-%s Final clearance granted...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
    else if (ship->launching == 6)
    {
      notify_srooms(ship, NOTHING, hs_options.engines_activating);
      return;
    }
    else if (ship->launching == 9)
    {
      notify_consoles(ship, tprintf("%s%s-%s Launch clearance requested...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
  }
  else if (ship->docked && ship->landing)
  {
    ship->landing--;
    
    if (ship->landing <= 0)
    {
      ship->landing = 0;
      obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
      notify_except(obj, ship->dropto, obj, tprintf("%s glides gently into the docking bay.", Name(obj)), 0);
      moveto(obj, ship->dropto, hs_options.space_wiz, NULL);
      do_view(ship->objnum);
      notify_srooms(ship, NOTHING, hs_options.docking_msg);
      return;
    }
    else if (ship->landing == 3)
    {
      notify_consoles(ship, tprintf("%s%s-%s Final clearance granted...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
    else if (ship->landing == 6)
    {
      notify_srooms(ship, NOTHING, hs_options.begin_docking);
      return;
    }
    else if (ship->landing == 9)
    {
      notify_consoles(ship, tprintf("%s%s-%s Awaiting docking clearance...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
  }
  else if (ship->docked && ship->launching)
  {
    ship->launching--;
    
    if (ship->launching <= 0)
    {
      ship->launching = 0;
      obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
      notify_except(obj, Location(obj), obj, tprintf("%s roars as the engines ignite and it undocks.", Name(obj)), 0);
      move_ship(ship, ship->docked->uid, ship->docked->x, ship->docked->y, ship->docked->z);
      notify_srooms(ship, NOTHING, hs_options.lift_off);
      notify_consoles(ship->docked, tprintf("%s%s-%s %s%s%s has undocked.", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
           ANSI_HILITE, ship_name(ship), ANSI_NORMAL));
      notify_contacts(ship->docked, NULL, tprintf("%s%s%s has undocked.", ANSI_HILITE, ship_name(ship), ANSI_NORMAL));
      return;
    }
    else if (ship->launching == 3)
    {
      notify_consoles(ship, tprintf("%s%s-%s Final clearance granted...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
    else if (ship->launching == 6)
    {
      notify_srooms(ship, NOTHING, hs_options.engines_activating);
      return;
    }
    else if (ship->launching == 9)
    {
      notify_consoles(ship, tprintf("%s%s-%s Launch clearance requested...", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      return;
    }
  }
}

/* decrement duration and run callbacks for effects */
void update_effects(hship *ship)
{
  hbuff *b, *bptr, **bb;
  int bflags, ctype;
  
  if (!ship)
    return;
  
  bb = &ship->head_buff;
  for (b = ship->head_buff; b; )
  {
    bptr = b;
    b = b->next;
    
    bptr->duration--;
    
    if (!bptr->owner || HasFlag(bptr->owner->type, HS_DESTROYED))
      bptr->duration = 0;
    
    /* buff is expiring */
    if (bptr->duration <= 0)
    {

      (*bb) = b;
      ship->nbuffs--;

      if (bptr->buff)
      {
        bflags = bptr->buff->flags;
        if (HasFlag(bflags, HS_POST) && bptr->buff->post && bptr->owner && !HasFlag(bptr->owner->type, HS_DESTROYED))
        {
          bptr->buff->post(ship, bptr);
        }
      
        if (HasFlag(bflags, HS_ANY_COOLDOWN))
        {
          switch (HasFlag(bflags, HS_ANY_COOLDOWN))
          {
          case HS_NAV_COOLDOWN:
            ctype = HS_NAV;
            break;
          case HS_ENG_COOLDOWN:
            ctype = HS_ENG;
            break;
          case HS_GUN_COOLDOWN:
            ctype = HS_GUN;
            break;
          case HS_OPS_COOLDOWN:
            ctype = HS_OPS;
            break;
          default:
            ctype = HS_NAV;
            break;
          }
      
          notify_consoles(ship, tprintf("%s%s**%s %s%3s%s - %s%s%s%s cooldown has expired.", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
                  ANSI_HILITE, STR(hs_console_types, ctype), ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN,
                  STR(hs_cooldowns, bptr->buff), ANSI_NORMAL));
        }
      }

      free(bptr);
      continue;
    }
    
    bb = &(bptr->next);
    
    /* buff is still going */
    if (bptr->buff && HasFlag(bptr->buff->flags, HS_PERIODIC) && bptr->buff->beat && bptr->buff->frequency && !(bptr->duration % bptr->buff->frequency))
    {
      bptr->buff->beat(ship, bptr);
    }
  }
}

/* check ship health for disabled/destroyed status */
/* also up date in/out of combat status */
void update_health(hship *ship)
{
  hcelestial *cel;
  double armor, maxarmor;
  int rnum;
  
  if (!ship)
    return;
  
  /* check if armor is negative */
  armor = get_stat(ship, SYS_ARMOR);
  if (armor < 0)
  {
    /* ship has been disabled! */
    if (!HasFlag(ship->type, HS_DISABLED))
    {
      FlagOn(ship->type, HS_DISABLED);
      notify_consoles(ship, tprintf("%s%s*%s The ship has been %s%sdisabled%s!", ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL));
      notify_contacts(ship, NULL, tprintf("%s%s%s has been %s%sdisabled%s!", ANSI_HILITE, ship_name(ship), ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL));
      
      ship->speed = 0.0;
      ship->desired_speed = 0.0;
    }

    maxarmor = get_stat(ship, SYS_MAX_ARMOR);
    if (armor < -maxarmor && !HasFlag(ship->type, HS_DESTROYED))
    {
      FlagOn(ship->type, HS_DESTROYED);
      notify_srooms(ship, NOTHING, tprintf("%s%s*%s The ship has been %s%sdestroyed%s!", ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
      notify_contacts(ship, NULL, tprintf("%s%s%s has been %s%sdestroyed%s!", ANSI_HILITE, ship_name(ship), ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
      
      ship->desired_xyhead = ship->xyhead;
      ship->desired_zhead = ship->zhead;
      ship->speed = 0.0;
      ship->desired_speed = 0.0;
      
      clear_contacts(ship);
      clear_buffs(ship);
      
      /* create a debris object as a ship corpse */
      cel = create_debris(ship->uid, ship->x, ship->y, ship->z);
      if (cel)
      {
        cel->objnum = ship->objnum;
        cel->radius = 120.0;
        
        rnum = getrandom(100);
        if (rnum > 90) /* && has_cargo) */
        {
          /* put one of the systems/weapons into contents */
          rnum = getrandom(2);
          switch (rnum)
          {
          case 0:
            if (IsWeapon(ship->primary.objnum))
            {
              cel->contents = (char *) malloc(sizeof(char) * 32);
              strcpy(cel->contents, unparse_dbref(ship->primary.objnum));
            }
            break;
          case 1:
            if (IsWeapon(ship->secondary.objnum))
            {
              cel->contents = (char *) malloc(sizeof(char) * 32);
              strcpy(cel->contents, unparse_dbref(ship->secondary.objnum));
            }
            break;
          default:
            break;
          }
        } else if (rnum > 60) {
          /* place some of the cargo */
          cel->mass = 3;
          cel->contents = (char *) malloc(sizeof(char) * 32);
          strcpy(cel->contents, "ELECTRONICS");
        } else {
          /* give the equivalent cash value */
          cel->contents = (char *) malloc(sizeof(char) * 32);
          strcpy(cel->contents, "1000");
        }
      }
    }
  }
}


/* update mission respawns */
void update_mission(hmission *m)
{
  int i, n;
  hspawn *s;
  hship *new_mothership;
  
  if (!m)
    return;
  
  if (!RealGoodObject(m->objnum))
    return;
  
  if (!m->spawns || m->nspawns < 1)
    return;
  
  for (i = 0; i < m->nspawns; i++)
  {
    s = &(m->spawns[i]);
    if (!s || !s->uid || s->ndrones < 1)
      continue;
    
    s->last_respawn++;

    if (s->last_respawn < s->respawn)
      continue;
    
    new_mothership = NULL;
    for (n = 0; n < s->ndrones; n++)
    {
      if (s->drones[n])
      {
        new_mothership = s->drones[n];
        break;
      }
    }
    
    for (n = 0; n < s->ndrones; n++)
    {
      if (!s->drones[n])
      {
        s->drones[n] = spawn_drone(s->objnum, s->uid, s->x, s->y, s->z);
  
        if (s->drones[n])
        {
          if (!new_mothership)
          {
            new_mothership = s->drones[n];
          } else {
            s->drones[n]->mothership = new_mothership;
            s->drones[n]->wp_contact = new_mothership->wp_contact;
          }
          s->drones[n]->spawn = s;
        }
  
        s->last_respawn = 0;
      }
    }
  }
}


/************************************************************/
/* penn external interface routines */

/* initialize arrays and load objects */
void hs_init()
{
  dbref obj;

  hs_cycle_time = 0;
  hs_cycle_max = 0;

  spacelog_fp = fopen(SPACELOG, "a");
  if (!spacelog_fp)
    SPACEWALL("HSPACE: SPACELOG ERROR!");

  if (!hs_load_config(NOTHING))
  {
    SPACEWALL("HSPACE: CONFIG ERROR!");
    return;
  }
  
  /* allocate the arrays so we have some place to put objects */
  init_arrays();

  /* load objects from the database */
  /* scan all objects for universes */
  for (obj = 0; GoodObject(obj); obj++)
  {
    if (IsUniverse(obj))
      load_universe(hs_options.space_wiz, obj);
  }

  /* scan all objects for celestials */
  for (obj = 0; GoodObject(obj); obj++)
  {
    if (IsCelestial(obj))
      load_celestial(hs_options.space_wiz, obj);
  }
  
  /* scan all objects for ships */
  for (obj = 0; GoodObject(obj); obj++)
  {
    if (IsShip(obj))
      load_ship(hs_options.space_wiz, obj);
  }

  /* we do this twice because of docked ships not being loadable
     until the host ship has been loaded */
  for (obj = 0; GoodObject(obj); obj++)
  {
    if (IsShip(obj))
      load_ship(hs_options.space_wiz, obj);
  }

  /* scan all objects for consoles */
  for (obj = 0; GoodObject(obj); obj++)
  {
    if (IsConsole(obj))
      load_console(hs_options.space_wiz, obj);
  }
  
  /* now load missions */
  for (obj = 0; GoodObject(obj); obj++)
  {
    if (IsMission(obj))
      load_mission(hs_options.space_wiz, obj);
  }

  /* honor the autostart config option */
  if (hs_options.autostart)
  {
    SPACEWALL("HSPACE: Space started.");
    cycling = 1;
  }
  else
  {
    SPACEWALL("HSPACE: Space not started.");
    cycling = 0;
  }

}

/* clear object arrays */
void hs_shutdown()
{
  int i, n;
  
  hs_num_universes = 0;
  free(hs_universes);

  hs_num_celestials = 0;
  free(hs_celestials);

  hs_num_ships = 0;
  free(hs_ships);
  
  for (i = 0; i < hs_num_missions; i++)
  {
    for (n = 0; n < hs_missions[i].nspawns; n++)
    {
      if (hs_missions[i].spawns[n].ndrones > 0)
      {
        free(hs_missions[i].spawns[n].drones);
      }
    }
    
    free(hs_missions[i].spawns);
  }

  hs_num_missions = 0;
  free(hs_missions);

}

/* start or stop the space cycle */
void set_cycle(dbref player, int action)
{
  int     n = 0;

  if (!action)
  {				/*
				 * Stop the system 
				 */
    if (cycling == 0)
      notify(player,
	     tprintf("%sSPACE%s: The ship system is not currently running.",
		     ANSI_HILITE, ANSI_NORMAL));
    else
    {
      cycling = 0;
      for (n = 0; n < db_top; n++)
	notify(n,
	       tprintf("%sYou suddenly feel as if the universe has come to a halt.%s",
		       ANSI_HILITE, ANSI_NORMAL));
      notify(player,
	     tprintf("%sSPACE%s: The ship system has now been halted.",
		     ANSI_HILITE, ANSI_NORMAL));
    }
  }
  else
  {				/*
				 * Start the system 
				 */
    if (cycling == 1)
      notify(player,
	 tprintf("%sSPACE%s: The ship system is already currently running.",
		 ANSI_HILITE, ANSI_NORMAL));
    else
    {
      for (n = 0; n < db_top; n++)
	notify(n,
	       tprintf("%sSuddenly the universe seems to flow much more smoothly.%s",
		       ANSI_HILITE, ANSI_NORMAL));
      cycling = 1;
      notify(player,
	     tprintf("%sSPACE%s: The ship system has now been activated.",
		     ANSI_HILITE, ANSI_NORMAL));
    }
  }
}

/*
 * Gives the current status of the space system, the number of ships loaded,
 * and various other information 
 */
void hs_status(dbref player)
{
  notify(player,
    tprintf("%sSPACE%s: %s",
	   ANSI_HILITE, ANSI_NORMAL, HSPACE_VERSION));
  if (cycling) {
    notify(player, "       The space system is currently ACTIVE.");
    notify(player, tprintf("       Space cycle time: %.3fms (%.3fms max)", 
		hs_cycle_time * 1.0, hs_cycle_max * 1.0));
  } else
    notify(player, "       The space system is currently INACTIVE.");

  notify(player, tprintf("       %d/%d universes loaded", hs_num_universes, hs_options.max_universes));
  notify(player, tprintf("       %d/%d celestials loaded", hs_num_celestials, hs_options.max_celestials));
  notify(player, tprintf("       %d/%d ships loaded", hs_num_ships, hs_options.max_ships));

}

void hs_version(dbref player)
{
  notify_format(player, "HSPACE: Currently running version %s%s%s.", ANSI_HILITE, HSPACE_VERSION, ANSI_NORMAL);
  return;
}


/* run the space cycle on a single ship */
void hs_cycle_ship(hship *ship)
{
  int i, copy_contacts;
  hship *new_mothership;
  
  if (!ship)
    return;

  /* update combat timer here so you can reman consoles */
  ship->combat_timer++;

  /* check combat timer */
  if (InCombat(ship) && !HasFlag(ship->type, HS_COMBAT))
  {
    FlagOn(ship->type, HS_COMBAT);
    notify_consoles(ship, tprintf("%s%s!!%s Entering combat %s%s!!%s",
         ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
  }
  
  if (!InCombat(ship) && HasFlag(ship->type, HS_COMBAT))
  {
    FlagOff(ship->type, HS_COMBAT);
    notify_consoles(ship, tprintf("%s%s-%s Leaving combat.", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
    
    /* clear combat id */
    ship->cid = 0;
  }

  if (HasFlag(ship->type, HS_DESTROYED))
  {
    if (HasFlag(ship->type, HS_DRONE))
    {
      /* check to see if it's part of a spawn */
      if (ship->spawn)
      {
        /* find the first viable spawn to be a new mothership */
        new_mothership = NULL;
        copy_contacts = 0;
        for (i = 0; i < ship->spawn->ndrones; i++)
        {
          if (!ship->spawn->drones[i])
            continue;
          
          if (ship->spawn->drones[i] != ship)
          {
            new_mothership = ship->spawn->drones[i];
            break;
          }
        }
        
        /* cycle drones, if it's the ship the null it, else if the
          drone's mothership is our ship then set the drone's mothership
          to be the new_mothership we found */

        for (i = 0; i < ship->spawn->ndrones; i++)
        {
          if (!ship->spawn->drones[i])
            continue;
            
          if (ship->spawn->drones[i] == ship)
          {
            /* remove the drone spawn */
            ship->spawn->last_respawn = 0;
            ship->spawn->drones[i] = NULL;
          }
          else if (new_mothership && ship->spawn->drones[i]->mothership == ship)
          {
            /* set the drones new_mothership */
            ship->spawn->drones[i]->mothership = new_mothership;
            ship->spawn->drones[i]->wp_contact = new_mothership->wp_contact;
            copy_contacts = 1;
          }
        }
        
        if (copy_contacts && new_mothership && !new_mothership->head_contact)
        {
          new_mothership->head_contact = ship->head_contact;
          new_mothership->ncontacts = ship->ncontacts;
          new_mothership->wp_contact = ship->wp_contact;
          
          ship->head_contact = NULL;
          ship->ncontacts = 0;
        }
      }

      clear_contacts(ship);
      clear_buffs(ship);
      remove_ship_from_universe(ship);

      free(ship);
    }
    return;
  }

  /* call the heartbeat event */
  //if (ship->heartbeat) ((*ship).heartbeat)(ship, NULL);
  TriggerEvent(heartbeat,ship,NULL);

  if (!HasFlag(ship->type, HS_DISABLED))
  {
    /* regenerate system/shield power */
    update_systems(ship);
    
    if (!ship->landed && !ship->docked)
    {
      /* update ship's velocity vector */
      update_heading(ship);
      update_speed(ship);
    
      /* move the ship */
      update_position(ship);
    } else {
      update_location(ship);
    }

    /* check for proximity triggers */
    /* asteroid fields, plasma storms, debris clouds, etc */
    update_proximity(ship);
  }

  /* buff and debuff status effects */
  update_effects(ship);
  
  /* finally, update sensors */
  /* this is done even for landed/docking ships */
  /* so they can gracefully lose the contacts */
  update_contacts(ship);
  
  /* check whether disabled/destroyed */
  /* done after status effects in case they cause it */
  update_health(ship);
  
  /* update prompts */
  if ((HasFlag(ship->prompt, HS_PROMPT_ALWAYS) ||
     (HasFlag(ship->type, HS_COMBAT) && HasFlag(ship->prompt, HS_PROMPT_COMBAT)) ||
     (ship->uid && HasFlag(ship->prompt, HS_PROMPT_SPACE))) &&
     HasFlag(ship->prompt, HS_PROMPT_FREQUENCY) && 
     !(hs_cycle_time % HasFlag(ship->prompt, HS_PROMPT_FREQUENCY)))
  {
    hs_prompt(ship->objnum);
  }

  for (i = 0; i < ship->nconsoles; i++)
  {
    if ((HasFlag(ship->console[i].prompt, HS_PROMPT_ALWAYS) ||
       (HasFlag(ship->type, HS_COMBAT) && HasFlag(ship->console[i].prompt, HS_PROMPT_COMBAT)) ||
       (ship->uid && HasFlag(ship->console[i].prompt, HS_PROMPT_SPACE))) &&
       HasFlag(ship->prompt, HS_PROMPT_FREQUENCY) &&
       !(hs_cycle_time % HasFlag(ship->console[i].prompt, HS_PROMPT_FREQUENCY)))
    {
      hs_prompt(ship->console[i].objnum);
    }
  }
}

/* main space loop */
void hs_cycle()
{
  static time_t tt = 0;
  hship  *ship;
  int i;
  
  hcelestial *cel, *cptr, **cc;
  unsigned long ms;
  struct timeval tv;

  if (!cycling)
    return;

  gettimeofday(&tv, (struct timezone *) NULL);
  ms = (tv.tv_sec * 1000000) + tv.tv_usec;

  if ((time(NULL) - tt) >= hs_options.cyc_interval)
  {
    tt = time(NULL);

    for (i = 0; i < hs_num_missions; i++)
    {
      update_mission(&hs_missions[i]);
    }
    
    /* first do the player ships */
    for (i = 0; i < hs_num_ships; i++)
    {
      hs_cycle_ship(&hs_ships[i]);
    }
    
    /* and now the drones and debris */
    for (i = 0; i < hs_num_universes; i++)
    {
      for (ship = hs_universes[i].head_drone; ship; ship = ship->next)
      {
        hs_cycle_ship(ship);
      }
      
      cc = &(hs_universes[i].head_celestial);
      for (cel = hs_universes[i].head_celestial; cel; )
      {
        cptr = cel;
        cel = cel->next;

        if (HasFlag(cptr->type, HS_DEBRIS))
        {
          cptr->radius -= 1.0;
          if (HasFlag(cptr->type, HS_DESTROYED))
          {
            /* delete the object */
            remove_celestial_from_universe(cptr);
            (*cc) = cel;

            if (cptr->contents)
              free(cptr->contents);

            free(cptr);
            hs_universes[i].num_celestials--;
            continue;
          }
          cc = &(cptr->next);
            
          if (cptr->radius <= 0.0)
          {
            FlagOn(cptr->type, HS_DESTROYED);
          }
        }
      }
    }
  }

  gettimeofday(&tv, (struct timezone *) NULL);
  hs_cycle_time = (((tv.tv_sec * 1000000) + tv.tv_usec) - ms) * .001;
  if (hs_cycle_time > hs_cycle_max) hs_cycle_max = hs_cycle_time;
  //hs_cycle_time++;
}


/* save all ship attributes to their mush objects */
void hs_dump()
{
  int i;

  for (i = 0; i < hs_num_celestials; i++)
    dump_celestial(&hs_celestials[i]);

  for (i = 0; i < hs_num_ships; i++)
    dump_ship(&hs_ships[i]);

  atr_add(hs_options.space_wiz, "HS_COMBAT_IDS", unparse_integer(hs_num_combat_ids), hs_options.space_wiz, 0);
  return;
}


/***********************************************************/
/* math routines */

int get_firing_cone(hship *ship, hconsole *con, hship *target)
{
  double     zangle, xyangle, tmpz, zfac;
  double   ig, jg, kg, it, jt, kt;
  double   dp;
  double xyhead, zhead;


  if (!ship || !target)
    return 0.0;
  
  /* weapon heading vector */
  if (!con)
  {
    xyhead = ship->xyhead;
    zhead = ship->zhead;
  } else {
    xyhead = con->xyhead;
    zhead = con->zhead;
  }
    
  if (ship->zhead < 0)
    tmpz = zhead + 360;
  else
    tmpz = zhead;

  tmpz *= DEG2RAD;
  zfac = cos(tmpz);
  ig = sin(xyhead * DEG2RAD) * zfac;
  jg = cos(xyhead * DEG2RAD) * zfac;
  kg = sin(tmpz);

  /* target bearing vector */
  zangle = zang(ship->x, ship->y, ship->z, target->x, target->y, target->z);
  xyangle = xyang(ship->x, ship->y, target->x, target->y);

  if (zangle < 0)
    tmpz = zangle + 360;
  else
    tmpz = zangle;

  tmpz *= DEG2RAD;
  zfac = cos(tmpz);
  it = sin(xyangle * DEG2RAD) * zfac;
  jt = cos(xyangle * DEG2RAD) * zfac;
  kt = sin(tmpz);
 
  dp = (ig * it) + (jg * jt) + (kg * kt);

  return rint(acos(dp) * RAD2DEG);
}


/* standard vector magnitude distance */
double dist3d(double fx, double fy, double fz, double sx, double sy, double sz)
{
  return sqrt((sz - fz) * (sz - fz) + (sx - fx) * (sx - fx) + (sy - fy) * (sy - fy));
}

/* elevation calculation */
int zang(double fx, double fy, double fz, double sx, double sy, double sz)
{
  char    tbuf[64];

  if (fz == sz)
    return 0;
  else
  {
    if (sy == fy && sx == fx)
    {
      if (sz > fz)
	return 90;
      else
	return -90;
    }
    else
    {
      sprintf(tbuf, "%.0f",
	       RAD2DEG *
	       atan((sz - fz) / sqrt(((sx - fx) * (sx - fx)) +
				      ((sy - fy) * (sy - fy)))));
      return atoi(tbuf);
    }
  }
}

/* azimuth calculation */
int xyang(double fx, double fy, double sx, double sy)
{
  char    tbuf[64];
  int     ang;

  if (sx - fx == 0)
  {
    if (sy - fy == 0)
      return 0;
    else if (sy > fy)
      return 0;
    else
      return 180;
  }
  else if (sy - fy == 0)
  {
    if (sx > fx)
      return 90;
    else
      return 270;
  }
  else if (sx < fx)
  {
    sprintf(tbuf, "%.0f",
	    270 - RAD2DEG * atan((sy - fy) / (sx - fx)));
    ang = atoi(tbuf);
    if (ang < 0)
      ang += 360;
    else if (ang > 359)
      ang -= 360; 
    return ang;
  }
  else
  {
    sprintf(tbuf, "%.0f",
	    90 - RAD2DEG * atan((sy - fy) / (sx - fx)));
    ang = atoi(tbuf);
    if (ang < 0)
      ang += 360;
    else if (ang > 359)
      ang -= 360; 
    return ang;
  }
}


/***********************************************************/
/* movement utility routines */

/* remove a ship from it's current universe */
void remove_ship_from_universe(hship *ship)
{
  hship *s, **last;
  int *lastnum;
  
  if (!ship || !ship->uid)
    return;
  
  if (HasFlag(ship->type, HS_DRONE))
  {
    last = &(ship->uid->head_drone);
    lastnum = &(ship->uid->num_drones);
  } else {
    last = &(ship->uid->head_ship);
    lastnum = &(ship->uid->num_ships);
  }
  
  if (*last)
  {
    /* remove ship from the old universe's linked list */
    /* make sure that old->head_* is cleared */
    for (s = *last; s; s = s->next)
    {
      if (s == ship)
      {
        (*last) = s->next;
        (*lastnum)--;
        break;
      }
      last = &(s->next);
    }
  }
  
  ship->uid = NULL;
  
//  clear_contacts(ship);
}

/* move a ship to a new universe/xyz */
void move_ship(hship *ship, huniverse *uid, double x, double y, double z)
{
  dbref obj;
  hship *s;
  int i;
  int finish_with_remove = 0;
  
  if (!ship)
    return;
  
  ship->x = x;
  ship->y = y;
  ship->z = z;
  
  /* assumed same universe if uid is NULL */
  if (!uid)
    return;
  
  /* there was a uid specified, so launch immediately */
  ship->landed = NULL;
  ship->docked = NULL;
  ship->landing = 0;
  ship->launching = 0;
  
  /* ship wasn't in a universe before
     probably a newly loaded ship, but maybe docked/landed
     we should walk all universe lists and make sure */
  if (!ship->uid)
  {
    for (i = 0; i < hs_num_universes; i++)
    {
      if (finish_with_remove)
        break;
      
      if (HasFlag(ship->type, HS_DRONE))
      {
        /* walk the drone list */
        for (s = hs_universes[i].head_drone; s; s = s->next)
        {
          if (s == ship)
          {
            ship->uid = &hs_universes[i];
            finish_with_remove = 1;
            break;
          }
        }
      } else {
        /* walk the standard ship list */
        for (s = hs_universes[i].head_ship; s; s = s->next)
        {
          if (s == ship)
          {
            SPACEWALL("HSPACE: ERROR #137 in move_ship()! ship had a bad uid but we fixed it.");
            ship->uid = &hs_universes[i];
            finish_with_remove = 1;
            break;
          }
        }
      }
    }
  
    /* was never in a universe, go ahead and place it at the head
       of the new universe and return */
    if (!finish_with_remove)
    {
      ship->uid = uid;
      
      if (HasFlag(ship->type, HS_DRONE))
      {
        ship->next = uid->head_drone;
        uid->head_drone = ship;
        uid->num_drones++;
      } else {
        ship->next = uid->head_ship;
        uid->head_ship = ship;
        uid->num_ships++;
      }

      /* move the ship's shipobj inside uid->objnum */
      obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
      if (obj != NOTHING)
        moveto(obj, uid->objnum, hs_options.space_wiz, NULL);

      return;
    }
  }
  
  /* same universe, already did xyz */
  if (ship->uid == uid)
    return;

  /* remove it from its current universe */
  remove_ship_from_universe(ship);

  /* place the ship at the head of the new universe */
  ship->uid = uid;
  
  if (HasFlag(ship->type, HS_DRONE))
  {
    ship->next = uid->head_drone;
    uid->head_drone = ship;
    uid->num_drones++;
  } else {
    ship->next = uid->head_ship;
    uid->head_ship = ship;
    uid->num_ships++;
  }
  
  /* move the ship's shipobj inside uid->objnum */
  obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
  if (obj != NOTHING)
    moveto(obj, uid->objnum, hs_options.space_wiz, NULL);

  return;
}

/* remove a celestial from it's current universe */
void remove_celestial_from_universe(hcelestial *cel)
{
  hcelestial *c, **last;
  int *lastnum;
  
  if (!cel || !cel->uid)
    return;
  
  last = &(cel->uid->head_celestial);
  lastnum = &(cel->uid->num_celestials);
  
  if (*last)
  {
    /* remove ship from the old universe's linked list */
    /* make sure that old->head_* is cleared */
    for (c = *last; c; c = c->next)
    {
      if (c == cel)
      {
        if (last && lastnum)
        {
          (*last) = c->next;
          (*lastnum)--;
        }
        break;
      }
      last = &(c->next);
    }
  }
  
  cel->uid = NULL;
}

/* move a celestial to a new universe and/or position */
void move_celestial(hcelestial *cel, huniverse *uid, double x, double y, double z)
{
  huniverse *old;
  hcelestial *c, **last;
  int i;
  int finish_with_remove = 0;
  
  if (!cel)
    return;
  
  cel->x = x;
  cel->y = y;
  cel->z = z;
  
  /* assumed same universe if uid is NULL */
  if (!uid)
    return;

  /* cel wasn't in a universe before
     probably a newly loaded cel, but maybe a bug?
     we should walk all universe lists and make sure */
  if (!cel->uid)
  {
    for (i = 0; i < hs_num_universes; i++)
    {
      if (finish_with_remove)
        break;
      
      for (c = hs_universes[i].head_celestial; c; c = c->next)
      {
        if (c == cel)
        {
          SPACEWALL("HSPACE: ERROR #137 in move_cel()! cel had a bad uid but we fixed it.");
          cel->uid = &hs_universes[i];
          finish_with_remove = 1;
          break;
        }
      }
    }
  
    if (!finish_with_remove)
    {
      cel->uid = uid;
      cel->next = uid->head_celestial;
      uid->head_celestial = cel;
      uid->num_celestials++;

      /* move cel->objnum inside uid->objnum */
      moveto(cel->objnum, uid->objnum, hs_options.space_wiz, NULL);

      return;
    }
  }
  
  /* save universe, already did xyz */
  if (cel->uid == uid)
    return;
  
  /* nothing left to remove */
  remove_celestial_from_universe(cel);
    
  /* place the new cels at the head of the new universe */
  cel->uid = uid;
  cel->next = uid->head_celestial;
  uid->head_celestial = cel;
  uid->num_celestials++;

  /* move cel->objnum inside uid->objnum */
  moveto(cel->objnum, uid->objnum, hs_options.space_wiz, NULL);

  return;
}

/* land a ship on a celestial */
void enter_celestial(hship *ship, hcelestial *landed, dbref pad)
{
  if (!ship || !landed || !RealGoodObject(pad))
    return;
  
  remove_ship_from_universe(ship);
  ship->docked = NULL;
  ship->landed = landed;
  ship->landing = 10;
  ship->launching = 0;
  ship->dropto = pad;
}

/* launch a ship */
void leave_ship(hship *ship)
{
  if (!ship)
    return;
  
  if (!ship->landed && !ship->docked)
    return;
  
  ship->launching = 10;
  ship->dropto = NOTHING;
}

/* dock a ship with another ship */
void enter_ship(hship *ship, hship *docked, dbref bay)
{
  if (!ship || !docked || !RealGoodObject(bay))
    return;
  
  remove_ship_from_universe(ship);
  ship->docked = docked;
  ship->landed = NULL;
  ship->landing = 10;
  ship->launching = 0;
  ship->dropto = bay;
}


