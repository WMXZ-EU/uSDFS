//Copyright 2017 by Walter Zimmer
// Version 07-07-17
//

/**
 * @param: R output autocorrelation function
 * @param: nc number of desired reflecting coefficients
 * @param: x 1st input vector
 * @param: y 2nd input vector
 * @param: nn length of input vectors
 */
void XCorr_f32(float *R, int nc, float *x, float *y, int nn)
{
    for(int ii=0; ii<nc; ii++)
    {
    	R[ii] = 0;
    	float *xptr = &x[0];
    	float *yptr = &y[ii];
    	int kk=(nn-ii);
    	float acc=0;
    	while (kk>0)
    	{
    		acc += (*xptr++) * (*yptr++);
    		acc += (*xptr++) * (*yptr++);
    		acc += (*xptr++) * (*yptr++);
    		acc += (*xptr++) * (*yptr++);
    		kk--;
    	}
    	kk = nn % 4;
    	while (kk>0)
    	{
    		acc += (*xptr++) * (*yptr++);
    		kk--;
    	}
    	R[ii] = acc/(nn-ii); // or div by nn; or nothing
    }
}

/**
 * @param: R output autocorrelation function
 * @param: nc number of desired reflecting coefficients
 * @param: x 1st input vector
 * @param: y 2nd input vector
 * @param: nn length of input vectors
 */
void Copy_f32(float *y, float *x, int nn)
{
	float *xptr = x;
	float *yptr = y;
	int kk=nn;
	while (kk>0)
	{
		(*yptr++) = (*xptr++);
		(*yptr++) = (*xptr++);
		(*yptr++) = (*xptr++);
		(*yptr++) = (*xptr++);
		kk--;
	}
	kk = nn % 4;
	while (kk>0)
	{
		(*yptr++) = (*xptr++);
		kk--;
	}
}

/**
 * @param: A output array of prediction coefficients
 * @param: R input autocorrelation function
 * @param: nc number of desired reflecting coefficients
 * @return: Residual Energy
 */
float Levinson_Durbin_f32(float *A, float *R, int nc)
{   float Em,km,err;
    A[0]=1;
    Em=R[0];
    //
    for(int ii=1; ii<nc;ii++)
    {
		km=R[ii]; for(int jj=1; jj<ii; jj++) km -= A[jj]*R[ii-jj];
		km /= Em;
		Em *= (1-km*km);

		A[ii]=km;
		for(int jj=1, kk=ii-1; jj<kk; jj++, kk--)
		{
			float tmp = A[kk]+km*A[jj];
			A[jj] = A[jj]+km*A[kk];
			A[kk] = tmp;
			//
		}
		if((ii % 2 ) == 0 ) A[ii/2] += km*A[ii/2];
    }

    // negate coefficients for equalization filter
    for(int ii=1;ii<nc+1;ii++) A[ii] = -A[ii];
    return Em;
}

/*
 * ii=1;
 * A[1]=km;
 *
 * ii=2;
 * A[2]=km;
 * A[1]=A[1]+km*A[1};
 *
 * ii=3;
 * A[3]=km
 * A[1]=A[1]+km*A[2] <-|
 * A[2]=A[2]+km*A[1] <-|
 *
 * ii=4;
 * A[4]=km
 * A[1]=A[1]+km*A[3] <----|
 * A[2]=A[2]+km*A[2] <-   |
 * A[3}=A[3}+km*A[1} <----|
 *
 * ii=5;
 * A[5]=km
 * A[1]=A[1]+km*A[4] <----|
 * A[2]=A[2]+km*A[3] <--| |
 * A[3}=A[3}+km*A[2} <--| |
 * A[4}=A[4}+km*A[1} <----|
 *
 */

