#include <libq/config.h>#include <stdlib.h>#include <stdio.h>#include <string.h>#include <math.h>#include "horoscope.h"/*#include <android/log.h>#define APPNAME "sphinx"#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, APPNAME, __VA_ARGS__))*/const char *pattnam[] = { "stell","tsquare","gsquare","gcross","gtrine","yod","mystr","kite","gquint","hexa" };const char *plnam[] = { "","sun","mon","mer","ven","mar","jup","sat","ura","nep","plu","nnd","tnn","snd","tsn","chi","cer","pal","jun","ves","ver","eas","for","asc","mc" };const char *aspnam[] = { "conj","semisex","dec","nov","semisq","sep","sex","quin","squ","tri","sesq","quin","opp" };typedef struct pattern_asp {	int p1;	int p2;	int asp;} pattern_asp;typedef struct pattern {	int type;	int npl;	int nasp;	pattern_asp *asp;} pattern;pattern_asp tsquare_asp[] = {	{0,1,ASTRO_SQUARE},{0,2,ASTRO_SQUARE},{1,2,ASTRO_OPPOSITION}};pattern_asp gsquare_asp[] = {	{0,1,ASTRO_SQUARE},{0,2,ASTRO_SQUARE},{1,3,ASTRO_SQUARE},{2,3,ASTRO_SQUARE}};pattern_asp gcross_asp[] = {	{0,1,ASTRO_OPPOSITION},{0,2,ASTRO_SQUARE},{0,3,ASTRO_SQUARE},{2,3,ASTRO_OPPOSITION},{1,2,ASTRO_SQUARE},{1,3,ASTRO_SQUARE}};pattern_asp gtrine_asp[] = {	{0,1,ASTRO_TRINE},{0,2,ASTRO_TRINE},{1,2,ASTRO_TRINE}};pattern_asp yod_asp[] = {	{0,1,ASTRO_QUINCUNX},{0,2,ASTRO_QUINCUNX},{1,2,ASTRO_SEXTILE}};pattern_asp kite_asp[] = {	{0,1,ASTRO_OPPOSITION},{0,2,ASTRO_TRINE},{0,3,ASTRO_TRINE},{2,3,ASTRO_TRINE},{1,2,ASTRO_SEXTILE},{1,3,ASTRO_SEXTILE}};pattern_asp penta_asp[] = {	{0,1,ASTRO_QUINTILE},{0,2,ASTRO_QUINTILE},{1,3,ASTRO_QUINTILE},{2,4,ASTRO_QUINTILE},{3,4,ASTRO_QUINTILE}};pattern_asp hexagram_asp[] = {	{0,1,ASTRO_SEXTILE},{0,2,ASTRO_TRINE},{0,3,ASTRO_SEXTILE},{0,4,ASTRO_TRINE},	{1,2,ASTRO_SEXTILE},{1,3,ASTRO_TRINE},{1,5,ASTRO_TRINE},{2,4,ASTRO_TRINE},	{2,5,ASTRO_SEXTILE},{3,4,ASTRO_SEXTILE},{3,5,ASTRO_TRINE},{4,5,ASTRO_SEXTILE}};/*	ASTRO_STELLIUM,	ASTRO_TSQUARE,	ASTRO_GSQUARE,	ASTRO_GCROSS,	ASTRO_GTRINE,	ASTRO_YOD,	ASTRO_MYSTRECT,	ASTRO_KITE,	ASTRO_GQUINTILE,	ASTRO_HEXAGRAM,*/pattern patterns[] = {	{ ASTRO_STELLIUM,0,0,NULL }, // Is calculated by separate algorithm.	{ ASTRO_TSQUARE,3,3,tsquare_asp },	{ ASTRO_GSQUARE,4,4,gsquare_asp },	{ ASTRO_GCROSS,4,6,gcross_asp },	{ ASTRO_GTRINE,3,3,gtrine_asp },	{ ASTRO_YOD,3,3,yod_asp },	{ ASTRO_MYSTRECT,0,0,NULL },	{ ASTRO_KITE,4,6,kite_asp },	{ ASTRO_GQUINTILE,5,5,penta_asp },	{ ASTRO_HEXAGRAM,6,12,hexagram_asp },};static void clear(horoscope *h) {	if(h->planets!=NULL) free(h->planets);	if(h->aspects!=NULL) free(h->aspects);	if(h->asppat!=NULL) {		int i;		for(i=0; i<h->nasppat; ++i) free(h->asppat[i].planets);		free(h->asppat);	}	h->nplanets  = 0;	h->planets   = NULL;	h->naspects  = 0;	h->aspects   = NULL;	h->nasppat   = 0;	h->fasppat   = 0;	h->asppat    = NULL;}horoscope *horoscope_new(const char *name,double lon,double lat,double tz) {	horoscope *h1 = (horoscope *)malloc(sizeof(horoscope));	memset(h1,0,sizeof(horoscope));	if(name!=NULL) h1->name = strdup(name);	h1->lon     = lon;	h1->lat     = lat;	h1->tz      = tz;	h1->isun    = -1;	h1->imoon   = -1;	h1->imer    = -1;	h1->iven    = -1;	h1->imar    = -1;	h1->ijup    = -1;	h1->isat    = -1;	h1->iura    = -1;	h1->inep    = -1;	h1->iplu    = -1;	h1->iasc    = -1;	h1->imc     = -1;	h1->irulpl  = -1;	h1->irulh   = -1;	return h1;}void horoscope_delete(horoscope *h) {	if(h->name!=NULL) free(h->name);	clear(h);	free(h);}void horoscope_set_time(horoscope *h,int y,int m,int d,int t,int n,double s,double dst,int greg) {	calendar_set(&h->time,y,m,d,t,n,s,greg);	calendar_set_gmt(&h->gmt,y,m,d,t,n,s,h->tz,dst,greg);	h->dst = dst;}void horoscope_set_julian_day(horoscope *h,double jd,int greg) {	calendar_set_julian_day(&h->time,jd,greg);	calendar_set_julian_day(&h->gmt,jd,greg);	h->dst = 0.0;}static void planets_in_houses(horoscope *h) {	int i,j,flag360;	double cusp1,cusp2,temp;	for(i=0; i<12; ++i) {		flag360 = 0;		cusp1 = h->houses[i].cusp;		if(i==11) cusp2 = h->houses[0].cusp;		else cusp2 = h->houses[i+1].cusp;		if(cusp1>cusp2) {			cusp1 = mod360(cusp1+180);			cusp2 = mod360(cusp2+180);			flag360 = 1;		}		for(j=0; j<h->nplanets; ++j) {			if(!flag360) temp = h->planets[j].lon;			else temp = mod360(h->planets[j].lon+180);			if(temp>=cusp1 && temp<cusp2) h->planets[j].house = i;		}	}}static void write_mutual_receptive(horoscope *h) {	int i,j,p1,p2;	for(i=ASTRO_SUN; i<=ASTRO_PLUTO; ++i) {		p1 = h->iplanets[i];		if(p1!=-1 && !(h->planets[i].factors&ASTRO_MRECEPTIVE)) {			j = astro_ruling_planets[h->planets[p1].sign];			p2 = h->iplanets[j];			if(i!=j && p2!=-1 && astro_ruling_planets[h->planets[p2].sign]==i) {				h->planets[i].factors |= ASTRO_MRECEPTIVE;				h->planets[j].factors |= ASTRO_MRECEPTIVE;			}		}	}}static void write_aspects(horoscope *h) {	int x,y,n = h->nplanets,i,s1,s2,pi,pi1,pi2;	double p,asp,r,orb,*ao;	int sign = h->style&HOROSCOPE_ASPECT_IN_SIGN;	int ruler = h->iasc>-1? astro_ruling_planets[h->planets[h->iasc].sign] : -1;	int rsun = h->isun>-1? astro_ruling_planets[h->planets[h->isun].sign] : -1;	int rmoon = h->imoon>-1? astro_ruling_planets[h->planets[h->imoon].sign] : -1;	astro_planet *p1,*p2;	astro_aspect *a;	h->naspects = 0;	for(i=n*n-1; i>=0; --i) h->aspects[i].type = -1;	for(x=0; x<n; ++x) {		for(y=x; y<n; ++y) {			if(x!=y) {				p1 = &h->planets[x];				p2 = &h->planets[y];				p = astro_angle(p1->lon,p2->lon);				if(p>180) p = 360-p;				if(x==ruler || y==ruler) pi = 3;				else if(x==rsun || y==rsun || x==rmoon || y==rmoon) pi = 4;				else pi = 9;				pi1 = astro_aspect_orb_planet_index[p1->id&0xff];				pi2 = astro_aspect_orb_planet_index[p2->id&0xff];				s1 = abs(p1->sign-p2->sign);				a = &h->aspects[x+y*n];				a->type = -1;				for(i=ASTRO_CONJUNCTION; i<=ASTRO_OPPOSITION; ++i) {					r = astro_aspect_ratios[i];					ao = (double *)&astro_aspect_orbs[i*ASTRO_ASPECT_ORB_LEVELS];					orb = ao[pi];					if(orb>=0.0 && ao[pi1]>orb) orb = ao[pi1];					if(orb>=0.0 && ao[pi2]>orb) orb = ao[pi2];					if(orb>=0.0 && p>=r-orb && p<=r+orb) {						s2 = astro_aspect_signs[i];						if(!sign || s2==-1 || s1==s2 || s1==ASTRO_NUM_ZODIAC-s2) {//printf("Aspect: %-4s/%-4s   %-8s  [%2d %2d %2d %2d]   p=%6.2f  r=%6.2f  orb=%6.2f  r-orb=%6.2f  r+orb=%6.2f  p-r=%6.2f\n",plnam[p1->id],plnam[p2->id],aspnam[i],i,pi,pi1,pi2,p,r,orb,r-orb,r+orb,p-r);							a->type = i;							break;						}					}				}				if(a->type>=0) a->orb = p-r,++h->naspects;				else a->orb = p;				memcpy(&h->aspects[y+x*n],a,sizeof(astro_aspect));			}		}	}	if(h->iasc!=-1) for(x=0; x<n; ++x) {		asp = h->aspects[x+h->iasc*n].type;		if(asp==ASTRO_CONJUNCTION)			h->planets[x].factors |= ASTRO_ASCENDING;		if(asp==ASTRO_CONJUNCTION || asp==ASTRO_OPPOSITION)			h->planets[x].factors |= ASTRO_ANGULAR;	}}void print_match(horoscope *h,int pi,int p1,int p2,int asp) {	int z;	printf("\n");	for(z=0; z<pi; ++z) printf("   ");	printf("match[ %s %s %s ]",plnam[h->planets[p1].id],p2>=0? plnam[h->planets[p2].id] : "---",aspnam[asp]);}static int pattern_exist(horoscope *h,int type,int *pl,int npl) {	if(h->nasppat>0) {		astro_pattern *ap;		int i,j,n = (h->nplanets/32)+((h->nplanets%32)>0? 1 : 0);		uint32_t p[n];		memset(p,0,sizeof(uint32_t)*n);		for(i=0; i<npl; ++i) p[pl[i]/32] |= 1<<(pl[i]%32);		for(i=0; i<h->nasppat; ++i) {			ap = &h->asppat[i];			if(type!=ap->type) continue;			for(j=0; j<n; ++j) if(ap->planets[j]!=p[j]) break;			if(j==n) {//printf("Pattern already exist!\n");				return 1;			}		}	}	return 0;}static int add_pattern(horoscope *h,int type,int *pl,int npl) {	if(!pattern_exist(h,type,pl,npl)) {		int i;		astro_pattern *ap;		++h->nasppat;		h->fasppat |= 1<<type;		if(h->asppat==NULL) h->asppat = (astro_pattern *)malloc(sizeof(astro_pattern)*h->nasppat);		else h->asppat = (astro_pattern *)realloc(h->asppat,sizeof(astro_pattern)*h->nasppat);		ap = &h->asppat[h->nasppat-1];		ap->type = type;		ap->nplanets = (h->nplanets/32)+((h->nplanets%32)>0? 1 : 0);		ap->planets = (uint32_t *)malloc(sizeof(uint32_t)*h->nplanets);		memset(ap->planets,0,sizeof(uint32_t)*h->nplanets);		for(i=0; i<npl; ++i) ap->planets[pl[i]/32] |= 1<<(pl[i]%32);		return 1;	}	return 0;}static void find_pattern(horoscope *h,int i,int n,int *a,int pi,pattern *pt,pattern_asp *pa,int *pl) {	int x = pl[pa[pi].p1],y,z,p2 = pl[pa[pi].p2],pp,asp = pa[pi].asp;//print_match(h,pi,x,p2,asp);	if(p2!=-1) {		y = p2;		if(asp==a[x+y*n]) {			pp = pl[pa[pi].p2],pl[pa[pi].p2] = y;			if(pi==pt->nasp-1) add_pattern(h,pt->type,pl,pt->npl);								else find_pattern(h,i,n,a,pi+1,pt,pa,pl);			pl[pa[pi].p2] = pp;		}	} else for(y=0; y<n; ++y)		if(x!=y && asp==a[x+y*n]) {			for(z=pa[pi].p2-1; z>=0; --z) if(y==pl[z]) break;			if(z>=0) continue;//printf("   %s %s",plnam[h->planets[y].id],aspnam[a[x+y*n]]);			pp = pl[pa[pi].p2],pl[pa[pi].p2] = y;			if(pi==pt->nasp-1) add_pattern(h,pt->type,pl,pt->npl);								else find_pattern(h,i,n,a,pi+1,pt,pa,pl);			pl[pa[pi].p2] = pp;		}}static void write_patterns(horoscope *h) {	pattern_asp *pa;	int i,n = h->nplanets,x,y;	int a[n*n];	for(i=n*n-1; i>=0; --i) a[i] = h->aspects[i].type;	// Find stelliums:	{		int pl[n];		for(x=0; x<n; ++x) {			memset(pl,0,sizeof(int)*n);			pl[0] = x,i = 1;			for(y=0; y<n; ++y)				if(x!=y) {					if(h->planets[x].sign==h->planets[y].sign && h->planets[x].house==h->planets[y].house) {						pl[i++] = y;					}				}			if(i>2) add_pattern(h,ASTRO_STELLIUM,pl,i);		}	}	// Find other patterns:	for(i=0; i<ASTRO_NUM_PATTERNS; ++i)		if(patterns[i].asp!=NULL) {			int pl[patterns[i].npl];			for(x=patterns[i].npl-1; x>=0; --x) pl[x] = -1;//printf("\n\nTest pattern: %s",pattnam[i]);			pa = patterns[i].asp;			for(x=0; x<n; ++x)				for(y=0; y<n; ++y)					if(x!=y && pa->asp==a[x+y*n]) {//print_match(h,0,x,y,pa->asp);						pl[pa->p1] = x,pl[pa->p2] = y;						find_pattern(h,i,n,a,1,&patterns[i],pa,pl);						pl[pa->p1] = -1,pl[pa->p2] = -1;					}		}//printf("\n\n");}void horoscope_cast(horoscope *h,astronomy *a,int st,int hsys) {	int i;	double t = (h->gmt.jd-J1900)/36525.0;	double g = (h->gmt.jd+0.5-(double)((int)(h->gmt.jd+0.5)))*24;	astro_planet *p1;	astronomy_planet *p2;	clear(h);	h->style    = st;	h->hsystem  = hsys;	h->deltat   = a->deltat;	h->sdrlt    = a->sdrlt;	h->ra = mod360((6.6460656+(2400.0513*t)+(2.58E-05*t*t)+g)*15-h->lon)*DTR;	h->ob = (23.452294-0.0130125*t)*DTR;	h->la = h->lat*DTR;//printf("gmt=%lf\nra=%lf\nob=%lf\nla=%lf\nsdrlt=%lf\n",g,h->ra,h->ob,h->la,a->sdrlt*24.);	switch(h->hsystem) {		case ASTRO_PLACIDUS:astro_placidus(h->houses,h->ra,h->ob,h->la);break;		case ASTRO_KOCH:astro_koch(h->houses,h->ra,h->ob,h->la);break;	}//printf("asc=%lf\n",h->houses[0].cusp);	h->nplanets = a->nplanets;	h->planets = (astro_planet *)malloc(sizeof(astro_planet)*h->nplanets);	memset(h->planets,0,sizeof(astro_planet)*h->nplanets);	for(i=0; i<=ASTRO_MC; ++i) h->iplanets[i] = -1;	for(i=0; i<h->nplanets; ++i) {		p1 = &h->planets[i];		p2 = &a->planets[i];		p1->id = p2->id;		p1->lon = h->style==HOROSCOPE_HELIOCENTRIC? p2->hlon : p2->glon;		if(p1->id>=ASTRO_SUN && p1->id<=ASTRO_MC) {			h->iplanets[p1->id] = i;			switch(p1->id) {				case ASTRO_SUN:        h->isun   = i;break;				case ASTRO_MOON:       h->imoon  = i;break;				case ASTRO_MERCURY:    h->imer   = i;break;				case ASTRO_VENUS:      h->iven   = i;break;				case ASTRO_MARS:       h->imar   = i;break;				case ASTRO_JUPITER:    h->ijup   = i;break;				case ASTRO_SATURN:     h->isat   = i;break;				case ASTRO_URANUS:     h->iura   = i;break;				case ASTRO_NEPTUNE:    h->inep   = i;break;				case ASTRO_PLUTO:      h->iplu   = i;break;				case ASTRO_ASCENDANT:  h->iasc   = i;break;				case ASTRO_MC:         h->imc    = i;break;			}		}	}	for(i=0; i<h->nplanets; ++i) {		p1 = &h->planets[i];		switch(p1->id) {			case ASTRO_VERTEX:p1->lon = astro_vertex(h->ra,h->ob,h->la);break;			case ASTRO_EASTPOINT:p1->lon = astro_eastpoint(h->ra,h->ob);break;			case ASTRO_FORTUNE:				p1->lon = h->imoon!=-1 && h->isun!=-1?						astro_fortune(h->houses[0].cusp,h->planets[h->imoon].lon,h->planets[h->isun].lon) : 0.0;				break;			case ASTRO_ASCENDANT:p1->lon = h->houses[0].cusp;break;			case ASTRO_MC:p1->lon = h->houses[9].cusp;break;		}	}	h->aspects = (astro_aspect *)malloc(sizeof(astro_aspect)*h->nplanets*h->nplanets);	h->asppat = NULL;	h->aspshp = -1;	planets_in_houses(h);	for(i=0; i<h->nplanets; ++i) h->planets[i].sign = astro_zodiac(h->planets[i].lon);	for(i=0; i<12; ++i) h->houses[i].sign = astro_zodiac(h->houses[i].cusp);	h->irulpl = h->iplanets[astro_ruling_planets[h->houses[0].sign]];	h->irulh = h->irulpl!=-1? h->planets[h->irulpl].house : 0;	write_mutual_receptive(h);	write_aspects(h);	write_patterns(h);}int horoscope_in_aspect_pattern(horoscope *h,int p1,int p2,int a) {	if(p1!=p2 && p1>=0 && p1<h->nplanets && p2>=0 && p2<h->nplanets) {		int i;		astro_pattern *ap;		for(i=0,ap=h->asppat; i<h->nasppat; ++i,++ap)			if((a<0 || ap->type==a) &&					(ap->planets[p1/32]&(1<<(p1%32))) &&					(ap->planets[p2/32]&(1<<(p2%32)))) return i;	}	return -1;}void horoscope_elements(horoscope *h,int e[ASTRO_NUM_ELEMENTS]) {	int i;	for(i=0; i<ASTRO_NUM_ELEMENTS; ++i) e[i] = 0;	for(i=0; i<h->nplanets; ++i) ++e[astro_element[h->planets[i].sign]];}void horoscope_qualities(horoscope *h,int q[ASTRO_NUM_QUALITIES]) {	int i;	for(i=0; i<ASTRO_NUM_QUALITIES; ++i) q[i] = 0;	for(i=0; i<h->nplanets; ++i) ++q[astro_quality[h->planets[i].sign]];}void horoscope_energies(horoscope *h,int e[ASTRO_NUM_ENERGIES]) {	int i;	for(i=0; i<ASTRO_NUM_ENERGIES; ++i) e[i] = 0;	for(i=0; i<h->nplanets; ++i) ++e[astro_energy[h->planets[i].sign]];}