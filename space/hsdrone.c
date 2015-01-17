
#include "hspace.h"

/*
  Find the hship and hcontact data structures in hstypes.h.

  Contacts can be either a ship or a celestial.
  Use HasFlag(q->type, HS_SHIP) or HasFlag(q->type, HS_CELESTIAL) to distinguish them.
  The HS_ANY_SHIP mask is HS_SHIP | HS_DRONE, but drones will also be set HS_SHIP so it's a bit redundant I guess
  The HS_ANY_CELESTIAL mask is HS_CELESTIAL | (all the celestial subtypes).

  Callback arguments are (hship *ship, hcontact *q, char *event).
  ship is the calling ship, q is the other object that caused the event.

  Here are some of the constructs and macros that may be useful:

  Walking the sensor contacts
  hcontact *q;
  for (q = get_head_contact(ship); q; q = q->next)
    if (HasFlag(q->type, HS_SHIP))
      do_some_ship_stuff();

  Define a callback function.
  #define HCALLBACK(x)            void (HS_ ## x)(hship *ship, hcontact *q, char *event)

  *** MUST ADD CALLBACKS TO hs_callbacks STRINGMAP ARRAY AT BOTTOM OF FILE ***
  #define STRINGMAP(x)		{#x, (void *) HS_ ## x}

  This is how the events are called.
  #define TriggerEvent(e,s,c)     (hs_call_basic_hook(#e,(s),(c))

  Basic bitfield flag manipulation.
  #define HasFlag(x,f)	((x) & (f))
  #define FlagOn(x,f)	((x) |= (f))
  #define FlagOff(x,f)	((x) &= ~(f))

  Retrieve a pointer to the contact object with the correct typing
    e.g., hship *ship = ContactShip(q);
  #define ContactShip(x)	((hship*)((x)->contact))
  #define ContactCelestial(x)	((hcelestial*)((x)->contact))

  Some generalized conditionals that will auto detect ship/celestial types and call the appropriate function
  #define ContactName(x)	(HasFlag((x)->type, HS_ANY_SHIP) ? ship_name(ContactShip(x)) : celestial_name(ContactCelestial(x)))
  #define ContactObj(x)	(HasFlag((x)->type, HS_ANY_SHIP) ? ContactShip(x)->objnum : ContactCelestial(x)->objnum)
  #define ContactDistance(s,x)	(HasFlag((x)->type, HS_ANY_SHIP) ? ship_distance(s,ContactShip(x)) : ship_celestial_distance(s,ContactCelestial(x)))

  If you already know the contact type, you can use ShipName(q->contact) and bypass the conditional
  #define ShipName(x)	ship_name((hship*)(x))
  #define CelestialName(x)	celestial_name((hcelestial*)(x))

*/

/* stringmap for callback functions */
hstringmap hs_callbacks[] =
{
  STRINGMAP(drone_heart),
  STRINGMAP(drone_contact),
  STRINGMAP(station_keeping),
  STRINGMAP(generic_turret),
  STRINGMAP(generic_heartbeat),
  STRINGMAP(generic_proximity),
  STRINGMAP(dummy_event_handler),
  {NULL, NULL}
};

/* dummy event handler that just prints out a message */
/* can be used in any event slot to test it */
HCALLBACK(dummy_event_handler)
{
  if (!ship)
    return;
    
  if (!q || !q->contact)
    hs_std_notice(ship->objnum, tprintf("DUMMY: %s", event));
  else
    hs_std_sensor(ship, q, tprintf("DUMMY: %s", event));
}