/*********************************************************************************
* Description:
* Levinson-Durbin is the scalar (not MMX code) version of the Levinson-Durbin
* algorithm. It is used to calculate the reflection and prediction coefficients of
* a given set of normal equations.
*
* Inputs:
* r short int *  a pointer to the first element
* *			     of the input 'r' matrix
* a short int *  a pointer to the output
*    			 prediction coefficients array
* k short int 	*  a pointer to the output
*     				reflection coefficients array
* p short int  	the number of reflection coefficients
*    			 to solve for (typically 10 or 16).
*
* https://software.intel.com/sites/landingpage/legacy/mmx/MMX_App_Levinson_Durbin.pdf
*  written for short data
**********************************************************************************/
void Levinson_Durbin_Q15(short *r, short *a, short *k, short p)
{
	// begin Levinson_Durbin()
	short i,	// inner loop index
	m,			// outer loop index
	b[11]; 		// temporary vector to store the prediction coefficients

	long Rn,	// inner loop numerator accumulator
	Rd,			// inner loop denominator accumulator
	temp;		// temporary variable used for intermediate result calculations

	// initialize a[0] to 1/4
	a[0] = 8192;
	// For each order, calculate a new prediction and reflection coefficient
	for (m = 1; m < p + 1; m++)
	{
		// Initialize the numerator and denominator accumulators to zero
		Rn = Rd = 0;
		// Calculate the numerator and denominator values for the integer division
		for (i = 0; i < m; i++)
		{   Rn = Rn + (r[m-1] * (long)a[i]);
			Rd = Rd + (r[i]   * (long)a[i]);
		}
		// Calculate the reflection coefficient k[m]. Round the Q28 number prior
		// to converting it to a Q15 number. Also, scale it by the scaling factor
		// to help keep the input data in proper range.
		k[m] = -Rn / ((Rd + 0x4000) >> 15);
		k[m] = (((long)k[m] * 0x7ff8) + 0x4000) >> 15;
		// Calculate the new prediction coefficient by converting k[m] from
		// a Q15 to a Q13 number
		b[m] = (k[m] + 0x2) >> 2;
		// Calculate the new prediction coefficients for the next iteration
		for (i = 1; i < m; i++)
			b[m] = (((long)a[i] << 15) + (k[m] * (long)a[m-i]) + 0x4000) >> 15;

		// Copy the prediction coefficients from the temporary b[] array to a[]
		for ( i = 1; i < m+1; i++) a[i] = b[i];
	}
}   // end Levinson_Durbin()

#ifdef NOTUSED

/* find the lpc coefficents and reflection coefficients from the
   autocorrelation function */
float Sacf2lpc(float *acf, int nacf, float *lpc, float *ref)
{
	static float *tmp;
	static int ntmp = 0;
	double e, ci;
	int i, j;
	if(ntmp != nacf)
	{
//		if(ntmp != 0) Sfree(tmp);
//		tmp = SvectorFloat(nacf);
		ntmp = nacf;
	}
	/* find lpc coefficients */
	e = acf[0];
	lpc[0] = 1.0;
	for(i = 1; i <= nacf; i++)
	{
		ci = 0.0;
		for(j = 1; j < i; j++) ci += lpc[j] * acf[i-j];
		ref[i] = ci = (acf[i] - ci) / e;
		lpc[i] = ci;
		for(j = 1; j < i; j++) tmp[j] = lpc[j] - ci * lpc[i-j];
		for(j = 1; j < i; j++) lpc[j] = tmp[j];
		e = (1 - ci * ci) * e;
	}
	return(e);
}


/**********************************************************************
*                                                                     *
* Levinson-Durbin Algorithm for LPC Coefficients                      *
*                                                                     *
*   input:                                                            *
*   n  = LPC order                                                    *
*   r -> r[0] to r[n]    autocorrelation values                       *
*                                                                     *
*   output:                                                           *
*   a -> a[0] to a[n]    LPC coefficients, a[0] = 1.0                 *
*   k -> k[0] to k[n]    reflection coefficients, k[0] = unused       *
*                                                                     *
*                                                                     *
**********************************************************************/
//https://dsp.stackexchange.com/questions/24322/division-by-zero-in-levinson-durbin-recursion
#define N   17

