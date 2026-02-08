/* Modified WEEKS.H with dielectric support for MWEEKS
 * Added support for FR4 and other dielectric materials
 */

typedef struct {
    double w;              /* width */
    double h;              /* height */
    double x;              /* x position */
    double y;              /* y position */
    double b;              /* mesh density parameter */
    int nw;                /* number of width divisions */
    int nh;                /* number of height divisions */
    int n;                 /* total number of elements (nw * nh) */
    
    /* Dielectric properties (new) */
    double er;             /* relative permittivity (dielectric constant) */
    double substrate_h;    /* substrate height (distance to ground plane) */
    double tan_delta;      /* loss tangent (dielectric loss) */
} conductor;

conductor *getinput (FILE *, int *);

typedef struct {
    double x1, x2, y1, y2;
} element;

element *build_elements (int, int, conductor *, element *);
double lp (element *, element *);

/* New functions for dielectric calculations */
double calc_eff_dielectric(double w, double h, double er);
double calc_dielectric_loss(double er, double tan_delta, 
                            double Omega, double w, double h);

/* Common dielectric materials (for reference) */
#define ER_AIR         1.0
#define ER_FR4         4.4      /* Typical FR4 at low frequencies */
#define ER_FR4_MIN     4.3      /* FR4 range minimum */
#define ER_FR4_MAX     4.7      /* FR4 range maximum */
#define ER_ROGERS4003  3.38     /* Rogers RO4003C */
#define ER_ROGERS4350  3.48     /* Rogers RO4350B */
#define ER_ALUMINA     9.8      /* Alumina ceramic */
#define ER_PTFE        2.1      /* PTFE (Teflon) */

#define TAN_DELTA_FR4      0.02    /* FR4 loss tangent at 1 GHz */
#define TAN_DELTA_ROGERS   0.0027  /* Rogers 4003C */
#define TAN_DELTA_ALUMINA  0.001   /* Alumina */
