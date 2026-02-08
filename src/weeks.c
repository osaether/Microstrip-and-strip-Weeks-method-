#include <crtdbg.h>
#include <time.h>
#include <math.h>
#include "machine.h"
#include "matrix2.h"
#include "zmatrix2.h"
#include "weeks.h"
#include "calcl.h"
#include "mf.h"

#ifndef PI
#define PI 3.141592653589793116
#endif
#define	is_zero(z)	((z).re == 0.0 && (z).im == 0.0 )

extern size_t mmax;
ZMAT *zzLUfactor(ZMAT *A, PERM *pivot);

void print_mat(ZMAT *x)
{
    int i, j;
    for(i=0;i<x->m;i++)
    {
        for (j=0;j<x->n;j++)
        {
            printf("(%+7.3lf, %+7.3lf) ", x->me[i][j].re, x->me[i][j].im);
        }
        printf("\n");
    }
}

void print_vec(ZVEC *x)
{
    int i;
    for(i=0;i<x->dim;i++)
    {
		printf("(%+7.3lf, %+7lf) ", x->ve[i].re, x->ve[i].im);
    }
	printf("\n");
}

void print_perm(PERM* x)
{
	int i;
    for(i=0;i<x->size;i++)
    {
		printf("%d ", x->pe[i]);
    }
	printf("\n");	
}

ZVEC *ppx_zvec(PERM* px, int n,ZVEC* out)
{
  int i, size;
  
  size = px->size;
  for ( i=0; i<size; i++ )
    if ( px->pe[i] >= size )
      error(E_BOUNDS,"px_vec");
    else
      {
        out->ve[i].im = 0;
        out->ve[i].re = (px->pe[i] == n)? 1.0:0;
      }
  return out;
}

ZVEC	*zzUsolve(ZMAT* matrix, ZVEC* b, ZVEC* out, double diag)
{
    int	dim /* , j */;
    int	i, i_lim;
    complex	**mat_ent, *mat_row, *b_ent, *out_ent, *out_col, sum;

    if ( matrix==ZMNULL || b==ZVNULL )
	error(E_NULL,"zUsolve");
    dim = min(matrix->m,matrix->n);
    if ( b->dim < dim )
	error(E_SIZES,"zUsolve");
    if ( out==ZVNULL || out->dim < dim )
	out = zv_resize(out,matrix->n);
    mat_ent = matrix->me;	b_ent = b->ve;	out_ent = out->ve;

    for ( i=dim-1; i>=0; i-- )
	if ( ! is_zero(b_ent[i]) )
	    break;
	else
	    out_ent[i].re = out_ent[i].im = 0.0;
    i_lim = i;

    for ( i = i_lim; i>=0; i-- )
    {
	sum = b_ent[i];
	mat_row = &(mat_ent[i][i+1]);
	out_col = &(out_ent[i+1]);
	sum = zsub(sum,__zip__(mat_row,out_col,i_lim-i,Z_NOCONJ));
	/******************************************************
	  for ( j=i+1; j<=i_lim; j++ )
	  sum -= mat_ent[i][j]*out_ent[j];
	  sum -= (*mat_row++)*(*out_col++);
	******************************************************/
	if ( diag == 0.0 )
	{
	    if ( is_zero(mat_ent[i][i]) )
		error(E_SING,"zUsolve");
	    else
		/* out_ent[i] = sum/mat_ent[i][i]; */
		out_ent[i] = zdiv(sum,mat_ent[i][i]);
	}
	else
	{
	    /* out_ent[i] = sum/diag; */
	    out_ent[i].re = sum.re / diag;
	    out_ent[i].im = sum.im / diag;
	}
    }

    return (out);
}

