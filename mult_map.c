#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* really dumb. */
#define MAX	200

int storage=0;
int **lijst;
int pairs[6][4] = {{0,1,2,3},{0,2,1,3},{0,3,1,2},{1,2,0,3},{1,3,0,2},{2,3,0,1}};

void my_alloc(void )
{
	int i;

	if (!storage) {
		lijst = (int **) malloc(100*sizeof(int *));
		storage = 100;
		for(i=0; i+1 <= 100;i++) {
			lijst[i] = (int *) malloc(4*sizeof(int));
		}
		return;
	}
	lijst = (int **) realloc(lijst, 2*storage*sizeof(int *));
	for(i=storage; i+1 <= 2*storage;i++) {
		lijst[i] = (int *) malloc(4*sizeof(int));
	}
	storage = 2*storage;
	return;
}
		

/* Returns the number of monomials in degree degree, and sets lijst
 * equal to the list of them. Set cutoff equal to zero if you want
 * all of them otherwise set it to the cutoff you want. */
int count_sum(int d1, int d2, int d3, int d4, int degree, int save, int cutoff)
{
	int len,a2,a3,a4;
	
	len = 0;
	a4 = 0;
	while (d4*a4 <= degree) {
		a3 = 0;
		while (d4*a4+d3*a3 <= degree) {
			a2 = 0;
			while (d4*a4+d3*a3+d2*a2 <= degree) {
				if((degree - (a4*d4+a3*d3+a2*d2)) % d1 == 0) {
					if (save) {
						if (len == storage) my_alloc();
						lijst[len][0] = (degree -
							(a4*d4+a3*d3+a2*d2))
								/ d1;
						lijst[len][1] = a2;
						lijst[len][2] = a3;
						lijst[len][3] = a4;
					}
					len++;
					if (len == cutoff) return(len);
				}
				a2++;
			}
			a3++;
		}
		a4++;
	}
	return(len);
}

/* This prints out all the multiindices whose associated monomials
 * have degree degree. */
void print_sum(int len)
{
	int i;

	for(i=0;i+1<=len;i++) {
		printf("[%d, %d, %d, %d]\n",
			lijst[i][0],
			lijst[i][1],
			lijst[i][2],
			lijst[i][3]);
	}
	return;
}

int gcd(int a, int b)
{
	int t;

	while (b > 0) {
		t = a % b;
		a = b;
		b = t;
	}
	return a;
}

/* This tests whether three out of four are divisible by the same prime.*
 * If so then the weighted projective space has a codimension 1 locus	*
 * of quotient singularities.						*/
int well_formed(int d1,  int d2,  int d3,  int d4)
{
	int g;

	g = gcd(d1,d2);
	if (g > 1) {
		if (gcd(g,d3) > 1) return(0);
		if (gcd(g,d4) > 1) return(0);
	}
	g = gcd(d3,d4);
	if (g > 1) {
		if (gcd(g,d1) > 1) return(0);
		if (gcd(g,d2) > 1) return(0);
	}
	return(1);
}

/* This tests whether there can be a quasi-smooth surface in the linear
 * system. The rule is that for each i either x_i^power can occur or that
 * x_i^power x_j should occur.... etc. See paper by Iano-Fletcher. 
 * ASSUMES: degree is bigger than d_i for all i. */
int d_suitable(int len)
{
	int i, j, success, sum;
	int a,b,c,d;
	int test_one[4];
	int test_two[4][4];
	int test_three[4];

	for (j=0;j<=3;j++) {
		test_three[j] = 0;
		test_one[j] = 0;
		for (i=j+1;i<=3;i++) test_two[j][i]=0;
	}	

	for (i=0;i+1<=len;i++) {
		sum = lijst[i][0] + lijst[i][1] + lijst[i][2] + lijst[i][3];
		for(j=0;j<=3;j++) {
			if (lijst[i][j] == 0) test_three[j] = 1;
			if (sum == lijst[i][j]) test_one[j] = 1;
		}
		for(j=0;j<=5;j++) {
			a=pairs[j][0];
			b=pairs[j][1];
			c=pairs[j][2];
			d=pairs[j][3];
			if ((lijst[i][c] == 0) && (lijst[i][d] == 0)) {
				test_two[a][b] = 1;
				if (lijst[i][a] == 1) test_one[b] = 1;
				if (lijst[i][b] == 1) test_one[a] = 1;
			}
			if ((lijst[i][c] == 1) && (lijst[i][d] == 0)) {
				if (test_two[a][b] != 1) {
					if (test_two[a][b] == -d-1) {
						test_two[a][b] = 1;
					} else {
						test_two[a][b] = -c-1;
					}
				}
			}
			if ((lijst[i][c] == 0) && (lijst[i][d] == 1)) {
				if (test_two[a][b] != 1) {
					if (test_two[a][b] == -c-1) {
						test_two[a][b] = 1;
					} else {
						test_two[a][b] = -d-1;
					}
				}
			}
		}
	}
	success=0;
	for (j=0;j<=3;j++) {
		if (test_three[j] == 1) success++;
		if (test_one[j] == 1) success++;
		for (i=j+1;i<=3;i++) {
			if (test_two[j][i] == 1) {
				success++;
			}
		}
	}
	if (success == 14) return 1;
	return 0;
}

