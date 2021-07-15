#ifndef _LIBQ_ASTRO_ASTRONOMY_DATA_H_
#define _LIBQ_ASTRO_ASTRONOMY_DATA_H_

#include "astronomy.h"

#define WILLIAMS 1
#define SIMON 0
#define IAU 0
#define LASKAR 0


#define DISFAC 2.3454780e4						// Earth radii per au
#define COSSUN -0.014543897651582657		// cosine of 90 degrees 50 minutes
#define COSZEN -9.8900378587411476e-3		// cosine of 90 degrees 34 minutes

#define TABSTART 1620
#define TABEND 2013
#define TABSIZ (TABEND-TABSTART+1)

#define NARGS 18

#define NCON 89
#define NGREEK 24
#define NBNDRIES 357


extern const double A[3];
extern const double Ad[3];
extern const double matrix[36];
extern const int nutation_table[105*9];
extern const double freqs[9];
extern const double phases[9];
extern const int delta_table[TABSIZ];
extern const double pAcof[10];
extern const double nodecof[11];
extern const double inclcof[11];
extern const char *constel[NCON];
extern const char *constel_abbr[NCON];
extern const char *greek[NGREEK];
extern const int bndries[4*NBNDRIES];

typedef struct _planet_table planet_table;
typedef struct _astro_orbit astro_orbit;
typedef struct _astro_star astro_star;
typedef struct _astro_data astro_data;
typedef struct _astro_orbit_data astro_orbit_data;
typedef struct _astro_fixedstar_data astro_fixedstar_data;


struct _planet_table {
	int maxargs;
	int max_harmonic[NARGS];
	int max_power_of_t;
	signed char *arg_tbl;
	void *lon_tbl;
	void *lat_tbl;
	void *rad_tbl;
	double distance;
	double timescale;
	double trunclvl;
};


extern planet_table mer404,ven404,ear404,mar404;
extern planet_table jup404,sat404,ura404,nep404,plu404;


struct _astro_orbit {
	double epoch;
	double i;
	double W;
	double w;
	double a;
	double dm;
	double ecc;
	double M;
	double equinox;
	double mag;
	double sdiam;
	double L;
	double r;
	double plat;
	void *ptable;
};

struct _astro_star {
	double epoch;
	double ra;
	double dec;
	double px;
	double mura;
	double mudec;
	double v;
	double equinox;
	double mag;		
};


struct _astro_data {
	astronomy *a;            //!< Astronomy object   
	int data;                //!< Data to calculate. The data-parameter in astronomy_get_planets().

	double JD;               //!< Julian Day
	double TDT;              //!< 
	double UT;               //!< Universal Time

	double robject[3];       //!< Position
	double obpolar[3];       //!< Ecliptic polar coordinates: longitude, latitude in radians, radius in au
	double rearth[3];        //!< Coordinates of Earth: Heliocentric rectangular equatorial position of the earth at time TDT re equinox J2000
	double eapolar[3];       //!< Corresponding polar coordinates of earth: longitude and latitude in radians, radius in au
	double vearth[3];        //!< Estimated velocity vector of the Earth.
	double dradt;            //!< Approximate motion of right ascension and declination of object,in radians per day
	double ddecdt;           //!< 
	double jdnut;            //!< Time to which the nutation applies
	double nutl;             //!< Nutation in longitude(radians)
	double nuto;             //!< Nutation in obliquity(radians)
	double jdeps;            //!< 
	double eps;              //!< 
	double coseps;           //!< 
	double sineps;           //!< 

	double SE;               //!< Earth-sun distance
	double SO;               //!< Object-sun distance
	double EO;               //!< Object-earth distance
	double pq;               //!< Cosine of sun-object-earth angle
	double ep;               //!< -Cosine of sun-earth-object angle
	double qe;               //!< Cosine of earth-sun-object angle

	astro_star fstar;        //!< Space for star description read from a disc file.
	astro_orbit forbit;      //!< Space for orbit read from a disc file.
	astro_orbit *elobj;      //!< Pointer to orbital elements of object.
	astronomy_planet *wrpl;  //!< 

	double ra;               //!< Right ascension.
	double dec;              //!< Declination.
	double jvearth;
};

extern astro_orbit astro_mercury;
extern astro_orbit astro_venus;
extern astro_orbit astro_earth;
extern astro_orbit astro_mars;
extern astro_orbit astro_jupiter;
extern astro_orbit astro_saturn;
extern astro_orbit astro_uranus;
extern astro_orbit astro_neptune;
extern astro_orbit astro_pluto;


struct _astro_orbit_data {
	int year;
	int month;
	double day;
	double M;
	double a;
	double ecc;
	double w;
	double W;
	double i;
	double mag;
};

extern const astro_orbit_data astro_orbits_data[];


struct _astro_fixedstar_data {
	int epoch;
	int rh;
	int rm;
	double rs;
	int dd;
	int dm;
	double ds;
	double mura;
	double mudec;
	double v;
	double px;
	double mag;
};

extern const astro_fixedstar_data astro_fixedstars_data[];


#endif /* _LIBQ_ASTRO_ASTRONOMY_DATA_H_ */