/* zLsolve -- forward elimination with (optional) default diagonal value */
ZVEC	*zzLsolve(ZMAT* matrix, ZVEC* b, ZVEC* out, double diag)
{
    int	dim, i, i_lim, j;
    complex	**mat_ent, *mat_row, *b_ent, *out_ent, *out_col, sum;

    if ( matrix==ZMNULL || b==ZVNULL )
	error(E_NULL,"zLsolve");
    dim = min(matrix->m,matrix->n);
    if ( b->dim < dim )
	error(E_SIZES,"zLsolve");
    if ( out==ZVNULL || out->dim < dim )
	out = zv_resize(out,matrix->n);
    mat_ent = matrix->me;	b_ent = b->ve;	out_ent = out->ve;

    for ( i=0; i<dim; i++ )
	if ( ! is_zero(b_ent[i]) )
	    break;
	else
	    out_ent[i].re = out_ent[i].im = 0.0;
//	printf ("A: ");
//	print_vec(out);
    i_lim = i;

    for ( i = i_lim; i<dim; i++ )
    {
	sum = b_ent[i];
//	printf("b[i]=(%lf, %lf) ", b_ent[i].re, b_ent[i].im);
	mat_row = &(mat_ent[i][i_lim]);
	out_col = &(out_ent[i_lim]);
/*	for(j=0;j<(i-i_lim);j++)
	{
		printf("m[j]=(%lf, %lf) o[j]=(%lf, %lf) ", mat_row[j].re, mat_row[j].im, out_col[j].re, out_col[j].im);
	}
*/
	sum = zsub(sum,__zip__(mat_row,out_col,(int)(i-i_lim),Z_NOCONJ));
//	printf("sum=(%lf, %lf) ", sum.re, sum.im);
	/*****************************************************
	  for ( j=i_lim; j<i; j++ )
	  sum -= mat_ent[i][j]*out_ent[j];
	  sum -= (*mat_row++)*(*out_col++);
	******************************************************/
	if ( diag == 0.0 )
	{
	    if ( is_zero(mat_ent[i][i]) )
		error(E_SING,"zLsolve");
	    else
		out_ent[i] = zdiv(sum,mat_ent[i][i]);
	}
	else
	{
	    out_ent[i].re = sum.re / diag;
	    out_ent[i].im = sum.im / diag;
	}
    }
//	printf("\n");
    return (out);
}



ZVEC	*zzLUsolve(ZMAT* A,PERM* pivot, ZVEC* b, ZVEC* x)
{
	if ( A==ZMNULL || b==ZVNULL || pivot==PNULL )
		error(E_NULL,"zLUsolve");
	if ( A->m != A->n || A->n != b->dim )
		error(E_SIZES,"zLUsolve");

	x = px_zvec(pivot,b,x);	/* x := P.b */
//	print_vec(x);
	zzLsolve(A,x,x,1.0);	/* implicit diagonal = 1 */
//	print_vec(x);
	zzUsolve(A,x,x,0.0);	/* explicit diagonal */
//	print_vec(x);

	return (x);
}


