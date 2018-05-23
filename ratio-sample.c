#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
/* TODO:
* static struct pcpu_ratio ratio should be on hugepage
* dpdk_apl should be run on a thread and use 'while(1)' instead of 'for'
* More debug...^^;
*/

#define 	likely(x)   __builtin_expect(!!(x), 1)
#define 	unlikely(x)   __builtin_expect(!!(x), 0)

struct pcpu_ratio {
	struct timespec used_ts; /* time for used/busy */
	struct timespec idle_ts; /* time for idle */
	struct timespec prev_ts; /* previous time for calculate used/busy */
	uint16_t prev_nb_pkts; /* previous number of pkts */
};

static struct pcpu_ratio ratio = {
	{(time_t)0,(long)0},
	{(time_t)0,(long)0},
	{(time_t)0,(long)0},
	(uint16_t)0
}; /* NULL initialize might not be necessary */

/* Dummy pkt receive API / DPDK */
static uint16_t rte_eth_rx_burst(
/*		uint16_t 	port_id,
		uint16_t 	queue_id,
		struct rte_mbuf ** 	rx_pkts, */
		const uint16_t 	nb_pkts
		)
{
	struct timespec ts;
	uint16_t pkts;
	/* dummpy codes :randamly defining the number of pkts*/
	// For future, replaced by original code.
	//pkts = 0;
	//pkts = nb_pkts;
	pkts = rand() % (nb_pkts+1);

	clock_gettime(CLOCK_REALTIME, &ts);
	if( likely(ratio.prev_nb_pkts == 0) ){
		ratio.idle_ts.tv_sec += (ts.tv_sec - ratio.prev_ts.tv_sec);
		ratio.idle_ts.tv_nsec += (ts.tv_nsec - ratio.prev_ts.tv_nsec);
	}
	else{
		ratio.used_ts.tv_sec += (ts.tv_sec - ratio.prev_ts.tv_sec);
		ratio.used_ts.tv_nsec += (ts.tv_nsec - ratio.prev_ts.tv_nsec);
	}
	ratio.prev_ts = ts;
       	ratio.prev_nb_pkts = pkts;
	return( pkts );
}

/* dummy dpdk aplication */
void dpdk_apl( const uint16_t nb_pkts )
{
	int i,pkts;
	// For future, following line should be replaced by 'while(1)'
	for( i = 0 ; i < 1000 ; i++ ){
		/* non-0 pkts are recieved */
		pkts = rte_eth_rx_burst((const uint16_t)nb_pkts);
		//printf( "Apl pkts = %d\n", nb_pkts );
		if( likely(nb_pkts != 0) )
			/* Maybe some overhead exisits +10ns/pkts */
			//ratio.used_ts.tv_nsec += 10 * pkts;
			ratio.used_ts.tv_nsec += pkts;
		/* 0 pkt is recieved */
		else
			continue;
	}
}

double get_pcpu_ratio()
{
	double used;
	double idle;

#define SEC2NSEC 100000000
	used = ratio.used_ts.tv_sec * SEC2NSEC + ratio.used_ts.tv_nsec;
	idle = ratio.idle_ts.tv_sec * SEC2NSEC + ratio.idle_ts.tv_nsec;
	printf( "used = %lf, ilde = %lf\n", used, idle );

	return( 100 * (used / (used + idle)) );
}

unsigned int main( int argc, char *argv[] )
{
	if( unlikely(argc != 2) ){
		printf( "Wrong use.\n" );
		exit(1);
	}
	struct timespec ts;
	int i;
	srand( (unsigned)time(NULL) );
	for( i=0 ; i < 1000 ; i++ ){
		clock_gettime(CLOCK_REALTIME, &ratio.prev_ts);
		dpdk_apl( atoi( argv[1]) );
		printf( "Psudo CPU Ratio = %02.21f\n", get_pcpu_ratio() );
	}
}
