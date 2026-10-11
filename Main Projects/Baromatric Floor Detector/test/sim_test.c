#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*
 * sim_test.c - tests the floor-tracking algorithm on a PC (no ESP32 needed)
 *
 * Simulates a trip through a building: 2 floors up by stairs, elevator 3
 * floors down, long waits - with BMP280-like noise and a weather-driven
 * pressure drift. Checks every floor change and the stairs/elevator labels.
 *
 *   make            (in this folder)
 *   ./sim_test [noise_pa] [drift_hpa_per_hour]     defaults: 1.5 0.3
 */
#include "altitude.h"
static double gauss(void){double u=(rand()+1.0)/(RAND_MAX+2.0),v=(rand()+1.0)/(RAND_MAX+2.0);return sqrt(-2*log(u))*cos(2*M_PI*v);}
/* true height profile */
typedef struct{double dur,target,speed;const char*label;}seg_t;
int main(int argc,char**argv){
  double noise_pa = argc>1?atof(argv[1]):1.5, drift_hpa_h = argc>2?atof(argv[2]):0.3;
  srand(42);
  seg_t segs[]={
   {60,0,0,"still F0"},
   {0,3,0.4,"stairs up"},{5,3,0,"landing"},{0,6,0.4,"stairs up"},
   {120,6,0,"still F2"},
   {0,-3,1.2,"elevator down"},
   {600,-3,0,"still F-1 (drift)"},
   {0,0,0.4,"stairs up"},
   {1800,0,0,"still F0 30min (drift)"},
  };
  int nseg=sizeof segs/sizeof segs[0];
  altitude_ctx_t c; altitude_init(&c,3.0f);
  double dt=0.05,t=0,h=0,base_alt=250.0; /* building at 250 m */
  int last_floor=0,changes=0,warm=1; double worst_still_err=0;
  for(int s=0;s<nseg;s++){
    seg_t g=segs[s]; int lbl[5]={0};
    double seg_t0=t;
    while(1){
      if(g.speed>0){ double d=g.target-h; if(fabs(d)<1e-6) break; double step=g.speed*dt; h+= (fabs(d)<step)?d:(d>0?step:-step);}
      else if(t-seg_t0>=g.dur) break;
      double p0=101325.0 + drift_hpa_h*100.0*(t/3600.0);
      double p=p0*pow(1-(base_alt+h)/44330.0,5.255)+noise_pa*gauss();
      altitude_update(&c,(float)p,(float)dt); t+=dt;
      if(warm && t>3.0){altitude_zero(&c);warm=0;}
      if(c.floor!=last_floor){printf("  t=%7.1fs  floor %d -> %d  (true h=%.2f m, est rel=%.2f, %s)\n",t,last_floor,c.floor,h,altitude_relative(&c),altitude_motion_str(c.motion));last_floor=c.floor;changes++;}
      lbl[c.motion]++;
      if(g.speed==0 && t-seg_t0>10){double e=fabs(altitude_relative(&c)-h); if(e>worst_still_err)worst_still_err=e;}
    }
    printf("[%-24s] end t=%7.1f  floor=%d  rel=%.2f  labels: still=%d sUp=%d sDn=%d eUp=%d eDn=%d\n",g.label,t,c.floor,altitude_relative(&c),lbl[0],lbl[1],lbl[2],lbl[3],lbl[4]);
  }
  printf("floor changes: %d (expected 6)   worst height error while still: %.2f m\n",changes,worst_still_err);
  int pass = (changes==6) && (c.floor==0) && (worst_still_err < 1.0);
  printf("%s\n", pass ? "PASS" : "FAIL");
  return pass ? 0 : 1;
}
