#ifndef __HSDB_H
#define __HSDB_H

#include "hsdefines.h"
/***************************************************/
/* type enumerators */

typedef enum {
  HS_IDENT_TC = 0x01,
  HS_IDENT_EK = 0x02,
  HS_IDENT_BW = 0x04,
  HS_IDENT_CIV = 0x08,
  HS_IDENT_PIRATE = 0x10,
  HS_ANY_IDENT = 0x1F,
  HS_ENEMIES_TC = HS_IDENT_EK | HS_IDENT_PIRATE,
  HS_ENEMIES_EK = HS_IDENT_TC | HS_IDENT_BW | HS_IDENT_CIV,
  HS_ENEMIES_BW = HS_IDENT_EK,
  HS_ENEMIES_CIV = HS_IDENT_EK | HS_IDENT_PIRATE,
  HS_ENEMIES_PIRATE = HS_IDENT_TC | HS_IDENT_CIV
} hident;

typedef enum {
  HS_PROMPT_FREQUENCY = 0x0F,
  HS_PROMPT_COMBAT = 0x10,
  HS_PROMPT_SPACE = 0x20,
  HS_PROMPT_ALWAYS = 0x40
} hprompt;

typedef enum {
  HS_NODAMAGE = 0,
  HS_MINSHIELD = 1,
  HS_CRITSHIELD = 2,
  HS_MINHULL = 3,
  HS_CRITHULL = 4,
  HS_DISABLESHOT = 5,
  HS_KILLSHOT = 6
} hdamval;

typedef enum {
  HS_PERIODIC = 0x01,
  HS_POST = 0x02,
  HS_PRE = 0x04,
  HS_STACKABLE = 0x08,
  HS_REFRESH = 0x10,
  HS_REMOTE = 0x20,
  HS_SHUNT = 0x40,
  HS_HACK = 0x80,
  HS_DEBUFF = 0x100,
  HS_BUFF = 0x200,
  HS_MISSILE_STANCE = 0x1000,
  HS_EMITTER_STANCE = 0x2000,
  HS_CANNON_STANCE = 0x4000,
  HS_CAPACITOR_STANCE = 0x8000,
  HS_WIRETAP_STANCE = 0x10000,
  HS_BOOSTER_STANCE = 0x20000,
  HS_WEAPON_STANCE = 0x40000,
  HS_ANY_STANCE = 0xFF000,
  HS_NAV_COOLDOWN = 0x100000,
  HS_GUN_COOLDOWN = 0x200000,
  HS_OPS_COOLDOWN = 0x400000,
  HS_ENG_COOLDOWN = 0x800000,
  HS_ANY_COOLDOWN = 0xF00000
} hefct;

typedef enum {
  HS_IDENTIFIED = 0x01,
  HS_UPDATED = 0x02,
  HS_DELETED = 0x04,
  HS_NEW = 0x08,
  HS_NEWTRIGGER = 0x10,
  HS_INSIDE = 0x20
} hcont;

typedef enum {
  /* weapons */
  HS_WEAPON = 0x01,
  HS_CANNON = 0x02,
  HS_MISSILE = 0x04,
  HS_EMITTER = 0x08,
  HS_WIRETAP = 0x10,
  HS_CAPACITOR = 0x20,
  HS_BOOSTER = 0x40,
  HS_ANY_WEAPON = 0xFF,
  /* other flags */
  HS_PRIMARY = 0x100,
  HS_SECONDARY = 0x200,
  HS_PROTOSLOT = 0x400,
  HS_TORPEDO = 0x800
} hslot;

typedef enum {
  HS_RELOAD = 0x1000,
  HS_RANGE = 0x2000,
  HS_DAMAGE = 0x4000,
  HS_ACCURACY = 0x8000,
  HS_POWER = 0x10000,
  HS_CUR_POWER = 0x20000,
  HS_MAX_POWER = 0x40000,
  HS_SPEED = 0x80000
} hwstat;

typedef enum {
  HS_HULL = 0x01,
  HS_SHIELD = 0x02,
  HS_REACTOR = 0x04,
  HS_ENGINE = 0x08,
  HS_SENSOR = 0x10,
  HS_COMPUTER = 0x20,
  HS_SYSTEMS = 0x3F,
  HS_PROTOSYS = 0x40,
  HS_AFTERBURNER = 0x80
} hsys;