/* basic heartbeat callback for a standard drone */
/* check for targets, goto targets */
HCALLBACK(drone_heart)
{
  double threat, dist;
  hconsole *console;
  hcontact *qptr, *qset;
  double fx, fy, fz, maxspeed;
  int i, burn;
  
  if (!ship)
    return;

  maxspeed = get_stat(ship, SYS_VELOCITY);
  burn = 1;
  if (HasFlag(ship->type, HS_MODE_SLOW))
  {
    maxspeed = 200.0;
    burn = 0;
  }
  
  /* assess the threat list. check if any contacts 
     are 10% higher than the current threat */
  if (!ship->lock || !ship->lock->contact)
  {
    threat = 0.0;
  } else {
    threat = ship->lock->threat * 1.1;
  }
  
  qset = ship->lock;
  /* walk the contacts and compare threat */
  for (qptr = get_head_contact(ship); qptr; qptr = qptr->next)
  {
    if (qptr->threat > threat)
      qset = qptr;
  }
  
  /* we have a new target, change target lock */
  if (qset != ship->lock)
    change_lock(ship, NULL, qset);
  

  /* continue going to something */
  if (ship->lock && ship->lock->contact)
  {
    ship->desired_xyhead = xyang(ship->x, ship->y, ContactShip(ship->lock)->x, ContactShip(ship->lock)->y);
    ship->desired_zhead = zang(ship->x, ship->y, ship->z, ContactShip(ship->lock)->x, ContactShip(ship->lock)->y, ContactShip(ship->lock)->z);
    
    /*if (ship_distance(ship, ContactShip(ship->lock)) < 10.0))
    {
      change_speed(ship, 0.0);
      change_afterburner(ship, 0);
    }
    else*/
    
    if (ship->desired_speed != maxspeed)
    {
      change_speed(ship, maxspeed);
      change_afterburner(ship, burn);
    }
    
    dist = ship_distance(ship, ContactShip(ship->lock));
    if (!ship->primary.loading && ship->primary.range > dist)
      fire_weapon(ship, NULL, &(ship->primary), ship->lock, 0.0, 0);
    if (!ship->secondary.loading && ship->secondary.range > dist)
      fire_weapon(ship, NULL, &(ship->secondary), ship->lock, 0.0, 0);
  }
  else if (ship->wp_contact && ship->wp_contact->contact)
  {
    if (HasFlag(ship->wp_contact->type, HS_ANY_CELESTIAL))
    {
      fx = ContactCelestial(ship->wp_contact)->x;
      fy = ContactCelestial(ship->wp_contact)->y;
      fz = ContactCelestial(ship->wp_contact)->z;
    }
    else if (HasFlag(ship->wp_contact->type, HS_ANY_SHIP))
    {
      fx = ContactShip(ship->wp_contact)->x;
      fy = ContactShip(ship->wp_contact)->y;
      fz = ContactShip(ship->wp_contact)->z;
    } else {
      fx = ship->x;
      fy = ship->y;
      fz = ship->z;
    }

    ship->desired_xyhead = xyang(ship->x, ship->y, fx, fy);
    ship->desired_zhead = zang(ship->x, ship->y, ship->z, fx, fy, fz);

    if (dist3d(ship->x, ship->y, ship->z, fx, fy, fz) < hs_options.max_waypoint_dist)
    {
      change_speed(ship, 0.0);
      change_afterburner(ship, 0);
    }
    else if (ship->desired_speed != maxspeed)
    {
      change_speed(ship, maxspeed);
      change_afterburner(ship, burn);
    }

  }
}

/* just maintain basic station keeping and run turrets */
HCALLBACK(station_keeping)
{
  double threat, dist;
  hconsole *console;
  hcontact *qptr, *qset;
  int i;
  
  if (!ship)
    return;

  /* run console heartbeats */
  for (i = 0; i < ship->nconsoles; i++)
  {
    console = &(ship->console[i]);
    if (!RealGoodObject(get_user(console->objnum)))
    {
      TriggerConsole(heartbeat,ship,console,NULL);
    }
  }
  
  /* check that the drone isn't manned */
  if (RealGoodObject(ship->objnum) && RealGoodObject(get_user(ship->objnum)))
  {
    return;
  }
  
  /* assess the threat list. check if any contacts 
     are 10% higher than the current threat */
  if (!ship->lock || !ship->lock->contact)
  {
    threat = 0.0;
  } else {
    threat = ship->lock->threat * 1.1;
  }
  
  qset = ship->lock;
  /* walk the contacts and compare threat */
  for (qptr = get_head_contact(ship); qptr; qptr = qptr->next)
  {
    if (qptr->threat > threat)
      qset = qptr;
  }
  
  /* we have a new target, change target lock */
  if (qset != ship->lock)
    change_lock(ship, NULL, qset);
  

  /* continue going to something */
  if (ship->lock && ship->lock->contact)
  {
    dist = ship_distance(ship, ContactShip(ship->lock));
    if (!ship->primary.loading && ship->primary.range > dist)
      fire_weapon(ship, NULL, &(ship->primary), ship->lock, 0.0, 0);
    if (!ship->secondary.loading && ship->secondary.range > dist)
      fire_weapon(ship, NULL, &(ship->secondary), ship->lock, 0.0, 0);
  }
}

