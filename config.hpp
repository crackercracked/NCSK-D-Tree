// @xl242
#ifndef KD_CONFIG_H_
#define KD_CONFIG_H_

#include <cstdlib>
#include <cmath>

enum policy_t {
	BY_HEIGHT_FROM_BOTTOM,
	BY_PERCENTILE,
};

struct config_t {
	int dimension;
	policy_t policy;
	double value;
	int fanout;
	config_t(int d, policy_t p, double v, int f=2):
			dimension(d),
			policy(p),
			value(v),
			fanout(f) {}
};


inline int bottomheight(int N, int fanout) {
	double Nf = double(N);
	double fanoutf = double(fanout);
	return int(1.0 + (log(Nf) / log(fanoutf)));
}
#endif
