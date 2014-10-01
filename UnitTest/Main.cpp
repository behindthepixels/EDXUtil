#define randu() (rand()/(RAND_MAX+1.))
#define randp(p) (rand()<(RAND_MAX+1.)*(p))
const float pi = 3.141592653f;

#include "Math/Vector.h"
#include "EDXPrerequisites.h"

using namespace EDX;

template <class V, int MAX_DIM>
class MetropolisSampler {
protected:
	void doIt(unsigned n) {
		double i, p = 0, s = 0, w[2];
		V v[2];
		for (w[1] = j = k[0] = k[1] = 0, l = 1; l <= n; l++) {
			f(v[j], i);
			if (l & 1)
				s += i;
			double a = (i<p) ? i / p : 1.;
			if (s>0.) {
				w[j] = (a + (l & 1)) / (((l + 1) / 2)*i / s + 0.5);
				w[j ^ 1] += (1 - a) / (((l + 1) / 2)*p / s + 0.5);
			}
			else
				w[j] = (1 + (l & 1)) / 1.5;
			if (randp(a)) {
				p = i;
				j = j ^ 1;
			}
			if ((l & 1) == 0)
				out(v[j], w[j]);
			k[j] = 0;
		}
		out(v[j ^ 1], w[j ^ 1]);
	}
	double iteratorOnX() {
		double v = ((l & 1) || k[j]>k[j ^ 1]) ? randu() : mutate(x[j ^ 1][k[j]], 1. / 1024, 1. / 64);
		return x[j][k[j]++] = v;
	}
	virtual void f(V& y, double& i) = 0;
	virtual void out(const V&, double w) = 0;
private:
	double mutate(double v, double r0, double r1) const {
		double r = r1*exp(-log(r1 / r0)*randu());
		return (rand() & 1) ? (v + r>1 ? v + r - 1 : v + r) : (v - r<0 ? v - r + 1 : v - r);
	}
	double x[2][MAX_DIM];
	unsigned l, j, k[2];
};

class Test : private MetropolisSampler<Vec<2, double>, 1> {
public:
	Test() : v(0), w(0) {
		v.resize(RES);
		w.resize(RES);
		doIt(4 * 1024 * 1024);
		for (auto i = 0; i < RES; i++)
			v[i] /= w[i];

		for (unsigned i = 0; i<RES; i++) {
			double s = fi(double(i) / RES, double(i + 1) / RES);
			std::cout << s << "\t" << v[i] << "\t" << (s - v[i]) << std::endl;
		}
	}
private:
	double f(double x) const {
		return 10 * (1 + cos(2 * pi*x));
	}
	double fi(double a, double b) const {
		unsigned s = 128 * 1024;
		double sum = 0;
		for (unsigned i = 0; i<s; i++) {
			double w = (i + 0.5) / s;
			sum += f((1 - w)*a + w*b);
		}
		return sum / s;
	}
	void f(Vec<2, double>& v, double& i) {
		v[0] = iteratorOnX();
		i = v[1] = f(v[0]);
	}
	void out(const Vec<2, double>& v, double w) {
		unsigned x = unsigned(RES*v[0]);
		Test::v[x] += w*v[1];
		Test::w[x] += w;
	}
	enum { RES = 1024 };
	vector<double> v, w;
};

void main()
{
	Test t;
}