/* generic turret */
HCALLBACK(generic_turret)
{
  double threat, dist;
  hcontact *qptr, *qset;
  
  if (!ship || !con)
    return;

  /* check that the drone isn't manned */
  if (RealGoodObject(con->objnum) && RealGoodObject(get_user(con->objnum)))
  {
    return;
  }

  /* assess the threat list. check if any contacts 
     are 10% higher than the current threat */
  if (!con->lock || !con->lock->contact)
  {
    threat = 0.0;
  } else {
    threat = con->lock->threat * 1.1;
  }
  
  qset = con->lock;
  /* walk the contacts and compare threat */
  for (qptr = get_head_contact(ship); qptr; qptr = qptr->next)
  {
    if (qptr->threat > threat)
      qset = qptr;
  }
  
  /* we have a new target, change target lock */
  if (qset != con->lock)
    change_lock(ship, con, qset);
  

  /* continue going to something */
  if (con->lock && con->lock->contact)
  {
    dist = ship_distance(ship, ContactShip(con->lock));
    if (!con->primary.loading && con->primary.range > dist)
      fire_weapon(ship, con, &(con->primary), con->lock, 0.0, 0);
    if (!con->secondary.loading && con->secondary.range > dist)
      fire_weapon(ship, con, &(con->secondary), con->lock, 0.0, 0);
  }
}

/* send a welcome message */
HCALLBACK(drone_contact)
{
  hcontact *qptr;
  
  if (!ship)
    return;
  
  if (!q || !q->contact)
    return;
  
  if (!HasFlag(q->type, HS_SHIP))
    return;
  
  if (check_friend(ship, q))
    return;
  
  /* it's an enemy! */
  if (q->threat < 1.0)
  {
    q->threat = 1.0;
  }
}

/***************************************************/
/* generic callbacks */

