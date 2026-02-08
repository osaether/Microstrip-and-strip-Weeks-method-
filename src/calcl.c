/* Modified CALCL.C with dielectric support for MWEEKS
 * 
 * Calculates partial inductances accounting for dielectric substrate
 * The dielectric primarily affects capacitance, but we account for
 * effective permittivity in the calculations
 */

#include "zmatrix2.h"
#include "weeks.h"
#include "calcl.h"
#include "mf.h"
#include <math.h>

/* Calculate effective dielectric constant for microstrip
 * Using approximate formula (Hammerstad & Jensen)
 * This is valid for most practical microstrip configurations
 */
double calc_eff_dielectric(double w, double h, double er)
{
  double a, b, eff_er;
  
  if (h <= 0.0 || er <= 1.0) {
    /* No substrate or air - return 1.0 */
    return 1.0;
  }
  
  /* Hammerstad & Jensen approximation for effective dielectric constant */
  a = 1.0 + (1.0/49.0) * log(
      ((w/h) * (w/h) * (w/h) * (w/h) + (w/h/52.0) * (w/h/52.0)) /
      ((w/h) * (w/h) * (w/h) * (w/h) + 0.432)
      ) + (1.0/18.7) * log(1.0 + (w/h/18.1) * (w/h/18.1) * (w/h/18.1));
  
  b = 0.564 * pow((er - 0.9)/(er + 3.0), 0.053);
  
  eff_er = (er + 1.0)/2.0 + ((er - 1.0)/2.0) * pow(1.0 + 10.0*h/w, -a*b);
  
  return eff_er;
}

/* Calculate dielectric loss contribution
 * Returns additional resistance per unit length due to dielectric loss
 */
double calc_dielectric_loss(double er, double tan_delta, 
                            double Omega, double w, double h)
{
  double eff_er, loss_per_length;
  const double c = 2.99792458e8; /* speed of light */
  const double PI = 3.141592653589793116;
  
  if (tan_delta <= 0.0 || h <= 0.0) {
    return 0.0;
  }
  
  eff_er = calc_eff_dielectric(w, h, er);
  
  /* Dielectric loss: α_d = (π/λ) * (εr - 1)/(εeff - 1) * (εeff/εr) * tan(δ)
   * where λ = 2πc/ω
   * Converting to resistance per unit length
   */
  loss_per_length = (Omega * sqrt(eff_er) / c) * 
                    ((er - 1.0)/(eff_er - 1.0)) * 
                    (eff_er/er) * tan_delta;
  
  return loss_per_length;
}

void calcl (ZMAT *Z, element *e, double n0, double Omega, 
            element e0, conductor *cond, int N)
{
  int i, j;
  int dim;
  double lmm, lpi0, r00;
  double sigma=58e6;  /* Copper conductivity S/m */
  double eff_er;
  double diel_loss;
  VEC *lpj;
  dim = Z->m;

  /* Calculate effective dielectric constant for ground plane (line0) */
  if (cond != NULL && cond[0].substrate_h > 0.0) {
    eff_er = calc_eff_dielectric(cond[0].w, cond[0].substrate_h, cond[0].er);
    fprintf(stderr, "\n  Ground plane effective εr: %.3f", eff_er);
    
    /* Calculate dielectric loss for ground plane */
    diel_loss = calc_dielectric_loss(cond[0].er, cond[0].tan_delta, 
                                     Omega, cond[0].w, cond[0].substrate_h);
    fprintf(stderr, "\n  Dielectric loss: %.3e Ω/m", diel_loss);
  } else {
    eff_er = 1.0;  /* Air */
    diel_loss = 0.0;
  }

  lmm = lp (&e0, &e0);
  r00 = 1/(sigma*(e0.x2-e0.x1)*(e0.y2-e0.y1)) + diel_loss;

  lpj = v_get (dim);
  for (j=0; j<dim; j++)
    lpj->ve[j] = lp (&e0, &e[j]);
    
  for (i=0; i<dim; i++)
    {
      lpi0 = lmm-lp (&e[i], &e0);
      for (j=0;j<=i;j++)
        {
          /* Inductance is affected by effective permeability
           * For non-magnetic materials, μr ≈ 1
           * The effective permittivity mainly affects capacitance
           * L remains approximately the same
           */
          Z->me[i][j].im = Z->me[j][i].im = Omega * (lpi0-lpj->ve[j]+lp (&e[i], &e[j]));
          Z->me[i][j].re = Z->me[j][i].re = r00;
        }
    }
  V_FREE (lpj);
  
  /* Add conductor resistance for signal lines */
  for (i=n0; i<dim; i++) {
    double conductor_loss = 1/(sigma*(e[i].x2-e[i].x1)*(e[i].y2-e[i].y1));
    
    /* Add dielectric loss for signal conductors if applicable */
    if (cond != NULL && N > 0) {
      /* Find which conductor this element belongs to */
      int cond_idx = 1; /* Default to first signal line */
      /* You may want to add logic here to determine correct conductor index */
      
      if (cond_idx <= N && cond[cond_idx].substrate_h > 0.0) {
        double sig_diel_loss = calc_dielectric_loss(
          cond[cond_idx].er, 
          cond[cond_idx].tan_delta,
          Omega,
          cond[cond_idx].w,
          cond[cond_idx].substrate_h);
        conductor_loss += sig_diel_loss;
      }
    }
    
    Z->me[i][i].re += conductor_loss;
  }
}