ZMAT	*zzm_inverse(ZMAT* A, ZMAT* out)
{
	int	i;
	ZVEC	*tmp, *tmp2;
	ZMAT	*A_cp;
	PERM	*pivot;

	if ( ! A )
	    error(E_NULL,"zm_inverse");
	if ( A->m != A->n )
	    error(E_SQUARE,"zm_inverse");
	if ( ! out || out->m < A->m || out->n < A->n )
	    out = zm_resize(out,A->m,A->n);

	A_cp = zm_copy(A,ZMNULL);
	tmp = zv_get(A->m);
	tmp2 = zv_get(A->m);
	pivot = px_get(A->m);
	tracecatch(zLUfactor(A_cp,pivot),"zm_inverse");
	for ( i = 0; i < A->n; i++ )
	{
	    zv_zero(tmp);
	    tmp->ve[i].re = 1.0;
	    tmp->ve[i].im = 0.0;
	    tracecatch(zzLUsolve(A_cp,pivot,tmp,tmp2),"m_inverse");
//		print_vec(tmp2);
	    zset_col(out,i,tmp2);
	}

	ZM_FREE(A_cp);
	ZV_FREE(tmp);	ZV_FREE(tmp2);
	PX_FREE(pivot);


void main (void)
{
  int i, j, k, m;
  conductor *test;
  element *e, e0;
  time_t tb, ts, t1;
  int M,N,tk,ti, temp, n0;
  
  double f, Omega;
  ZMAT *Z=ZMNULL, *Y=ZMNULL, *z=ZMNULL,*y=ZMNULL, *ztmp=ZMNULL;
  ZVEC *tmp2;
  PERM *pivot;
  FILE *fp;
  
  /* Declare external frequency variable from input.c */
  extern double global_frequency;

  fprintf(stderr, "\n========================================\n");
  fprintf(stderr, "Microstrip Resistance Calculator\n");
  fprintf(stderr, "With Dielectric Substrate Support\n");
  fprintf(stderr, "YAML Input Format\n");
  fprintf(stderr, "========================================\n");

  temp = 0;
  tb = time(&tb);
  setbuf(stdout, (char *)NULL);
  setbuf(stderr, (char *)NULL);

  if ((fp = fopen ("test.yaml", "rt")) == NULL)
    {
      fprintf (stderr, "ERROR: Can not open input file '%s'\n", "test.yaml");
      fprintf (stderr, "Please create a YAML input file with conductor definitions.\n");
      exit (EXIT_FAILURE);
    }
    
  fprintf(stderr, "\nReading YAML input file...");
  test = getinput (fp, &N);
  fclose(fp);
  N--;
  
  /* Use frequency from YAML file */
  f = global_frequency;
  Omega = 2.0*PI*f;
  
  fprintf(stderr, "\n");

  fprintf(stderr, "\n\nBuilding partial elements...");

  n0 = M = test[0].nw*test[0].nh-1;
  for(i=1;i<=N;i++)
    M += test[i].nw*test[i].nh;
  e = build_elements (M, N, test, &e0);
  if (e == NULL)
    exit (EXIT_FAILURE);
  fprintf(stderr, "\nNumber of elements: %d", M);

  /* Display dielectric information */
  fprintf(stderr, "\n\nDielectric Properties:");
  fprintf(stderr, "\n  Ground plane (line0): εr=%.2f, h=%.2e m", 
          test[0].er, test[0].substrate_h);
  if (test[0].tan_delta > 0.0)
    fprintf(stderr, ", tan δ=%.4f", test[0].tan_delta);
  
  for(i=1; i<=N; i++) {
    fprintf(stderr, "\n  Line %d: εr=%.2f", i, test[i].er);
    if (test[i].substrate_h > 0.0)
      fprintf(stderr, ", h=%.2e m", test[i].substrate_h);
    if (test[i].tan_delta > 0.0)
      fprintf(stderr, ", tan δ=%.4f", test[i].tan_delta);
  }

  t1 = time(&t1);
  Z = zm_get (M,M);
  fprintf (stderr,"\n\nCalculating partial inductances with dielectric...");
  
  /* Call modified calcl with conductor array for dielectric info */
  calcl (Z, e, n0, Omega, e0, test, N);
  
  Free (e);
  e = NULL;
  fprintf (stderr, " -> %lu seconds", time(NULL)-t1);

  fprintf (stderr,"\n\nInverting matrix of partial impedances:\n");

  Y = zm_get (M,M);
  Y = zm_inverse(Z, Z);
  
  y = zm_get (N, N);

  ti = n0;
  for (i=0;i<N;i++)
  {
    tk = n0;      
    for (k=0;k<N;k++)
    {
      y->me[i][k].re = 0.0;
      y->me[i][k].im = 0.0;
      for (j=0; j<test[i+1].n; j++)
      {
        for(m=0;m<test[k+1].n;m++)
        {
          y->me[i][k].re += Y->me[ti+j][tk+m].re;
          y->me[i][k].im += Y->me[ti+j][tk+m].im;
        }
      }
      tk += test[k+1].n;
    }
    ti += test[i+1].n;
  }
  Free(test);
  test=0;
  z = zm_inverse (y, y);
  y = ZMNULL;  

  /* ===== RESULTS OUTPUT ===== */
  printf ("\n\n========================================\n");
  printf ("RESULTS\n");
  printf ("========================================\n");
  printf ("\nFREQUENCY: %e Hz (%.2f MHz)\n", f, f/1e6);
    
  printf("\n*** RESISTANCE MATRIX (Ohm/m) ***\n\n");
  printf("    ");
  for(j=1;j<=N;j++)
    printf("%12d",j);
  printf ("\n\n");
  for(i=0;i<N;i++) {
    printf ("%3d ",i+1);
    for(j=0;j<N;j++)
      printf("%+0.4e ",z->me[i][j].re);
    printf("\n");
  }

  printf("\n*** INDUCTANCE MATRIX (H/m) ***\n\n");
  printf("    ");
  for(j=1;j<=N;j++)
    printf("%12d",j);
  printf ("\n\n");

  for(i=0;i<N;i++) {
    printf ("%3d ",i+1);
    for(j=0;j<N;j++)
      printf("%+0.4e ",z->me[i][j].im/Omega);
    printf("\n");
  }
  
  printf("\n*** IMPEDANCE MAGNITUDE (Ohm) at %.2f MHz ***\n\n", f/1e6);
  printf("    ");
  for(j=1;j<=N;j++)
    printf("%12d",j);
  printf ("\n\n");

  for(i=0;i<N;i++) {
    printf ("%3d ",i+1);
    for(j=0;j<N;j++) {
      double magnitude = sqrt(z->me[i][j].re * z->me[i][j].re + 
                              z->me[i][j].im * z->me[i][j].im);
      printf("%+0.4e ", magnitude);
    }
    printf("\n");
  }
  
  ZM_FREE (z);
  ts = time(&ts);
  
  printf("\n========================================\n");
  printf("Time used: %lu seconds\n", ts-tb);
  printf("Peak memory: %u kbytes\n", mmax/1024);
  printf("========================================\n");
}
