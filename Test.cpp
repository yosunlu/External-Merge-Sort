#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "gen.h"

int main (int argc, char * argv [])
{
	TRACE (true);

	genDataRecords(200);
	Plan * const plan = new SortPlan (new ScanPlan (100));
	// Plan * const plan = new FilterPlan ( new ScanPlan (7) );
	// new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main