typedef enum {
  HS_RATE = 0x1000,
  HS_STRENGTH = 0x2000,
  HS_EFFICIENCY = 0x4000,
  HS_ENERGY = 0x8000,
  HS_MAX_ENERGY = 0x10000,
  /* hull */
  SYS_REPAIR = HS_HULL | HS_RATE,
  SYS_TOUGHNESS = HS_HULL | HS_STRENGTH,
  SYS_ABLATION = HS_HULL | HS_EFFICIENCY,
  SYS_ARMOR = HS_HULL | HS_ENERGY,
  SYS_MAX_ARMOR = HS_HULL | HS_MAX_ENERGY,
  /* shield */
  SYS_REGENERATION = HS_SHIELD | HS_RATE,
  SYS_ABSORPTION = HS_SHIELD | HS_STRENGTH,
  SYS_DEFLECTION = HS_SHIELD | HS_EFFICIENCY,
  SYS_CAPACITY = HS_SHIELD | HS_ENERGY,
  SYS_MAX_CAPACITY = HS_SHIELD | HS_MAX_ENERGY,
  /* engine */
  SYS_VELOCITY = HS_ENGINE | HS_RATE,
  SYS_THRUST = HS_ENGINE | HS_STRENGTH,
  SYS_DISSIPATION = HS_ENGINE | HS_EFFICIENCY,
  SYS_HEAT = HS_ENGINE | HS_ENERGY,
  SYS_MAX_HEAT = HS_ENGINE | HS_MAX_ENERGY,
  /* reactor */
  SYS_RECHARGE = HS_REACTOR | HS_RATE,
  SYS_DISCHARGE = HS_REACTOR | HS_STRENGTH,
  SYS_RESISTANCE = HS_REACTOR | HS_EFFICIENCY,
  SYS_ENERGY = HS_REACTOR | HS_ENERGY,
  SYS_MAX_ENERGY = HS_REACTOR | HS_MAX_ENERGY,
  /* sensor */
  SYS_TRACKING = HS_SENSOR | HS_RATE,
  SYS_RESOLUTION = HS_SENSOR | HS_STRENGTH,
  SYS_SENSITIVITY = HS_SENSOR | HS_EFFICIENCY,
  SYS_FOCUS = HS_SENSOR | HS_ENERGY,
  SYS_MAX_FOCUS = HS_SENSOR | HS_MAX_ENERGY,
  /* computer */
  SYS_DECRYPTION = HS_COMPUTER | HS_RATE,
  SYS_ENCRYPTION = HS_COMPUTER | HS_STRENGTH,
  SYS_COMPRESSION = HS_COMPUTER | HS_EFFICIENCY,
  SYS_MEMORY = HS_COMPUTER | HS_ENERGY,
  SYS_MAX_MEMORY = HS_COMPUTER | HS_MAX_ENERGY
} hstat;

typedef enum {
  HS_NAV = 0x01,
  HS_GUN = 0x02,
  HS_OPS = 0x04,
  HS_ENG = 0x08,
  HS_CIV = 0x10
} hcons;

