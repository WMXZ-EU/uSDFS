/*
 * fft_conv.cpp
 *
 *  Created on: May 10, 2017
 *      Author: WMXZ
 */

#include "fft_conv.h"

/*
 * filter window M+1
 * FFTlength N = L + M
 * overlap add method
 * take L sample from input
 * zero pad to N , i.e. M zeros
 * filter (fft - mult - ifft)
 * add previous last M samples to actual first M samples
 * example
 * 		M = 128
 * 		L = 3*128
 * 		N = 512 = 4*128
 */
const arm_rfft_fast_instance_f32  arm_rfft_f32_len256  = {arm_cfft_sR_f32_len128,   256, (float *) twiddleCoef_rfft_256};
const arm_rfft_fast_instance_f32  arm_rfft_f32_len512  = {arm_cfft_sR_f32_len256,   512, (float *) twiddleCoef_rfft_512};
const arm_rfft_fast_instance_f32  arm_rfft_f32_len1024 = {arm_cfft_sR_f32_len512,  1024, (float *) twiddleCoef_rfft_1024};


/**
 * Filter initialization routine
 * @param imp	impulse function
 * @param buff	working buffer
 * @param nf	number of partioned filters
 * @param ll	length of input data
 * @param nn	fft block length
 * @param mm	length of partial filter (reduced by 1)
 */
void C_CONV::init(float *imp, float * buff, int nch, int nf, int ll, int nn, int mm)
{
	int ii,jj;
	if(ll==0 && nn==0) return;
	if(ll==0 && mm==0) return;
	if(nn==0 && mm==0) return;
	//
	if(ll==0) ll = nn-mm;
	if(mm==0) mm = nn-ll;
	if(nn==0) nn = ll+mm;
	//
	this->nch = nch;
	this->nf = nf;
	this->ll = ll;
	this->nn = nn;
	this->mm = mm;

	uu = buff;    // input buffer;            size: uu[nn]
	vv = uu + nn; // spectrum result;         size: vv[nn]
	ww = vv + nn; // spectrum accumulator;    size: ww[nn]
	bb = ww + nn; // filter store;            size: bb[nch*nf*nn]
	zz = bb + nch*nf*nn;  // spectrum store;  size: zz[nch*nf*nn]
	ov = zz + nch*nf*nn;  // overlap store;   size: ov[nch*nf*(nn-ll)]
  nj = (int32_t *)ov + nch*nf*(nn-ll); // index to FDL

	for(ii=0; ii<nch*(nn-ll);ii++) ov[ii]=0;

	if(nn == 256)
		rfft_instance = (arm_rfft_fast_instance_f32*)&arm_rfft_f32_len256;
	else if (nn == 512)
		rfft_instance = (arm_rfft_fast_instance_f32*)&arm_rfft_f32_len512;
	else if (nn == 1024)
		rfft_instance = (arm_rfft_fast_instance_f32*)&arm_rfft_f32_len1024;

	for(jj=0;jj<nf;jj++)
	{ nj[jj]= (jj*mm)/ll;
	  int nd = (jj*mm) % ll;
    for(ii=0; ii<nd; ii++) uu[ii]=0;                 // delay
		for(ii=0; ii<mm; ii++) uu[nd+ii]=imp[jj*mm+ii];  // mm coefficients
		for(ii=nd+mm; ii<nn; ii++)  uu[ii]=0;            // zero padding 
		arm_rfft_fast_f32(rfft_instance, uu, &bb[jj*nn], 0);
	}
}

/**
 * filter with overlap and discard method
 * here sig 1: LL = 10
 *      sig 2: MM + 1 = 7
 *      fft length: NN+MM = 16
 * -----------------------------------
 * input: (with overlap)
 * 000000XXXXXXXXXX
 *           ooooooXXXXXXXXXX
 *                     ooooooXXXXXXXXXX
 * output: (overlap and discard)
 * ------YYYYYYYYYY
 *           ------YYYYYYYYYY
 *                     ------YYYYYYYYYY
 */

/**
 * uniform partitioned convolution execution step
 * @param yy	first output vector (is filtered output)
 * @param pp	second output vector (is accumulated power)
 * @param xx	input vector
 */
void C_CONV::exec_upos(int32_t *yy, float *pp, int32_t *xx)
{   int ich,ii,jj,kk;

    static int ij=0;	// counter into circular partioning buffer
    
    float *bp;
    
    for(ich=0;ich<nch;ich++)
    {
      // copy data to FFT buffer and handle overlap
      float *ovp = &ov[ich*(nn-ll)];
      for(ii = 0; ii < nn-ll; ii++) uu[ii]    = ovp[ii];   // first overlap
      for(ii = 0; ii < ll; ii++) uu[nn-ll+ii] = (float) xx[ich + ii*nch];  // then input with de-multiplexing
      for(ii = 0; ii < nn-ll; ii++) ovp[ii] = uu[ll+ii];  //store last part from input to overlap
      //
      float *zp = &zz[(ich*nf + ij)*nn];  // pointer where actual spectrum will go
      
      // perform forward fft
      arm_rfft_fast_f32(rfft_instance, uu, zp, 0);
  
      // clear accumulators
      for(ii=0;ii<nn;ii++) ww[ii]=0;
      for(ii=0;ii<nn;ii++) pp[ii + ich*nn]=0;
  
      // multiply and sum all spectra
      for(jj=0; jj<nf; jj++)
      {
        // multiply with filter
        zp = &zz[(ich*nf + (ij+nj[jj])%nf)*nn];
        bp = &bb[jj*nn];
//        arm_cmplx_mult_cmplx_f32(zp, bp, vv, nn/2);
        arm_cmplx_mult_cmplx_f32(&zp[2], &bp[2], &vv[2], nn/2-1);
        vv[0]=zp[0]*bp[0];
        vv[1]=zp[1]*bp[1];
        // accumulate previous filtered spectra
        for(ii=0;ii<nn;ii++) 
        { ww[ii] += vv[ii]; ii++;
          ww[ii] += vv[ii]; ii++;
          ww[ii] += vv[ii]; ii++;
          ww[ii] += vv[ii]; } 
        //
        float *ppp=&pp[ich*nn];
        for(ii=0;ii<nn;ii++) 
        { ppp[ii] += zp[ii]; ii++;
          ppp[ii] += zp[ii]; ii++;
          ppp[ii] += zp[ii]; ii++;
          ppp[ii] += zp[ii];}
      }
      // inverse FFT
      arm_rfft_fast_f32(rfft_instance, ww, uu, 1);
  
      // copy data to result buffer with multiplexing
      for(ii = 0; ii < ll; ii++) yy[ich + ii*nch] = uu[nn-ll+ii];  // discard first part
    }
    //
    ij++; ij %= nf;
}


