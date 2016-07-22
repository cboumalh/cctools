/*
Copyright (C) 2016- The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file COPYING for details.
*/

#include "histogram.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

	// use bucket size of .5
	struct histogram *h = histogram_create(.5);

	// bucket [3.0, 3.5)
	histogram_insert(h, 3.00);
	histogram_insert(h, 3.14);

	// bucket [21.5, 22.0)
	histogram_insert(h, 21.99);

	// bucket [22.0, 22.5)
	histogram_insert(h, 22.00);
	histogram_insert(h, 22.20);
	histogram_insert(h, 22.49);

	// bucket [22.5, 23.0)
	histogram_insert(h, 22.50);

	// bucket [-22.0, -21.5)
	histogram_insert(h, -21.51);
	histogram_insert(h, -22.00);

	// bucket [-21.5, -21.0)
	histogram_insert(h, -21.49);
	histogram_insert(h, -21.20);
	histogram_insert(h, -21.01);

	double *buckets = histogram_buckets(h);
	double b        = histogram_bucket_size(h);

	int expected_counts[] = {2, 3, 2, 1, 3, 1};

	int i;
	for(i = 0; i < histogram_size(h); i++) {

		double start = buckets[i];
		int count    = histogram_count(h, start);

		if(expected_counts[i] != count) {
			fprintf(stderr, "Expected a count of %d, got %d.", expected_counts[i], count);
			return -1;
		}

		fprintf(stdout, "[%6.2lf, %6.2f) count: %d\n", start, start + b, histogram_count(h, start));
	}

	free(buckets);
	histogram_delete(h);

	return 0;
}