void levinson_durbin(const int n, const double* r, double* a, double* k)
{
	double a_temp[N], alpha, epsilon;         /* n <= N = constant  */
	int i, j;
	k[0]      = 0.0;                          /* unused */

	a[0]      = 1.0;
	a_temp[0] = 1.0;                          /* unnecessary but consistent */

	alpha     = r[0];

	for(i=1; i<=n; i++)
	{
		epsilon = r[i];                       /* epsilon = a[0]*r[i]; */
		for(j=1; j<i; j++)
		{
			epsilon += a[j]*r[i-j];
		}

		a[i] = k[i] = -epsilon/alpha;
		alpha = alpha*(1.0 - k[i]*k[i]);

		for(j=1; j<i; j++)
		{
			a_temp[j] = a[j] + k[i]*a[i-j];   /* update a[] array into temporary array */
		}
		for(j=1; j<i; j++)
		{
			a[j] = a_temp[j];                 /* update a[] array */
		}
	}
}


#include <math.h>
#include <vector>
using namespace std;
// Returns in vector linear prediction coefficients calculated using Levinson Durbin
void ForwardLinearPrediction( vector<double> &coeffs, const vector<double> &x )
{   // GET SIZE FROM INPUT VECTORS
	size_t N = x.size() - 1;
	size_t m = coeffs.size();

    // INITIALIZE R WITH AUTOCORRELATION COEFFICIENTS
	vector<double> R( m + 1, 0.0 );
	for ( size_t i = 0; i <= m; i++ )
	{   for ( size_t j = 0; j <= N - i; j++ )
		{
			R[ i ] += x[ j ] * x[ j + i ];
		}
	}

    // INITIALIZE Ak
	vector<double> Ak( m + 1, 0.0 );
	Ak[ 0 ] = 1.0;
    // INITIALIZE Ek
	double Ek = R[ 0 ];
    // LEVINSON-DURBIN RECURSION
	for ( size_t k = 0; k < m; k++ )
	{   // COMPUTE LAMBDA
		double lambda = 0.0;
		for ( size_t j = 0; j <= k; j++ )
		{
			lambda -= Ak[ j ] * R[ k + 1 - j ];
		}
		lambda /= Ek;

		// UPDATE Ak
		for ( size_t n = 0; n <= ( k + 1 ) / 2; n++ )
		{	double temp = Ak[ k + 1 - n ] + lambda * Ak[ n ];
			Ak[ n ] = Ak[ n ] + lambda * Ak[ k + 1 - n ];
			Ak[ k + 1 - n ] = temp;
		}

		// UPDATE Ek
		Ek *= 1.0 - lambda * lambda;
	}

    // ASSIGN COEFFICIENTS
	coeffs.assign( ++Ak.begin(), Ak.end() );
}

// Example program using Forward Linear Prediction

int main( int argc, char *argv[] )
{  // CREATE DATA TO APPROXIMATE
	vector<double> original( 128, 0.0 );
	for ( size_t i = 0; i < original.size(); i++ )
	{
		original[ i ] = sin( i * 0.01 )
						+ 0.75 * sin( i * 0.03 )
					  	+ 0.5  * sin( i * 0.05 )
						+ 0.25 * sin( i * 0.11 );
	}

	// GET FORWARD LINEAR PREDICTION COEFFICIENTS
	vector<double> coeffs( 4, 0.0 );
	ForwardLinearPrediction( coeffs, original );

	// PREDICT DATA LINEARLY
	vector<double> predicted( original );
	size_t m = coeffs.size();
	for ( size_t i = m; i < predicted.size(); i++ )
	{
		predicted[ i ] = 0.0;
		for ( size_t j = 0; j < m; j++ )
		{
			predicted[ i ] -= coeffs[ j ] * original[ i - 1 - j ];
		}
	}

	// CALCULATE AND DISPLAY ERROR
	double error = 0.0;
	for ( size_t i = m; i < predicted.size(); i++ )
	{
		printf( "Index: %.2d / Original: %.6f / Predicted: %.6f\n", i, original[ i ], predicted[ i ] );
		double delta = predicted[ i ] - original[ i ];
		error += delta * delta;
	}
	printf( "Forward Linear Prediction Approximation Error: %f\n", error );

	return 0;
}

#endif