int hilbert_function(int d1, int d2, int d3, int d4, int d, int i)
{
        int phi;

        if (i < 0) return(0);

        phi = count_sum(d1,d2,d3,d4,i,0,0);
        if (i >= d-d1) phi -= count_sum(d1,d2,d3,d4,i-d+d1,0,0);
        if (i >= d-d2) phi -= count_sum(d1,d2,d3,d4,i-d+d2,0,0);
        if (i >= d-d3) phi -= count_sum(d1,d2,d3,d4,i-d+d3,0,0); 
        if (i >= d-d4) phi -= count_sum(d1,d2,d3,d4,i-d+d4,0,0); 
        if (i >= 2*d-d1-d2) phi += count_sum(d1,d2,d3,d4,i-2*d+d1+d2,0,0);
        if (i >= 2*d-d1-d3) phi += count_sum(d1,d2,d3,d4,i-2*d+d1+d3,0,0);
        if (i >= 2*d-d1-d4) phi += count_sum(d1,d2,d3,d4,i-2*d+d1+d4,0,0);
        if (i >= 2*d-d2-d3) phi += count_sum(d1,d2,d3,d4,i-2*d+d2+d3,0,0);
        if (i >= 2*d-d2-d4) phi += count_sum(d1,d2,d3,d4,i-2*d+d2+d4,0,0);
        if (i >= 2*d-d3-d4) phi += count_sum(d1,d2,d3,d4,i-2*d+d3+d4,0,0);
        if (i >= 3*d-d1-d2-d3) phi -= count_sum(d1,d2,d3,d4,i-3*d+d1+d2+d3,0,0);
        if (i >= 3*d-d1-d2-d4) phi -= count_sum(d1,d2,d3,d4,i-3*d+d1+d2+d4,0,0);
        if (i >= 3*d-d1-d3-d4) phi -= count_sum(d1,d2,d3,d4,i-3*d+d1+d3+d4,0,0);
        if (i >= 3*d-d2-d3-d4) phi -= count_sum(d1,d2,d3,d4,i-3*d+d2+d3+d4,0,0);
        if (i >= 4*d-d1-d2-d3-d4) phi += count_sum(d1,d2,d3,d4,i-4*d+d1+d2+d3+d4,0,0);
        return(phi);
}

int test_mult(int d1, int d2, int d3, int d4, int degree, int count)
{
	int a1,a2,a3,a4,i,uit;
	
	uit = 1;
	a4 = 0;
	while (d4*a4 <= degree) {
		a3 = 0;
		while (d4*a4+d3*a3 <= degree) {
			a2 = 0;
			while (d4*a4+d3*a3+d2*a2 <= degree) {
				if((degree - (a4*d4+a3*d3+a2*d2)) % d1 == 0) {
					a1 = (degree-(a4*d4+a3*d3+a2*d2))/d1;
					i = 0;
					while((i < count) &&
						((a1 < lijst[i][0]) ||
						(a2 < lijst[i][1]) ||
						(a3 < lijst[i][2]) ||
						(a4 < lijst[i][3]))) i++;
					if (i == count) {
						uit = 0;
/*						printf("%d %d %d %d\n",
							a1,a2,a3,a4); */
					}
				}
				a2++;
			}
			a3++;
		}
		a4++;
	}
	return(uit);
}	

int test_line(int d1, int d2, int d3, int d4, int count)
{
	int i,j,uit;
	int test[6];

	uit = 1;
	test[0] = 1;
	test[1] = 1;
	test[2] = 1;
	test[3] = 1;
	test[4] = 1;
	test[5] = 1;
	i = 0;
	while (i < count) {
		j = 0;
		while (j < 6) {
			if ((lijst[i][pairs[j][0]] == 0) &&
				(lijst[i][pairs[j][1]] == 0)) test[j] = 0;
			j++;
		}
		i++;
	}
	j = 0;
	while (j < 6) {
		if (test[j]) {
			printf("Line for X_%d=0 and X_%d=0.\n",
				pairs[j][0], pairs[j][1]);
			uit = 0;
		}
		j++;
	}
	return(uit);
}

