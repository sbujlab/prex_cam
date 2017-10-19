#ifndef __REMOLLGENPREX_HH 
#define __REMOLLGENPREX_HH 
/*!
 * Prex (ee) event generator
 *
 * Seamus Riordan
 * February 4, 2013
 *
 * Based heavily on previous work from mollersim
*/

#include "remollVEventGen.hh"

class remollGenPrex : public remollVEventGen {
    public:
	remollGenPrex();
	virtual ~remollGenPrex();

    private:
	void SamplePhysics(remollVertex *, remollEvent *);

};

#endif//__REMOLLGENPREX_HH 