typedef enum {
  HS_DRONE		= 0x00000001,
  HS_CAPITAL		= 0x00000002,
  HS_STATION		= 0x00000004,
  HS_STAR		= 0x00000008,
  HS_PLANET		= 0x00000010,
  HS_WORMHOLE		= 0x00000020,
  HS_ASTEROID		= 0x00000040,
  HS_ANOMALY		= 0x00000080,
  HS_WAYPOINT		= 0x00000100,
  HS_DEBRIS		= 0x00000200,

  HS_OBJECT_FLAGS	= 0x000003FF,

  HS_SHIP		= 0x00000400,
  HS_ANY_SHIP		= 0x00000407,

  HS_CELESTIAL		= 0x00000800,
  HS_ANY_CELESTIAL	= 0x00000BF8,

  HS_ANY_OBJECT		= 0x00000FFF,

  /* some flags used by ships */
  HS_DISABLED		= 0x00001000,
  HS_DESTROYED		= 0x00002000,
  HS_COMBAT		= 0x00004000,
  HS_AFTERBURNING	= 0x00008000,
  HS_CARGO		= 0x00010000,
  HS_BAY_OPEN		= 0x00020000,
  HS_LINK_EXTENDED	= 0x00040000,
  HS_SENSOR_JAM		= 0x00080000,
  
  /* navigation modes */
  HS_MODE_GOTO		= 0x00100000,
  HS_MODE_INTERCEPT	= 0x00200000,
  HS_MODE_EVADE		= 0x00400000,
  HS_MODE_FORMATION	= 0x00800000,
  HS_MODE_ALLSTOP	= 0x01000000,
  HS_MODE_SLOW		= 0x02000000,
  HS_ANY_MODE		= 0x01F00000,
  
  /* more ship flag space */
  HS_PVP		= 0x10000000,
  
  HS_ALL_BITS		= 0x7FFFFFFF,
} hobj;

typedef enum {
  HS_INTEGER = 0,
  HS_DOUBLE = 1,
  HS_STRING = 2
} hconf;



/* string map for type enumerators */
typedef struct {
  const char *str;
  void *val;
} hstringmap;


/* hweapon */
typedef struct {
  hslot type;		/* hardpoint slot type */

  dbref objnum;		/* object representing the weapon */

  double reload;	/* reload time, seconds between firing */
  double range;		/* maximum range in km */
  double damage;	/* base strength */
  double accuracy;	/* base accuracy of weapon */
  double condition;	/* condition of the weapon */

  /* used by guns, cannons, emitters, and capacitors */
  double power;		/* power discharged per second/per use */
  double curpower;	/* current power remaining */
  double maxpower;	/* maximum weapon buffer power */

  /* used by missiles and wiretaps */
  double speed;		/* projetile velocity (missiles), bandwidth speed (wiretaps) */
  int   loading;	/* seconds remaining until loading */
} hweapon;

/* sensor contact */
typedef struct HCONTACT hcontact;
struct HCONTACT
{
  hobj type;
  void *contact;
  char flags;  
  int cnum;
  double threat;
  hcontact *next;
};

/* system component */
typedef struct HSYSTEM
{
  hslot type;
  
  dbref objnum;
  
  double condition;
  
  double rate;
  double strength;
  double efficiency;
  double energy;	/* available resource power */
  double maxenergy;	/* maximum resource power */
} hsystem;

/* prototyping some of the structs here */
typedef struct HUNIVERSE huniverse;
typedef struct HCELESTIAL hcelestial;
typedef struct HSHIP hship;
typedef struct HBUFF hbuff;
typedef struct HDRONE hdrone;
typedef struct HSPAWN hspawn;
typedef struct HCONSOLE hconsole;

/* ship event callback */
typedef void (*hcallback)(hship *, hconsole *, hcontact *, char *);

/* hconsole */
struct HCONSOLE
{
  hcons type;

  dbref objnum;		/* Dbref number of the console */
  dbref nav;		/* Dbref of this console's navigation console */
  
  char prompt;		/* prompt flags and frequency */

  hcontact  *lock;	/* Contact # that the console is locked on */

  double xyhead;	/* XY heading of the turret */
  double desired_xyhead;
  double zhead;		/* Z heading of the turret */
  double desired_zhead;

  hweapon primary;	/* main item slot */
  hweapon secondary;	/* auxiliary slot */
  
  /* event callbacks */
  hcallback heartbeat;		/* called every cycle */
};

/* The definition for a planet */
struct HCELESTIAL
{

  dbref objnum;

  hobj type;

  huniverse *uid;  	/* The universe it belongs in */

  double x;
  double y;
  double z;

  double radius;
  
  double mass;

  char *contents;
  
  hcelestial *next;
};

/* The standard ship definition */
struct HSHIP {
  dbref objnum;		/* main navigation console */
  
  hobj type;		/* the ship object type, HS_SHIP or HS_DRONE */
  
  int combat_timer;	/* last time the ship dealt or received damage */
  int cid;		/* id of the current combat session */
  char prompt;		/* prompt frequency and flags */
  
