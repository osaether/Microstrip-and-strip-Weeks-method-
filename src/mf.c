#include <stdio.h>
#include <stdlib.h>
#include "mf.h"

struct {
  void *m;
  size_t n;
} a[3000];

size_t tot=0,mmax=0;
unsigned w=0;

void Free(void *m)
{
  int i;
  i = 0;
  while(1) {
    if(i>2999) {
      printf("#");
      break;
    }
    if(a[i].m==m) {
      tot -= a[i].n;
      a[i].n = 0;
      a[i].m = NULL; 
      break;
    }
    i++;
  }
  free(m);
}

void *Calloc(size_t n, size_t m)
{
  void *b;
  b=calloc(n,m);
  a[w].n=n*m;
  tot += n*m;
  a[w].m=b;
  w++;
  if(tot>mmax) mmax=tot;
  return(b);
}

void *Malloc(size_t n)
{
  void *b;
  b=malloc(n);
  a[w].n=n;
  tot += n;
  a[w].m=b;
  w++;
  if(tot>mmax) mmax=tot;
  return(b);
}

void *Realloc(void *m, size_t n)
{
  int i;
  void *b;
  i = 0;
  while(1) {
    if(i>2999) {
      printf("#");
      break;
    }
    if(a[i].m==m) {
      tot -= a[i].n;
      break;
    }
    i++;
  }
  b = realloc(m,n);
  a[i].m=b;
  a[i].n=n;
  tot += n;
  return b;
}

size_t get_max_memory (void)
{
  return mmax;
}
