//
//  mat.cpp
//  mania
//
//  Created by Antony Searle on 14/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "mat.hpp"



// #include <math.h>
// #include "nrutil.h"

#define TINY 1.0e-20 // A small number.
void ludcmp(float **a, int n, int *indx, float *d) /*
                                                    Given a matrix a[1..n][1..n], this routine replaces it by the LU decomposition of a rowwise
                                                    permutation of itself. a and n are input. a is output, arranged as in equation (2.3.14) above;
                                                    indx[1..n] is an output vector that records the row permutation effected by the partial
                                                    pivoting; d is output as ±1 depending on whether the number of row interchanges was even
                                                    or odd, respectively. This routine is used in combination with lubksb to solve linear equations
                                                    or invert a matrix. */
{
    int i,imax=0,j,k;
    float big,dum,sum,temp;
    float *vv; /* vv stores the implicit scaling of each row. */
    vv = (float*) malloc(n * sizeof(float)); --vv;
    *d=1.0; /* No row interchanges yet. */
    for (i=1;i<=n;i++) { //Loop over rows to get the implicit scaling information.
        big=0.0;
        
        for (j=1;j<=n;j++)
            if ((temp=fabs(a[i][j])) > big) big=temp;
        
        //if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
        // No nonzero largest element.
        vv[i]=1.0/big; /*Save the scaling.*/
    }
    for (j=1;j<=n;j++) { /* This is the loop over columns of Crout’s method.*/
        for (i=1;i<j;i++) { /*This is equation (2.3.12) except for i = j.*/
            sum=a[i][j];
            for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
            a[i][j]=sum;
        }
        big=0.0; //Initialize for the search for largest pivot element.
        for (i=j;i<=n;i++) { //This is i = j of equation (2.3.12) and i = j + 1 ...N
            sum=a[i][j]; //of equation (2.3.13).
            for (k=1;k<j;k++)
                
                sum -= a[i][k]*a[k][j];
            a[i][j]=sum;
            if ( (dum=vv[i]*fabs(sum)) >= big) {
                //Is the figure of merit for the pivot better than the best so far?
                big=dum;
                imax=i;
            }
        }
        if (j != imax) { //Do we need to interchange rows?
            for (k=1;k<=n;k++) { //Yes, do so...
                dum=a[imax][k];
                a[imax][k]=a[j][k];
                a[j][k]=dum;
            }
            *d = -(*d); //...and change the parity of d.
            vv[imax]=vv[j]; //Also interchange the scale factor.
        }
        indx[j]=imax;
        if (a[j][j] == 0.0) a[j][j]=TINY;
        //If the pivot element is zero the matrix is singular (at least to the precision of the
        //                                                   algorithm). For some applications on singular matrices, it is desirable to substitute
        //TINY for zero.
        if (j != n) { //Now, finally, divide by the pivot element.
            dum=1.0/(a[j][j]);
            for (i=j+1;i<=n;i++) a[i][j] *= dum;
        }
    } //Go back for the next column in the reduction.
    //free_vector(vv,1,n);
    free(++vv);
}