/* This computes the dimension of the automorphism group. *
 * Note that we are computing the dimension of the cone.  */
int dim_aut(int d1, int d2, int d3, int d4)
{
	int totaal;
	totaal = 0;
	totaal = totaal + count_sum(d1,d2,d3,d4,d1,0,0);
	totaal = totaal + count_sum(d1,d2,d3,d4,d2,0,0);
	totaal = totaal + count_sum(d1,d2,d3,d4,d3,0,0);
	totaal = totaal + count_sum(d1,d2,d3,d4,d4,0,0);
	return(totaal);
}

int det(int a, int b, int c, int d)
{
	return(
lijst[a][0]*lijst[b][1]*lijst[c][2]*lijst[d][3] -
lijst[a][1]*lijst[b][0]*lijst[c][2]*lijst[d][3] -
lijst[a][0]*lijst[b][2]*lijst[c][1]*lijst[d][3] +
lijst[a][1]*lijst[b][2]*lijst[c][0]*lijst[d][3] +
lijst[a][2]*lijst[b][0]*lijst[c][1]*lijst[d][3] -
lijst[a][2]*lijst[b][1]*lijst[c][0]*lijst[d][3] -
lijst[a][0]*lijst[b][1]*lijst[c][3]*lijst[d][2] +
lijst[a][1]*lijst[b][0]*lijst[c][3]*lijst[d][2] +
lijst[a][0]*lijst[b][2]*lijst[c][3]*lijst[d][1] -
lijst[a][1]*lijst[b][2]*lijst[c][3]*lijst[d][0] -
lijst[a][2]*lijst[b][0]*lijst[c][3]*lijst[d][1] +
lijst[a][2]*lijst[b][1]*lijst[c][3]*lijst[d][0] +
lijst[a][0]*lijst[b][3]*lijst[c][1]*lijst[d][2] -
lijst[a][1]*lijst[b][3]*lijst[c][0]*lijst[d][2] -
lijst[a][0]*lijst[b][3]*lijst[c][2]*lijst[d][1] +
lijst[a][1]*lijst[b][3]*lijst[c][2]*lijst[d][0] +
lijst[a][2]*lijst[b][3]*lijst[c][0]*lijst[d][1] -
lijst[a][2]*lijst[b][3]*lijst[c][1]*lijst[d][0] -
lijst[a][3]*lijst[b][0]*lijst[c][1]*lijst[d][2] +
lijst[a][3]*lijst[b][1]*lijst[c][0]*lijst[d][2] +
lijst[a][3]*lijst[b][0]*lijst[c][2]*lijst[d][1] -
lijst[a][3]*lijst[b][1]*lijst[c][2]*lijst[d][0] -
lijst[a][3]*lijst[b][2]*lijst[c][0]*lijst[d][1] +
lijst[a][3]*lijst[b][2]*lijst[c][1]*lijst[d][0]
);
}

int main()
{
	int flag1, flag2, nrf;
	int count,totaal,d1,d2,d3,d4,degree,pg,dimaut,h11;

	while ((scanf("%d %d %d %d %d %d %d %d %d %d\n",
	&d1,&d2,&d3,&d4,&degree,&nrf,&dimaut,&pg,&h11,&totaal) > 0)) {
		count = count_sum(d1, d2, d3, d4, degree-d1-d2-d3-d4, 1, 0);
		flag1 = test_mult(d1, d2, d3, d4, 2*degree-d1-d2-d3-d4, count);
		if (!flag1) {
			printf("%d %d %d %d %d %d %d %d %d %d\t",
				d1, d2, d3, d4, degree, nrf, dimaut, pg, h11, totaal);
			count = count_sum(d1, d2, d3, d4, degree, 1, 0);
/*			printf("List monomials for f: \n");
			print_sum(count); */
			flag2 = test_line(d1, d2, d3, d4, count);
			if (flag2) printf("\n");
/*			count = count_sum(d1, d2, d3, d4, degree-d1-d2-d3-d4, 1, 0);
			printf("List monomials for d-s: \n");
			print_sum(count);
			count = count_sum(d1, d2, d3, d4, 2*degree-(d1+d2+d3+d4), 1, 0);
			printf("List monomials for 2d-s: \n");
			print_sum(count);
			count = count_sum(d1, d2, d3, d4, 3*degree-(d1+d2+d3+d4), 1, 0);
			printf("\n"); */
		}

	}
	return(0);
}

