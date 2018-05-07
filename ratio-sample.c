#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

struct pcpu_ratio {
	struct timespec used_ts;
	struct timespec idle_ts;
	struct timespec prev_ts;
	uint16_t prev_nb_pkts;
};

static struct pcpu_ratio ratio = {
	{(time_t)0,(long)0},
	{(time_t)0,(long)0},
	{(time_t)0,(long)0},
	(uint16_t)0
};

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
	//pkts = 0;
	//pkts = nb_pkts;
	pkts = rand() % (nb_pkts+1);

	clock_gettime(CLOCK_REALTIME, &ts);
	if( ratio.prev_nb_pkts == 0 ){
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

void dpdk_apl()
{
	int i;
	uint16_t nb_pkts;
	for( i = 0 ; i < 1000 ; i++ ){
		/* some pkts are recived */
		nb_pkts = rte_eth_rx_burst((const uint16_t)32);
		//printf( "Apl pkts = %d\n", nb_pkts );
		if( nb_pkts != 0 )
			ratio.used_ts.tv_nsec += 10 * nb_pkts;
		/* 0 pkt is recived */
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
	struct timespec ts;
	int i;
	srand( (unsigned)time(NULL) );
	for( i=0 ; i < 1000 ; i++ ){
		clock_gettime(CLOCK_REALTIME, &ratio.prev_ts);
		dpdk_apl();
		printf( "Psudo CPU Ratio = %02.21f\n", get_pcpu_ratio() );
	}
}
