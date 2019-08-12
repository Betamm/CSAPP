/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 8;
    int i, j, m, n;
    int count = 0;
    for (i = 0; i < N-1; i += bsize){
	for (j = 0; j < M-1; j += bsize){
	    for (m = j; m < j+bsize; m++){
		if (count%5 == 0){
		    B[m][m] = A[m][m];
		}
		for (n = i; n < i+bsize; n++){
		    if (m != n){
		        B[m][n] = A[n][m];
		    }
		}
	    }
	    count++;
	}
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 
char transpose_submit_desc_1[] = "Transpose submission_1";
void transpose_submit_1(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, m, n;
    int bsize, Bsize;
    int count = 0;
    if ((M == 32) && (N == 32)){
	bsize = 8;
	for (i = 0; i < N; i += bsize){
	    for (j = 0; j < M; j += bsize){
		for (m = j; m < j+bsize; m++){
		    for (n = i; n < i+bsize; n++){
		        B[m][n] = A[n][m];
		    }
	        }
	    }
	}
    } else if ((M == 64) && (N == 64)){
	bsize = 4;
	Bsize = 8;
	count = 0;
	for (i = 0; i < N; i += Bsize){
	    for (j = 0; j < M; j += Bsize){
		for (k = 0; k < 4; k++){
		    switch (k){
			case 0:
			    for (n = i; n < i+bsize; n++){
				if (count%9 == 0){
				    B[n][n] = A[n][n];
				}
				for (m = j; m < j+bsize; m++){
				    if (m != n){
					B[m][n] = A[n][m];
				    }
				}
			    }
			    break;
			case 1:
			    for (n = i; n < i+bsize; n++){
				if (count%9 == 0){
				    B[n+bsize][n] = A[n][n+bsize];
				}
				for (m = j+bsize; m < j+Bsize; m++){
				    if (m != (n+bsize)){
					B[m][n] = A[n][m];
				    }
				}
			    }
			    break;
			case 3:
			    for (n = i+bsize; n < i+Bsize; n++){
				if (count%9 == 0){
				    B[n-bsize][n] = A[n][n-bsize];
				}
				for (m = j; m < j+bsize; m++){
				    if (m != (n-bsize)){
					B[m][n] = A[n][m];
				    }
				}
			    }
			    break;
			case 2:
			    for (n = i+bsize; n < i+Bsize; n++){
				if (count%9 == 0){
				    B[n][n] = A[n][n];
				}
				for (m = j+bsize; m < j+Bsize; m++){
				    if (m != n){
					B[m][n] = A[n][m];
				    }
				}
			    }
			    break;
			default:
			    break;
		    }
		}
		count++;
	    }
	}
    }else {
	
    }
}

char transpose_submit_desc_2[] = "Transpose submission_2";
void transpose_submit_2(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, m, n;
    int bsize, Bsize;
    bsize = 4;
    Bsize = 8;
    for (i = 0; i < N; i += Bsize){
       for (j = 0; j < M; j += Bsize){
	   for (k = 0; k < 4; k++){
		switch (k){
		    case 0:
			for (n = i; n < i+bsize; n++){
			    for (m = j; m < j+bsize; m++){
				B[m][n] = A[n][m];
			    }
			}
			break;
		    case 1:
			for (n = i; n < i+bsize; n++){
			    for (m = j+bsize; m < j+Bsize; m++){
				B[m][n] = A[n][m];
			    }
			}
			break;
		    case 2:
			for (n = i+bsize; n < i+Bsize; n++){
			    for (m = j+bsize; m < j+Bsize; m++){
				B[m][n] = A[n][m];
			    }
			}
			break;
		    case 3:
			for (n = i+bsize; n < i+Bsize; n++){
			    for (m = j; m < j+bsize; m++){
				B[m][n] = A[n][m];
			    }
			}
			break;
		    default:
			break;
		}
	   }
	}
    }

    //int i, j, m, n;
    //int bsize;
    //bsize = 4;
    //for (i = 0; i < N; i += bsize){
       //for (j = 0; j < M; j += bsize){
	    //for (m = j; m < j+bsize; m++){
		//for (n = i; n < i+bsize; n++){
		    //B[m][n] = A[n][m];
		//}
	    //}
	//}
    //}
}

char transpose_submit_desc_3[] = "Transpose submission_3";
void transpose_submit_3(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, m, n;
    int bsize, Bsize;
    //int count = 0;
    int val0, val1, val2, val3;
    bsize = 4;
    Bsize = 8;
    for (i = 0; i < N; i += Bsize) {
    	for (j = 0; j < M; j += Bsize) {
	  // if ((count % 9) != 0) {
		for (n = i; n < i + bsize; n++) {
		    for (m = j; m < j + bsize; m++) {
			B[m][n] = A[n][m];
			B[m+bsize-1-2*(m-j)][n+bsize] = A[n][m+bsize];
		    }
		}
		for (n = i + bsize; n < i + Bsize; n++) {
		    for (m = j + bsize; m < j + Bsize; m++) {
			B[m + bsize - 1 - 2 * (m - j - bsize)][n - bsize] = A[n][m - bsize];
			B[m][n] = A[n][m];
		    }
		}
		for (m = j; m < j + bsize; m++) {
		    val0 = B[m][i + bsize];
		    val1 = B[m][i + bsize + 1];
		    val2 = B[m][i + bsize + 2];
		    val3 = B[m][i + bsize + 3];
		    for (n = i; n < i + bsize; n++) {
			B[m][n + bsize] = B[m + bsize + 3 - 2 * (m - j)][n];
		    }
		    B[m + bsize + 3 - 2 * (m - j)][i] = val0;
		    B[m + bsize + 3 - 2 * (m - j)][i+1] = val1;
		    B[m + bsize + 3 - 2 * (m - j)][i+2] = val2;
		    B[m + bsize + 3 - 2 * (m - j)][i+3] = val3;
		}
	    }
	   /*else{
		for (n = i; n < i+bsize; n++){
		    for (m = j; m < j+bsize; m++){
			B[m][n] = A[n][m];
		    }
		}
		for (n = i; n < i+bsize; n++){
		    for (m = j+bsize; m < j+Bsize; m++){
			B[m][n] = A[n][m];
		    }
		}
		for (n = i+bsize; n < i+Bsize; n++){
		    for (m = j+bsize; m < j+Bsize; m++){
			B[m][n] = A[n][m];
		    }
		}
		for (n = i+bsize; n < i+Bsize; n++){
		    for (m = j; m < j+bsize; m++){
			B[m][n] = A[n][m];
		    }
		}
	    }*/
	    //count++;
	//}
    }
}

char transpose_submit_desc_4[] = "Transpose submission_4";
void transpose_submit_4(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, m, n;
    int val0, val1, val2, val3, val4, val5, val6, val7;
    for (i = 0; i < N; i += 8){
	for (j = 0; j < M; j += 8){
	    m = j;
	    for (n = i; n < i+4; n++){
		val0 = A[n][m];
		val1 = A[n][m+1];
		val2 = A[n][m+2];
		val3 = A[n][m+3];
		val4 = A[n][m+4];
		val5 = A[n][m+5];
		val6 = A[n][m+6];
		val7 = A[n][m+7];

		B[m][n] = val0;
		B[m+1][n] = val1;
		B[m+2][n] = val2;
		B[m+3][n] = val3;
		B[m][n+4] = val4;
		B[m + 1][n+4] = val5;
		B[m + 2][n+4] = val6;
		B[m + 3][n+4] = val7;

		/*for (m = j; m < j+4; m++){
		    B[m][n] = A[n][m];
		    B[m][n+4] = A[n][m+4];
		}*/
	    }
	    n = i+4;
	    for (m = j; m < j+4; m++){
		val0 = A[n][m];
		val1 = A[n+1][m];
		val2 = A[n+2][m];
		val3 = A[n+3][m];

		val4 = B[m][n];
		val5 = B[m][n+1];
		val6 = B[m][n+2];
		val7 = B[m][n+3];
		
		B[m][n] = val0;
		B[m][n+1] = val1;
		B[m][n+2] = val2;
		B[m][n+3] = val3;

		B[m+4][i] = val4;
		B[m+4][i+1] = val5;
		B[m+4][i+2] = val6;
		B[m+4][i+3] = val7;
		
		/*val0 = A[n][m];
		val1 = A[n+1][m];
		val2 = A[n+2][m];
		val3 = A[n+3][m];

		val4 = B[m][n];
		val5 = B[m][n+1];
		val6 = B[m][n+2];
		val7 = B[m][n+3];
		
		B[m][n] = val0;
		B[m][n+1] = val1;
		B[m][n+2] = val2;
		B[m][n+3] = val3;

		B[m+4][i] = val4;
		B[m+4][i+1] = val5;
		B[m+4][i+2] = val6;
		B[m+4][i+3] = val7;*/
	    }
	    m = j + 4;
	    for (n = i + 4; n < i + 8; n++) {			
		val0 = A[n][m];
		val1 = A[n][m+1];
		val2 = A[n][m+2];
		val3 = A[n][m+3];

		B[m][n] = val0;
		B[m+1][n] = val1;
		B[m+2][n] = val2;
		B[m+3][n] = val3;
	    }
	}
    }
}
char transpose_submit_desc_5[] = "Transpose submission_5";
void transpose_submit_5(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 4;
    int i, j, m, n;
    for (i = 0; i < N-1; i += bsize){
	for (j = 0; j < M-1; j += bsize){
	    for (n = i; n < i+bsize; n++){
		for (m = j; m < j+bsize; m++){
		    B[m][n] = A[n][m];
		}
	    }
	}
    }
}
char transpose_submit_desc_6[] = "Transpose submission_6";
void transpose_submit_6(int M, int N, int A[N][M], int B[M][N])
{
    int bsize = 4;
    int i, j, m, n;
    for (i = 0; i < N-1; i += bsize){
	for (j = 0; j < M-1; j += bsize){
	    for (m = j; m < j+bsize; m++){
		for (n = i; n < i+bsize; n++){
		    B[m][n] = A[n][m];
		}
	    }
	}
    }
}
/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
    registerTransFunction(transpose_submit_1, transpose_submit_desc_1); 
    registerTransFunction(transpose_submit_2, transpose_submit_desc_2); 
    registerTransFunction(transpose_submit_3, transpose_submit_desc_3); 
    registerTransFunction(transpose_submit_4, transpose_submit_desc_4); 
    registerTransFunction(transpose_submit_5, transpose_submit_desc_5); 
    registerTransFunction(transpose_submit_6, transpose_submit_desc_6); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