  huniverse *uid;	/* The universe it belongs in */

  double vx, vy, vz; /* velocity vector */
  double x;
  double y;
  double z;

  double speed;
  double desired_speed;

  double xyhead;
  double desired_xyhead;
  double zhead;
  double desired_zhead;

  hweapon primary;
  hweapon secondary;

  /* passive systems */
  hsystem hull;
  hsystem shield;

  /* active systems */
  hsystem engine;
  hsystem sensor;
  hsystem computer;
  hsystem reactor;

  int nconsoles;
  hconsole console[MAX_CONSOLES];
  
  int ncontacts;
  hcontact *head_contact;
  hcontact *lock;
  
  hcontact *wp_contact;
  hcelestial waypoint;
  
  /* master control ship for combined sensor telemetry */
  hship *mothership;

  int nbuffs;
  hbuff *head_buff;

  hcelestial *landed;
  hship *docked;
  hship *linked;

  int landing;		/* 0 if not landing, time left otherwise */
  int launching;		/* same, for docking */
  dbref dropto;
  
  hship *next;		/* next ship in the universes active ship list */
  hspawn *spawn;	/* the spawn that controls a drone */

  /* event callbacks */
  hcallback heartbeat;		/* called every cycle */
  hcallback update_prox;	/* called when entering/leaving hazards */
  hcallback update_contact;	/* called when a new contact is found */
};


/*
 * Storage variables for all configuration options.  This is one
 * mother of a struct.
 */
typedef struct HCONFIG
{
  int 		autostart;
  dbref 	space_wiz;
  int           forbid_puppets;
  int		cyc_interval;
  int		idle_timeout;
  
  double	burn_multiplier;

  int    	max_universes;
  int		max_celestials;
  int		max_ships;
  int		max_missions;

  int		max_comm_dist;
  int		max_dock_dist;
  int		max_board_dist;
  int		max_dock_size;
  int		max_cargo_dist;
  int		max_land_dist;
  int		max_gate_dist;
  int		max_waypoint_dist;

  /* Messages */
  char		speed_increase[256];
  char		speed_decrease[256];

  char		engine_reverse[256];
  char          engine_forward[256];

  char          speed_halt[256];

  char          ship_is_docking[256];
  char          ship_is_docked[256];
  char          ship_is_undocking[256];

  char		lift_off[256];
  char		engines_activating[256];

  char		landing_msg[256];
  char          begin_descent[256];

  char		docking_msg[256];
  char          begin_docking[256];

  char		wormhole_travel[256];
  
  char		engage_afterburner[256];
  char		disengage_afterburner[256];

  char		notify_engage[256];
  char		notify_disengage[256];
} hconfig;

typedef struct HCONFIGITEM {
  const char *name;
  hconf type;
  void *value;
} hconfigitem;


/* A dynamic universe */
struct HUNIVERSE
{
  dbref objnum;		/* universe object */

  int pvp;

  int num_ships;
  int num_drones;
  int num_celestials;
  
  hship *head_ship;
  hship *head_drone;
  hcelestial *head_celestial;
  hcelestial *head_debris;
};



/* buff callback */
typedef void (*hbuff_callback)(hship *,hbuff *);

/* buff effect definition */
typedef struct {
  hefct flags;
  
  int duration;
  int frequency;

  double cost;
  
  hbuff_callback pre;
  hbuff_callback beat;
  hbuff_callback post;
  
  hweapon gun;
  hsystem sys;
  
} hbuff_effect;

/* buff list container */
struct HBUFF {
  hbuff_effect *buff;
  hbuff *next;
  hship *owner;
  int stacks;
  int duration;
  
  double var1;
  double var2;
  double var3;
  double var4;
  double var5;
};

/* drone spawns */
typedef struct HMISSION hmission;

struct HSPAWN {
  dbref objnum;

  hmission *mission;

  /* time in seconds to respawn drones */
  int respawn;
  int last_respawn;
  
  huniverse *uid;
  
  double x;
  double y;
  double z;
  
  int ndrones;
  hship **drones;
};

/* missions */
struct HMISSION {
  dbref objnum;
  
  hspawn *spawns;
  int nspawns;
};



#endif