/* once per cycle heartbeat */
HCALLBACK(generic_heartbeat)
{
  huniverse *uid;
  double fx, fy, fz, maxspeed;
  int burn;

  if (!ship)
    return;
  
  if (!HasFlag(ship->type, HS_ANY_MODE))
    return;
  
  maxspeed = get_stat(ship, SYS_VELOCITY);
  burn = 1;
  if (HasFlag(ship->type, HS_MODE_SLOW))
  {
    maxspeed = 200.0;
    burn = 0;
  }

  switch (HasFlag(ship->type, HS_ANY_MODE))
  {
  /* GOTO nav mode */
  case HS_MODE_GOTO:

    if (!ship->wp_contact)
    {
      if (ship->desired_speed != 0.0)
      {
        change_speed(ship, 0.0);
        change_afterburner(ship, 0);
      }
        
      break;
    }
      
    /* find a goto point */
    if (HasFlag(ship->wp_contact->type, HS_ANY_SHIP))
    {
      uid = ContactShip(ship->wp_contact)->uid;
      fx = ContactShip(ship->wp_contact)->x;
      fy = ContactShip(ship->wp_contact)->y;
      fz = ContactShip(ship->wp_contact)->z;
    }
    else if (HasFlag(ship->wp_contact->type, HS_ANY_CELESTIAL))
    {
      uid = ContactCelestial(ship->wp_contact)->uid;
      fx = ContactCelestial(ship->wp_contact)->x;
      fy = ContactCelestial(ship->wp_contact)->y;
      fz = ContactCelestial(ship->wp_contact)->z;
    } else {
      fx = ship->x;
      fy = ship->y;
      fz = ship->z;
    }
    
    if (ship->uid != uid)
      break;
    
    if (dist3d(ship->x, ship->y, ship->z, fx, fy, fz) > hs_options.max_waypoint_dist)
    {
      /* same as intercept, until we get into range */
      ship->desired_xyhead = xyang(ship->x, ship->y, fx, fy);
      ship->desired_zhead = zang(ship->x, ship->y, ship->z, fx, fy, fz);
      
      if (ship->desired_speed < maxspeed)
      {
        change_speed(ship, maxspeed);
        change_afterburner(ship, burn);
      }
    }
    else if (ship->desired_speed != 0.0)
    {
      /* instead, let's trigger a console attribute */
      //TriggerEvent(reached_waypoint,ship,NULL);
      change_speed(ship, 0.0);
      change_afterburner(ship, 0);
    }
    
    break;
  
  /* INTERCEPT nav mode */
  /*   similar to goto, but it keeps the ship facing the target */
  case HS_MODE_INTERCEPT:

    if (!ship->lock)
    {
      if (ship->desired_speed != 0.0)
      {
        change_speed(ship, 0.0);
        change_afterburner(ship, 0);
      }
        
      break;
    }
    
    fx = ContactShip(ship->lock)->x;
    fy = ContactShip(ship->lock)->y;
    fz = ContactShip(ship->lock)->z;

    /* point toward the target ship */
    ship->desired_xyhead = xyang(ship->x, ship->y, fx, fy);
    ship->desired_zhead = zang(ship->x, ship->y, ship->z, fx, fy, fz);

//    if (dist3d(ship->x, ship->y, ship->z, fx, fy, fz) > hs_options.max_waypoint_dist)
//    {
      if (ship->desired_speed < maxspeed)
      {
        change_speed(ship, maxspeed);
        change_afterburner(ship, burn);
      }
//    }
//    else if (ship->desired_speed != 0.0)
//    {
//      change_speed(ship, 0.0);
//    }
    
    break;

  /* EVADE nav mode */
  /*   similtar to intercept, but opposite direction */
  case HS_MODE_EVADE:

    if (!ship->lock)
    {
      if (ship->desired_speed != 0.0)
      {
        change_speed(ship, 0.0);
        change_afterburner(ship, 0);
      }
        
      break;
    }
    
    fx = ContactShip(ship->lock)->x;
    fy = ContactShip(ship->lock)->y;
    fz = ContactShip(ship->lock)->z;

    /* point away from the target ship */
    ship->desired_xyhead = xyang(fx, fy, ship->x, ship->y);
    ship->desired_zhead = zang(fx, fy, fz, ship->x, ship->y, ship->z);

    if (ship->desired_speed < maxspeed)
    {
      change_speed(ship, maxspeed);
      change_afterburner(ship, burn);
    }
    
    break;
    
  /* FORMATION nav mode */
  /*   similar to goto, but only works on ships and it matches heading/speed when in range */
  case HS_MODE_FORMATION:

    if (!ship->wp_contact || !HasFlag(ship->wp_contact->type, HS_ANY_SHIP))
    {
      if (ship->desired_speed != 0.0)
      {
        change_speed(ship, 0.0);
        change_afterburner(ship, 0);
      }
      
      break;
    }
      
    if (ship_distance(ship, ContactShip(ship->wp_contact)) > hs_options.max_waypoint_dist)
    {
      fx = ContactShip(ship->wp_contact)->x;
      fy = ContactShip(ship->wp_contact)->y;
      fz = ContactShip(ship->wp_contact)->z;
      /* same as intercept, until we get into range */
      ship->desired_xyhead = xyang(ship->x, ship->y, fx, fy);
      ship->desired_zhead = zang(ship->x, ship->y, ship->z, fx, fy, fz);
      
      if (ship->desired_speed < maxspeed)
      {
        change_speed(ship, maxspeed);
        change_afterburner(ship, burn);
      }
    } else {
      /* we're close enough, match speed and heading */
      ship->desired_xyhead = ContactShip(ship->wp_contact)->desired_xyhead;
      ship->desired_zhead = ContactShip(ship->wp_contact)->desired_zhead;
      
      if (maxspeed < ContactShip(ship->wp_contact)->desired_speed)
      {
        if (ship->desired_speed != maxspeed)
        {
          change_speed(ship, maxspeed);
          change_afterburner(ship, burn);
        }
      } else {
        if (ship->desired_speed != ContactShip(ship->wp_contact)->desired_speed)
        {
          change_speed(ship, ContactShip(ship->wp_contact)->desired_speed);
        }
        
        if (HasFlag(ship->type, HS_AFTERBURNING) != HasFlag(ContactShip(ship->wp_contact)->type, HS_AFTERBURNING))
        {
          change_afterburner(ship, HasFlag(ContactShip(ship->wp_contact)->type, HS_AFTERBURNING));
        }
      }
    }
    
    break;
  
  /* ALLSTOP nav mode */
  /*   comes to a complete stop and removes itself once there */
  case HS_MODE_ALLSTOP:
  
    if (ship->desired_speed != 0.0)
    {
      change_speed(ship, 0.0);
      change_afterburner(ship, 0);
    }
    else if (ship->speed == 0.0)
    {
      FlagOff(ship->type, HS_MODE_ALLSTOP);
    }

    break;

  default:
    //SPACEWALL(tprintf("HSPACE: Navigational mode corrupted %X", ship->type));
    break;
  }
}


HCALLBACK(generic_proximity)
{
  if (!ship || !q || !q->contact)
    return;
  
  if (HasFlag(q->type, HS_ASTEROID) && HasFlag(q->flags, HS_INSIDE) && ContactCelestial(q)->mass > 0.0)
  {
    if (ship->speed > 200.0)
    {
      change_speed(ship, 200.0);
    }
    
    if (HasFlag(ship->type, HS_AFTERBURNING))
    {
      change_afterburner(ship, 0);
    }
  }
}









